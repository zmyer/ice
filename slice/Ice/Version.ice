// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:ICE_API", "objc:header-dir:objc", "objc:dll-export:ICE_API", "js:ice-build"]]

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * A version structure for the protocol version.
 *
 **/
struct ProtocolVersion
{
    byte major;
    byte minor;
};

/**
 *
 * A version structure for the encoding version.
 *
 **/
struct EncodingVersion
{
    byte major;
    byte minor;
};

};
