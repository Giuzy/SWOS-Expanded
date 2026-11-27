"""Run the three research launchers with a single replace-on-start UTF-8 log each."""
import datetime
import os
import re
from pathlib import Path
import signal
import subprocess
import sys
import threading
import time
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


def elapsed_text(seconds):
    seconds=max(0,int(seconds));hours,rem=divmod(seconds,3600);minutes,secs=divmod(rem,60)
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"


class Run:
    def __init__(self, log, mode):
        self.log = log
        self.mode = mode
        self.total = ""
        self.detail = ""
        self.title = ""
        self.started = time.monotonic()
        self.timer_stop = threading.Event()
        self.timer_lock = threading.Lock()
        self.timer_thread = None
        self.coverage = ""
        self.cancelled = False
        self.env = dict(os.environ, PYTHONUNBUFFERED="1", PYTHONIOENCODING="utf-8", PYTHONUTF8="1")

    def elapsed(self):
        return elapsed_text(time.monotonic()-self.started)

    def _render_title(self):
        if os.name=="nt":
            with self.timer_lock:
                try:
                    import ctypes
                    ctypes.windll.kernel32.SetConsoleTitleW("SWOS "+self.title+" | TOTAL TIME "+self.elapsed())
                except (OSError,AttributeError):
                    pass

    def set_title(self,value):
        with self.timer_lock:self.title=value
        self._render_title()

    def start_timer(self):
        self.set_title(self.mode)
        if os.name=="nt":
            def tick():
                while not self.timer_stop.wait(1):self._render_title()
            self.timer_thread=threading.Thread(target=tick,daemon=True)
            self.timer_thread.start()

    def stop_timer(self):
        self.timer_stop.set()
        if self.timer_thread:self.timer_thread.join()

    def display_line(self,line):
        clean=line.rstrip("\r\n")
        if clean.startswith(("WORKLOAD EA SWOS |","RESULT EA SWOS |")):
            m=re.search(r"named_club_slots=(\d+).*without_unambiguous_candidate=(\d+)",clean)
            if m:self.coverage=f"SWOS without exact unique EA candidate {m[2]}/{m[1]}"
        if clean.startswith("TOTAL "):
            self.total=clean[6:];self.detail=""
        elif clean.startswith("PHASE ") or (clean.startswith("PROGRESS ") and " phases:" in clean.lower()):
            self.total=clean.removeprefix("PROGRESS ");self.detail=""
        elif re.match(r"^.+ — ACTION \d+/\d+ ",clean):
            self.detail=clean.split(" (",1)[0].replace(" — ACTION "," ")
        elif clean.startswith("RESULT "):
            self.detail=""
        context=" | ".join(x for x in (self.coverage,self.total,self.detail) if x)
        self.set_title(self.mode+(" | "+context if context else ""))
        if clean:
            return "["+"TOTAL TIME "+self.elapsed()+(" | "+context if context and not clean.startswith(("TOTAL ","RESULT ")) else "")+"] "+line
        return line

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
        self.total="";self.detail="";self.coverage=""
        self.set_title(self.mode)
        with subprocess.Popen([str(executable), "-u", *map(str, args)], cwd=ROOT,
                              env=self.env, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, text=True,
                              encoding="utf-8", errors="replace") as child:
            for line in child.stdout:
                self.write(self.display_line(line))
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
        run = Run(log, mode)
        assert elapsed_text(0)=="00:00:00" and elapsed_text(3661)=="01:01:01" and elapsed_text(360001)=="100:00:01"
        run.start_timer()
        previous = signal.signal(signal.SIGINT, run.interrupt)
        rc = 1
        try:
            run.write(f"[START] {stamp()} | {mode}\n[LOG] diagnostica/LOG/{path.name}\n")
            rc = run.run(mode, args)
        except Exception:
            run.write(traceback.format_exc())
        finally:
            signal.signal(signal.SIGINT, previous)
            run.stop_timer()
            run.write(f"\n[END] {stamp()} | EXIT={rc} | TOTAL TIME={run.elapsed()}\n")
            run.set_title(mode+f" | EXIT={rc}")
        return rc


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
