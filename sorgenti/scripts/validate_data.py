#!/usr/bin/env python3
"""Read-only, standard-library preflight for existing SWOS competition JSON.

Phase 7 A/B: inventory + structural and reference validation. This tool does
NOT install competitions into OG, simulate games, rewrite JSON, or touch CAR.
Python is already used by the repository's development import/export scripts;
this optional tool is NOT a Windows build/gameplay prerequisite.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import uuid
from pathlib import Path
from typing import Any

BASE = {
    "countries": "countries",
    "country_competitions": "countries",
    "career": "contests",
    "cups": "competitions",
    "tournaments": "competitions",
    "leagues": "leagues",
    "playoffs": "playoffs",
    "qualifying": "groups",
    "league_blocks": "groups",
}
DEFINITION_FILES = ("cups", "tournaments", "leagues", "playoffs", "league_blocks")
VIRTUAL_COUNTRIES = {72, 100, 252, 253, 254}  # OG synthetic selectors, not team files


class Audit:
    def __init__(self, root: Path, world: str, teams: bool) -> None:
        self.root, self.world, self.teams = root, world, teams
        self.errors: list[dict[str, str]] = []
        self.warnings: list[dict[str, str]] = []
        self.pending: list[dict[str, str]] = []
        self.totals: dict[str, int] = {}
        self._documents: dict[str, dict[str, Any]] = {}

    def note(self, level: str, path: Path, where: str, message: str) -> None:
        try:
            name = path.relative_to(self.root).as_posix()
        except ValueError:
            name = str(path)
        getattr(self, level).append(
            {"file": name, "field": where, "message": message}
        )

    def read(self, path: Path) -> dict[str, Any] | None:
        try:
            raw = json.loads(path.read_text(encoding="utf-8-sig"))
        except (OSError, UnicodeError, ValueError) as exc:
            self.note("errors", path, "$", "cannot read/parse JSON: " + str(exc))
            return None
        if not isinstance(raw, dict):
            self.note("errors", path, "$", "root must be an object")
            return None
        return raw

    def collection(self, path: Path, doc: dict[str, Any], key: str) -> list[Any]:
        array = doc.get(key)
        if not isinstance(array, list):
            self.note("errors", path, "$." + key, "required array missing")
            return []
        count = doc.get("count")
        if count is not None and (type(count) is not int or count != len(array)):
            self.note("errors", path, "$.count", "must equal len(" + key + ")")
        return array

    def check(self) -> None:
        comp_dir = self.root / "data_json" / "competitions"
        docs: dict[str, dict[str, Any]] = {}
        for name, key in BASE.items():
            path = comp_dir / (name + ".json")
            doc = self.read(path)
            if doc is None:
                continue
            docs[name] = doc
            values = self.collection(path, doc, key)
            self.totals["9697." + name] = len(values)
        self._documents = docs
        self.check_base(docs, comp_dir)
        if self.world in ("2627", "both"):
            self.check_world2627()
        self.totals["9697.native_team_files"] = len(list(
            (self.root / "data_json/teams").glob("*.json")))
        if self.world in ("2627", "both"):
            self.totals["2627.native_team_files"] = len(list(
                (self.root / "data_json/worlds/2627/teams").glob("*.json")))
            self.totals["2627.squad_extras_files"] = len(list(
                (self.root / "data_json/worlds/2627/squad_extras").glob("*.json")))
            self.note("pending", self.root / "data_json/worlds/2627/squad_extras",
                      "extra_players UUID links",
                      "extra squad-player UUID cross-references are phase D, not validated yet")
        if self.teams:
            self.check_team_rosters()
        else:
            self.note("pending", self.root / "data_json", "teams",
                      "deep team/player UUID and roster validation not executed; use --teams")
        self.note("pending", comp_dir, "OG runtime",
                  "JSON validation is not proof of a resident OG edition, match, trophy or CAR restore")

    def check_base(self, docs: dict[str, dict[str, Any]], directory: Path) -> None:
        registry: dict[str, str] = {}
        comp_ids: dict[int, str] = {}
        for kind in DEFINITION_FILES:
            for index, row in enumerate(docs.get(kind, {}).get(BASE[kind], [])):
                path = directory / (kind + ".json")
                loc = "$." + BASE[kind] + "[" + str(index) + "]"
                if not isinstance(row, dict) or not isinstance(row.get("id"), str):
                    self.note("errors", path, loc, "definition needs a string id")
                    continue
                identity = row["id"]
                if identity in registry:
                    self.note("errors", path, loc + ".id",
                              "duplicate " + identity + " (first " + registry[identity] + ")")
                else:
                    registry[identity] = kind
                comp = row.get("comp")
                if isinstance(comp, dict):
                    num = comp.get("comp_id")
                    if type(num) is int:
                        if num in comp_ids:
                            self.note("errors", path, loc + ".comp.comp_id",
                                      "numeric competition id reused by " + comp_ids[num])
                        else:
                            comp_ids[num] = identity
                    if comp.get("type") not in (0, 1, 2):
                        self.note("errors", path, loc + ".comp.type",
                                  "OG competition type must be 0, 1 or 2")
                if kind == "leagues":
                    divisions = row.get("divisions")
                    if not isinstance(divisions, list) or (
                        row.get("divisions_count") != len(divisions)
                    ):
                        self.note("errors", path, loc + ".divisions_count",
                                  "number of divisions must match divisions[]")
                    # Never use obsolete docs' division[4] limit: modern Italy has 14.
                if kind in ("cups", "tournaments"):
                    self.check_seeds(path, loc, row, docs.get("countries", {}))
        countries = docs.get("countries", {}).get("countries", [])
        by_number: dict[int, tuple[str, Any]] = {}
        by_define: set[str] = set()
        for i, c in enumerate(countries):
            path = directory / "countries.json"
            if not isinstance(c, dict) or type(c.get("number")) is not int:
                self.note("errors", path, "countries[" + str(i) + "]", "invalid country")
                continue
            num = c["number"]
            if num in by_number:
                if by_number[num] == (str(c.get("name")), c.get("team_file")):
                    self.note("warnings", path, "countries[" + str(i) + "].number",
                              "legacy identical country record repeated; no new identity")
                else:
                    self.note("errors", path, "countries[" + str(i) + "].number",
                              "conflicting duplicate country number " + str(num))
            by_number[num] = (str(c.get("name")), c.get("team_file"))
            if isinstance(c.get("define"), str):
                by_define.add(c["define"])
            team_file = c.get("team_file")
            if not isinstance(team_file, str) or not (
                self.root / "data_json" / team_file
            ).is_file():
                self.note("errors", path, "countries[" + str(i) + "].team_file",
                          "referenced base team file is missing")
        for i, c in enumerate(docs.get("country_competitions", {}).get("countries", [])):
            path = directory / "country_competitions.json"
            if not isinstance(c, dict):
                self.note("errors", path, "countries[" + str(i) + "]", "invalid entry")
                continue
            num = c.get("number")
            if num not in by_number:
                if num in VIRTUAL_COUNTRIES:
                    self.note("warnings", path, "countries[" + str(i) + "].number",
                              "original OG virtual selector; not a real team file")
                else:
                    self.note("errors", path, "countries[" + str(i) + "].number",
                              "unknown country number " + str(num))
            for kind in ("leagues", "cups"):
                for j, value in enumerate(c.get(kind, [])):
                    if value not in registry:
                        self.note("errors", path,
                                  "countries[" + str(i) + "]." + kind + "[" + str(j) + "]",
                                  "unresolved competition id " + str(value))
        path = directory / "career.json"
        for i, c in enumerate(docs.get("career", {}).get("contests", [])):
            if not isinstance(c, dict):
                self.note("errors", path, "contests[" + str(i) + "]", "invalid entry")
                continue
            ref = c.get("competition")
            if ref is None:
                continue  # native career terminator
            if ref not in registry:
                self.note("errors", path, "contests[" + str(i) + "].competition",
                          "unresolved competition " + str(ref))
        path = directory / "league_blocks.json"
        block_ids = {x.get("id") for x in docs.get("league_blocks", {}).get("groups", [])
                     if isinstance(x, dict)}
        for i, group in enumerate(docs.get("league_blocks", {}).get("groups", [])):
            if not isinstance(group, dict):
                continue
            for j, link in enumerate(group.get("leagues", [])):
                ref = link.get("league") if isinstance(link, dict) else None
                if ref not in registry:
                    self.note("errors", path, f"groups[{i}].leagues[{j}]",
                              "unresolved league " + str(ref))
            for key, targets in (("playoffs", registry), ("next_group", block_ids)):
                ref = group.get(key)
                if ref is not None and ref not in targets:
                    self.note("errors", path, f"groups[{i}].{key}",
                              "unresolved reference " + str(ref))
        path = directory / "qualifying.json"
        for i, group in enumerate(docs.get("qualifying", {}).get("groups", [])):
            if not isinstance(group, dict):
                continue
            for j, ref in enumerate(group.get("comp_cup", [])):
                # OpenSWOS comp_cup contains numeric metadata selectors:
                # -2 (special link) and 252 (Europe pseudo-country), not IDs.
                if ref not in (-2, -1, 252, None) and ref not in registry:
                    self.note("errors", path, f"groups[{i}].comp_cup[{j}]",
                              "unresolved competition " + str(ref))
        # The tenth baseline competition document uses a tier list rather
        # than a count+array envelope; validate its real SWOS format.
        path = directory / "seed_buckets.json"
        buckets = self.read(path)
        if buckets is not None:
            tiers = buckets.get("euro_cup_qual_seed_buckets")
            if not isinstance(tiers, list):
                self.note("errors", path, "$.euro_cup_qual_seed_buckets",
                          "seed tier list required")
            else:
                self.totals["9697.seed_buckets"] = len(tiers)
                for i, tier in enumerate(tiers):
                    if not isinstance(tier, dict):
                        self.note("errors", path, f"tiers[{i}]",
                                  "seed tier must be a JSON object")
                        continue
                    entries = tier.get("allocation")
                    if not isinstance(entries, list):
                        self.note("errors", path, f"tiers[{i}].allocation",
                                  "allocation must be an array")
                        continue
                    total = 0
                    for j, item in enumerate(entries):
                        number = item.get("number") if isinstance(item, dict) else None
                        berth = item.get("berths") if isinstance(item, dict) else None
                        if type(number) is not int or number not in by_number or (
                            type(berth) is not int or berth <= 0
                        ):
                            self.note("errors", path, f"tiers[{i}].allocation[{j}]",
                                      "unresolved country or invalid berth count")
                        else:
                            total += berth
                    size = tier.get("size")
                    if type(size) is not int or size not in (total, total + 1):
                        self.note("errors", path, f"tiers[{i}].size",
                                  "must equal total berths (or include OG sentinel)")
                    elif size == total + 1:
                        self.note("warnings", path, f"tiers[{i}].size",
                                  "legacy OG terminator slot included in size")
        self.totals["9697.definition_ids"] = len(registry)

    def check_seeds(self, path: Path, loc: str, competition: dict[str, Any],
                    countries_doc: dict[str, Any]) -> None:
        valid = {c.get("define") for c in countries_doc.get("countries", [])
                 if isinstance(c, dict)}
        for i, team in enumerate(competition.get("team_seed_list", [])):
            where = loc + ".team_seed_list[" + str(i) + "]"
            if not isinstance(team, dict) or team.get("country") not in valid or (
                type(team.get("ordinal")) is not int or team["ordinal"] < 0
            ):
                self.note("errors", path, where,
                          "invalid seed country define or negative team ordinal")
        # Actual ordinal-to-roster resolution is checked later with --teams.

    def check_world2627(self) -> None:
        path = self.root / "data_json/worlds/2627/competitions/leagues.json"
        doc = self.read(path)
        if doc:
            rows = self.collection(path, doc, "leagues")
            ids: set[str] = set()
            for i, row in enumerate(rows):
                if not isinstance(row, dict) or not isinstance(row.get("id"), str):
                    self.note("errors", path, f"leagues[{i}]", "missing overlay id")
                    continue
                identity = row["id"]
                if identity in ids:
                    self.note("errors", path, f"leagues[{i}].id", "duplicate overlay id")
                ids.add(identity)
                divisions = row.get("divisions")
                if not isinstance(divisions, list) or row.get("divisions_count") != len(divisions):
                    self.note("errors", path, f"leagues[{i}].divisions_count",
                              "number of divisions must match divisions[]")
            self.totals["2627.leagues"] = len(rows)
        path = self.root / "data_json/worlds/2627/competitions/libertadores_2027_first.json"
        doc = self.read(path)
        if doc:
            participants = doc.get("participants")
            fmt = doc.get("format", {})
            if not isinstance(participants, list) or not isinstance(fmt, dict):
                self.note("errors", path, "$", "participants[] and format{} required")
            else:
                self.totals["2627.libertadores_provisional_entrants"] = len(participants)
                direct = sum(isinstance(x, dict) and x.get("entry") == "direct"
                             for x in participants)
                preliminary = sum(isinstance(x, dict) and
                                  x.get("entry") == "preliminary" for x in participants)
                if (fmt.get("all") != len(participants) or
                    fmt.get("direct") != direct or
                    fmt.get("qualifying") != preliminary):
                    self.note("errors", path, "$.format",
                              "participant counts differ from format totals")
                seen: set[tuple[int, int]] = set()
                for i, p in enumerate(participants):
                    where = f"participants[{i}]"
                    if not isinstance(p, dict) or type(p.get("country_number")) is not int or (
                        type(p.get("team_ordinal")) is not int or p["team_ordinal"] < 0
                    ):
                        self.note("errors", path, where, "invalid club identity")
                        continue
                    key = (p["country_number"], p["team_ordinal"])
                    if key in seen:
                        self.note("errors", path, where, "duplicate country/ordinal")
                    seen.add(key)
                    team_file = p.get("team_file")
                    if not isinstance(team_file, str) or not (
                        self.root / "data_json/worlds/2627/teams" / team_file
                    ).is_file():
                        self.note("errors", path, where + ".team_file",
                                  "referenced world team file missing")
            calendar = doc.get("calendar_2027")
            if not isinstance(calendar, dict):
                self.note("errors", path, "$.calendar_2027", "calendar object missing")
            else:
                for round_name, days in calendar.items():
                    if not isinstance(days, list) or not days:
                        self.note("errors", path, "calendar_2027." + round_name,
                                  "nonempty list of ISO dates required")
                        continue
                    for j, raw in enumerate(days):
                        try:
                            d = dt.date.fromisoformat(raw)
                            if d.year != doc.get("edition"):
                                raise ValueError("not in edition year")
                        except (TypeError, ValueError):
                            self.note("errors", path,
                                      f"calendar_2027.{round_name}[{j}]",
                                      "invalid ISO date or edition year")
            if doc.get("runtime_status") != "DATA_ONLY_NOT_YET_LINKED_TO_OG_MATCHES":
                self.note("warnings", path, "runtime_status",
                          "status changed; verify native OG integration")
            else:
                self.note("pending", path, "runtime_status",
                          "provisional definition is DATA ONLY, not playable OG")
        self.note("pending", self.root / "data_json/worlds/2627", "competitions",
                  "no universal OG fixture/edition linker from arbitrary JSON yet")

    def check_team_rosters(self) -> None:
        roots = [("9697", self.root / "data_json/teams")]
        if self.world in ("2627", "both"):
            roots.append(("2627", self.root / "data_json/worlds/2627/teams"))
        for world, folder in roots:
            count = 0
            for path in sorted(folder.glob("*.json")):
                obj = self.read(path)
                if obj is None:
                    continue
                if obj.get("type") != "team":
                    continue  # pool/TMD files use different native layouts
                rows = self.collection(path, obj, "teams")
                count += len(rows)
                seen: set[tuple[int, int]] = set()
                team_uuids: set[str] = set()
                player_uuids: set[str] = set()
                for i, entry in enumerate(rows):
                    team = entry.get("team") if isinstance(entry, dict) else None
                    players = entry.get("players") if isinstance(entry, dict) else None
                    if not isinstance(team, dict) or not isinstance(players, list):
                        self.note("errors", path, f"teams[{i}]",
                                  "missing team object or players list")
                        continue
                    cn, ordinal = team.get("country_number"), team.get("team_ordinal")
                    if type(cn) is not int or type(ordinal) is not int:
                        self.note("errors", path, f"teams[{i}].team",
                                  "country_number and team_ordinal must be integers")
                        continue
                    key = (cn, ordinal)
                    team_uuid = entry.get("uuid")
                    if isinstance(team_uuid, str) and team_uuid:
                        try:
                            uuid.UUID(team_uuid)
                        except ValueError:
                            self.note("errors", path, f"teams[{i}].uuid",
                                      "invalid team UUID")
                        if team_uuid in team_uuids:
                            self.note("errors", path, f"teams[{i}].uuid",
                                      "duplicate team UUID in file")
                        team_uuids.add(team_uuid)
                    else:
                        self.note("warnings", path, f"teams[{i}].uuid",
                                  "team UUID missing: identity bridge must be audited")
                    if key in seen:
                        self.note("errors", path, f"teams[{i}].team.team_ordinal",
                                  "duplicate OG team identity within file")
                    seen.add(key)
                    if len(players) != 16:
                        self.note("errors", path, f"teams[{i}].players",
                                  "native OG roster requires exactly 16 base players; extras live separately")
                    names = [p.get("player_name") for p in players if isinstance(p, dict)]
                    if len(names) != len(players):
                        self.note("errors", path, f"teams[{i}].players",
                                  "player entries must be objects")
                    for j, p in enumerate(players):
                        if not isinstance(p, dict):
                            continue
                        player_uuid = p.get("uuid")
                        if not isinstance(player_uuid, str) or not player_uuid:
                            self.note("warnings", path, f"teams[{i}].players[{j}].uuid",
                                      "player UUID missing")
                            continue
                        try:
                            uuid.UUID(player_uuid)
                        except ValueError:
                            self.note("errors", path, f"teams[{i}].players[{j}].uuid",
                                      "invalid player UUID")
                        if player_uuid in player_uuids:
                            self.note("errors", path, f"teams[{i}].players[{j}].uuid",
                                      "duplicate player UUID within file")
                        player_uuids.add(player_uuid)
                # Cross-file WORLD_SLOTS, squad_extras and SAVE migrations
                # need a future runtime check; this is an offline preflight.
            self.totals[world + ".team_records_checked"] = count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2],
                        help="SWOS-Expanded repository root (default: autodetected)")
    parser.add_argument("--world", choices=("9697", "2627", "both"), default="both")
    parser.add_argument("--teams", action="store_true",
                        help="also parse team/player JSON (slower; optional)")
    parser.add_argument("--json", action="store_true",
                        help="print machine-readable report instead of text")
    args = parser.parse_args()
    audit = Audit(args.root.resolve(), args.world, args.teams)
    audit.check()
    report = {"result": "FAIL" if audit.errors else "PASS_SCOPED",
              "world": args.world, "team_files_checked": args.teams,
              "totals": audit.totals,
              "errors": audit.errors, "warnings": audit.warnings,
              "pending": audit.pending}
    if args.json:
        print(json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True))
    else:
        print("SWOS_DEFINITIONS_" + report["result"])
        for k, v in sorted(audit.totals.items()):
            print(f"  {k}: {v}")
        for severity in ("errors", "warnings", "pending"):
            for entry in getattr(audit, severity):
                print(f"{severity.upper()} {entry['file']}:{entry['field']}: {entry['message']}")
    return 1 if audit.errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
