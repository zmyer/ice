// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder base class for Ice objects that are in- or inout-parameters.
 **/
public abstract class ObjectHolderBase<T extends Ice.Object> implements ReadValueCallback
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public
    ObjectHolderBase()
    {
    }

    /**
     * Instantiates the class with the passed Ice object.
     *
     * @param obj The initial value for this holder.
     **/
    public
    ObjectHolderBase(T obj)
    {
        this.value = obj;
    }

    /**
     * The Ice object stored by this holder.
     **/
    public T value;
}
