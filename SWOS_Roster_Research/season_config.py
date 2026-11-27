"""Research season, separate from the immutable game world identifiers."""
import re
from pathlib import Path

def year_value(value):
    year=int(value)
    if not 2026<=year<=2098:raise ValueError("Research year must be 2026..2098")
    return year

def output_dir(root,year):
    year=year_value(year)
    return Path(root)/"SWOS_Roster_Research"/f"results_{year%100:02d}{(year+1)%100:02d}"

def seasons(year):
    return (str(year),f"{year}/{(year+1)%100:02d}")

def season_map(year):
    a,b=seasons(year)
    return {a:a,b:b,f"{year%100:02d}/{(year+1)%100:02d}":b}

def edition_label(text,year):
    text=str(text or "")
    tokens=set()
    for match in re.finditer(r"(?<![\d/–-])(?:20)?\d{2}[/–-](?:20)?\d{2}(?![\d/–-])|(?<![\d/–-])20\d{2}(?![\d/–-])",text):
        t=match.group()
        if re.search(r"[/–-]",t):
            a,b=re.split(r"[/–-]",t)
            tokens.add(a[-2:]+"/"+b[-2:])
        else:tokens.add(t)
    return season_map(year).get(next(iter(tokens))) if len(tokens)==1 else None

def cutoff(year):
    return f"{year_value(year)}-07-01T00:00:00Z"

def selftest():
    assert edition_label("League 2027–28",2027)=="2027/28"
    assert edition_label("League 2027",2027)=="2027"
    assert edition_label("League 2026/27",2027) is None
    assert edition_label("League 2027/28 and 2026/27",2027) is None
    assert output_dir("/x",2027).name=="results_2728"
