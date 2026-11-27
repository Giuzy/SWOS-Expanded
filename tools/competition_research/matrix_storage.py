"""Canonical country shards; lossless hydration for existing research readers."""
import json
import re

SCHEMA = "COUNTRY_SHARDS_V1"

def raw(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))

def key(row):
    country = row.get("technicalIdentity", {}).get("country")
    if isinstance(country, str) and re.fullmatch(r"[A-Z]{3}", country):
        return country
    world = row["world"]
    if world not in ("9697", "2627"):
        raise ValueError("Unknown world")
    return "GLOBAL_" + world

def child(path, relative):
    p = (path.parent / relative).resolve()
    if not p.is_relative_to(path.parent.resolve()) or p.suffix != ".json":
        raise ValueError("Invalid matrix shard path")
    return p

def read_matrix(path):
    obj = raw(path)
    if "matrixStorage" not in obj:
        return obj
    if obj.pop("matrixStorage") != SCHEMA:
        raise ValueError("Unknown matrix storage")
    rows = []
    for country, relative in obj.pop("rowShards").items():
        records = raw(child(path, relative))["rows"]
        if any(key(r) != country for r in records):
            raise ValueError("Wrong shard country")
        rows.extend(records)
    if len({r["row"] for r in rows}) != len(rows):
        raise ValueError("Duplicate matrix rows")
    obj["rows"] = sorted(rows, key=lambda r: r["row"])
    for field, relative in obj.pop("metadataShards").items():
        if field in obj:
            raise ValueError("Duplicate matrix metadata")
        obj[field] = raw(child(path, relative))
    return obj

def encode(obj):
    return (json.dumps(obj, ensure_ascii=False, separators=(",", ":")) + "\n").encode("utf-8")

def plan_matrix(path, obj):
    """Return changed bytes only; caller owns concurrency checks/rollback."""
    if not path.exists() or raw(path).get("matrixStorage") != SCHEMA:
        data = (json.dumps(obj, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
        return {path: data}
    old = raw(path)
    manifest = {k: v for k, v in obj.items() if k != "rows"}
    manifest.update(matrixStorage=SCHEMA, rowShards={}, metadataShards=old["metadataShards"])
    groups = {}
    seen = set()
    for r in obj["rows"]:
        if r["row"] in seen:
            raise ValueError("Duplicate matrix row")
        seen.add(r["row"])
        groups.setdefault(key(r), []).append(r)
    if set(groups) != set(old["rowShards"]):
        raise ValueError("Matrix inventory country set changed; explicit migration required")
    updates = {}
    def put(p, value, data):
        if raw(p) != value:
            updates[p] = data
    for country, rows in sorted(groups.items()):
        relative = old["rowShards"][country]
        manifest["rowShards"][country] = relative
        records = {"rows": sorted(rows, key=lambda r: r["row"])}
        data = b'{"rows":[\n' + b",\n".join(encode(r).rstrip(b"\n") for r in records["rows"]) + b"\n]}\n"
        put(child(path, relative), records, data)
    for field, relative in manifest["metadataShards"].items():
        value = manifest.pop(field)
        put(child(path, relative), value, encode(value))
    put(path, manifest, (json.dumps(manifest, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
    return updates

def read_country(path, country):
    """Selective review: only one shard, not the global matrix."""
    obj=raw(path)
    if obj.get("matrixStorage")!=SCHEMA:
        return [r for r in obj["rows"] if key(r)==country]
    relative=obj["rowShards"].get(country)
    if not relative:return []
    rows=raw(child(path,relative))["rows"]
    if any(key(r)!=country for r in rows):raise ValueError("Wrong shard country")
    return rows
def selftest():
    from tempfile import TemporaryDirectory
    from pathlib import Path
    with TemporaryDirectory() as td:
        p=Path(td)/"AUTOMATIC_INSERTION.json"
        a={"row":1,"world":"2627","technicalIdentity":{"country":"ITA"}}
        b={"row":2,"world":"9697","technicalIdentity":{"country":"ENG"}}
        for country,row in (("ITA",a),("ENG",b)):
            (p.parent/(country+".json")).write_bytes(encode({"rows":[row]}))
        p.write_bytes(encode({"matrixStorage":SCHEMA,"rowShards":{"ITA":"ITA.json","ENG":"ENG.json"},"metadataShards":{},"summary":{"count":2}}))
        data=read_matrix(p)
        assert data["rows"]==[a,b] and not plan_matrix(p,data)
        assert read_country(p,"ITA")==[a]
        data["rows"][0]["status"]="P"
        changes=plan_matrix(p,data)
        assert set(changes)=={p.parent/"ITA.json"}
        for path,value in changes.items():path.write_bytes(value)
        assert read_matrix(p)==data
