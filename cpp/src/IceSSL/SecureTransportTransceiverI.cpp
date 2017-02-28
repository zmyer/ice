// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SecureTransportTransceiverI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>

#ifdef ICE_USE_SECURE_TRANSPORT

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

namespace
{

string
trustResultDescription(SecTrustResultType result)
{
    switch(result)
    {
        case kSecTrustResultInvalid:
        {
            return "Invalid setting or result";
        }
        case kSecTrustResultDeny:
        {
            return "The user specified that the certificate should not be trusted";
        }
        case kSecTrustResultRecoverableTrustFailure:
        case kSecTrustResultFatalTrustFailure:
        {
            return "Trust denied";
        }
        case kSecTrustResultOtherError:
        {
            return "Other error internal error";
        }
        default:
        {
            assert(false);
            return "";
        }
    }
}

string
protocolName(SSLProtocol protocol)
{
    switch(protocol)
    {
        case kSSLProtocol2:
            return "SSL 2.0";
        case kSSLProtocol3:
            return "SSL 3.0";
        case kTLSProtocol1:
            return "TLS 1.0";
        case kTLSProtocol11:
            return "TLS 1.1";
        case kTLSProtocol12:
            return "TLS 1.2";
        default:
            return "Unknown";
    }
}

//
// Socket write callback
//
OSStatus
socketWrite(SSLConnectionRef connection, const void* data, size_t* length)
{
    const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
    assert(transceiver);
    return transceiver->writeRaw(reinterpret_cast<const char*>(data), length);
}

//
// Socket read callback
//
OSStatus
socketRead(SSLConnectionRef connection, void* data, size_t* length)
{
    const TransceiverI* transceiver = static_cast<const TransceiverI*>(connection);
    assert(transceiver);
    return transceiver->readRaw(reinterpret_cast<char*>(data), length);
}

bool
checkTrustResult(SecTrustRef trust, const SecureTransportEnginePtr& engine, const IceSSL::InstancePtr& instance,
                 const string& host)
{
    OSStatus err = noErr;
    SecTrustResultType trustResult = kSecTrustResultOtherError;
    if(trust)
    {
        if((err = SecTrustSetAnchorCertificates(trust, engine->getCertificateAuthorities())))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }

        //
        // Disable network fetch, we don't want this to block.
        //
        if((err = SecTrustSetNetworkFetchAllowed(trust, false)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }

        //
        // Add SSL trust policy if we need to check the certificate name.
        //
        if(engine->getCheckCertName() && !host.empty())
        {
            UniqueRef<SecPolicyRef> policy(SecPolicyCreateSSL(false, toCFString(host)));
            UniqueRef<CFArrayRef> policies;
            if((err = SecTrustCopyPolicies(trust, &policies.get())))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
            }
            UniqueRef<CFMutableArrayRef> newPolicies(CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, policies.get()));
            CFArrayAppendValue(newPolicies.get(), policy.release());
            if((err = SecTrustSetPolicies(trust, newPolicies.release())))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
            }
        }

        //
        // Evaluate the trust
        //
        if((err = SecTrustEvaluate(trust, &trustResult)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + errorToString(err));
        }
    }

    switch(trustResult)
    {
    case kSecTrustResultUnspecified:
    case kSecTrustResultProceed:
    {
        //
        // Trust verify success.
        //
        return true;
    }
    default:
    // case kSecTrustResultInvalid:
    // case kSecTrustResultConfirm: // Used in old OS X versions
    // case kSecTrustResultDeny:
    // case kSecTrustResultRecoverableTrustFailure:
    // case kSecTrustResultFatalTrustFailure:
    // case kSecTrustResultOtherError:
    {
        if(engine->getVerifyPeer() == 0)
        {
            if(instance->traceLevel() >= 1)
            {
                ostringstream os;
                os << "IceSSL: ignoring certificate verification failure\n" << trustResultDescription(trustResult);
                instance->logger()->trace(instance->traceCategory(), os.str());
            }
            return false;
        }
        else
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure\n" << trustResultDescription(trustResult);
            string msg = os.str();
            if(instance->traceLevel() >= 1)
            {
                instance->logger()->trace(instance->traceCategory(), msg);
            }
            throw SecurityException(__FILE__, __LINE__, msg);
        }
    }
    }
}
}

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if(!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;
    }

    //
    // Limit the size of packets passed to SSLWrite/SSLRead to avoid
    // blocking and holding too much memory.
    //
    _maxSendPacketSize = std::max(512, IceInternal::getSendBufferSize(_delegate->getNativeInfo()->fd()));
    _maxRecvPacketSize = std::max(512, IceInternal::getRecvBufferSize(_delegate->getNativeInfo()->fd()));

    OSStatus err = 0;
    if(!_ssl)
    {
        //
        // Initialize SSL context
        //
        _ssl.reset(_engine->newContext(_incoming));
        if((err = SSLSetIOFuncs(_ssl.get(), socketRead, socketWrite)))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: setting IO functions failed\n" +
                                    errorToString(err));
        }

        if((err = SSLSetConnection(_ssl.get(), reinterpret_cast<SSLConnectionRef>(this))))
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: setting SSL connection failed\n" +
                                    errorToString(err));
        }
    }

    SSLSessionState state;
    SSLGetSessionState(_ssl.get(), &state);

    //
    // SSL Handshake
    //
    while(state == kSSLHandshake || state == kSSLIdle)
    {
        err = SSLHandshake(_ssl.get());
        if(err == noErr)
        {
            break; // We're done!
        }
        else if(err == errSSLWouldBlock)
        {
            assert(_flags & SSLWantRead || _flags & SSLWantWrite);
            return _flags & SSLWantRead ? IceInternal::SocketOperationRead : IceInternal::SocketOperationWrite;
        }
        else if(err == errSSLPeerAuthCompleted)
        {
            assert(!_trust);
            err = SSLCopyPeerTrust(_ssl.get(), &_trust.get());

            if(_incoming && _engine->getVerifyPeer() == 1 && (err == errSSLBadCert || !_trust))
            {
                // This is expected if the client doesn't provide a certificate. With 10.10 and 10.11 errSSLBadCert
                // is expected, the server is configured to verify but not require the client
                // certificate so we ignore the failure. In 10.12 there is no error and trust is 0.
                continue;
            }
            if(err == noErr)
            {
                _verified = checkTrustResult(_trust.get(), _engine, _instance, _host);
                continue; // Call SSLHandshake to resume the handsake.
            }
            // Let it fall through, this will raise a SecurityException with the SSLCopyPeerTrust error.
        }
        else if(err == errSSLClosedGraceful || err == errSSLClosedAbort)
        {
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        ostringstream os;
        os << "IceSSL: ssl error occurred for new " << (_incoming ? "incoming" : "outgoing") << " connection:\n"
           << _delegate->toString() << "\n" << errorToString(err);
        throw ProtocolException(__FILE__, __LINE__, os.str());
    }

    for(int i = 0, count = SecTrustGetCertificateCount(_trust.get()); i < count; ++i)
    {
        SecCertificateRef cert = SecTrustGetCertificateAtIndex(_trust.get(), i);
        CFRetain(cert);

        CertificatePtr certificate = ICE_MAKE_SHARED(Certificate, cert);
        _nativeCerts.push_back(certificate);
        _certs.push_back(certificate->encode());
    }

    assert(_ssl);
    SSLCipherSuite cipher;
    SSLGetNegotiatedCipher(_ssl.get(), &cipher);
    _cipher = _engine->getCipherName(cipher);

    _engine->verifyPeer(_host, ICE_DYNAMIC_CAST(NativeConnectionInfo, getInfo()), toString());

    if(_instance->engine()->securityTraceLevel() >= 1)
    {
        
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        SSLProtocol protocol;
        SSLGetNegotiatedProtocolVersion(_ssl.get(), &protocol);
        const string sslProtocolName = protocolName(protocol);

        SSLCipherSuite cipher;
        SSLGetNegotiatedCipher(_ssl.get(), &cipher);
        const string sslCipherName = _engine->getCipherName(cipher);

        if(sslCipherName.empty())
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << sslCipherName << "\n";
            out << "protocol = " << sslProtocolName << "\n";
        }
        out << toString();
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceSSL::TransceiverI::close()
{
    _trust.reset(0);
    if(_ssl)
    {
        SSLClose(_ssl.get());
    }
    _ssl.reset(0);

    _delegate->close();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(!_connected)
    {
        return _delegate->write(buf);
    }

    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxSendPacketSize);
    while(buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = _buffered ? SSLWrite(_ssl.get(), 0, 0, &processed) :
                                   SSLWrite(_ssl.get(), reinterpret_cast<const void*>(buf.i), packetSize, &processed);

        if(err)
        {
            if(err == errSSLWouldBlock)
            {
                if(_buffered == 0)
                {
                    _buffered = processed;
                }
                assert(_flags & SSLWantWrite);
                return IceInternal::SocketOperationWrite;
            }

            if(err == errSSLClosedGraceful)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(err <= -9800 && err >= -9849)
            {
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during write:\n" + errorToString(err));
            }

            errno = err;
            if(IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        if(_buffered)
        {
            buf.i += _buffered;
            _buffered = 0;
        }
        else
        {
            buf.i += processed;
        }

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::read(IceInternal::Buffer& buf)
{
    if(!_connected)
    {
        return _delegate->read(buf);
    }

    if(buf.i == buf.b.end())
    {
        return  IceInternal::SocketOperationNone;
    }

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);

    size_t packetSize = std::min(static_cast<size_t>(buf.b.end() - buf.i), _maxRecvPacketSize);
    while(buf.i != buf.b.end())
    {
        size_t processed = 0;
        OSStatus err = SSLRead(_ssl.get(), reinterpret_cast<void*>(buf.i), packetSize, &processed);
        if(err)
        {
            if(err == errSSLWouldBlock)
            {
                buf.i += processed;
                assert(_flags & SSLWantRead);
                return IceInternal::SocketOperationRead;
            }

            if(err == errSSLClosedGraceful || err == errSSLClosedAbort)
            {
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }

            //
            // SSL protocol errors are defined in SecureTransport.h are in the range
            // -9800 to -9849
            //
            if(err <= -9800 && err >= -9849)
            {
                throw ProtocolException(__FILE__, __LINE__, "IceSSL: error during read:\n" + errorToString(err));
            }

            errno = err;
            if(IceInternal::connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        buf.i += processed;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = buf.b.end() - buf.i;
        }
    }

    //
    // Check if there's still buffered data to read. In this case, set the read ready status.
    //
    size_t buffered = 0;
    OSStatus err = SSLGetBufferedReadSize(_ssl.get(), &buffered);
    if(err)
    {
        errno = err;
        throw SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
    }
    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, buffered > 0);
    return IceInternal::SocketOperationNone;
}

string
IceSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    NativeConnectionInfoPtr info = ICE_MAKE_SHARED(NativeConnectionInfo);
    info->underlying = _delegate->getInfo();
    info->incoming = _incoming;
    info->adapterName = _adapterName;
    info->cipher = _cipher;
    info->certs = _certs;
    info->verified = _verified;
    info->nativeCerts = _nativeCerts;
    return info;
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

IceSSL::TransceiverI::TransceiverI(const IceSSL::InstancePtr& instance,
                                   const IceInternal::TransceiverPtr& delegate,
                                   const string& hostOrAdapterName,
                                   bool incoming) :
    _instance(instance),
    _engine(SecureTransportEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _delegate(delegate),
    _connected(false),
    _verified(false),
    _buffered(0)
{
}

IceSSL::TransceiverI::~TransceiverI()
{
}

OSStatus
IceSSL::TransceiverI::writeRaw(const char* data, size_t* length) const
{
    _flags &= ~SSLWantWrite;

    try
    {
        IceInternal::Buffer buf(reinterpret_cast<const Ice::Byte*>(data), reinterpret_cast<const Ice::Byte*>(data) + *length);
        IceInternal::SocketOperation op = _delegate->write(buf);
        if(op == IceInternal::SocketOperationWrite)
        {
            *length = buf.i - buf.b.begin();
            _flags |= SSLWantWrite;
            return errSSLWouldBlock;
        }
        assert(op == IceInternal::SocketOperationNone);
    }
    catch(const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch(const Ice::SocketException& ex)
    {
        return ex.error;
    }
    catch(...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

OSStatus
IceSSL::TransceiverI::readRaw(char* data, size_t* length) const
{
    _flags &= ~SSLWantRead;

    try
    {
        IceInternal::Buffer buf(reinterpret_cast<Ice::Byte*>(data), reinterpret_cast<Ice::Byte*>(data) + *length);
        IceInternal::SocketOperation op = _delegate->read(buf);
        if(op == IceInternal::SocketOperationRead)
        {
            *length = buf.i - buf.b.begin();
            _flags |= SSLWantRead;
            return errSSLWouldBlock;
        }
        assert(op == IceInternal::SocketOperationNone);
    }
    catch(const Ice::ConnectionLostException&)
    {
        return errSSLClosedGraceful;
    }
    catch(const Ice::SocketException& ex)
    {
        return ex.error;
    }
    catch(...)
    {
        assert(false);
        return IceInternal::getSocketErrno();
    }
    return noErr;
}

#endif
