#!/bin/bash

# Kde je tvůj překladač
BIN="./ifj"
# Kde jsou testy
EX_DIR="examples"

# Zkontrolujeme, jestli binárka existuje
if [ ! -f "$BIN" ]; then
    echo "Chyba: Překladač '$BIN' nenalezen. Nejdřív spusť 'make'."
    exit 1
fi

echo "Generuji .expected soubory pro 'ok' a 'ex' testy..."

# Projdi všechny soubory, které začínají na "ok-" nebo "ex-"
for testfile in "$EX_DIR"/ok-*.wren "$EX_DIR"/ex*.wren; do
    # Zkontrolujeme, jestli soubor vůbec existuje
    [ -e "$testfile" ] || continue
    
    basename=$(basename "$testfile")
    expected_file="${testfile}.expected"
    
    echo "  -> $basename"
    
    # Spusť parser a přesměruj výstup do .expected souboru
    cat "$testfile" | $BIN --dump-ast > "$expected_file"
done

echo "Hotovo."
echo "Nyní prosím RUČNĚ zkontroluj obsah všech nově vytvořených .expected souborů."