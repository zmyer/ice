// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Test;

public class TestI : TestIntfDisp_
{
    protected static void test(bool b)
    {
        if(!b)
        {
            Debug.Assert(false);
            throw new Exception();
        }
    }

    public TestI()
    {
    }

    override public void
    op(Ice.Current current)
    {
    }

    override public int
    opWithResult(Ice.Current current)
    {
        return 15;
    }

    override public void
    opWithUE(Ice.Current current)
    {
        throw new TestIntfException();
    }

    override public void
    opWithPayload(byte[] seq, Ice.Current current)
    {
    }

    override public void
    opBatch(Ice.Current current)
    {
        lock(this)
        {
            ++_batchCount;
            System.Threading.Monitor.Pulse(this);
        }
    }

    override public int
    opBatchCount(Ice.Current current)
    {
        lock(this)
        {
            return _batchCount;
        }
    }

    override public bool
    waitForBatch(int count, Ice.Current current)
    {
        lock(this)
        {
            while(_batchCount < count)
            {
                System.Threading.Monitor.Wait(this, 10000);
            }
            bool result = count == _batchCount;
            _batchCount = 0;
            return result;
        }
    }

    override public void
    close(CloseMode mode, Ice.Current current)
    {
        current.con.close((Ice.ConnectionClose)((int)mode));
    }

    override public void
    sleep(int ms, Ice.Current current)
    {
        Thread.Sleep(ms);
    }

    override public void
    shutdown(Ice.Current current)
    {
        lock(this)
        {
            _shutdown = true;
            if(_pending != null)
            {
                _pending.SetResult(null);
                _pending  = null;
            }
            current.adapter.getCommunicator().shutdown();
        }
    }

    override public bool
    supportsAMD(Ice.Current current)
    {
        return true;
    }

    override public bool
    supportsFunctionalTests(Ice.Current current)
    {
        return false;
    }

    override public async Task
    opAsyncDispatchAsync(Ice.Current current)
    {
        await System.Threading.Tasks.Task.Delay(10);
    }

    override public async Task<int>
    opWithResultAsyncDispatchAsync(Ice.Current current)
    {
        await System.Threading.Tasks.Task.Delay(10);
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
        var r = await self(current).opWithResultAsync();
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
        return r;
    }

    override public async Task
    opWithUEAsyncDispatchAsync(Ice.Current current)
    {
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
        await System.Threading.Tasks.Task.Delay(10);
        test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
        await self(current).opWithUEAsync();
    }

    override public void
    pingBiDir(Ice.Identity id, Ice.Current current)
    {
        PingReplyPrx p = PingReplyPrxHelper.uncheckedCast(current.con.createProxy(id));
        p.replyAsync().ContinueWith(
            (t) =>
            {
                test(Thread.CurrentThread.Name.Contains("Ice.ThreadPool.Server"));
            },
            p.ice_scheduler()).Wait();
    }

    TestIntfPrx
    self(Ice.Current current)
    {
        return TestIntfPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
    }

    override public Task
    startDispatchAsync(Ice.Current current)
    {
        lock(this)
        {
            if(_shutdown)
            {
                // Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
                // before start dispatch.
                var v = new TaskCompletionSource<object>();
                v.SetResult(null);
                return v.Task;
            }
            else if(_pending != null)
            {
                _pending.SetResult(null);
            }
            _pending = new TaskCompletionSource<object>();
            return _pending.Task;
        }
    }

    override public void
    finishDispatch(Ice.Current current)
    {
        lock(this)
        {
            if(_shutdown)
            {
                return;
            }
            else if(_pending != null) // Pending might not be set yet if startDispatch is dispatch out-of-order
            {
                _pending.SetResult(null);
                _pending  = null;
            }
        }
    }

    private int _batchCount;
    private bool _shutdown;
    private TaskCompletionSource<object> _pending = null;
}

public class TestII : Test.Outer.Inner.TestIntfDisp_
{
    override public int
    op(int i, out int j, Ice.Current current)
    {
        j = i;
        return i;
    }
}

public class TestControllerI : TestIntfControllerDisp_
{
    override public void
    holdAdapter(Ice.Current current)
    {
        _adapter.hold();
    }

    override public void
    resumeAdapter(Ice.Current current)
    {
        _adapter.activate();
    }

    public
    TestControllerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    private Ice.ObjectAdapter _adapter;
}
