# Zguide in Modern cpp

This repo is an experiment to solidify my learning of zmq and also a place for me
to do experiment with modern cpp standard

## Prequisites

- conan2 (>=2.17): https://docs.conan.io/2/
- cmake (>=4.0.3): https://cmake.org/download/
- gcc (>=15.1.0): https://gcc.gnu.org/gcc-15/

## Conan profile

```toml
[settings]
arch=x86_64
build_type=Debug
compiler=gcc
compiler.cppstd=gnu26
compiler.libcxx=libstdc++11
compiler.version=15
os=Linux
```

## How to build

Create a build directory. In my case, I build out-of-source

```bash
cd zmq_do
cd ../ && mkdir -p build/x86/Debug
```

Before install dependencies, we need to externally set minimum cmake policy
version to be able to build zmq

```bash
export CMAKE_POLICY_VERSION_MINIMUM=4.0
```

Install dependencies with conan

```bash
cd build/x86/Debug
conan install ../../../zmq_do --output-folder . --build=missing
```

Configure using cmake and use ninja to build

```bash
cmake -GNinja ../../../zmq_do
```

Then we can build the project now

```bash
ninja
```

