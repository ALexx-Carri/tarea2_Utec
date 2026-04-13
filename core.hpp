//Concepts: restringen qué tipos pueden usarse en cada función
//Variadic templates: funciones que aceptan cualquier cantidad de args
//Fold expressions: operaciones compactas sobre paquetes de argumentos
//if constexpr: bifurcación de lógica en tiempo de compilación
#include <concepts>
#include <iterator>
#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <limits>
namespace core_numeric {
// P1: DEFINICIÓN DE CONCEPTS
// Un "concept" es una condición sobre un tipo que se evalúa en tiempo de compilación. Si el tipo no la cumple, el compilador lanza un error claro en lugar de un solo "template error".
// Concept: Es iterable, sumable, divisible, comparable.
// Un tipo C es Iterable debido a que se le puede aplicar std::begin() y std::end().
// Esto aplica al vector, array, list, set, string, arreglos.
template <typename C>
concept Iterable = requires(C c) {
    std::begin(c);
    std::end(c);
};
// Concept: Addable
// Un tipo T es Addable si la expresión (a + b) existe y devuelve exactamente T.
// Esto descarta, por ejemplo, const char*, donde + no está definido.
template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};
// Concept: Divisible
// Un tipo T, es divisible si se puede dividir por un size_t y el resultado es del mismo tipo. Necesario para calcular promedios.
// Divisible: T puede dividirse por size_t.
// Usamos "convertible_to" en lugar de "same_as" para tipos como int,size_t devuelve size_t (o un tipo implícitamente convertible) y para clases personalizadas el operador puede retornar el mismo tipo T.
// Esto captura los dos casos de forma correcta.
template <typename T>
concept Divisible = requires(T a, std::size_t n) {
    { a / n } -> std::convertible_to<T>;
};
// Concept: Comparable 
// Un tipo T va a ser comparable si soporta los operadores < y >.
// Se aplica en: max(), max_variadic(), variance() (si o si necesita orden implícito).
// Esto descarta string de operaciones numéricas máximas.
// -----------------------------------------------------------------------------
template <typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
};
// Concept: Printable
// Un tipo T es printable(se puede imprimir) si se puede enviarse a ostream con operator<<.
// Lo usamos para hacer que "transform_reduce" imprima los resultados opcionales y también para documentar resultados en el ejemplos de test.
// -----------------------------------------------------------------------------
template <typename T>
concept Printable = requires(std::ostream& os, T a) {
    { os << a } -> std::same_as<std::ostream&>;
};
// SECCIÓN 2: ALGORITMOS SOBRE CONTENEDORES
// sum: Suma todos los elementos de un contenedor.
// Restricciones:
//   - C debe ser Iterable (para poder recorrerlo)
//   - C::value_type debe ser Addable (para poder sumar sus elementos)
// Uso de "if constexpr":
//   Diferenciamos el comportamiento según si el tipo de elemento es una integral
//   (int, long, etc.) o de punto flotante (float, double). En tipos enteros
//   el resultado se acumula con precisión exacta; en flotantes usamos la misma
//   lógica pero el compilador sabrá que puede aplicar optimizaciones.
// -----------------------------------------------------------------------------
template <Iterable C>
requires Addable<typename C::value_type>
auto sum(const C& container) {
    using T = typename C::value_type;
    // T{} inicializa a cero para tipos numéricos (int→0 y double→0.0) y llamara al constructor por defecto en clases personalizadas.
    T result{};
    for (const auto& value : container) {
        if constexpr (std::is_integral_v<T>) {
            // Para enteros: sumar directamente sin la conversión intermediaria a double, para mantener la precisión exacta.
            result = result + value;
        } else {
            // Para flotantes: misma operación pero el compilador puede aplicar optimizaciones específicas para tipos de punto flotante.
            result = result + value;
        }
    }
    return result;
}
// mean: Calcula el promedio del contenedor.
// Reutiliza sum() y luego divide entre el número de elementos.
// Requiere además divisible, porque no todos los tipos Addable soportan división por ejemplo un tipo que solo define "operator+".
template <Iterable C>
requires Addable<typename C::value_type> && Divisible<typename C::value_type>
auto mean(const C& container) {
    const std::size_t n = std::distance(std::begin(container), std::end(container));
    if (n == 0)
        throw std::invalid_argument("mean: el contenedor está vacio.");
    return sum(container) / n;
}
// variance: Calcula la varianza poblacional del contenedor.
// Fórmula: var(varianza) = mean( (xi - u)**2 ) donde u es la media del contenedor.
// Reutiliza mean(). Requiere Iterable, Addable, Divisible y Comparable.
// La restricción Comparable la añadimos nosotros con el concept propio, pues la varianza solo tiene sentido en tipos numéricos ordenables.
// Uso de if constexpr:
//   Para los integrales, el cuadrado de la diferencia puede llegar a producir un overflow muy fácilmente, así que lo convertimos a un double internamente.
//   Para flotantes, operamos directamente.
template <Iterable C>
requires Addable<typename C::value_type>
      && Divisible<typename C::value_type>
      && Comparable<typename C::value_type>
auto variance(const C& container) {
    using T = typename C::value_type;

    const std::size_t n = std::distance(std::begin(container), std::end(container));
    if (n == 0)
        throw std::invalid_argument("variance: el contenedor está vacio.");

    const T mu = mean(container);

    if constexpr (std::is_integral_v<T>) {
        // Para enteros: trabajamos en double para no perder precisión y no aproximar al elevar al cuadrado diferencias negativas.
        double acc = 0.0;
        for (const auto& x : container) {
            double diff = static_cast<double>(x) - static_cast<double>(mu);
            acc += diff * diff;
        }
        return acc / static_cast<double>(n);
    } else {
        // Para flotantes: operamos directamente en T
        T acc{};
        for (const auto& x : container) {
            T diff = x + (mu * static_cast<T>(-1));  // Evitamos restar directamente para no perder precisión en flotantes, sumamos el negativo de la media.
            acc = acc + (diff * diff);
        }
        return acc / n;
    }
}
// max: Devuelve el elemento máximo del contenedor.
// Requiere Iterable y Comparable
//string NO cumple Comparable tal como lo definimos en relación a tipos
// numéricos, aunque técnicamente tenga operator<. Lo excluimos de forma explícita añadiendo la restricción adicional de que no sea string.
// (En test.cpp aparece.)
template <Iterable C>
requires Comparable<typename C::value_type>
      && (!std::same_as<typename C::value_type, std::string>)
auto max(const C& container) {
    auto it = std::begin(container);
    auto end = std::end(container);

    if (it == end)
        throw std::invalid_argument("max: el contenedor está vacio.");

    auto result = *it;
    ++it;

    for (; it != end; ++it) {
        if constexpr (std::is_integral_v<typename C::value_type>) {
            // Para enteros: comparación directa
            if (*it > result) result = *it;
        } else {
            // Para flotantes: comparación en tipo T para aprovechar optimizaciones específicas de punto flotante.
            if (*it > result) result = *it;
        }
    }
    return result;
}
// transform_reduce: Aplica una función a cada elemento y reduce con sum().
// El parámetro Func es un template (lambda, puntero a función).
// Requiere que el resultado de Func(elem) sea Addable.
// Usamos Printable para imprimir un resumen si el tipo resultante lo permite.
// Esto demuestra el concept de Printable.
template <Iterable C, typename Func>
auto transform_reduce(const C& container, Func f) {
    using InputType  = typename C::value_type;
    using OutputType = decltype(f(std::declval<InputType>()));
    static_assert(Addable<OutputType>,
        "transform_reduce: el resultado de la funcion debe ser Addable.");
    OutputType result{};
    for (const auto& elem : container) {
        result = result + f(elem);
    }

    // Si el tipo de salida es Printable, mostramos el resultado automáticamente
    if constexpr (Printable<OutputType>) {
        std::cout << "[transform_reduce] resultado = " << result << "\n";
    }

    return result;
}
// SECCIÓN 3: VARIADIC TEMPLATES + FOLD EXPRESSIONS
// Las variadic templates permiten recibir cualquier cantidad de argumentos de tipos posiblemente distintos. 
// Permitiendo que las fold expressions aplican un operador binario sobre todos los argumentos del paquete de forma compacta y eficiente, sin necesidad de escribir bucles o recursión explícita. 
// sum_variadic: Suma N argumentos usando una fold expression.
// La expresión (... + args) se expande como:
//   arg0 + arg1 + arg2 + ... hasta + argN, en tiempo de compilación.
template <Addable... Args>
auto sum_variadic(Args... args) {
    return (... + args);  // fold expression izquierda sobre operator+
}
// mean_variadic: Promedio de N argumentos.
// Reutiliza sum_variadic y divide entre sizeof...(args), que es el conteo de argumentos evaluado EN TIEMPO DE COMPILACIÓN.
template <Addable... Args>
requires (sizeof...(Args) > 0)
auto mean_variadic(Args... args) {
    // auto sum_val retiene el tipo común de los argumentos (double si hay doubles)
    auto sum_val = sum_variadic(args...);
    if constexpr (std::is_integral_v<decltype(sum_val)>) {
        // Para enteros: retornamos double para no perder la parte decimal
        return static_cast<double>(sum_val) / sizeof...(args);
    } else {
        return sum_val / sizeof...(args);
    }
}
// variance_variadic: Varianza de N argumentos pasados directamente.
// Estrategia: usamos una fold expression para calcular Σ(xi - u)**2, donde u es la media calculada previamente con mean_variadic.
// La media se calcula primero con mean_variadic, y luego una lambda dentro de la fold expression se encarga de acumular la suma de las diferencias al cuadrado.
// eleva cada argumento al cuadrado de su diferencia respecto a la media.
template <Addable... Args>
requires (sizeof...(Args) > 0)
auto variance_variadic(Args... args) {
    auto mu = mean_variadic(args...);
    // Convertimos todo a double para uniformidad en la varianza
    double acc = 0.0;
    // Fold expression con una lambda que captura mu:
    // Para cada arg, calcula (arg - mu)**2 y lo acumula.
    ([&]() {
        double diff = static_cast<double>(args) - static_cast<double>(mu);
        acc += diff * diff;
    }(), ...);  // fold expression sobre el operador coma
    return acc / sizeof...(args);
}
// max_variadic: Máximo de N argumentos.
// Usamos una fold expression con un operador ternario envuelto en lambda  para comparar acumulativamente todos los argumentos.
template <typename First, typename... Rest>
requires Comparable<First> && (Comparable<Rest> && ...)
auto max_variadic(First first, Rest... rest) {
    auto result = first;
    // Fold expression: compara result con cada argumento sucesivamente
    ((result = (rest > result ? rest : result)), ...);
    return result;
}
} // namespace core_numeric