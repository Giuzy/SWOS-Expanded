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


# Competition-level storage. V1 and old monoliths remain readable.
V2 = "COMPETITION_SHARDS_V2"
PROJECT = ("row","world","name","status","fields","operationalFields","operationalStatus",
           "technicalIdentity","inventoryPath","acquisitionAvailability","strictAcquisitionFields")
_v1_read,_v1_plan,_v1_country,_v1_test=read_matrix,plan_matrix,read_country,selftest

def project(row,relative):
    return dict({k:row[k] for k in PROJECT if k in row},path=relative)

def row_path(row):
    if row["world"] not in ("9697","2627") or type(row["row"])!=int:
        raise ValueError("Invalid competition identity")
    return f"automatic_insertion/{key(row)}/{row['world']}/{row['row']}.json"

def country_index(path,m,c):
    relative=m["rowShards"].get(c)
    if relative is None:return []
    rows=raw(child(path,relative))["rowIndex"];seen=set()
    for e in rows:
        rid=str(e["row"])
        if rid in seen or key(e)!=c or e["path"]!=row_path(e) or m["rowLookup"].get(rid)!=c:
            raise ValueError("Invalid country index: "+rid)
        seen.add(rid)
    if seen!={rid for rid,country in m["rowLookup"].items() if country==c}:
        raise ValueError("Country index inventory mismatch")
    return rows

def read_entry(path,e):
    r=raw(child(path,e["path"]))
    if project(r,e["path"])!=e:raise ValueError("Stale country index: "+str(e["row"]))
    return r

def read_index(path,country=None):
    m=raw(path)
    if m.get("matrixStorage")!=V2:
        rows=_v1_country(path,country) if country else _v1_read(path)["rows"]
        return [project(r,r.get("inventoryPath","")) for r in rows]
    return sorted([e for c in ([country] if country else m["rowShards"]) for e in country_index(path,m,c)],key=lambda e:e["row"])

def read_row(path,rid):
    m=raw(path)
    if m.get("matrixStorage")!=V2:
        matches=[r for r in _v1_read(path)["rows"] if r["row"]==rid]
    else:
        c=m["rowLookup"].get(str(rid))
        if c is None:raise KeyError(rid)
        matches=[read_entry(path,e) for e in country_index(path,m,c) if e["row"]==rid]
    if len(matches)!=1:raise KeyError(rid)
    return matches[0]

def read_country(path,country):
    m=raw(path)
    if m.get("matrixStorage")!=V2:return _v1_country(path,country)
    return [read_entry(path,e) for e in country_index(path,m,country)]

def read_matrix(path):
    m=raw(path)
    if m.get("matrixStorage")!=V2:return _v1_read(path)
    rows=[read_entry(path,e) for e in read_index(path)]
    if len(rows)!=len(m["rowLookup"]) or len({r["row"] for r in rows})!=len(rows):
        raise ValueError("Duplicate or missing row")
    m.pop("matrixStorage");m.pop("rowLookup");m.pop("rowShards")
    metadata=m.pop("metadataShards");m["rows"]=rows
    for field,relative in metadata.items():
        if field in m:raise ValueError("Duplicate metadata")
        m[field]=raw(child(path,relative))
    return m

def plan_matrix(path,obj):
    if not path.exists() or raw(path).get("matrixStorage")!=V2:return _v1_plan(path,obj)
    old=raw(path);groups={};seen=set()
    for r in obj["rows"]:
        rid=str(r["row"]);c=key(r)
        if rid in seen or old["rowLookup"].get(rid)!=c:raise ValueError("Inventory change")
        seen.add(rid);groups.setdefault(c,[]).append(r)
    if seen!=set(old["rowLookup"]) or set(groups)!=set(old["rowShards"]):raise ValueError("Inventory change")
    m={k:v for k,v in obj.items() if k!="rows"}
    m.update(matrixStorage=V2,rowShards=old["rowShards"],rowLookup=old["rowLookup"],metadataShards=old["metadataShards"])
    updates={}
    def put(p,v):
        if raw(p)!=v:updates[p]=encode(v)
    for c,rows in sorted(groups.items()):
        previous={e["row"]:e for e in country_index(path,old,c)};index=[]
        for r in sorted(rows,key=lambda r:r["row"]):
            relative=previous[r["row"]]["path"]
            if relative!=row_path(r):raise ValueError("World changed")
            put(child(path,relative),r);index.append(project(r,relative))
        put(child(path,old["rowShards"][c]),{"rowIndex":index})
    for field,relative in old["metadataShards"].items():put(child(path,relative),m.pop(field))
    put(path,m)
    return updates

def selftest():
    _v1_test()
    from pathlib import Path
    from tempfile import TemporaryDirectory
    with TemporaryDirectory() as td:
        p=Path(td)/"AUTOMATIC_INSERTION.json"
        r={"row":1,"world":"2627","name":"Test","status":"I","fields":{"name":"P"},"technicalIdentity":{"country":"ITA"},"evidenceRefs":{"x":[1]}}
        rel=row_path(r);target=p.parent/rel;target.parent.mkdir(parents=True)
        target.write_bytes(encode(r));c=p.parent/"ITA.json"
        c.write_bytes(encode({"rowIndex":[project(r,rel)]}))
        p.write_bytes(encode({"matrixStorage":V2,"rowLookup":{"1":"ITA"},"rowShards":{"ITA":"ITA.json"},"metadataShards":{},"summary":{}}))
        data=read_matrix(p)
        assert data["rows"]==[r] and read_row(p,1)==r and read_country(p,"ITA")==[r]
        assert not plan_matrix(p,data)
        data["rows"][0]["evidenceRefs"]["x"].append(2)
        changes=plan_matrix(p,data);assert set(changes)=={target}
        for f,b in changes.items():f.write_bytes(b)
        data["rows"][0]["status"]="P"
        changes=plan_matrix(p,data);assert set(changes)=={target,c}
        for f,b in changes.items():f.write_bytes(b)
        assert read_matrix(p)==data
        data["rows"][0]["status"]="V";target.write_bytes(encode(data["rows"][0]))
        try:read_row(p,1)
        except ValueError:pass
        else:raise AssertionError("Stale index accepted")
