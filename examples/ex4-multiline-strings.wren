// Program 4: Viceradkove retezce
import "ifj25" for Ifj
class Program {
    static main() {
        var s

        __d = Ifj.write("=== Priklady nize se vytisknou stejne: ===\n")

        // Zadne bile znaky ani \n
        s = """ahoj"""
        __d = printChars(s)

        // Retezec nasleduje hned za """
        // ale pred zaviracimi """ jsou jen bile znaky a \n (uriznou se)
        s = """ahoj
    """
        __d = printChars(s)

        // Za """ jsou bile znaky a \n (uriznou se)
        // ale pred zaviracimi """ se nic neignoruje
        s = """     
ahoj"""
        __d = printChars(s)

        // Za """ jsou bile znaky (\t) a \n (uriznou se)
        // pred zaviracimi """ je jen \n (urizne se)
        s = """		
ahoj
"""
        __d = printChars(s)

        // Za """ jsou bile znaky (\t) a \n (uriznou se)
        // pred zaviracimi """ jsou taky (uriznou se)
        s = """		
ahoj
      """
        __d = printChars(s)

        __d = Ifj.write("\n=== Dalsi priklady: ===\n")

        // Zde jsou bile znaky za """ na stejnem radku, kde zacina text (budou soucasti retezce)
        s = """  	  ahoj"""
        __d = printChars(s)

        // Zde je na radku pred """ jeste nejaky nebily znak (cely radek i predchazejici \n jsou soucasti retezce)
        s = """ahoj
    x """
        __d = printChars(s)

        // Pozor, zde se spravne ignoruji bile znaky nasledujici po """, ale pouze po prvni \n (vcetne).
        // Pak uz zacina dalsi radek, ktery obsahuje ctyri mezery a \n, ktere uz jsou soucasti retezce!
        s = """    
    
ahoj"""
        __d = printChars(s)

        // Podobne to funguje i s ukoncovacimi """: \n za ahoj a 8 mezer na nasledujicim radku jsou vsechno
        // soucasti retezce, urizne se jen posledni \n a bile znaky na radku pred """
        s = """ahoj
        
        """
        __d = printChars(s)
    }

    // Pomocna funkce pro nazorne tisknuti retezce znak po znaku (vc. decimalni ASCII hodnoty)
    static printChars(s) {
        var l
        l = Ifj.length(s)
        var i
        i = 0
        var char
        while (i < l) {
            char = Ifj.ord(s, i)
            if (char < 100) {
                if (char >= 10) {
                    __d = Ifj.write(" ")
                } else {
                    __d = Ifj.write("  ")
                }
            } else {
            }
            __d = Ifj.write(char)
            if (i != l - 1) {
                __d = Ifj.write(", ")
            } else {
                __d = Ifj.write("\n")
            }
            i = i + 1
        }
        l = Ifj.length(s)
        i = 0
        while (i < l) {
            char = Ifj.ord(s, i)
            if (char < 32) {
                if (char == 10) {
                    __d = Ifj.write(" \\n")
                } else {
                    if (char == 13) {
                        __d = Ifj.write(" \\r")
                    } else {
                        if (char == 9) {
                            __d = Ifj.write(" \\t")
                        } else {
                            __d = Ifj.write("  -")
                        }
                    }
                }
            } else {
                if (char == 32) {
                    __d = Ifj.write("___")
                } else {
                    __d = Ifj.write("  ")
                    char = Ifj.chr(char)
                    __d = Ifj.write(char)
                }
            }
            if (i != l - 1) {
                __d = Ifj.write(", ")
            } else {
                __d = Ifj.write("\n")
            }
            i = i + 1
        }
    }
}