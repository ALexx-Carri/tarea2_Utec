// =============================================================================
// main.cpp — Demostración de uso válido de CoreNumeric
// =============================================================================
// Aquí se muestran ÚNICAMENTE llamadas que deben compilar correctamente.
// Los casos que NO compilan se documentan en test.cpp.
// =============================================================================

#include "core_numeric.hpp"
#include <iostream>
#include <vector>
#include <array>

// =============================================================================
// Clases personalizadas para demostrar que los concepts funcionan
// con tipos definidos por el usuario, no solo con primitivos.
// =============================================================================

// Clase Coordenada2D: punto en el plano con soporte para suma, división y comparación.
// Al definir los operadores requeridos, cumple Addable, Divisible y Comparable.
struct Coordenada2D {
    double x, y;

    Coordenada2D() : x(0.0), y(0.0) {}
    Coordenada2D(double x, double y) : x(x), y(y) {}

    // Necesario para Addable
    Coordenada2D operator+(const Coordenada2D& o) const {
        return {x + o.x, y + o.y};
    }

    // Necesario para Divisible
    Coordenada2D operator/(std::size_t n) const {
        return {x / static_cast<double>(n), y / static_cast<double>(n)};
    }

    // Necesario para Comparable (concept propio)
    // Comparamos por magnitud (distancia al origen)
    bool operator<(const Coordenada2D& o) const {
        return (x*x + y*y) < (o.x*o.x + o.y*o.y);
    }
    bool operator>(const Coordenada2D& o) const {
        return (x*x + y*y) > (o.x*o.x + o.y*o.y);
    }

    // Necesario para Printable (concept propio adicional)
    friend std::ostream& operator<<(std::ostream& os, const Coordenada2D& c) {
        return os << "(" << c.x << ", " << c.y << ")";
    }
};

// Clase Vector3D: vector en espacio 3D con los mismos operadores.
struct Vector3D {
    double x, y, z;

    Vector3D() : x(0.0), y(0.0), z(0.0) {}
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3D operator+(const Vector3D& o) const {
        return {x + o.x, y + o.y, z + o.z};
    }

    Vector3D operator/(std::size_t n) const {
        return {x / static_cast<double>(n),
                y / static_cast<double>(n),
                z / static_cast<double>(n)};
    }

    // Comparación por magnitud euclidiana
    double magnitude() const { return x*x + y*y + z*z; }
    bool operator<(const Vector3D& o) const { return magnitude() < o.magnitude(); }
    bool operator>(const Vector3D& o) const { return magnitude() > o.magnitude(); }

    friend std::ostream& operator<<(std::ostream& os, const Vector3D& v) {
        return os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    }
};

int main() {
    std::cout << "=== CoreNumeric: Demostración de uso válido ===\n\n";

    // -------------------------------------------------------------------------
    // 1. sum() con tipos primitivos
    // -------------------------------------------------------------------------
    std::cout << "--- sum() ---\n";
    std::vector<int>    vi{1, 2, 3, 4, 5};
    std::vector<double> vd{1.1, 2.2, 3.3};

    std::cout << "sum(int):    " << core_numeric::sum(vi) << "\n";   // 15
    std::cout << "sum(double): " << core_numeric::sum(vd) << "\n\n"; // 6.6

    // -------------------------------------------------------------------------
    // 2. mean() con tipos primitivos
    // -------------------------------------------------------------------------
    std::cout << "--- mean() ---\n";
    std::cout << "mean(int):    " << core_numeric::mean(vi) << "\n";   // 3
    std::cout << "mean(double): " << core_numeric::mean(vd) << "\n\n"; // 2.2

    // -------------------------------------------------------------------------
    // 3. variance()
    // -------------------------------------------------------------------------
    std::cout << "--- variance() ---\n";
    std::vector<double> vvar{1.0, 2.0, 3.0, 4.0, 5.0};
    std::cout << "variance(double): " << core_numeric::variance(vvar) << "\n\n"; // 2.0

    // -------------------------------------------------------------------------
    // 4. max()
    // -------------------------------------------------------------------------
    std::cout << "--- max() ---\n";
    std::vector<double> vmax{3.5, 1.0, 4.2, 2.8};
    std::cout << "max(double): " << core_numeric::max(vmax) << "\n\n"; // 4.2

    // -------------------------------------------------------------------------
    // 5. transform_reduce() — También demuestra el concept Printable
    //    porque double es Printable, se imprimirá el resultado internamente.
    // -------------------------------------------------------------------------
    std::cout << "--- transform_reduce() ---\n";
    std::vector<double> vtr{1.0, 2.0, 3.0};
    // Aplica f(x) = x² a cada elemento y luego suma: 1 + 4 + 9 = 14
    auto tr_result = core_numeric::transform_reduce(vtr, [](double x) {
        return x * x;
    });
    std::cout << "transform_reduce resultado capturado: " << tr_result << "\n\n";

    // -------------------------------------------------------------------------
    // 6. Variadic templates
    // -------------------------------------------------------------------------
    std::cout << "--- Variadic templates ---\n";
    auto s1 = core_numeric::sum_variadic(1, 2, 33, 4);
    auto s2 = core_numeric::mean_variadic(0.1, 2.0, 3.0, 4.0);
    auto s3 = core_numeric::variance_variadic(1.0, 2.0, 3.0, 4.0);
    auto s4 = core_numeric::max_variadic(1, 2, 7, 3, 4);

    std::cout << "sum_variadic(1,2,33,4):          " << s1 << "\n"; // 40
    std::cout << "mean_variadic(0.1,2,3,4):        " << s2 << "\n"; // 2.275
    std::cout << "variance_variadic(1,2,3,4):      " << s3 << "\n"; // 1.25
    std::cout << "max_variadic(1,2,7,3,4):         " << s4 << "\n\n"; // 7

    // -------------------------------------------------------------------------
    // 7. Clases personalizadas — Coordenada2D
    // -------------------------------------------------------------------------
    std::cout << "--- Clases personalizadas: Coordenada2D ---\n";
    std::vector<Coordenada2D> coords{
        {1.0, 2.0}, {3.0, 4.0}, {0.5, 1.5}
    };
    std::cout << "sum(Coordenada2D):      " << core_numeric::sum(coords)      << "\n";
    std::cout << "mean(Coordenada2D):     " << core_numeric::mean(coords)     << "\n";
    std::cout << "max(Coordenada2D):      " << core_numeric::max(coords)      << "\n\n";

    // -------------------------------------------------------------------------
    // 8. Clases personalizadas — Vector3D
    // -------------------------------------------------------------------------
    std::cout << "--- Clases personalizadas: Vector3D ---\n";
    std::vector<Vector3D> vecs{
        {1.0, 0.0, 0.0}, {0.0, 2.0, 0.0}, {0.0, 0.0, 3.0}
    };
    std::cout << "sum(Vector3D):  " << core_numeric::sum(vecs)  << "\n";
    std::cout << "mean(Vector3D): " << core_numeric::mean(vecs) << "\n";
    std::cout << "max(Vector3D):  " << core_numeric::max(vecs)  << "\n\n";

    // -------------------------------------------------------------------------
    // 9. std::array — demuestra que Iterable funciona con más que vector
    // -------------------------------------------------------------------------
    std::cout << "--- std::array también es Iterable ---\n";
    std::array<double, 4> arr{10.0, 20.0, 5.0, 15.0};
    std::cout << "sum(array<double>):  " << core_numeric::sum(arr)  << "\n"; // 50
    std::cout << "max(array<double>):  " << core_numeric::max(arr)  << "\n"; // 20
    std::cout << "mean(array<double>): " << core_numeric::mean(arr) << "\n"; // 12.5

    std::cout << "\n=== Fin de main.cpp ===\n";
    return 0;
}
