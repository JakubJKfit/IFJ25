// EXPECT-EXIT: 0
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj .  write("Ahoj svete")
        Ifj.
            write("Ahoj svete")
    }

    static func1(
        x) {
        Ifj.write("Ahoj")
    }
}
