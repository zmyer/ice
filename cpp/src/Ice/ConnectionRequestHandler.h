// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_REQUEST_HANDLER_H
#define ICE_CONNECTION_REQUEST_HANDLER_H

#include <Ice/RequestHandler.h>
#include <Ice/ReferenceF.h>
#include <Ice/ProxyF.h>

namespace IceInternal
{

class ConnectionRequestHandler : public RequestHandler
#ifdef ICE_CPP11_MAPPING
                               , public std::enable_shared_from_this<ConnectionRequestHandler>
#endif
{
public:

    ConnectionRequestHandler(const ReferencePtr&, const Ice::ConnectionIPtr&, bool);

    virtual RequestHandlerPtr update(const RequestHandlerPtr&, const RequestHandlerPtr&);

    virtual AsyncStatus sendAsyncRequest(const ProxyOutgoingAsyncBasePtr&);

    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    virtual Ice::ConnectionIPtr getConnection();
    virtual Ice::ConnectionIPtr waitForConnection();

private:

    Ice::ConnectionIPtr _connection;
    bool _compress;
};

}

#endif
