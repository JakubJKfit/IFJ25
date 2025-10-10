#!/usr/bin/env python3
import subprocess as sp
import sys, os, re, glob

BIN = os.environ.get("IFJ_BIN", "./ifj")
EX_DIR = "examples"

def expected_exit(path):
    # Bezpečně načti prvních pár řádků a hledej // EXPECT-EXIT: <kód>
    head = ""
    try:
        with open(path, "r", encoding="utf-8") as f:
            for _ in range(12):  # stačí pár řádků na začátku
                line = f.readline()
                if not line:
                    break
                head += line
    except Exception:
        return 0
    m = re.search(r"(?m)^\s*//\s*EXPECT-EXIT\s*:\s*(\d+)\s*$", head)
    return int(m.group(1)) if m else 0

def run_one(path):
    exp = expected_exit(path)
    with open(path, "rb") as fin:
        p = sp.run([BIN], stdin=fin, stdout=sp.PIPE, stderr=sp.PIPE)
    ok = (p.returncode == exp)
    name = os.path.basename(path)
    if ok:
        print(f"✅ {name:<32} [OK]  (exit {p.returncode})")
    else:
        print(f"❌ {name:<32} [FAIL] (exit {p.returncode}, expected {exp})")
        if p.stdout:
            print("   --- Stdout ---")
            sys.stdout.buffer.write(p.stdout)
            print()
        if p.stderr:
            print("   --- Stderr ---")
            sys.stdout.buffer.write(p.stderr)
            print()
    return ok

def main():
    if not os.path.exists(BIN):
        print(f"Chybí binárka '{BIN}'. Nejdřív proveď 'make'.")
        sys.exit(2)

    print(f"Spouštím testy ve složce '{EX_DIR}'...\n")
    files = sorted(glob.glob(os.path.join(EX_DIR, "*.wren")))
    if not files:
        print("Nenalezeny žádné .wren testy.")
        sys.exit(1)
    ok = sum(run_one(f) for f in files)
    print(f"\nShrnutí:\n   Úspěšné: {ok}/{len(files)}")
    sys.exit(0 if ok == len(files) else 1)

if __name__ == "__main__":
    main()
