// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Base class for all Ice system exceptions.
 **/
public abstract class SystemException extends Exception
{
    public SystemException()
    {
    }

    public SystemException(Throwable cause)
    {
        super(cause);
    }

    public static final long serialVersionUID = 0L;
}
