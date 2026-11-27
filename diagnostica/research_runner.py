"""Run the three research launchers with a single replace-on-start UTF-8 log each."""
import datetime
import os
from pathlib import Path
import signal
import subprocess
import sys
import traceback

ROOT = Path(__file__).resolve().parent.parent
MODES = {
    "COMPETIZIONI": "tools/competition_research/strict_online.py",
    "GIOCATORI": "SWOS_Roster_Research/collect_names.py",
    "STATS_EA": "SWOS_Roster_Research/collect_ea.py",
}


def lock_log(log):
    # Lock the existing log BEFORE truncating, without creating lock/history files.
    log.seek(0)
    if os.name == "nt":
        import msvcrt
        msvcrt.locking(log.fileno(), msvcrt.LK_NBLCK, 1)
    else:
        import fcntl
        fcntl.flock(log.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)


class Run:
    def __init__(self, log):
        self.log = log
        self.cancelled = False
        self.env = dict(os.environ, PYTHONUNBUFFERED="1", PYTHONIOENCODING="utf-8", PYTHONUTF8="1")

    def interrupt(self, signum, frame):
        # The child shares the console and receives Ctrl+C too. Drain its final
        # output instead of killing it while it is saving a checkpoint.
        self.cancelled = True

    def write(self, text):
        self.log.write(text)
        self.log.flush()
        sys.stdout.write(text)
        sys.stdout.flush()

    def python(self, executable, *args):
        if self.cancelled:
            return 130
        with subprocess.Popen([str(executable), "-u", *map(str, args)], cwd=ROOT,
                              env=self.env, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, text=True,
                              encoding="utf-8", errors="replace") as child:
            for line in child.stdout:
                self.write(line)
            rc = child.wait()
        return 130 if self.cancelled else rc

    def run(self, mode, args):
        executable = sys.executable
        script = ROOT / MODES[mode]
        if mode == "COMPETIZIONI":
            self.write("Verifica automatica e ripresa ricerca competizioni...\n")
            return self.python(executable, script, *args)
        if mode == "GIOCATORI":
            roster = ROOT / "SWOS_Roster_Research"
            first = args[0].lower() if args else ""
            defaults = []
            if not args or (len(args) == 1 and first in ("--export-only", "--recover-snapshot")):
                defaults.append("--restart-from-json")
            if not args and not (roster / "results_2627/research.sqlite3.restart").exists():
                defaults.append("--refresh-wiki")
            if first not in ("--self-test", "--export-only", "--recover-snapshot", "--prepare-import"):
                venv = roster / ".venv"
                vpython = venv / "Scripts/python.exe" if os.name == "nt" else venv / "bin/python"
                if not vpython.exists():
                    self.write("Preparazione ambiente Python giocatori...\n")
                    self.python(executable, "-m", "venv", venv)
                if self.cancelled:
                    return 130
                if vpython.exists():
                    check = subprocess.run([str(vpython), "-c", "import duckdb"], cwd=ROOT,
                                           env=self.env, stdout=subprocess.DEVNULL,
                                           stderr=subprocess.DEVNULL)
                    if check.returncode and not self.cancelled:
                        self.write("Installazione dipendenza duckdb...\n")
                        self.python(vpython, "-m", "pip", "install", "duckdb>=1.1,<2")
                    executable = vpython
        else:
            defaults = []
        if not args:
            self.write("Verifica automatica ricerca " + ("giocatori" if mode == "GIOCATORI" else "statistiche EA") + "...\n")
            rc = self.python(executable, script, "--self-test")
            if rc:
                self.write("Verifica non superata. Dati esistenti preservati. Controllare il log.\n")
                return rc
        return self.python(executable, script, *defaults, *args)


def stamp():
    return datetime.datetime.now().astimezone().isoformat(timespec="seconds")


def main(argv):
    if not argv or argv[0] not in MODES:
        print("Invalid research launcher", file=sys.stderr)
        return 2
    mode, *args = argv
    path = ROOT / "diagnostica" / "LOG" / ("RICERCA_" + mode + ".log")
    try:
        path.parent.mkdir(parents=True, exist_ok=True)
        log = path.open("a+", encoding="utf-8", newline="\n")
    except OSError as exc:
        print(f"Impossibile aprire il log {path}: {exc}", file=sys.stderr)
        return 2
    with log:
        try:
            lock_log(log)
        except OSError as exc:
            print(f"Log occupato o non bloccabile; nessuna sovrascrittura: {path}: {exc}", file=sys.stderr)
            return 2
        log.truncate(0)
        log.seek(0)
        if hasattr(sys.stdout, "reconfigure"):
            sys.stdout.reconfigure(errors="backslashreplace")
        run = Run(log)
        previous = signal.signal(signal.SIGINT, run.interrupt)
        rc = 1
        try:
            run.write(f"[START] {stamp()} | {mode}\n[LOG] diagnostica/LOG/{path.name}\n")
            rc = run.run(mode, args)
        except Exception:
            run.write(traceback.format_exc())
        finally:
            signal.signal(signal.SIGINT, previous)
            run.write(f"\n[END] {stamp()} | EXIT={rc}\n")
        return rc


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
