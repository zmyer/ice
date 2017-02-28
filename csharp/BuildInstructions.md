# Building Ice for .NET

This page describes how to build and install Ice for .NET from source code using
Visual Studio. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Build Requirements

### Operating Systems and Compilers

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][3], you must install
version 4.3.6 or greater to build Ice.

## Compiling Ice for .NET with Visual Studio

### Preparing to Build

The build system requires the `slice2cs` compiler from Ice for C++. If you have
not built Ice for C++ in this source distribution, refer to [C++ build instructions](../cpp/BuildInstructionsWindows.md).

### Building Ice for .NET

Open a Visual Studio command prompt and change to the `csharp` subdirectory:

    cd csharp

To build the Ice assemblies, services and tests, run

    MSBuild msbuild\ice.proj

It is also possible to build the test suite using the binary Nuget packages, use:

    MSbuild msbuild\ice.proj /p:UseBinDist=yes

Upon completion, the Ice assemblies are placed in the `Assemblies` subdirectory.

## Running the .NET Tests

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    > pip install passlib

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:

    > python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Targeting Managed Code

Ice invokes unmanaged code to implement the following features:

- Protocol compression
- Signal processing in the Ice.Application class

if you do not require these features and prefer that the Ice run time use only 
managed code, you can build using the `Debug-Managed` or `Release-Manage` 
configurations.

    MSBuild msbuild\ice.proj /p:Configuration=Release-Managed

## Nuget packages

To create a Nuget package for the distribution use the following command:

    MSbuild msbuild\ice.proj /t:NugetPack

This will create `zeroc.ice.net\zeroc.ice.net.nupkg`.

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: https://github.com/zeroc-ice/ice-builder-visualstudio
