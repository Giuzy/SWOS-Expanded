"""Consistent research console status; no storage side effects."""
def report(label,status,detail=""):
    print(f"\n{label} — {status}"+(f" ({detail})" if detail else ""),flush=True)

def outcome(found,added,review=0):
    if added:return "ACQUIRED",f"+{added} new; {found} eligible; {review} review"
    if review:return "REVIEW",f"+{review} unresolved matches"
    if found:return "NO NEW DATA",f"{found} eligible; no new records"
    return "NO DATA","no eligible records from this source"


def show_progress(label,done,total):
    """Count handled work units, not successful acquisitions or writes."""
    print(f"\nPROGRESS {label}: {done}/{total} done | {total-done} remaining",flush=True)


class WorkTotal:
    """Pass-wide workload; nested lists cannot reset it, errors are not acquisitions."""
    def __init__(self,label,total,subset_label="",subset_total=0):
        self.label=label;self.total=total;self.done=0
        self.subset_label=subset_label;self.subset_total=subset_total;self.subset_done=0
        self.emit()

    def emit(self,active=False,subset=False):
        text=f"TOTAL {self.label}: {self.done+int(active)}/{self.total} "+("active" if active else "handled")
        if self.subset_label:
            text+=f" | {self.subset_label}: {self.subset_done+int(active and subset)}/{self.subset_total} "+("active" if active and subset else "handled")
        print(text,flush=True)

    def walk(self,items,is_subset=lambda item:False,every=1):
        for item in items:
            subset=bool(is_subset(item))
            if self.done%every==0:self.emit(True,subset)
            yield item
            self.done+=1;self.subset_done+=int(subset)
            if self.done%every==0 or self.done==self.total:self.emit()


def progress(label,items,every=1,overall=False):
    """Sized, already planned jobs. A break/exception never counts the active job."""
    total=len(items)
    show_progress(label,0,total)
    if overall:items=WorkTotal(label,total).walk(items,every=every)
    for done,item in enumerate(items):
        if done%every==0:
            print(f"\n{label} — ACTION {done+1}/{total} ({done} done; {total-done} remaining including current)",flush=True)
        yield item
        if (done+1)%every==0 or done+1==total:
            show_progress(label,done+1,total)


def selftest():
    from contextlib import redirect_stdout
    from io import StringIO
    output=StringIO()
    with redirect_stdout(output):
        assert list(progress("normal",range(3)))==[0,1,2]
        assert list(progress("empty",[]))==[]
        for item in progress("break",range(3)):
            if item==1:break
        try:
            for item in progress("fatal",range(2)):
                raise RuntimeError("test")
        except RuntimeError:pass
        assert list(progress("batched",range(3),every=2))==[0,1,2]
        report("sample","ERROR","test")
        counter=WorkTotal("global",3,"missing",2)
        assert list(counter.walk([1],lambda n:True))==[1]
        assert list(counter.walk([2]))==[2]
        for item in counter.walk([3],lambda n:True):break
        assert counter.done==2 and counter.subset_done==1
        assert list(progress("overall",["x","y"],overall=True))==["x","y"]
    log=output.getvalue()
    assert "normal: 3/3 done | 0 remaining" in log
    assert "empty: 0/0 done | 0 remaining" in log
    assert "break: 1/3 done" in log and "break: 2/3 done" not in log
    assert "fatal: 0/2 done" in log and "fatal: 1/2 done" not in log
    assert "batched: 3/3 done" in log and "\n\nsample — ERROR" in log
    assert "TOTAL global: 3/3 active | missing: 2/2 active" in log
    assert "TOTAL global: 3/3 handled" not in log
    assert "TOTAL overall: 2/2 handled" in log
