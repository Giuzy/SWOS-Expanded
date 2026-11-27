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


def progress(label,items,every=1):
    """Sized, already planned jobs. A break/exception never counts the active job."""
    total=len(items)
    show_progress(label,0,total)
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
    log=output.getvalue()
    assert "normal: 3/3 done | 0 remaining" in log
    assert "empty: 0/0 done | 0 remaining" in log
    assert "break: 1/3 done" in log and "break: 2/3 done" not in log
    assert "fatal: 0/2 done" in log and "fatal: 1/2 done" not in log
    assert "batched: 3/3 done" in log and "\n\nsample — ERROR" in log
