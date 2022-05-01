# Uran ![CI](https://github.com/cfnptr/uran/actions/workflows/cmake.yml/badge.svg)

Work in Progress!

## Features
* TODO

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 and C++11 compiler
* Objective-C compiler (macOS only)
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.16+](https://cmake.org/)
* [Vulkan SDK 1.2+](https://vulkan.lunarg.com/)
* [X11](https://www.x.org/) (Linux only)
* [OpenSSL 1.1.1+](https://openssl.org/)

### X11 installation

* Ubuntu: sudo apt install xorg-dev

### OpenSSL installation

* Ubuntu: sudo apt install libssl-dev
* MacOS: [brew](https://brew.sh/) install openssl
* Windows: [choco](https://chocolatey.org/) install openssl

### CMake options

| Name                   | Description                        | Default value   |
|------------------------|------------------------------------|-----------------|
| URAN_MACOS_BUNDLE      | Build as macOS bundle              | `OFF`           |

### CMake variables
| Name                   | Description                        | Default value   |
|------------------------|------------------------------------|-----------------|
| URAN_PROGRAM_NAME      | Custom program executable name     | `uran-editor`   |
| URAN_PROGRAM_SOURCES   | Custom program source files        | `source/main.c` |
| URAN_PROGRAM_INCLUDES  | Custom program include directories |                 |
| URAN_PROGRAM_LIBRARIES | Custom program link libraries      |                 |
| URAN_MACOS_INFO_PLIST  | Custom macOS bundle Info.plist     |                 |
| URAN_MACOS_RESOURCES   | Custom macOS bundle resources      |                 |

## Cloning

```
git clone --recursive https://github.com/cfnptr/uran
```

TODO: tutorial on how to make pack files.

## Third-party

* [conf](https://github.com/cfnptr/conf/) (Apache-2.0 License)
* [FreeType](https://www.freetype.org/) (FreeType License)
* [WebP](https://developers.google.com/speed/webp) (BSD License)
* [logy](https://github.com/cfnptr/logy/) (Apache-2.0 License)
* [mpgx](https://github.com/cfnptr/mpgx/) (Apache-2.0 License)
* [mpnw](https://github.com/cfnptr/mpnw/) (Apache-2.0 License)
* [pack](https://github.com/cfnptr/pack/) (Apache-2.0 License)
