// Program 2: Vypocet faktorialu (rekurzivne)
import "ifj25" for Ifj
class Program {
    // Hlavni funkce
    static main() {
        Ifj.write("Zadejte cislo pro vypocet faktorialu: ")
        var inp 
        inp = Ifj.read_num()
        if (inp != null) {
            if (inp < 0) {
                Ifj.write("Faktorial nelze spocitat!\n")
            } else {
                // Overime celociselnost
                var inpFloored 
                inpFloored = Ifj.floor(inp)
                if (inp == inpFloored) {
                    var vysl 
                    vysl = factorial(inp)
                    vysl = Ifj.str(vysl)
                    Ifj.write("Vysledek: ")
                    Ifj.write(vysl)
                    Ifj.write("\n")
                } else {
                    Ifj.write("Cislo neni cele!\n")
                }
            }
        } else {
            Ifj.write("Chyba pri nacitani celeho cisla!\n")
        }
    }
    // Funkce pro vypocet hodnoty faktorialu
    static factorial(n) {
        var result
        if (n < 2) {
            result = 1
        } else {
            var decremented_n 
            decremented_n = n - 1
            result = factorial(decremented_n)
            result = n * result
        }
        return result
    }
}
