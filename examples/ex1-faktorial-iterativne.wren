// Program 1: Vypocet faktorialu (iterativne, bez overeni celociselnosti)
import "ifj25" for Ifj
class Program {
    // Hlavni telo programu - funkce main
    static main() {
        Ifj.write("Zadejte cislo pro vypocet faktorialu\n")
        var a 
        a = Ifj.read_num()
        if (a != null) {
            if (a < 0) {
                Ifj.write("Faktorial ")
                Ifj.write(a)
                Ifj.write(" nelze spocitat\n")
            } else {
                var vysl 
                vysl = 1
                while (a > 0) {
                    vysl = vysl * a
                    a = a - 1
                }
                vysl = Ifj.floor(vysl)
                vysl = Ifj.str(vysl)
                vysl = "Vysledek: " + vysl + "\n"
                Ifj.write(vysl)
            }
        } else { // a == null
            Ifj.write("Faktorial pro null nelze spocitat\n")
        }
    }
}
