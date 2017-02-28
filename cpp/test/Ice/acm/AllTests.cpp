// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
namespace
{

string
toString(int value)
{
    ostringstream os;
    os << value;
    return os.str();
}

class LoggerI : public Ice::Logger,
                private IceUtil::Mutex
#ifdef ICE_CPP11_MAPPING
              , public std::enable_shared_from_this<LoggerI>
#endif

{
public:

    LoggerI() : _started(false)
    {
    }

    void
    start()
    {
        Lock sync(*this);
        _started = true;
        dump();
    }

    virtual void
    print(const std::string& msg)
    {
        Lock sync(*this);
        _messages.push_back(msg);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    trace(const std::string& category, const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("[" + category + "] " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    warning(const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("warning: " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual void
    error(const std::string& message)
    {
        Lock sync(*this);
        _messages.push_back("error: " + message);
        if(_started)
        {
            dump();
        }
    }

    virtual string
    getPrefix()
    {
        return "";
    }

    virtual Ice::LoggerPtr
    cloneWithPrefix(const std::string&)
    {
        return ICE_SHARED_FROM_THIS;
    }

private:

    void
    dump()
    {
        for(vector<string>::const_iterator p = _messages.begin(); p != _messages.end(); ++p)
        {
            cout << *p << endl;
        }
        _messages.clear();
    }

    bool _started;
    vector<string> _messages;
};
ICE_DEFINE_PTR(LoggerIPtr, LoggerI);

class TestCase :
#ifdef ICE_CPP11_MAPPING
                 public enable_shared_from_this<TestCase>,
#else
                 public IceUtil::Thread,
                 public Ice::CloseCallback, public Ice::HeartbeatCallback,
#endif
                 protected IceUtil::Monitor<IceUtil::Mutex>
{
public:

    TestCase(const string& name, const RemoteCommunicatorPrxPtr& com) :
        _name(name), _com(com), _logger(new LoggerI()),
        _clientACMTimeout(-1), _clientACMClose(-1), _clientACMHeartbeat(-1),
        _serverACMTimeout(-1), _serverACMClose(-1), _serverACMHeartbeat(-1),
        _heartbeat(0), _closed(false)
    {
    }

    void
    init()
    {
        _adapter = _com->createObjectAdapter(_serverACMTimeout, _serverACMClose, _serverACMHeartbeat);

        Ice::InitializationData initData;
        initData.properties = _com->ice_getCommunicator()->getProperties()->clone();
        initData.logger = _logger;
        initData.properties->setProperty("Ice.ACM.Timeout", "1");
        if(_clientACMTimeout >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Timeout", toString(_clientACMTimeout));
        }
        if(_clientACMClose >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Close", toString(_clientACMClose));
        }
        if(_clientACMHeartbeat >= 0)
        {
            initData.properties->setProperty("Ice.ACM.Client.Heartbeat", toString(_clientACMHeartbeat));
        }
        //initData.properties->setProperty("Ice.Trace.Protocol", "2");
        //initData.properties->setProperty("Ice.Trace.Network", "2");
        _communicator = Ice::initialize(initData);
    }

    void
    destroy()
    {
        _adapter->deactivate();
        _communicator->destroy();
    }

#ifdef ICE_CPP11_MAPPING
    void join(thread& t)
#else
    void join()
#endif
    {
        cout << "testing " << _name << "... " << flush;
        _logger->start();
#ifdef ICE_CPP11_MAPPING
        t.join();
#else
        getThreadControl().join();
#endif
        if(_msg.empty())
        {
            cout << "ok" << endl;
        }
        else
        {
            cout << "failed! " << endl << _msg;
            test(false);
        }
    }

    virtual void
    run()
    {
        TestIntfPrxPtr proxy = ICE_UNCHECKED_CAST(TestIntfPrx, _communicator->stringToProxy(
                                                           _adapter->getTestIntf()->ice_toString()));
        try
        {
#ifdef ICE_CPP11_MAPPING
            auto self = shared_from_this();
            proxy->ice_getConnection()->setCloseCallback(
                [self](Ice::ConnectionPtr connection)
                {
                    self->closed(move(connection));
                });
            proxy->ice_getConnection()->setHeartbeatCallback(
                [self](Ice::ConnectionPtr connection)
                {
                    self->heartbeat(move(connection));
                });
#else
            proxy->ice_getConnection()->setCloseCallback(ICE_SHARED_FROM_THIS);
            proxy->ice_getConnection()->setHeartbeatCallback(ICE_SHARED_FROM_THIS);
#endif
            runTestCase(_adapter, proxy);
        }
        catch(const std::exception& ex)
        {
            _msg = string("unexpected exception:\n") + ex.what();
        }
        catch(...)
        {
            _msg = "unknown exception";
        }
    }

    virtual void
    heartbeat(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        ++_heartbeat;
    }

    virtual void
    closed(const Ice::ConnectionPtr&)
    {
        Lock sync(*this);
        _closed = true;
        notify();
    }

    void
    waitForClosed()
    {
        Lock sync(*this);
        IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
        while(!_closed)
        {
            timedWait(IceUtil::Time::seconds(1));
            if(IceUtil::Time::now(IceUtil::Time::Monotonic) - now > IceUtil::Time::seconds(1))
            {
                test(false); // Waited for more than 1s for close, something's wrong.
            }
        }
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr&, const TestIntfPrxPtr&) = 0;

    void
    setClientACM(int timeout, int close, int heartbeat)
    {
        _clientACMTimeout = timeout;
        _clientACMClose = close;
        _clientACMHeartbeat = heartbeat;
    }

    void
    setServerACM(int timeout, int close, int heartbeat)
    {
        _serverACMTimeout = timeout;
        _serverACMClose = close;
        _serverACMHeartbeat = heartbeat;
    }

protected:

    const string _name;
    const RemoteCommunicatorPrxPtr _com;
    string _msg;
    LoggerIPtr _logger;

    Ice::CommunicatorPtr _communicator;
    RemoteObjectAdapterPrxPtr _adapter;

    int _clientACMTimeout;
    int _clientACMClose;
    int _clientACMHeartbeat;
    int _serverACMTimeout;
    int _serverACMClose;
    int _serverACMHeartbeat;

    int _heartbeat;
    bool _closed;
};
ICE_DEFINE_PTR(TestCasePtr, TestCase);

class InvocationHeartbeatTest : public TestCase
{
public:

    InvocationHeartbeatTest(const RemoteCommunicatorPrxPtr& com) :
        TestCase("invocation heartbeat", com)
    {
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        proxy->sleep(2);

        Lock sync(*this);
        test(_heartbeat >= 2);
    }
};

class InvocationHeartbeatOnHoldTest : public TestCase
{
public:

    InvocationHeartbeatOnHoldTest(const RemoteCommunicatorPrxPtr& com) :
        TestCase("invocation with heartbeat on hold", com)
    {
        // Use default ACM configuration.
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        try
        {
            // When the OA is put on hold, connections shouldn't
            // send heartbeats, the invocation should therefore
            // fail.
            proxy->sleepAndHold(10);
            test(false);
        }
        catch(const Ice::ConnectionTimeoutException&)
        {
            adapter->activate();
            proxy->interruptSleep();

            waitForClosed();
        }
    }
};

class InvocationNoHeartbeatTest : public TestCase
{
public:

    InvocationNoHeartbeatTest(const RemoteCommunicatorPrxPtr& com) :
        TestCase("invocation with no heartbeat", com)
    {
        setServerACM(1, 2, 0); // Disable heartbeat on invocations
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        try
        {
            // Heartbeats are disabled on the server, the
            // invocation should fail since heartbeats are
            // expected.
            proxy->sleep(10);
            test(false);
        }
        catch(const Ice::ConnectionTimeoutException&)
        {
            proxy->interruptSleep();

            waitForClosed();

            Lock sync(*this);
            test(_heartbeat == 0);
        }
    }
};

class InvocationHeartbeatCloseOnIdleTest : public TestCase
{
public:

    InvocationHeartbeatCloseOnIdleTest(const RemoteCommunicatorPrxPtr& com) :
        TestCase("invocation with no heartbeat and close on idle", com)
    {
        setClientACM(1, 1, 0); // Only close on idle.
        setServerACM(1, 2, 0); // Disable heartbeat on invocations
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        // No close on invocation, the call should succeed this
        // time.
        proxy->sleep(2);

        Lock sync(*this);
        test(_heartbeat == 0);
        test(!_closed);
    }
};

class CloseOnIdleTest : public TestCase
{
public:

    CloseOnIdleTest(const RemoteCommunicatorPrxPtr& com) : TestCase("close on idle", com)
    {
        setClientACM(1, 1, 0); // Only close on idle
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

        waitForClosed();

        Lock sync(*this);
        test(_heartbeat == 0);
    }
};

class CloseOnInvocationTest : public TestCase
{
public:

    CloseOnInvocationTest(const RemoteCommunicatorPrxPtr& com) : TestCase("close on invocation", com)
    {
        setClientACM(1, 2, 0); // Only close on invocation
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

        Lock sync(*this);
        test(_heartbeat == 0);
        test(!_closed);
    }
};

class CloseOnIdleAndInvocationTest : public TestCase
{
public:

    CloseOnIdleAndInvocationTest(const RemoteCommunicatorPrxPtr& com) : TestCase("close on idle and invocation", com)
    {
        setClientACM(1, 3, 0); // Only close on idle and invocation
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        //
        // Put the adapter on hold. The server will not respond to
        // the graceful close. This allows to test whether or not
        // the close is graceful or forceful.
        //
        adapter->hold();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

        {
            Lock sync(*this);
            test(_heartbeat == 0);
            test(!_closed); // Not closed yet because of graceful close.
        }

        adapter->activate();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));

        waitForClosed();
    }
};

class ForcefulCloseOnIdleAndInvocationTest : public TestCase
{
public:

    ForcefulCloseOnIdleAndInvocationTest(const RemoteCommunicatorPrxPtr& com) :
        TestCase("forceful close on idle and invocation", com)
    {
        setClientACM(1, 4, 0); // Only close on idle and invocation
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        adapter->hold();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1500)); // Idle for 1.5 seconds

        waitForClosed();

        Lock sync(*this);
        test(_heartbeat == 0);
    }
};

class HeartbeatOnIdleTest : public TestCase
{
public:

    HeartbeatOnIdleTest(const RemoteCommunicatorPrxPtr& com) : TestCase("heartbeat on idle", com)
    {
        setServerACM(1, -1, 2); // Enable server heartbeats.
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(2000));

        Lock sync(*this);
        test(_heartbeat >= 3);
    }
};

class HeartbeatAlwaysTest : public TestCase
{
public:

    HeartbeatAlwaysTest(const RemoteCommunicatorPrxPtr& com) : TestCase("heartbeat always", com)
    {
        setServerACM(1, -1, 3); // Enable server heartbeats.
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        for(int i = 0; i < 12; ++i)
        {
            proxy->ice_ping();
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }

        Lock sync(*this);
        test(_heartbeat >= 3);
    }
};

class HeartbeatManualTest : public TestCase
{
public:

    HeartbeatManualTest(const RemoteCommunicatorPrxPtr& com) : TestCase("manual heartbeats", com)
    {
        //
        // Disable heartbeats.
        //
        setClientACM(10, -1, 0);
        setServerACM(10, -1, 0);
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        proxy->startHeartbeatCount();
        Ice::ConnectionPtr con = proxy->ice_getConnection();
        con->heartbeat();
        con->heartbeat();
        con->heartbeat();
        con->heartbeat();
        con->heartbeat();
        proxy->waitForHeartbeatCount(5);
    }
};

class SetACMTest : public TestCase
{
public:

    SetACMTest(const RemoteCommunicatorPrxPtr& com) : TestCase("setACM/getACM", com)
    {
        setClientACM(15, 4, 0);
    }

    virtual void runTestCase(const RemoteObjectAdapterPrxPtr& adapter, const TestIntfPrxPtr& proxy)
    {
        Ice::ACM acm;
        acm = proxy->ice_getCachedConnection()->getACM();
        test(acm.timeout == 15);
        test(acm.close == Ice::ICE_ENUM(ACMClose, CloseOnIdleForceful));
        test(acm.heartbeat == Ice::ICE_ENUM(ACMHeartbeat, HeartbeatOff));

        proxy->ice_getCachedConnection()->setACM(IceUtil::None, IceUtil::None, IceUtil::None);
        acm = proxy->ice_getCachedConnection()->getACM();
        test(acm.timeout == 15);
        test(acm.close == Ice::ICE_ENUM(ACMClose, CloseOnIdleForceful));
        test(acm.heartbeat == Ice::ICE_ENUM(ACMHeartbeat, HeartbeatOff));

        proxy->ice_getCachedConnection()->setACM(1, Ice::ICE_ENUM(ACMClose, CloseOnInvocationAndIdle),
                                                 Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));
        acm = proxy->ice_getCachedConnection()->getACM();
        test(acm.timeout == 1);
        test(acm.close == Ice::ICE_ENUM(ACMClose, CloseOnInvocationAndIdle));
        test(acm.heartbeat == Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

        // Make sure the client sends a few heartbeats to the server.
        proxy->startHeartbeatCount();
        proxy->waitForHeartbeatCount(2);
    }
};

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "communicator:" + getTestEndpoint(communicator, 0);
    RemoteCommunicatorPrxPtr com = ICE_UNCHECKED_CAST(RemoteCommunicatorPrx, communicator->stringToProxy(ref));

    vector<TestCasePtr> tests;

    tests.push_back(ICE_MAKE_SHARED(InvocationHeartbeatTest, com));
    tests.push_back(ICE_MAKE_SHARED(InvocationHeartbeatOnHoldTest, com));
    tests.push_back(ICE_MAKE_SHARED(InvocationNoHeartbeatTest, com));
    tests.push_back(ICE_MAKE_SHARED(InvocationHeartbeatCloseOnIdleTest, com));

    tests.push_back(ICE_MAKE_SHARED(CloseOnIdleTest, com));
    tests.push_back(ICE_MAKE_SHARED(CloseOnInvocationTest, com));
    tests.push_back(ICE_MAKE_SHARED(CloseOnIdleAndInvocationTest, com));
    tests.push_back(ICE_MAKE_SHARED(ForcefulCloseOnIdleAndInvocationTest, com));

    tests.push_back(ICE_MAKE_SHARED(HeartbeatOnIdleTest, com));
    tests.push_back(ICE_MAKE_SHARED(HeartbeatAlwaysTest, com));
    tests.push_back(ICE_MAKE_SHARED(HeartbeatManualTest, com));
    tests.push_back(ICE_MAKE_SHARED(SetACMTest, com));

    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->init();
    }

#ifdef ICE_CPP11_MAPPING
    vector<pair<thread, TestCasePtr>> threads;
    for(auto p = tests.begin(); p != tests.end(); ++p)
    {
        TestCasePtr testCase = *p;
        thread t([testCase]()
            {
                testCase->run();
            });
        threads.push_back(make_pair(move(t), testCase));
    }
    for(auto p = threads.begin(); p != threads.end(); ++p)
    {
        p->second->join(p->first);
    }
#else
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->start();
    }
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->join();
    }
#endif
    for(vector<TestCasePtr>::const_iterator p = tests.begin(); p != tests.end(); ++p)
    {
        (*p)->destroy();
    }

    cout << "shutting down... " << flush;
    com->shutdown();
    cout << "ok" << endl;
}
