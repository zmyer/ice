// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/InstanceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Object.h>
#include <Ice/Current.h>
#include <Ice/IncomingAsyncF.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ResponseHandlerF.h>

#include <deque>

#ifdef ICE_CPP11_MAPPING

namespace Ice
{

class ICE_API MarshaledResult
{
public:

    MarshaledResult(const Current&);

    std::shared_ptr<OutputStream> getOutputStream() const
    {
        return ostr;
    }

protected:

    std::shared_ptr<OutputStream> ostr;
};

}

#endif

namespace IceInternal
{

class ICE_API IncomingBase : private IceUtil::noncopyable
{
public:

    Ice::OutputStream* startWriteParams();
    void endWriteParams();
    void writeEmptyParams();
    void writeParamEncaps(const Ice::Byte*, Ice::Int, bool);

#ifdef ICE_CPP11_MAPPING
    void setMarshaledResult(const Ice::MarshaledResult&);
#endif

    void response(bool);
    void exception(const std::exception&, bool);
    void exception(const std::string&, bool);

protected:

    IncomingBase(Instance*, ResponseHandler*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte, Ice::Int);
    IncomingBase(IncomingBase&);

    void warning(const Ice::Exception&) const;
    void warning(const std::string&) const;

    bool servantLocatorFinished(bool);

    void handleException(const std::exception&, bool);
    void handleException(const std::string&, bool);

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
#ifdef ICE_CPP11_MAPPING
    ::std::shared_ptr<void> _cookie;
#else
    Ice::LocalObjectPtr _cookie;
#endif
    DispatchObserver _observer;
    bool _response;
    Ice::Byte _compress;
    Ice::FormatType _format;
    Ice::OutputStream _os;

    //
    // Optimization. The request handler may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    ResponseHandler* _responseHandler;

#ifdef ICE_CPP11_MAPPING
    using DispatchInterceptorCallbacks = std::deque<std::pair<std::function<bool()>,
                                                              std::function<bool(std::exception_ptr)>>>;
#else
    typedef std::deque<Ice::DispatchInterceptorAsyncCallbackPtr> DispatchInterceptorCallbacks;
#endif
    DispatchInterceptorCallbacks _interceptorCBs;
};

class ICE_API Incoming : public IncomingBase
{
public:

    Incoming(Instance*, ResponseHandler*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte, Ice::Int);

    const Ice::Current& getCurrent()
    {
        return _current;
    }

#ifdef ICE_CPP11_MAPPING
    void push(std::function<bool()>, std::function<bool(std::exception_ptr)>);
#else
    void push(const Ice::DispatchInterceptorAsyncCallbackPtr&);
#endif
    void pop();

    void setAsync(const IncomingAsyncPtr& in)
    {
        assert(!_inAsync);
        _inAsync = in;
    }

    void startOver();

    void setFormat(Ice::FormatType format)
    {
        _format = format;
    }

    void invoke(const ServantManagerPtr&, Ice::InputStream*);

    // Inlined for speed optimization.
    void skipReadParams()
    {
        _current.encoding = _is->skipEncapsulation();
    }
    Ice::InputStream* startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is->startEncapsulation();
        return _is;
    }
    void endReadParams() const
    {
        _is->endEncapsulation();
    }
    void readEmptyParams()
    {
        _current.encoding = _is->skipEmptyEncapsulation();
    }
    void readParamEncaps(const Ice::Byte*& v, Ice::Int& sz)
    {
        _current.encoding = _is->readEncapsulation(v, sz);
    }

private:

    friend class IncomingAsync;

    Ice::InputStream* _is;
    Ice::Byte* _inParamPos;

    IncomingAsyncPtr _inAsync;
};

}

#endif
