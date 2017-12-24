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
 * Unknown sliced value holds an instance of an unknown Slice class type.
 **/
public final class UnknownSlicedValue extends Value
{
    /**
     * Represents an instance of a Slice class type having the given Slice type.
     *
     * @param unknownTypeId The Slice type ID of the unknown object.
     **/
    public UnknownSlicedValue(String unknownTypeId)
    {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Returns the sliced data if the value has a preserved-slice base class and has been sliced during
     * un-marshaling of the value, null is returned otherwise.
     *
     * @return The sliced data or null.
     **/
    @Override
    public SlicedData ice_getSlicedData()
    {
        return _slicedData;
    }

    /**
     * Determine the Slice type ID associated with this object.
     *
     * @return The type ID.
     **/
    @Override
    public String ice_id()
    {
        return _unknownTypeId;
    }

    @Override
    public void _iceWrite(OutputStream ostr)
    {
        ostr.startValue(_slicedData);
        ostr.endValue();
    }

    @Override
    public void _iceRead(InputStream istr)
    {
        istr.startValue();
        _slicedData = istr.endValue(true);
    }

    private final String _unknownTypeId;
    private SlicedData _slicedData;

    public static final long serialVersionUID = 0L;
}
