"""Bounded, respectful retry with per-host pacing and Retry-After."""
import datetime
import email.utils
import time
import urllib.error
import urllib.parse
import urllib.request

NEXT={}
BLOCKED={}
RETRYABLE={408,429,500,502,503,504}
def wait_after(header, now=None):
    if not header:return 0
    try:return max(0,int(header))
    except ValueError:
        try:
            return max(0,(email.utils.parsedate_to_datetime(header)-(now or datetime.datetime.now(datetime.timezone.utc))).total_seconds())
        except (ValueError,TypeError,OverflowError):return 0
def fetch(req,timeout=60,attempts=4,interval=2,limit=64*1024*1024,opener=None,sleep=None,clock=None):
    opener=opener or urllib.request.urlopen;sleep=sleep or time.sleep;clock=clock or time.monotonic
    host=urllib.parse.urlsplit(req.full_url).netloc
    if host in BLOCKED and BLOCKED[host]>clock():
        raise RuntimeError("HTTP host cooldown: "+host)
    for n in range(attempts):
        sleep(max(0,NEXT.get(host,0)-clock()))
        NEXT[host]=clock()+interval
        try:
            with opener(req,timeout=timeout) as r:
                data=r.read(limit+1)
            if len(data)>limit:raise ValueError("HTTP response exceeds limit")
            return data
        except (urllib.error.HTTPError,urllib.error.URLError,TimeoutError) as e:
            if isinstance(e,urllib.error.HTTPError) and e.code not in RETRYABLE:raise
            delay=max(4*2**n,wait_after(e.headers.get("Retry-After")) if isinstance(e,urllib.error.HTTPError) and e.headers else 0)
            NEXT[host]=max(NEXT[host],clock()+delay)
            if delay>120 or n==attempts-1:
                if isinstance(e,urllib.error.HTTPError) and e.code==429:
                    BLOCKED[host]=clock()+max(delay,60)
                raise
    raise RuntimeError("Unreachable")
