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
 * Base class for extracting a Slice class instance from an input stream.
 **/
public abstract class ValueReader extends Value
{
    /**
     * Reads the state of this Slice class from an input stream.
     *
     * @param in The input stream to read from.
     **/
    public abstract void read(InputStream in);

    @Override
    public void _iceWrite(OutputStream os)
    {
        assert(false);
    }

    @Override
    public void _iceRead(InputStream is)
    {
        read(is);
    }

    public static final long serialVersionUID = 0L;
}
