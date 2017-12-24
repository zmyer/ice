// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final public class WSEndpointFactory extends EndpointFactoryWithUnderlying
{
    public WSEndpointFactory(ProtocolInstance instance, short type)
    {
        super(instance, type);
    }

    @Override
    public EndpointFactory cloneWithUnderlying(ProtocolInstance instance, short underlying)
    {
        return new WSEndpointFactory(instance, underlying);
    }

    @Override
    public EndpointI createWithUnderlying(EndpointI underlying, java.util.ArrayList<String> args, boolean oaEndpoint)
    {
        return new WSEndpoint(_instance, underlying, args);
    }

    @Override
    public EndpointI readWithUnderlying(EndpointI underlying, com.zeroc.Ice.InputStream s)
    {
        return new WSEndpoint(_instance, underlying, s);
    }
}
