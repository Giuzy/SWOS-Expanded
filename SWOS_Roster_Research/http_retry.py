"""Bounded HTTP retries, visible progress and optional caller time budget."""
import datetime
import email.utils
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
from contextlib import contextmanager

NEXT={}
BLOCKED={}
INFLIGHT={}
RETRYABLE={408,429,500,502,503,504}
_DEADLINE=None

class DeadlineExceeded(TimeoutError):pass
class HostCooldown(RuntimeError):pass

@contextmanager
def time_budget(seconds):
    global _DEADLINE
    previous=_DEADLINE
    now=time.monotonic()+seconds
    _DEADLINE=now if previous is None else min(previous,now)
    try:yield
    finally:_DEADLINE=previous

def check_budget():
    if _DEADLINE is not None and time.monotonic()>=_DEADLINE:
        raise DeadlineExceeded("Research time budget exhausted")

def wait_after(header, now=None):
    if not header:return 0
    try:return max(0,int(header))
    except ValueError:
        try:
            return max(0,(email.utils.parsedate_to_datetime(header)-(now or datetime.datetime.now(datetime.timezone.utc))).total_seconds())
        except (ValueError,TypeError,OverflowError):return 0

def fetch(req,timeout=60,attempts=4,interval=2,limit=64*1024*1024,opener=None,sleep=None,clock=None,total=180):
    opener=opener or urllib.request.urlopen;sleep=sleep or time.sleep;clock=clock or time.monotonic
    url=urllib.parse.urlsplit(req.full_url);host=url.netloc;label=host+url.path
    start=clock();deadline=start+total
    if _DEADLINE is not None:deadline=min(deadline,_DEADLINE)
    def log(message):print(f"HTTP {label} — {message} ({clock()-start:.0f}s elapsed)",flush=True)
    def remaining():
        left=deadline-clock()
        if left<=0:raise DeadlineExceeded("HTTP deadline: "+label)
        return left
    previous=INFLIGHT.get(host)
    if previous is not None:
        if previous.is_alive():
            log("SKIPPED: previous timed-out request still closing")
            raise HostCooldown("Previous timed-out request still closing: "+host)
        del INFLIGHT[host]
    for n in range(attempts):
        left=remaining()
        wait=max(0,NEXT.get(host,0)-clock(),BLOCKED.get(host,0)-clock())
        if wait>=left or wait>15:
            log(f"SKIPPED: host cooldown {wait:.0f}s")
            raise HostCooldown("HTTP host cooldown: "+host)
        if wait>=1:log(f"PACING: {wait:.0f}s")
        if wait:sleep(wait)
        left=remaining();NEXT[host]=clock()+interval
        result=[];socket_timeout=min(timeout,left)
        def request_once():
            try:
                with opener(req,timeout=socket_timeout) as response:
                    data=response.read(limit+1)
                if len(data)>limit:raise ValueError("HTTP response exceeds limit")
                result.append((True,data))
            except Exception as exc:result.append((False,exc))
        log(f"ATTEMPT {n+1}/{attempts}; budget {left:.0f}s")
        worker=threading.Thread(target=request_once,daemon=True)
        worker.start()
        while worker.is_alive():
            left=deadline-clock()
            if left<=0:
                INFLIGHT[host]=worker
                log("TIMEOUT: total deadline; response discarded")
                raise DeadlineExceeded("HTTP deadline: "+label)
            worker.join(min(10,left))
            if worker.is_alive() and deadline-clock()>0:log("WAITING: response/download")
        remaining()
        if not result:raise RuntimeError("HTTP worker returned no result")
        ok,value=result[0]
        if ok:
            log(f"DONE: {len(value)} bytes")
            return value
        e=value
        if not isinstance(e,(urllib.error.HTTPError,urllib.error.URLError,TimeoutError)) or (isinstance(e,urllib.error.HTTPError) and e.code not in RETRYABLE):
            log(f"ERROR: {type(e).__name__}: {e}");raise e
        delay=max(4*2**n,wait_after(e.headers.get("Retry-After")) if isinstance(e,urllib.error.HTTPError) and e.headers else 0)
        NEXT[host]=max(NEXT[host],clock()+delay)
        if isinstance(e,urllib.error.HTTPError) and e.code==429:
            BLOCKED[host]=max(BLOCKED.get(host,0),clock()+delay)
        if delay>15 or delay>=deadline-clock() or n==attempts-1:
            if isinstance(e,urllib.error.HTTPError) and e.code==429:
                BLOCKED[host]=max(BLOCKED.get(host,0),clock()+60)
            log(f"ERROR: {e}; cooldown {delay:.0f}s")
            raise e
        log(f"RETRY: {e}; backoff {delay:.0f}s")
    raise RuntimeError("Unreachable")


def selftest():
    import io
    from email.message import Message
    from contextlib import redirect_stdout
    saved=(dict(NEXT),dict(BLOCKED),dict(INFLIGHT))
    NEXT.clear();BLOCKED.clear();INFLIGHT.clear()
    now=[0.0];calls=[];logs=io.StringIO();release=threading.Event()
    def clock():return now[0]
    def sleep(seconds):now[0]+=seconds
    def req(host):return urllib.request.Request("https://"+host+"/test")
    def retry(request,timeout):
        calls.append(timeout)
        if len(calls)==1:raise urllib.error.URLError("temporary")
        return io.BytesIO(b"ok")
    def limited(request,timeout):
        headers=Message();headers["Retry-After"]="300"
        raise urllib.error.HTTPError(request.full_url,429,"limit",headers,None)
    def stalled(request,timeout):
        release.wait(2);return io.BytesIO(b"late")
    try:
        with redirect_stdout(logs):
            assert fetch(req("retry.invalid"),opener=retry,clock=clock,sleep=sleep,total=30)==b"ok"
            assert len(calls)==2 and now[0]==4
            try:fetch(req("limit.invalid"),opener=limited,clock=clock,sleep=sleep,total=30)
            except urllib.error.HTTPError:pass
            else:raise AssertionError("429 accepted")
            try:fetch(req("limit.invalid"),opener=lambda *a,**k:(_ for _ in ()).throw(AssertionError("cooldown bypass")),clock=clock,sleep=sleep,total=30)
            except HostCooldown:pass
            else:raise AssertionError("cooldown ignored")
            try:fetch(req("slow.invalid"),opener=stalled,total=.05)
            except DeadlineExceeded:pass
            else:raise AssertionError("hard deadline ignored")
            try:fetch(req("slow.invalid"),opener=stalled,total=.05)
            except HostCooldown:pass
            else:raise AssertionError("overlapping timed-out request")
            assert fetch(req("next.invalid"),opener=lambda *a,**k:io.BytesIO(b"next"),total=1)==b"next"
            try:
                with time_budget(0):check_budget()
            except DeadlineExceeded:pass
            else:raise AssertionError("country budget ignored")
            check_budget()
            try:fetch(req("size.invalid"),opener=lambda *a,**k:io.BytesIO(b"abc"),limit=2,total=1)
            except ValueError:pass
            else:raise AssertionError("oversize accepted")
        for token in ("ATTEMPT","RETRY","DONE","TIMEOUT","SKIPPED","ERROR"):assert token in logs.getvalue()
    finally:
        release.set()
        for worker in INFLIGHT.values():worker.join(2)
        for target,original in zip((NEXT,BLOCKED,INFLIGHT),saved):target.clear();target.update(original)
