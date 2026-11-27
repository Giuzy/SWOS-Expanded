"""Consistent research console status; no storage side effects."""
def report(label,status,detail=""):
    print(f"{label} — {status}"+(f" ({detail})" if detail else ""),flush=True)

def outcome(found,added,review=0):
    if added:return "ACQUIRED",f"+{added} new; {found} eligible; {review} review"
    if review:return "REVIEW",f"+{review} unresolved matches"
    if found:return "NO NEW DATA",f"{found} eligible; no new records"
    return "NO DATA","no eligible records from this source"
