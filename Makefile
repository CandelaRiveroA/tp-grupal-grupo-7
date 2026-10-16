.PHONY: all test clean editor client common server build

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build  build/ $(EXTRA_COMPILE)

run-tests:
	mkdir -p build-tests
	cmake -S . -B ./build-tests -DCMAKE_BUILD_TYPE=Debug \
		-DTALLER_CLIENT=OFF -DTALLER_SERVER=OFF \
		-DTALLER_EDITOR=OFF -DTALLER_TESTS=ON
	cmake --build build-tests
	./build-tests/taller_tests

all: clean run-tests

clean:
	rm -Rf build/ build-tests/
