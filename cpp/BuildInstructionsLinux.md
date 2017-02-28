# Building Ice for C++ on Linux

This file describes the Ice source distribution, including information about
compiler requirements, third-party dependencies, and instructions for building
and testing the distribution. If you prefer, you can install [binary
packages][1] for supported platforms that contain pre-compiled libraries,
executables, and everything else necessary to build Ice applications on Linux.

## C++ Build Requirements

### Operating Systems and Compilers

Ice is expected to build and run properly on any recent Linux distribution for
x86 and x86_64, and was extensively tested using the operating systems and
compiler versions listed for our [supported platforms][2].

### Third-Party Packages

Ice has dependencies on a number of third-party packages. Install these packages 
before building Ice for C++:

 - [bzip][3] 1.0
 - [Expat][4] 2.1
 - [LMDB][5] 0.9.16 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 (with patches)
 - [OpenSSL][7] 1.0.0 or later

Bzip, Expat and OpenSSL are included with most Linux distributions. 

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions 
that do not include them. You can install these packages as shown below:

#### Amazon Linux
    wget https://zeroc.com/download/GPG-KEY-zeroc-release
    sudo rpm --import GPG-KEY-zeroc-release
    cd /etc/yum.repos.d
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-amzn1.repo
    sudo yum install lmdb-devel mcpp-devel

#### RHEL 7
    wget https://zeroc.com/download/GPG-KEY-zeroc-release
    sudo rpm --import GPG-KEY-zeroc-release
    cd /etc/yum.repos.d
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-el7.repo
    sudo yum install lmdb-devel mcpp-devel

#### SLES 12
    wget https://zeroc.com/download/GPG-KEY-zeroc-release
    sudo rpm --import GPG-KEY-zeroc-release
    cd /etc/yum.repos.d
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-sles12.repo
    sudo yum install mcpp-devel

## Building Ice

In a command window, change to the `cpp` subdirectory:

    $ cd cpp

Edit `config/Make.rules` to establish your build configuration. The comments in
the file provide more information. Pay particular attention to the variables
that define the locations of the third-party libraries.

Now you're ready to build Ice:

    $ make

This will build the Ice core libraries, services, and tests.

### Build configurations and platforms

The C++ source tree supports multiple build configurations and platforms. To
see the supported configurations and platforms:

    make print V=supported-configs
    make print V=supported-platforms

To build all the supported configurations and platforms:

    make CONFIGS=all PLATFORMS=all

### C++11 mapping

The C++ source tree supports two different language mappings (C++98 and C++11),
the default build uses the C++98 mapping. The C++11 mapping is a new mapping
that uses the new language features.

To build the new C++11 mapping, use build configurations which are prefixed with
`cpp11`, for example:

    make CONFIGS=cpp11-shared

## Installing a C++ Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `<prefix>` variable in `config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your `PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `LD_LIBRARY_PATH`.

On an x86 system, the library directory is:

    <prefix>/lib                   (RHEL, SLES, Amazon)
    <prefix>/lib/i386-linux-gnu    (Ubuntu)

On an x86_64 system:

    <prefix>/lib64                 (RHEL, SLES, Amazon)
    <prefix>/lib/x86_64-linux-gnu  (Ubuntu)

When compiling Ice programs, you must pass the location of the `<prefix>/include`
directory to the compiler with the `-I` option, and the location of the library
directory with the `-L` option. 

If building a C++11 program, you must define `ICE_CPP11_MAPPING` macro during
compilation with the `-D` option (`g++ -DICE_CPP11_MAPING`) and add
the `++11` suffix to the library name when linking (such as `-lIce++11`).

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    $ pip install passlib

After a successful source build, you can run the tests as follows:

    $ make test

This command is equivalent to:

    $ python allTests.py
    
For C++11 mapping it also include the`--c++11` argument:

    $ python allTests.py --c++11

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice37/Using+the+Linux+Binary+Distributions
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: http://bzip.org
[4]: http://expat.sourceforge.net
[5]: http://symas.com/mdb/
[6]: https://github.com/zeroc-ice/mcpp
[7]: http://openssl.org
