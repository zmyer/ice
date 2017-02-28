// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/EndpointI.h>
#include <IceSSL/AcceptorI.h>
#include <IceSSL/ConnectorI.h>
#include <IceSSL/Instance.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Object.h>
#include <Ice/HashUtil.h>
#include <Ice/Comparable.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

namespace
{

Ice::IPEndpointInfoPtr
getIPEndpointInfo(const Ice::EndpointInfoPtr& info)
{
    for(Ice::EndpointInfoPtr p = info; p; p = p->underlying)
    {
        Ice::IPEndpointInfoPtr ipInfo = ICE_DYNAMIC_CAST(Ice::IPEndpointInfo, p);
        if(ipInfo)
        {
            return ipInfo;
        }
    }
    return ICE_NULLPTR;
}

}

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceSSL::upCast(EndpointI* p) { return p; }
#endif

IceSSL::EndpointI::EndpointI(const InstancePtr& instance, const IceInternal::EndpointIPtr& del) :
    _instance(instance), _delegate(del)
{
}

void
IceSSL::EndpointI::streamWriteImpl(Ice::OutputStream* stream) const
{
    _delegate->streamWriteImpl(stream);
}

Ice::EndpointInfoPtr
IceSSL::EndpointI::getInfo() const
{
    EndpointInfoPtr info = ICE_MAKE_SHARED(IceInternal::InfoI<EndpointInfo>, ICE_SHARED_FROM_CONST_THIS(EndpointI));
    info->underlying = _delegate->getInfo();
    info->compress = info->underlying->compress;
    info->timeout = info->underlying->timeout;
    return info;
}

Ice::Short
IceSSL::EndpointI::type() const
{
    return _delegate->type();
}

const std::string&
IceSSL::EndpointI::protocol() const
{
    return _delegate->protocol();
}

Int
IceSSL::EndpointI::timeout() const
{
    return _delegate->timeout();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::timeout(Int timeout) const
{
    if(timeout == _delegate->timeout())
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _delegate->timeout(timeout));
    }
}

const string&
IceSSL::EndpointI::connectionId() const
{
    return _delegate->connectionId();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _delegate->connectionId())
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _delegate->connectionId(connectionId));
    }
}

bool
IceSSL::EndpointI::compress() const
{
    return _delegate->compress();
}

IceInternal::EndpointIPtr
IceSSL::EndpointI::compress(bool compress) const
{
    if(compress == _delegate->compress())
    {
        return ICE_SHARED_FROM_CONST_THIS(EndpointI);
    }
    else
    {
        return ICE_MAKE_SHARED(EndpointI, _instance, _delegate->compress(compress));
    }
}

bool
IceSSL::EndpointI::datagram() const
{
    return _delegate->datagram();
}

bool
IceSSL::EndpointI::secure() const
{
    return _delegate->secure();
}

IceInternal::TransceiverPtr
IceSSL::EndpointI::transceiver() const
{
    return 0;
}

void
IceSSL::EndpointI::connectors_async(Ice::EndpointSelectionType selType,
                                    const IceInternal::EndpointI_connectorsPtr& callback) const
{
    class CallbackI : public IceInternal::EndpointI_connectors
    {
    public:

        CallbackI(const IceInternal::EndpointI_connectorsPtr& callback, const InstancePtr& instance,
                  const string& host) :
            _callback(callback), _instance(instance), _host(host)
        {
        }

        virtual void connectors(const vector<IceInternal::ConnectorPtr>& c)
        {
            vector<IceInternal::ConnectorPtr> connectors = c;
            for(vector<IceInternal::ConnectorPtr>::iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
                *p = new ConnectorI(_instance, *p, _host);
            }
            _callback->connectors(connectors);
        }

        virtual void exception(const Ice::LocalException& ex)
        {
            _callback->exception(ex);
        }

    private:

        const IceInternal::EndpointI_connectorsPtr _callback;
        const InstancePtr _instance;
        const string _host;
    };

    IPEndpointInfoPtr info = getIPEndpointInfo(_delegate->getInfo());
    _delegate->connectors_async(selType, ICE_MAKE_SHARED(CallbackI, callback, _instance, info ? info->host : string()));
}

IceInternal::AcceptorPtr
IceSSL::EndpointI::acceptor(const string& adapterName) const
{
    return new AcceptorI(ICE_SHARED_FROM_CONST_THIS(EndpointI), _instance, _delegate->acceptor(adapterName), adapterName);
}

EndpointIPtr
IceSSL::EndpointI::endpoint(const IceInternal::EndpointIPtr& delEndp) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, delEndp);
}

vector<IceInternal::EndpointIPtr>
IceSSL::EndpointI::expand() const
{
    vector<IceInternal::EndpointIPtr> endps = _delegate->expand();
    for(vector<IceInternal::EndpointIPtr>::iterator p = endps.begin(); p != endps.end(); ++p)
    {
        *p = p->get() == _delegate.get() ? ICE_SHARED_FROM_CONST_THIS(EndpointI) : ICE_MAKE_SHARED(EndpointI, _instance, *p);
    }
    return endps;
}

bool
IceSSL::EndpointI::equivalent(const IceInternal::EndpointIPtr& endpoint) const
{
    const EndpointI* endpointI = dynamic_cast<const EndpointI*>(endpoint.get());
    if(!endpointI)
    {
        return false;
    }
    return _delegate->equivalent(endpointI->_delegate);
}

Ice::Int
IceSSL::EndpointI::hash() const
{
    return _delegate->hash();
}

string
IceSSL::EndpointI::options() const
{
    return _delegate->options();
}

bool
#ifdef ICE_CPP11_MAPPING
IceSSL::EndpointI::operator==(const Ice::Endpoint& r) const
#else
IceSSL::EndpointI::operator==(const Ice::LocalObject& r) const
#endif
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(!Ice::targetEqualTo(_delegate, p->_delegate))
    {
        return false;
    }

    return true;
}

bool
#ifdef ICE_CPP11_MAPPING
IceSSL::EndpointI::operator<(const Ice::Endpoint& r) const
#else
IceSSL::EndpointI::operator<(const Ice::LocalObject& r) const
#endif
{
    const EndpointI* p = dynamic_cast<const EndpointI*>(&r);
    if(!p)
    {
        const IceInternal::EndpointI* e = dynamic_cast<const IceInternal::EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(Ice::targetLess(_delegate, p->_delegate))
    {
        return true;
    }
    else if (Ice::targetLess(p->_delegate, _delegate))
    {
        return false;
    }

    return false;
}

bool
IceSSL::EndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    return false;
}

IceSSL::EndpointFactoryI::EndpointFactoryI(const InstancePtr& instance,
                                           const IceInternal::EndpointFactoryPtr& delegate) :
    _instance(instance), _delegate(delegate)
{
}

IceSSL::EndpointFactoryI::~EndpointFactoryI()
{
}

Short
IceSSL::EndpointFactoryI::type() const
{
    return _instance->type();
}

string
IceSSL::EndpointFactoryI::protocol() const
{
    return _instance->protocol();
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::create(vector<string>& args, bool oaEndpoint) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, _delegate->create(args, oaEndpoint));
}

IceInternal::EndpointIPtr
IceSSL::EndpointFactoryI::read(Ice::InputStream* s) const
{
    return ICE_MAKE_SHARED(EndpointI, _instance, _delegate->read(s));
}

void
IceSSL::EndpointFactoryI::destroy()
{
    _delegate->destroy();
    _instance = 0;
}

IceInternal::EndpointFactoryPtr
IceSSL::EndpointFactoryI::clone(const IceInternal::ProtocolInstancePtr& inst,
                                const IceInternal::EndpointFactoryPtr& delegate) const
{
    InstancePtr instance = new Instance(_instance->engine(), inst->type(), inst->protocol());
    return new EndpointFactoryI(instance, delegate ? delegate : _delegate->clone(instance, 0));
}
