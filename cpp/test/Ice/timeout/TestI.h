// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <TestCommon.h>

class TimeoutI : public virtual Test::Timeout
{
public:

    virtual void op(const Ice::Current&);
    virtual void sendData(ICE_IN(Test::ByteSeq), const Ice::Current&);
    virtual void sleep(Ice::Int, const Ice::Current&);
    virtual void holdAdapter(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
