#!/usr/bin/env python3
import subprocess
import os
import sys
from pathlib import Path

# Cesta k binárce překladače
BIN = "./ifj"
# Cesta ke složce s testy
EXAMPLES_DIR = Path("examples")

# Očekávané návratové kódy pro jednotlivé testy
EXPECTED = {
    "ex0-vsechny-konstrukce.wren": 0,
    "ex1-faktorial-iterativne.wren": 0,
    "ex2-faktorial-rekurzivne.wren": 0,
    "ex3-prace-s-retezci.wren": 0,
}

def run_test(file: Path) -> tuple[int, str]:
    """Spustí překladač na daném souboru a vrátí návratový kód a stdout."""
    try:
        result = subprocess.run(
            [BIN],
            input=file.read_bytes(),
            capture_output=True,
            timeout=5
        )
        return result.returncode, result.stdout.decode(errors="ignore") + result.stderr.decode(errors="ignore")
    except subprocess.TimeoutExpired:
        return -1, "Timeout"
    except FileNotFoundError:
        print(f"❌ Nenalezen binární soubor {BIN}. Spusť nejdřív `make`.")
        sys.exit(1)

def main():
    if not Path(BIN).exists():
        print("❌ Překladač nebyl nalezen. Spusť nejdřív `make`.")
        sys.exit(1)

    total = 0
    passed = 0

    print(f"Spouštím testy ve složce '{EXAMPLES_DIR}/'...\n")

    for file in sorted(EXAMPLES_DIR.glob("*.wren")):
        total += 1
        expected = EXPECTED.get(file.name, 0)
        rc, output = run_test(file)
        if rc == expected:
            print(f"✅ {file.name:35} [OK]  (exit {rc})")
            passed += 1
        else:
            print(f"❌ {file.name:35} [FAIL] (exit {rc}, expected {expected})")
            if output.strip():
                print("   --- Výstup ---")
                print("   " + "\n   ".join(output.splitlines()[:5]))  # ukáže max 5 řádků

    print("\nShrnutí:")
    print(f"   Úspěšné: {passed}/{total}")
    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
