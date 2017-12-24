# Building Ice for Java

This page describes how to build and install Ice for Java from source code. If
you prefer, you can also download [binary distributions][1] for the supported
platforms.

* [Build Requirements](#build-requirements)
  * [Operating Systems](#operating-systems)
  * [Slice to Java Compiler](#slice-to-java-compiler)
  * [Java Version](#java-version)
  * [Gradle](#gradle)
  * [Bzip2 Compression](#bzip2-compression)
* [Compiling Ice for Java](#compiling-ice-for-java)
  * [Preparing to Build](#preparing-to-build)
  * [Building Ice for Java](#building-ice-for-java-1)
* [Installing Ice for Java](#installing-ice-for-java)
* [Running the Java Tests](#running-the-java-tests)

## Build Requirements

### Operating Systems

Ice for Java is expected to build and run properly on Windows, macOS, and any
recent Linux distribution for x86 and x86_64, and was extensively tested using
the operating systems and compiler versions listed for our [supported
platforms][2]. Due to the portability of Java, it is very likely that it will
also work on other platforms for which a suitable Java implementation is
available.

### Slice to Java Compiler

You will need the Slice to Java compiler. ZeroC provides compiler binaries
for our supported platforms. For other platforms, you will have to either port
Ice for C++ (which contains the Slice to Java compiler), or you will have to
translate your Slice files to Java on a supported platform and then copy the
generated Java files to your target platform.

### Java Version

Ice for Java requires J2SE 1.8.0 or later.

Make sure that the `javac` and `java` commands are present in your PATH.

### Gradle

Ice for Java uses the [Gradle][3] build system, and includes the Gradle wrapper
version 2.4 in the distribution. You cannot build the Ice for Java source
distribution without an Internet connection. Gradle will download all required
packages automatically from ZeroC's Maven repository located at

    https://repo.zeroc.com/nexus/content/repositories/thirdparty

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with [Apache Commons Compress][4].

The Maven package id for the commons-compress JAR file is as follows:

    groupId=org.apache.commons, version=1.14, artifactId=commons-compress

The demos and tests are automatically setup to enable protocol compression by
adding the commons-compress JAR to the manifest class path. For your own
applications you must add the commons-compress JAR to the application CLASSPATH
to enable protocol compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

## Compiling Ice for Java

### Preparing to Build

The build system requires the Slice to Java compiler from Ice for C++. If you
have not built Ice for C++ in this source distribution, you must set the
`ICE_HOME` environment variable with the path name of your Ice installation. For
example, on Unix:

    $ export ICE_HOME=/opt/Ice-3.7.1 (For local build)
    $ export ICE_HOME=/usr (For RPM installation)

On Windows:

    > set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.1 (MSI installation)

On Windows if you are using Ice for C++ from a source distribution, you must set
the `CPP_PLATFORM` and `CPP_CONFIGURATION` environment variables to match the
platform and configuration used in your C++ build:

    > set CPP_PLATFORM=x64
    > set CPP_CONFIGURATION=Debug

The supported values for `CPP_PLATFORM` are `Win32` and `x64` and the supported
values for `CPP_CONFIGURATION` are `Debug` and `Release`.

Before building Ice for Java, review the settings in the file
`gradle.properties` and edit as necessary.

### Building Ice for Java

To build Ice, all services, and tests, run

    > gradlew build

Upon completion, the Ice JAR and POM files are placed in the `lib` subdirectory.

If at any time you wish to discard the current build and start a new one, use
these commands:

    > gradlew clean
    > gradlew build

## Installing Ice for Java

To install Ice for Java in the directory specified by the `prefix` variable in
`gradle.properties` run the following command:

    > gradlew install

The installation installs the following JAR files to `<prefix>/lib`.

    glacier2-compat-3.7.1.jar
    ice-compat-3.7.1.jar
    icebox-compat-3.7.1.jar
    icebt-compat-3.7.1.jar
    icediscovery-compat-3.7.1.jar
    icegrid-compat-3.7.1.jar
    icelocatordiscovery-compat-3.7.1.jar
    icepatch2-compat-3.7.1.jar
    icestorm-compat-3.7.1.jar

POM files are also installed for ease of deployment to a Maven-based
distribution system.

## Running the Java Tests

Some of the Ice for Java tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation. On Unix:

    $ export ICE_HOME=/opt/Ice-3.7.1 (For local build)
    $ export ICE_HOME=/usr (For RPM installation)

On Windows:

    > set ICE_HOME=C:\Program Files\ZeroC\Ice-3.7.1

Python is required to run the test suite. To run the tests, open a command
window and change to the top-level directory. At the command prompt, execute:

    > python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[3]: https://gradle.org
[4]: https://commons.apache.org/proper/commons-compress/
