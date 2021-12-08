# Build Instructions
This document contains the instructions for building this repository on Linux and Windows.

This repository contains *Vulkan* development tools and layers,
supplementing the loader and validation layer core components found at https://github.com/KhronosGroup.

## System Requirements

### Windows System Requirements

Windows 7+ with additional required software packages:

- Microsoft Visual Studio 2015 Professional or 2017 Professional.  Note: it is possible that lesser/older versions may work, but not guaranteed.
- [CMake 3.14.0](https://github.com/Kitware/CMake/releases/download/v3.14.0/cmake-3.14.0-win64-x64.zip) is recommended.
  - Tell the installer to "Add CMake to the system `PATH`" environment variable.
- Python 3 (from https://www.python.org/downloads).  Notes:
  - Select to install the optional sub-package to add Python to the system `PATH` environment variable.
  - Need python3.3 or later to get the Windows `py.exe` launcher that is used to get `python3` rather than `python2` if both are installed on Windows
- Git (from http://git-scm.com/download/win).
  - Tell the installer to allow it to be used for "Developer Prompt" as well as "Git Bash".
  - Tell the installer to treat line endings "as is" (i.e. both DOS and Unix-style line endings).

Optional software packages:

- *Cygwin* (from https://www.cygwin.com/).
  - *Cygwin* provides some Linux-like tools, which are valuable for obtaining the source code, and running CMake.
    Especially valuable are the *BASH* shell and git packages.
  - If you do not wish to use *Cygwin*, there are other shells and environments that can be used.
    You can also use a Git package that does not come from *Cygwin*.

### Ubuntu System Requirements

Ubuntu 18.04 LTS and 20.04 have been tested with this repo.

[CMake 3.14.0](https://github.com/Kitware/CMake/releases/download/v3.14.0/cmake-3.14.0-Linux-x86_64.tar.gz) is recommended.

```
# Dependencies from included submodule components
sudo apt-get install git build-essential bison libx11-xcb-dev libxkbcommon-dev libwayland-dev libxrandr-dev libxcb-randr0-dev

# Additional dependencies for this repo:
sudo apt-get install wget autotools-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-ewmh-dev

# If performing 32-bit builds, you will also need:
sudo apt-get install libc6-dev-i386 g++-multilib
```

### Fedora Core System Requirements

Fedora Core 28 and 29 were tested with this repo.

[CMake 3.14.0](https://github.com/Kitware/CMake/releases/download/v3.14.0/cmake-3.14.0-Linux-x86_64.tar.gz) is recommended.

Additional package dependencies include:

```
# Dependencies from included submodule components
sudo dnf install git @development-tools glm-devel \
                 libpng-devel wayland-devel libpciaccess-devel \
                 libX11-devel libXpresent libxcb xcb-util libxcb-devel libXrandr-devel \
                 xcb-util-keysyms-devel xcb-util-wm-devel
```

Optional software packages:

### macOS System Requirements

macOS 10.11.4 have been tested with this repo.

- [CMake 3.14.0](https://github.com/Kitware/CMake/releases/download/v3.14.0/cmake-3.14.0-Darwin-x86_64.tar.gz) is recommended.

Setup Homebrew and components
- Follow instructions on [brew.sh](http://brew.sh) to get homebrew installed.
```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

- Ensure Homebrew is at the beginning of your PATH:
```
export PATH=/usr/local/bin:$PATH
```

- Add packages with the following (may need refinement)
```
brew install python python3 git
```

### Android Additional System Requirements
Install the required tools for Linux and Windows covered above, then add the
following.

#### Android Studio
- Install 2.1 or later version of [Android Studio](http://tools.android.com/download/studio/stable)
- From the "Welcome to Android Studio" splash screen, add the following components using Configure > SDK Manager:
  - SDK Tools > Android NDK

#### Add NDK to path

On Windows:
```
set PATH=%LOCALAPPDATA%\Android\sdk\ndk-bundle;%PATH%
```
On Linux:
```
export PATH=$HOME/Android/Sdk/ndk-bundle:$PATH
```
On macOS:
```
export PATH=$HOME/Library/Android/sdk/ndk-bundle:$PATH
```

## Getting Started Build Instructions

### 64-bit Windows Build 
```
    git clone git@github.com:LunarG/VulkanProfiles.git
    cd VulkanProfiles
    mkdir build
    cd build
    cmake -A x64 ..
    cmake --build . --parallel
```

### 32-bit Windows Build 
```
    git clone git@github.com:LunarG/VulkanProfiles.git
    cd VulkanProfiles
    mkdir build
    cd build
    cmake -A Win32 ..
    cmake --build . --parallel
```

### Windows Unit Tests

```
ctest -C Debug --output-on-failure --parallel 16
ctest -C Release  --output-on-failure --parallel 16
```

### Linux and macOS Build
```
    git clone git@github.com:LunarG/VulkanProfiles.git
    cd VulkanTools
    mkdir build
    cd build
    cmake ..
    cmake --build . --parallel
```

### Linux and macOS Unit Tests

```
ctest --parallel 8 --output-on-failure
```

### Android Build
Use the following to ensure the Android build works.

#### Android Build from Windows
From Developer Command Prompt for VS2015:
```
cd build-android
update_external_sources_android.bat
android-generate.bat
ndk-build
```

#### Android Build from Linux
From your terminal:
```
cd build-android
./update_external_sources_android.sh
./android-generate.sh
ndk-build -j $(nproc)
```

#### Android Build from macOS 
From your terminal:
```
cd build-android
./update_external_sources_android.sh
./android-generate.sh
ndk-build -j $(sysctl -n hw.ncpu)
```

### Repository Dependencies
This repository attempts to resolve some of its dependencies by using
components found from the following places, in this order:

1. CMake or Environment variable overrides (e.g., -DVULKAN_HEADERS_INSTALL_DIR)
1. LunarG Vulkan SDK, located by the `VULKAN_SDK` environment variable
1. System-installed packages, mostly applicable on Linux

Dependencies that cannot be resolved by the SDK or installed packages must be
resolved with the "install directory" override and are listed below. The
"install directory" override can also be used to force the use of a specific
version of that dependency.

#### Vulkan-Headers

This repository has a required dependency on the
[Vulkan Headers repository](https://github.com/KhronosGroup/Vulkan-Headers).
You must clone the headers repository and build its `install` target before
building this repository. The Vulkan-Headers repository is required because it
contains the Vulkan API definition files (registry) that are required to build
the validation layers. You must also take note of the headers' install
directory and pass it on the CMake command line for building this repository,
as described below.

#### Vulkan-Loader

The tools in this repository depend on the Vulkan loader.

A loader can be used from an installed LunarG SDK, an installed Linux package,
or from a driver installation on Windows.

If a loader is not available from any of these methods and/or it is important
to use a loader built from a repository, then you must build the
[Vulkan-Loader repository](https://github.com/KhronosGroup/Vulkan-Loader.git)
with its install target. Take note of its install directory location and pass
it on the CMake command line for building this repository, as described below.

#### Vulkan-ValidationLayers
The tools in this repository depend on the Vulkan validation layers.

Validation layers can be used from an installed LunarG SDK, an installed Linux
package, or from a driver installation on Windows.

If the validation layers are not available from any of these methods and/or
it is important to use the validation layers built from a repository, then you
must build the
[Vulkan-ValidationLayers repository](https://github.com/KhronosGroup/Vulkan-ValidationLayers.git)
with its install target. Take note of its install directory location and pass
it on the CMake command line for building this repository, as described below.
