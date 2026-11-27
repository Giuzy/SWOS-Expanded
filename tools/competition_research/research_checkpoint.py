"""Recoverable research-only multi-file publication (journal is local cache)."""
import base64
import hashlib
import json
import os
import tempfile
from contextlib import contextmanager

def checksum(data):
    return hashlib.sha256(data).hexdigest()

def replace(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent,suffix=".tmp",delete=False) as f:
        from pathlib import Path
        tmp=Path(f.name)
        f.write(data);f.flush();os.fsync(f.fileno())
    try:os.replace(tmp,path)
    finally:tmp.unlink(missing_ok=True)

@contextmanager
def writer_lock(path):
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open("a+b") as lock:
        lock.seek(0,2)
        if lock.tell()==0:lock.write(b"0");lock.flush()
        lock.seek(0)
        if os.name=="nt":
            import msvcrt
            msvcrt.locking(lock.fileno(),msvcrt.LK_NBLCK,1)
        else:
            import fcntl
            fcntl.flock(lock.fileno(),fcntl.LOCK_EX|fcntl.LOCK_NB)
        try:yield
        finally:
            lock.seek(0)
            if os.name=="nt":msvcrt.locking(lock.fileno(),msvcrt.LK_UNLCK,1)
            else:fcntl.flock(lock.fileno(),fcntl.LOCK_UN)

def recover(root,journal):
    if not journal.exists():return 0
    entries=json.loads(journal.read_text(encoding="utf-8"))
    pending=[]
    for relative,record in entries.items():
        path=(root/relative).resolve()
        if not path.is_relative_to(root.resolve()) or path.suffix!=".json":raise RuntimeError("Invalid research journal path")
        data=base64.b64decode(record["new"],validate=True)
        if checksum(data)!=record["sha"]:raise RuntimeError("Invalid research journal payload")
        current=checksum(path.read_bytes())
        if current not in (record["old"],record["sha"]):raise RuntimeError("External edit during research checkpoint: "+str(path))
        if current!=record["sha"]:pending.append((path,data))
    for path,data in pending:replace(path,data)
    for relative,record in entries.items():
        if checksum((root/relative).read_bytes())!=record["sha"]:raise RuntimeError("Research checkpoint verification failed")
    journal.unlink()
    return len(entries)

def publish(root,journal,pending):
    if journal.exists():raise RuntimeError("Unrecovered research checkpoint")
    entries={}
    for path,(old,new) in pending.items():
        if path.read_bytes()!=old:raise RuntimeError("Concurrent research change: "+str(path))
        entries[path.resolve().relative_to(root.resolve()).as_posix()]={"old":checksum(old),"sha":checksum(new),"new":base64.b64encode(new).decode("ascii")}
    if not entries:return 0
    replace(journal,json.dumps(entries,separators=(",",":")).encode())
    return recover(root,journal)

def selftest():
    from pathlib import Path
    with tempfile.TemporaryDirectory() as d:
        root=Path(d);j=root/"cache"/"pending.json";a=root/"a.json";b=root/"b.json"
        a.write_bytes(b"old");b.write_bytes(b"old")
        assert publish(root,j,{a:(b"old",b"new"),b:(b"old",b"new")})==2
        assert a.read_bytes()==b.read_bytes()==b"new" and not j.exists()
        entries={p.name:{"old":checksum(b"new"),"sha":checksum(b"next"),"new":base64.b64encode(b"next").decode()} for p in (a,b)}
        replace(j,json.dumps(entries).encode());a.write_bytes(b"next")
        b.write_bytes(b"external")
        try:recover(root,j)
        except RuntimeError:pass
        else:raise AssertionError("External edit accepted")
        assert b.read_bytes()==b"external" and j.exists()
        b.write_bytes(b"new");assert recover(root,j)==2
        assert a.read_bytes()==b.read_bytes()==b"next" and not j.exists()
