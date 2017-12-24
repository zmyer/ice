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
#include <TestI.h>
#include <Dispatcher.h>

DEFINE_TEST("collocated")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(communicator, 1, "tcp"));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    TestIntfControllerIPtr testController = ICE_MAKE_SHARED(TestIntfControllerI, adapter);

    adapter->add(ICE_MAKE_SHARED(TestIntfI), Ice::stringToIdentity("test"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    adapter2->add(testController, Ice::stringToIdentity("testController"));
    //adapter2->activate(); // Don't activate OA to ensure collocation is used.

    void allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
#endif
    int status;
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
#ifdef ICE_CPP11_MAPPING
        IceUtil::Handle<Dispatcher> dispatcher = new Dispatcher;
        initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
            {
                dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
            };
#else
        initData.dispatcher = new Dispatcher();
#endif
        Ice::CommunicatorHolder ich(argc, argv, initData);
        status = run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }
    Dispatcher::terminate();
    return status;
}
