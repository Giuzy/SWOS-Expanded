"""Source position evidence and conservative original SWOS role projection.

No packed player writes. General career positions are not dated memberships.
"""
import re
import unicodedata

# teams.c FindReplacementPlayerByPositionOrder: 0 GK 1 RB 2 LB 3 D
# 4 RW 5 LW 6 M 7 A. Generic DF/MF do not establish the exact OG slot.
ROLE_CODES={"GK":0,"RB":1,"LB":2,"D":3,"RW":4,"LW":5,"M":6,"A":7}
GROUPS={"GK":"GK","RB":"DEF","LB":"DEF","D":"DEF","RW":"MID","LW":"MID","M":"MID","A":"ATT"}

def normalize(value):
    if not isinstance(value,str):return ""
    return " ".join(re.sub(r"[^a-z0-9]+"," ",unicodedata.normalize("NFKD",value).encode("ascii","ignore").decode().casefold()).split())

EXACT={}
for role,labels in {
    "GK":("GK","G","Goalkeeper","Goal keeper"),
    "RB":("RB","Right back","Right full back","Right fullback","Right wing back","RWB"),
    "LB":("LB","Left back","Left full back","Left fullback","Left wing back","LWB"),
    "D":("CB","Centre back","Center back","Central defender","Central defence","Central defense","Sweeper","Libero"),
    "RW":("RW","Right winger","Right wing","Right midfield","Right midfielder","RM"),
    "LW":("LW","Left winger","Left wing","Left midfield","Left midfielder","LM"),
    "M":("CM","Central midfielder","Central midfield","Defensive midfielder","Defensive midfield","DM","CDM","Attacking midfielder","Attacking midfield","AM","CAM"),
    "A":("A","FW","F","ST","CF","Forward","Attacker","Attack","Striker","Centre forward","Center forward","Second striker","Secondary striker"),
}.items():
    for label in labels:EXACT[normalize(label)]=role
GENERAL={}
for group,labels in {"DEF":("D","DF","Defender","Defence","Defense","Full back","Fullback","Wing back"),
                     "MID":("M","MF","Midfielder","Midfield","Winger","Wide midfielder")}.items():
    for label in labels:GENERAL[normalize(label)]=group

def position_evidence(raw,source,scope,evidence=None,source_id=None):
    key=normalize(raw)
    if not key:return []
    role=EXACT.get(key);group=GROUPS.get(role) or GENERAL.get(key)
    row={"raw":raw.strip(),"source":source,"scope":scope,"group":group,"swosRole":role,"swosCode":ROLE_CODES.get(role)}
    if evidence:row["evidence"]=evidence
    if source_id:row["positionSourceId"]=source_id
    return [row]

def merge_positions(*lists):
    result=[]
    for rows in lists:
        for row in rows or []:
            if isinstance(row,dict) and row not in result:result.append(row)
    return result

def enrich(cp,old,row):
    merged=merge_positions(old.get("positionObservations"),row.get("positionObservations"))
    if merged!=old.get("positionObservations",[]):
        cp.update(old,positionObservations=merged)
        return True
    return False

def projection(rows):
    # Never hide conflicts, never force a position without a positive mapping.
    groups={x["group"] for x in rows if x.get("group")}
    roles={x["swosRole"] for x in rows if x.get("swosRole")}
    role=next(iter(roles)) if len(roles)==1 and len(groups)==1 else None
    group=next(iter(groups)) if len(groups)==1 else None
    status="CONFLICT_REVIEW" if len(roles)>1 or len(groups)>1 else "ROLE_CANDIDATE" if role else "GROUP_ONLY" if group else "UNKNOWN"
    return {"group":group,"swosRole":role,"swosCode":ROLE_CODES.get(role),"status":status}

def selftest():
    p=lambda x:position_evidence(x,"TEST","TEST")
    for label,role in (("Goalkeeper","GK"),("Right-Back","RB"),("Left-Back","LB"),("Centre-Back","D"),("Right Winger","RW"),("Left Midfield","LW"),("Defensive Midfield","M"),("Centre-Forward","A")):
        assert projection(p(label))["swosCode"]==ROLE_CODES[role]
    assert projection(p("DF"))=={"group":"DEF","swosRole":None,"swosCode":None,"status":"GROUP_ONLY"}
    assert projection(p("MF"))["status"]=="GROUP_ONLY"
    assert projection(p("FW"))["swosRole"]=="A"
    assert projection(p("Coach"))["status"]=="UNKNOWN"
    assert projection(p("Defender")+p("Centre-Back"))["swosRole"]=="D"
    assert projection(p("Left-Back")+p("Right-Back"))["status"]=="CONFLICT_REVIEW"
    assert projection(p("Goalkeeper")+p("Forward"))["swosCode"] is None
    assert len(merge_positions(p("GK"),p("GK")))==1
