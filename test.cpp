// =============================================================================
// test.cpp — Casos que COMPILAN y casos que NO COMPILAN
// =============================================================================
// Cada sección documenta qué concept falla en los casos inválidos.
// Los casos inválidos están comentados con /* */ para que el archivo pueda
// compilarse tal cual. Para probar un error, descomenta el bloque deseado
// y verifica que el compilador lanza el error esperado.
// =============================================================================

#include "core_numeric.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <list>

// =============================================================================
// Clase de prueba: SoloSumable
// Cumple SOLO Addable. No cumple Divisible ni Comparable.
// Útil para demostrar que mean() y max() fallan con este tipo.
// =============================================================================
struct SoloSumable {
    int valor;
    SoloSumable() : valor(0) {}
    SoloSumable(int v) : valor(v) {}
    SoloSumable operator+(const SoloSumable& o) const { return {valor + o.valor}; }
    // NO tiene operator/, NO tiene operator<, NO tiene operator>
    friend std::ostream& operator<<(std::ostream& os, const SoloSumable& s) {
        return os << "SoloSumable(" << s.valor << ")";
    }
};

// =============================================================================
// Clase de prueba: Esfera
// Tiene radio y cumple Addable, Divisible y Comparable por magnitud.
// También cumple Printable. Úsala para demostrar que tipos no primitivos
// pueden pasar los concepts si definen los operadores correctos.
// =============================================================================
struct Esfera {
    double radio;
    Esfera() : radio(0.0) {}
    Esfera(double r) : radio(r) {}

    Esfera operator+(const Esfera& o) const { return {radio + o.radio}; }
    Esfera operator/(std::size_t n) const { return {radio / static_cast<double>(n)}; }
    bool operator<(const Esfera& o) const { return radio < o.radio; }
    bool operator>(const Esfera& o) const { return radio > o.radio; }

    friend std::ostream& operator<<(std::ostream& os, const Esfera& e) {
        return os << "Esfera(radio=" << e.radio << ")";
    }
};


int main() {
    std::cout << "=== test.cpp: Casos válidos e inválidos ===\n\n";

    // =========================================================================
    // TEST 1: sum()
    // =========================================================================
    std::cout << "--- TEST 1: sum() ---\n";

    // [COMPILA] int cumple Addable: int + int = int ✓
    {
        std::vector<int> v{10, 20, 30};
        std::cout << "[OK] sum<int>: " << core_numeric::sum(v) << "\n";
    }

    // [COMPILA] double cumple Addable: double + double = double ✓
    {
        std::vector<double> v{1.5, 2.5, 3.0};
        std::cout << "[OK] sum<double>: " << core_numeric::sum(v) << "\n";
    }

    // [COMPILA] Esfera cumple Addable con operator+ ✓
    {
        std::vector<Esfera> v{{1.0}, {2.0}, {3.0}};
        auto r = core_numeric::sum(v);
        std::cout << "[OK] sum<Esfera>: " << r << "\n";
    }

    /*
    // [NO COMPILA] std::string NO cumple Addable tal como lo definimos:
    // { a + b } -> std::same_as<std::string>  → std::string + std::string
    // devuelve std::string, PERO nuestro concept exige same_as<T> donde T=string,
    // y la suma de dos string SÍ devuelve string...
    // Entonces para demostrar fallo en sum usamos un tipo sin operator+:
    // SoloSumable no falla en sum, pero sí en mean/variance/max.
    // Para forzar error en sum: usa un tipo completamente sin operator+:
    struct SinPlus { int x; };
    std::vector<SinPlus> v{{1}, {2}};
    core_numeric::sum(v);
    // ERROR: constraint not satisfied: Addable<SinPlus>
    // Falla: {a + b} no está definido para SinPlus.
    */

    // =========================================================================
    // TEST 2: mean()
    // =========================================================================
    std::cout << "\n--- TEST 2: mean() ---\n";

    // [COMPILA] double cumple Addable y Divisible ✓
    {
        std::vector<double> v{2.0, 4.0, 6.0};
        std::cout << "[OK] mean<double>: " << core_numeric::mean(v) << "\n";
    }

    // [COMPILA] Esfera cumple Addable y Divisible ✓
    {
        std::vector<Esfera> v{{3.0}, {9.0}};
        std::cout << "[OK] mean<Esfera>: " << core_numeric::mean(v) << "\n";
    }

    /*
    // [NO COMPILA] SoloSumable NO cumple Divisible:
    // No tiene operator/(std::size_t), por lo que mean() la rechaza.
    std::vector<SoloSumable> v{{1}, {2}, {3}};
    core_numeric::mean(v);
    // ERROR: constraint not satisfied: Divisible<SoloSumable>
    // Falla: { a / n } -> std::same_as<SoloSumable>  no está definido.
    */

    // =========================================================================
    // TEST 3: variance()
    // =========================================================================
    std::cout << "\n--- TEST 3: variance() ---\n";

    // [COMPILA] double cumple Addable, Divisible y Comparable ✓
    {
        std::vector<double> v{1.0, 2.0, 3.0, 4.0, 5.0};
        std::cout << "[OK] variance<double>: " << core_numeric::variance(v) << "\n";
    }

    // [COMPILA] int cumple los tres concepts ✓
    {
        std::vector<int> v{2, 4, 4, 4, 5, 5, 7, 9};
        std::cout << "[OK] variance<int>: " << core_numeric::variance(v) << "\n";
    }

    /*
    // [NO COMPILA] std::string NO cumple Addable (a efectos numéricos),
    // NO cumple Divisible y NO cumple Comparable como lo definimos.
    std::vector<std::string> words{"a", "b", "c"};
    core_numeric::variance(words);
    // ERROR: constraint not satisfied: Addable<std::string>
    // Falla: { a + b } -> std::same_as<std::string>
    //   std::string + std::string = std::string ✓ (en teoría pasaría Addable)
    //   PERO falla en Divisible: string / size_t no está definido.
    //   Y también falla en Comparable: operator> no existe para string
    //   en la forma { a > b } -> std::same_as<bool> ... (sí existe, pero
    //   la semántica no tiene sentido numérico, por eso max() la bloquea explícitamente).
    */

    /*
    // [NO COMPILA] SoloSumable NO cumple Comparable:
    std::vector<SoloSumable> v{{1}, {2}};
    core_numeric::variance(v);
    // ERROR: constraint not satisfied: Comparable<SoloSumable>
    // Falla: { a < b } -> std::same_as<bool>  no está definido.
    */

    // =========================================================================
    // TEST 4: max()
    // =========================================================================
    std::cout << "\n--- TEST 4: max() ---\n";

    // [COMPILA] double cumple Comparable ✓
    {
        std::vector<double> v{3.0, 1.0, 4.1, 1.5, 9.2};
        std::cout << "[OK] max<double>: " << core_numeric::max(v) << "\n";
    }

    // [COMPILA] int cumple Comparable ✓
    {
        std::vector<int> v{7, 3, 9, 1};
        std::cout << "[OK] max<int>: " << core_numeric::max(v) << "\n";
    }

    // [COMPILA] Esfera cumple Comparable ✓
    {
        std::vector<Esfera> v{{2.0}, {5.0}, {1.0}};
        std::cout << "[OK] max<Esfera>: " << core_numeric::max(v) << "\n";
    }

    /*
    // [NO COMPILA] std::string está bloqueada explícitamente en max():
    // requires (!std::same_as<typename C::value_type, std::string>)
    std::vector<std::string> v{"a", "b", "c"};
    core_numeric::max(v);
    // ERROR: constraint not satisfied
    // Falla: !std::same_as<std::string, std::string>  →  !true  →  false
    // std::string está explícitamente excluida del max() numérico.
    */

    /*
    // [NO COMPILA] SoloSumable NO tiene operator< ni operator>:
    std::vector<SoloSumable> v{{10}, {5}, {20}};
    core_numeric::max(v);
    // ERROR: constraint not satisfied: Comparable<SoloSumable>
    // Falla: { a < b } -> std::same_as<bool>  no está definido.
    */

    // =========================================================================
    // TEST 5: transform_reduce()
    // =========================================================================
    std::cout << "\n--- TEST 5: transform_reduce() ---\n";

    // [COMPILA] double → double, cumple Addable y Printable ✓
    {
        std::vector<double> v{1.0, 2.0, 3.0};
        // f(x) = x * 2 → 2 + 4 + 6 = 12
        auto r = core_numeric::transform_reduce(v, [](double x) { return x * 2.0; });
        std::cout << "[OK] transform_reduce<double>: " << r << "\n";
    }

    // [COMPILA] int → int, cumple Addable ✓
    {
        std::vector<int> v{1, 2, 3, 4};
        // f(x) = x + 1 → 2 + 3 + 4 + 5 = 14
        auto r = core_numeric::transform_reduce(v, [](int x) { return x + 1; });
        std::cout << "[OK] transform_reduce<int>: " << r << "\n";
    }

    /*
    // [NO COMPILA] Si la función devuelve un tipo NO Addable:
    std::vector<int> v{1, 2, 3};
    struct NoAddable { int x; };
    auto r = core_numeric::transform_reduce(v, [](int x) -> NoAddable { return {x}; });
    // ERROR: static_assert failed: "transform_reduce: el resultado debe ser Addable."
    // Falla: NoAddable no define operator+.
    */

    // =========================================================================
    // TEST 6: Variadic templates
    // =========================================================================
    std::cout << "\n--- TEST 6: Variadic templates ---\n";

    // [COMPILA] int y double son Addable ✓
    std::cout << "[OK] sum_variadic(1,2,33,4):     " << core_numeric::sum_variadic(1, 2, 33, 4) << "\n";
    std::cout << "[OK] mean_variadic(1.0,2.0,3.0): " << core_numeric::mean_variadic(1.0, 2.0, 3.0) << "\n";
    std::cout << "[OK] variance_variadic(2,4,4,4,5,5,7,9): "
              << core_numeric::variance_variadic(2,4,4,4,5,5,7,9) << "\n";
    std::cout << "[OK] max_variadic(3,1,4,1,5,9,2): "
              << core_numeric::max_variadic(3,1,4,1,5,9,2) << "\n";

    /*
    // [NO COMPILA] std::string no cumple Addable (nuestro concept):
    core_numeric::sum_variadic(std::string("a"), std::string("b"));
    // ERROR: constraint not satisfied: Addable<std::string>
    // Aunque string tiene operator+, la restricción también exige
    // que el resultado sea same_as<T>. Aquí sí lo sería... pero
    // mean_variadic lo rechaza porque no puede dividir string/size_t.
    // Para sum_variadic específicamente podría compilar con strings,
    // por eso la mejor demo de fallo variadic es con mean o variance:
    core_numeric::mean_variadic(std::string("a"), std::string("b"));
    // ERROR: sum_val / sizeof...(args) → string / size_t no está definido.
    */

    /*
    // [NO COMPILA] max_variadic requiere Comparable:
    core_numeric::max_variadic(SoloSumable{1}, SoloSumable{2});
    // ERROR: constraint not satisfied: Comparable<SoloSumable>
    // Falla: SoloSumable no define operator< ni operator>.
    */

    // =========================================================================
    // TEST 7: Iterable con contenedores distintos a vector
    // =========================================================================
    std::cout << "\n--- TEST 7: Otros contenedores Iterable ---\n";

    // [COMPILA] std::list también es Iterable ✓
    {
        std::list<double> lst{1.0, 2.0, 3.0};
        std::cout << "[OK] sum<list<double>>: " << core_numeric::sum(lst) << "\n";
    }

    /*
    // [NO COMPILA] int puro NO es Iterable:
    int x = 5;
    core_numeric::sum(x);
    // ERROR: constraint not satisfied: Iterable<int>
    // Falla: std::begin(x) y std::end(x) no están definidos para int.
    */

    std::cout << "\n=== Fin de test.cpp ===\n";
    return 0;
}
