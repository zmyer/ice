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

DEFINE_TEST("server")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    string endpt = getTestEndpoint(communicator, 0);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpt);
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyDerivedClassI), Ice::stringToIdentity("test"));
    adapter->activate();
    TEST_READY
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
    Ice::registerIceUDP(true);
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        //
        // Its possible to have batch oneway requests dispatched after
        // the adapter is deactivated due to thread scheduling so we
        // supress this warning.
        //
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
