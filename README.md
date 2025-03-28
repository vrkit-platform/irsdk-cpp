# Modern C++ SDK for iRacing `irsdkcpp`

A Modern C++ SDK for iRacing is finally here `irsdkcpp`.

## Table of Contents

<!-- TOC -->
* [Modern C++ SDK for iRacing `irsdkcpp`](#modern-c-sdk-for-iracing-irsdkcpp)
  * [Table of Contents](#table-of-contents)
  * [Features](#features)
  * [Install/Build](#installbuild)
    * [VCPKG](#vcpkg)
    * [Build from Source](#build-from-source)
      * [Prerequisites](#prerequisites)
      * [Dependencies](#dependencies)
      * [Configure, Build, and Install](#configure-build-and-install)
  * [Usage](#usage)
    * [Simple Example](#simple-example)
      * [Source Code `main.cpp`](#source-code-maincpp)
      * [VCPKG Config `vcpkg.json`](#vcpkg-config-vcpkgjson)
      * [CMake Config `CMakeLists.txt`](#cmake-config-cmakeliststxt)
      * [Build Simple Example](#build-simple-example)
<!-- TOC -->

## Features

- Install & Link with `vcpkg`
- Provides both `LiveClient` & `DiskClient` implementations of `Client`, which allows for a singular codebase capable of
  using either IBT data and/or Live data via shared memory (basically its all in one).
- Part of a larger effort `vrkit-platform`

## Install/Build

> NOTE: As iRacing is Windows only, as too is this library

### CMake

#### VCPKG

> NOTE: `vcpkg` registry entry is pending [PR acceptance](https://github.com/microsoft/vcpkg/pull/44447)

> NOTE: Both `static` & `dynamic` linkage is supported

Either integrate it via `vcpkg` by adding `irsdkcpp` to
your list of dependencies in `vcpkg.json` or [build from source](#build-from-source).

#### `FetchContent`

```cmake
FetchContent_Declare(
  irsdkcpp
  GIT_REPOSITORY https://github.com/vrkit-platform/irsdk-cpp.git
  GIT_TAG        origin/develop
)

FetchContent_MakeAvailable(irsdkcpp)
```


### Build from Source

#### Prerequisites

- `cmake`
- [vcpkg](https://github.com/microsoft/vcpkg)
- Visual Studio 2022 Build Tools (`msvc`)

#### Dependencies

Dependencies are all install via `vcpkg`; however if you'd prefer not to use `vcpkg`,
the required dependency list is:

```
vcpkg-cmake
vcpkg-cmake-config
fmt
magic-enum
nlohmann-json
ms-gsl
range-v3
yaml-cpp
spdlog
```

> NOTE: To generate a fresh list yourself, just run `cat <workspace-root>/vcpkg.json | jq -r '.dependencies[].name'`.
> 
> Also, `gtest` & `cli11` are only needed for development purposes & running tests 




#### Configure, Build, and Install

As long as you have the `VCPKG_ROOT` env variable set properly, the following command should properly configure your
build (This is a Debug build, but changing Debug to Release is all that's needed for a release build:

```powershell
# NOTE: In powershell, the tick(`) at the end of the line
#   indicates a line-wrap, similar to `\` in a posix shell
cmake -B build/debug -S . `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

Assuming configuration was successful, run the following to build

```powershell
cmake --build .\build\debug
```

If you'd like to manually install either to the default path `C:\Program Files\irsdkcpp` or
to `CMAKE_INSTALL_PREFIX`, which you specified in the configuration stage, run the following

```powershell
cmake --build .\build\debug --target install
```

## Usage

### Simple Example

The absolute simplest example, is to check
if the live client is available.

This will only print `true` if an iRacing
session is active, and shared memory
is available to access.

#### Source Code `main.cpp`

```c++
#include <IRacingSDK/LiveClient.h>
#include <fmt/format.h>
#include <iostream>

int main() {
  fmt::println("LiveClient isAvailable={}", 
    IRacingSDK::LiveClient::Get().isAvailable());

  return 0;
}
```

#### VCPKG Config `vcpkg.json`

```json
{
  "dependencies": [
    "fmt",
    "irsdkcpp"
  ]
}
```

#### CMake Config `CMakeLists.txt`

As mentioned above, we strongly advise that you use `vcpkg`

```cmake
FIND_PACKAGE(irsdkcpp CONFIG REQUIRED)

ADD_EXECUTABLE(simple main.cpp)

TARGET_LINK_LIBRARIES(simple PRIVATE 
  fmt::fmt 
  irsdkcpp::irsdkcpp_static)
```

#### Build Simple Example


```powershell
# Configure
cmake -B build/debug -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build without installing
cmake --build .\build\debug

# Build & Install
cmake --build .\build\debug --target install
```