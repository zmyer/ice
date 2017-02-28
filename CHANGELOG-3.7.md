The entries below contain brief descriptions of the changes in a release, in no
particular order. Some of the entries reflect significant new additions, while
others represent minor corrections. Although this list is not a comprehensive
report of every change we made in a release, it does provide details on the
changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your
applications to this release, and the manual for complete details on a
particular aspect of Ice.

- [Changes in Ice 3.7.0](#changes-in-ice-370)
  - [General Changes](#general-changes)

# Changes in Ice 3.7.0

These are the changes since Ice 3.6.3.

## General Changes

- The `findObjectByType`, `findAllObjectsByType`, `findObjectByTypeOnLeastLoadedNode`
  operations from the `IceGrid::Query` interface and the `allocateObjectByType`
  operation from the `IceGrid::Session` interfaces now only returns proxies for
  Ice objects from enabled servers. If a server is disabled, its well-known or
  allocatable Ice objects won't be returned anymore to clients.

- A Slice enumeration (enum) creates now a new namespace scope for its
  enumerators. In previous releases, the enumerators were in the same
  namespace scope as the enumeration. For example:
  ```
     enum Fruit { Apple, Orange, Pear };
     enum ComputerBrands { Apple, Dell, HP }; // Ok as of Ice 3.7, error in prior releases
  ```
  The mapping of enum to C++, C#, Java etc. is not affected by this
  change. Slice constants and data member default values that reference
  enumerators should be updated to use only the enumerator's name when the
  enclosing enum is in a different module. For example:
  ```
  module M1
  {
      enum Fruit { Apple, Orange, Pear };
      enum ComputerBrands { Apple, Dell, HP };

      const Fruit a = Apple; // Recommended syntax for all Ice releases
  };

  module M2
  {
      const M1::Fruit a1 = Apple;             // The recommended syntax as of Ice 3.7
      const M1::Fruit a2 = M1::Fruit::Apple;  // Ok as well
      const M1::Fruit a3 = M1::Apple;         // Supported for backwards compatibility
                                              // with earlier Ice releases
  };

  ```
- The communicator and connection flushBatchRequests method now takes
  an additional argument to specify whether or not the batch requests
  to flush should be compressed. See the documentation of the
  Ice::CompressBatch enumeration for the different options available
  to specify whether or not the batch should be compressed.

- The UDP server endpoint now supports specifying `--interface *` to join the
  multicast group on using all the local interfaces. It's also now the default
  behavior if no `--interface` option is specified.

- Ice no longer halt the program if can't accept new incoming connections when
  the system runs out of file descriptors. Instead, it rejects queued pending
  connections and temporarily stops accepting new connections. An error message
  is also printed on the Ice logger.

- Added Bluetooth transport plug-in for C++ and Android. The C++ plug-in
  requires BlueZ 5.40 or later.

- Dispatch interceptors and ice_dispatch can now handle user exceptions. User
  exceptions raised by a servant dispatch are propagated to ice_dispatch and
  can also be raised from the Ice::DispatchInterceptor::dispatch implementation.
  As a result, the Ice::DispatchStatus enumeration has been removed. See the
  Ice manual for details on the new dispatch interceptor API.

- The ice_getConnection() method now correctly returns a connection if
  connection caching is disabled (it previously returned a null connection).

- The iOS SSL transport is now based on the same implementation as macOS. Most
  of the functionality supported on macOS is now also supported on iOS. There
  are still few limitations however:

  - the `checkValidity`, `getNotBefore`, `getNotAfter` methods are not supported
    on the `IceSSL::Certificate` class.

  - only PKCS12 certificates are supported (no support for PEM).

- Added support for iAP transport to allow iOS clients to communicate with
  connected accessories.

- The `Ice::ConnectionInfo` `sndSize` and `rcvSize` data members have been moved
  to the TCP and UDP connection info classes. The `Ice::WSEndpointInfo` and
  `IceSSL::EndpointInfo` classes no longer inherit `Ice::IPConnectionInfo` and
  instead directly extend `Ice::ConnectionInfo`. IP connection information can
  still be retrieved by accessing the connection information object stored with
  the new `underlying` data member.

- IceGrid and IceStorm now use LMDB for their persistent storage instead of
  Freeze/BerkeleyDB.

- Added command line tools, `icegriddb` and `icestormdb`, to import/export the
  IceGrid and IceStorm databases.

- Added support for two additional IceGrid variables: `server.data` and
  `service.data`. These variables point to server and service specific data
  directories created by IceGrid on the node. These data directories are
  automatically removed by IceGrid if you remove the server from the
  deployment.

  For consistency, the `node.datadir` variable has been deprecated, use the
  `node.data` variable instead.

- Added the new metadata tag `delegate` for local interfaces with one operation.
  Interfaces with this metadata will be generated as a `std::function` in C++11,
  `delegate` in C#, `FunctionalInterface` in Java, `function callback` in
  JavaScript, `block` in Objective-C, `function/lambda` in Python. Other language
  mappings keep their default behavior.

- `ObjectFactory` has been deprecated in favor of the new local interface
  `ValueFactory`. Communicator operations `addObjectFactory`and
  `findObjectFactory` have been deprecated in favor of similar operations on the
  new interface `ValueFactoryManager`.

- The Slice compiler options --ice and --underscore are now deprecated, and
  replaced by the global Slice metadata ice-prefix and underscore.

- Renamed local interface metadata `async` to `async-oneway`.

- Replaced `ConnectionCallback` by delegates `CloseCallback` and `HeartbeatCallback`.
  Also replaced `setCallback` by `setCloseCallback` and `setHeartbeatCallback` on
  the `Connection` interface.

- Updating Windows build system to use MSBuild instead of nmake.

- Changed the parsing of hex escape sequences (\x....) in Slice string literals:
  the parsing now stops after 2 hex digits. For example, \x0ab is now read as '\x0a'
  followed by 'b'. Previously all the hex digits where read like in C++.

- Stringified identities and proxies now support non-ASCII characters
  and universal character names (\unnnn and \Unnnnnnnn). See the property
  Ice.ToStringMode and the static function/method identityToString.

- Fixed proxies stringification: `Communicator::proxyToString` and equivalent
  "to string" methods on fixed proxies no longer raise a `FixedProxyException`;
  the proxy is just stringified without endpoints.

- An empty endpoint in an Object Adapter endpoint list is now rejected with an
  `EndpointParseException`; such an endpoint was ignored in previous releases.

- IcePatch2 and IceGrid's distribution mechanism have been deprecated.

- Updated IceSSL hostname verification (enabled with IceSSL.CheckCertName) to
  use the native checks of the platform SSL implementation.

## C++ Changes

- The Ice::Communicator and Ice::ObjectAdapter destroy methods are now
  declared as `noexcept` (C++11) or `throw()` (C++98).

- The --dll-export option of slice2cpp is now deprecated, and replaced by the global
  Slice metadata `cpp:dll-export:SYMBOL`.

- Added `cpp:scoped` metadata for enums in the C++98 mapping. The generated C++
  enumerators for a "scoped enum" are prefixed with the enumeration's name. For
  example:
  ```
     // Slice
     ["cpp:scoped"] enum Fruit { Apple, Orange, Pear };
  ```
  corresponds to:
  ```
     // C++98
     enum Fruit { FruitApple, FruitOrange, FruitPear };
  ```

- Upgrade UWP IceSSL implementation to support client side certificates and custom
  certificate verification.

- Added getEngineName and getEngineVersion methods to IceSSL::Plugin to retrieve
  the SSL engine name and version used by the Ice runtime.

- Added getAuthorityKeyIdentifier and getSubjectKeyIdentifier methods to IceSSL::Certificate.
  These methods are not supported on iOS or UWP.

- Upgrade IceSSL Certificate API to allow retrive X509v3 extensions. This feature
  is currently only supported with OpenSSL and SChannel SSL engines.

## C# Changes

- Added new interface/class metadata cs:tie. Use this metadata to generate a tie
  class for a given interface or class.

- `cs:` and `clr:` are now interchangeable in metadata directives.

- Add support to preload referenced assemblies. The property Ice.PreloadAssemblies
  controls this behavior. If set to a value greater than 0 the Ice runtime will try
  to load all the assemblies referenced by the process during communicator initialization,
  otherwise the referenced assemblies will be initialized when the Ice run-time needs
  to lookup a C# class. The default value is 0.

## Java Changes

- The Ice communicator now implements `java.lang.AutoCloseable`. This enables
  the code to initialize the communicator within a `try-with-resources` block.
  The communicator will implicitly be destroyed when the block exits.

- Fixed a bug where unmarshaling Ice objects was really slow when using
  compact type IDs.

- (Java Compat) Added new interface/class metadata java:tie. Use this metadata
  to generate a tie class for a given interface or class.

## JavaScript Changes

- Improve Ice.Long class to allow creating Ice.Long instance from
  JavaScript Numbers.

## Objective-C Changes

- Fixed a bug where optional object dictionary parameters would
  trigger an assert on marshaling.

- The --dll-export option of slice2objc is now deprecated, and replaced by the
  global Slice metadata `objc:dll-export:SYMBOL`.

- Added `objc:scoped` metadata for enums. The generated Objective-C enumerators
  for a "scoped enum" are prefixed with the enumeration's name. For example:
  ```
  // Slice
  module M
  {
     ["objc:scoped"] enum Fruit { Apple, Orange, Pear };
  };

  ```
  corresponds to:
  ```
  // Objective-C
  typedef enum : ICEInt
  {
      MFruitApple,
      MFruitPear,
      MFruitOrange
  } MFruit;
  ```
## Python Changes

- The Ice communicator now implements context manager protocol. This enables
  the code to initialize the communicator within a `with` block. The
  communicator will implicitly be destroyed when the `with` block exits.

- Added a new AMI mapping that returns Ice.Future. The Future class provides an API
  that is compatible with concurrent.futures.Future, with some additional Ice-specific
  methods. Programs can use the new mapping by adding the suffix `Async` to operation
  names, such as `sayHelloAsync`. The existing `begin_/end_` mapping is still supported.

- Changed the AMD mapping. AMD servant methods must no longer append the `_async` suffix to
  their names. Additionally, an AMD callback is no longer passed to a servant method.
  Now a servant method always uses the mapped name, and it can either return the results
  (for a synchronous implementation) or return an Ice.Future (for an asynchronous
  implementation).

  With Python 3, a servant method can also be implemented as a coroutine. Ice will start
  the coroutine, and coroutines can `await` on Ice.Future objects. Note that because Ice
  is multithreaded, users who also want to use the asyncio package must make sure it's
  done in a thread-safe manner. To assist with this, the Ice.wrap_future() function accepts
  an Ice.Future and returns an asyncio.Future.

- Renamed optional invocation context parameter to `context` for consistency with other
  language mappings (was `_ctx` in previous versions).
