# Building the Ice for Android Test Suite

This directory contains an Android Studio project for the Ice test suite. For
ease of development and testing, this project also builds a subset of the Ice
JAR files. This is not necessary for your own projects as it considerably
complicates the project configuration.

Building [Ice for Java](../BuildInstructions.md) is the only way to build
all of the Ice JAR files from source. The JAR files produced by the Ice for Java
build fully support Android. If you prefer, our [binary distributions][1]
include pre-compiled JAR files.

## Build Requirements

### Android Development Tools

Building any Ice application for Android requires Android Studio and the Android
SDK build tools. We tested with the following components:

- Android Studio 3.0
- Android SDK 24
- Android Build Tools 27.0.1

Using Ice's Java mapping with Java 8 requires at minimum API level 24:

- Android 7 (API24)

If you want to target a later version of the Android API level for the test
suite, edit `gradle.properties` and change the following variables:

```
ice_compileSdkVersion
ice_minSdkVersion
ice_targetSdkVersion
```

*NOTE: Do not use Android Studio to modify the project's settings.*

### Slice to Java Compiler

To build this project you'll need the Slice to Java compiler, which generates
Java code from Slice definitions. The compiler is written in C++. If you have
a suitable C++ development environment, you can build [Ice for C++](../../cpp)
yourself. Otherwise, you can obtain the compiler by installing a
[binary distribution][1].

The project's Gradle-based build system will automatically search for the
compiler in this repository and in the default installation directories used
by the binary distributions for our supported platforms.

### Bzip2 Compression

Ice for Java supports protocol compression using the bzip2 classes included
with [Apache Commons Compress][2].

The Maven package id for the commons-compress JAR file is as follows:

```
groupId=org.apache.commons, version=1.14, artifactId=commons-compress
```

The demos and tests are automatically setup to enable protocol compression by
adding the commons-compress JAR to the manifest class path. For your own
applications you must add the commons-compress JAR to the application CLASSPATH
to enable protocol compression.

> *These classes are a pure Java implementation of the bzip2 algorithm and
therefore add significant latency to Ice requests.*

## Building the Project

Follow these steps to open the project in Android Studio:

1. Start Android Studio
2. Select "Open an existing Android Studio project"
3. Navigate to and select the "android" subdirectory
4. Click OK and wait for the project to open and build

The Android Studio project contains a `testController` application for the Ice
test suite. To run the application, select it in the configuration pull down and
run it.

## Running the Test Suite

To run the test suite you need to add the `tools` and `platform-tools`
directories of Android SDK to the PATH.

  On macOS you can use the following command:

```
export PATH=~/Library/Android/sdk/tools:~/Library/Android/sdk/platform-tools:$PATH

```

  On Windows you can use the following command:

```
set PATH=%LOCALAPPDATA%\Android\sdk\tools;%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
```

The Instant Run feature of Android Studio causes some problems with the test
suite application and you need to disable it in order to run the test controller
application from Android Studio. Check the following page for instructions on
disabling it:

    https://developer.android.com/studio/run/index.html#disable-ir

Start the `testController` application from Android Studio and once it has
started, run the testsuite by using the `allTests.py` script:

```
cd android
python allTests.py
```

This will try to run the testsuite on the connected device. If there are
multiple devices connected to this machine you can use the `--device` argument
to select a device:

```
python allTests.py --device=ZX1C2234XF

```

You can see the list of connected devices with the `adb` command:

```
adb devices -l
```

If you are running the application on an emulator, you must pass the
`--androidemulator` command line option:

```
python allTests.py --androidemulator --device=emulator-5554
```

You can also start the emulator from the `allTests.py` script:

```
python allTests.py --androidemulator --avd=Nexus_6P_API_25 --controller-app
```

Where `--avd` is set to the Android emulator image you want to use. You can list
the available image names in your host by using `emulator -list-avds`. Images
can be created using Android Studio.

[1]: https://zeroc.com/distributions/ice
[2]: https://commons.apache.org/proper/commons-compress/
