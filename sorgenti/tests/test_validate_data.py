#!/usr/bin/env python3
"""Isolated read-only phase-7 validator tests. No OpenSWOS executable needed."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


SOURCE = Path(__file__).resolve().parents[1] / "scripts" / "validate_data.py"
spec = importlib.util.spec_from_file_location("swos_validate_data", SOURCE)
module = importlib.util.module_from_spec(spec)
assert spec is not None and spec.loader is not None
spec.loader.exec_module(module)


def save(root: Path, path: str, payload: dict) -> None:
    target = root / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(payload), encoding="utf-8")


class DefinitionPreflightTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        d = "data_json/competitions/"
        save(self.root, d + "countries.json",
             {"type": "countries", "count": 1,
              "countries": [{"number": 8, "name": "ENGLAND",
                             "define": "CN_ENGLAND",
                             "team_file": "teams/ENGLAND.json"}]})
        save(self.root, "data_json/teams/ENGLAND.json", {"type": "team",
             "country": 8, "count": 0, "teams": []})
        save(self.root, d + "country_competitions.json",
             {"type": "country_competitions", "count": 1,
              "countries": [{"number": 8, "name": "ENGLAND",
                             "leagues": ["_league"], "cups": ["_cup"]}]})
        save(self.root, d + "career.json",
             {"type": "career_index", "count": 2,
              "contests": [{"competition": "_cup"}, {"competition": None}]})
        save(self.root, d + "cups.json",
             {"type": "competition_cups", "count": 1, "competitions": [{
                 "id": "_cup", "comp": {"type": 1, "comp_id": 10},
                 "team_seed_list": [{"country": "CN_ENGLAND", "ordinal": 0}]}]})
        for name, key, values in (
            ("tournaments", "competitions", []),
            ("leagues", "leagues", [{"id": "_league",
               "comp": {"type": 0, "comp_id": 11},
               "divisions_count": 1, "divisions": [{"teams": 2}]}]),
            ("playoffs", "playoffs", []),
            ("qualifying", "groups", [{"id": "_qual",
               "comp_cup": [-2, 252, "_cup", -1]}]),
            ("league_blocks", "groups", [{"id": "_block",
               "leagues": [{"league": "_league"}], "next_group": None}])
        ):
            save(self.root, d + name + ".json",
                 {"type": name, "count": len(values), key: values})

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def run_audit(self) -> module.Audit:
        audit = module.Audit(self.root, "9697", False)
        audit.check()
        return audit

    def test_valid_registry_and_native_sentinels(self) -> None:
        audit = self.run_audit()
        self.assertEqual(audit.errors, [])
        self.assertGreaterEqual(audit.totals["9697.definition_ids"], 3)
        self.assertTrue(audit.pending)  # a scoped PASS is NOT an OG gameplay PASS

    def test_unresolved_seed_country_is_rejected(self) -> None:
        path = self.root / "data_json/competitions/cups.json"
        raw = json.loads(path.read_text(encoding="utf-8"))
        raw["competitions"][0]["team_seed_list"][0]["country"] = "CN_MISSING"
        save(self.root, "data_json/competitions/cups.json", raw)
        self.assertTrue(any("seed" in e["field"] for e in self.run_audit().errors))

    def test_bad_count_is_rejected(self) -> None:
        path = self.root / "data_json/competitions/leagues.json"
        raw = json.loads(path.read_text(encoding="utf-8"))
        raw["count"] = 5
        save(self.root, "data_json/competitions/leagues.json", raw)
        self.assertTrue(any(e["field"] == "$.count" for e in self.run_audit().errors))

    def test_conflicting_country_duplicate_is_rejected(self) -> None:
        path = self.root / "data_json/competitions/countries.json"
        raw = json.loads(path.read_text(encoding="utf-8"))
        raw["countries"].append({"number": 8, "name": "DIFFERENT",
                                 "define": "CN_ENGLAND",
                                 "team_file": "teams/ENGLAND.json"})
        raw["count"] = 2
        save(self.root, "data_json/competitions/countries.json", raw)
        self.assertTrue(any("conflicting duplicate" in e["message"]
                            for e in self.run_audit().errors))


if __name__ == "__main__":
    unittest.main()
