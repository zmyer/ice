// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SAFE_STDIO_H
#define ICEE_SAFE_STDIO_H

#include <IceE/Config.h>

namespace IceE
{

//
// This is for two reasons.
// 1. snprintf is _snprintf under windows.
// 2. This function ensures the buffer is null terminated.
//
ICEE_API void safesnprintf(char*, size_t, const char*, ...);
ICEE_API std::string printfToString(const char*, ...);

}

#endif
