// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <SlicingExceptionsTestClient.h>

static int
run(id<ICECommunicator> communicator)
{
    id<TestSlicingExceptionsClientTestIntfPrx> slicingExceptionsAllTests(id<ICECommunicator>);
    id<TestSlicingExceptionsClientTestIntfPrx> thrower = slicingExceptionsAllTests(communicator);
    [thrower shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main slicingExceptionsClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

   int status;
   @autoreleasepool
   {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultClientProperties(&argc, argv);
#if TARGET_OS_IPHONE
        initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @"TestSlicingExceptionsClient", @"::Test",
                                  nil];
#endif
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
