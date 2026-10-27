## Compilar y ejecutar

### Requisitos

- CMake 3.24 o superior.
- Un compilador compatible con C++20.
- Git, porque CMake descarga SDL2, SDL2pp y GoogleTest durante la
  configuración.

El cliente y el editor utilizan SDL2 y SDL2pp. En Linux también pueden ser
necesarias las dependencias de desarrollo indicadas en
[`CMakeLists.txt`](CMakeLists.txt).

### Compilación general con CMake

Desde la raíz del repositorio, configurar el proyecto:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Luego compilar todos los targets habilitados:

```sh
cmake --build build
```

La configuración por defecto habilita el cliente, el editor y los tests. El
servidor todavía no tiene un ejecutable integrado, por lo que permanece
deshabilitado hasta completar la etapa de integración. Para una compilación
más rápida se pueden deshabilitar los componentes que no se necesitan:

```sh
cmake -S . -B build \
  -DTALLER_CLIENT=ON \
  -DTALLER_SERVER=OFF \
  -DTALLER_EDITOR=OFF \
  -DTALLER_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Para una compilación optimizada, reemplazar `Debug` por `Release`:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

#### Editor

Si el editor fue habilitado durante la configuración, ejecutarlo con:

```sh
./build/taller_editor
```

#### Tests

Los tests usan [GoogleTest](https://github.com/google/googletest), una
biblioteca estándar para C++ que ya está integrada en el proyecto mediante
CMake.

Los tests se ejecutan con:

```sh
./build/taller_tests
```

También se pueden ejecutar mediante:

```sh
make run-tests
```