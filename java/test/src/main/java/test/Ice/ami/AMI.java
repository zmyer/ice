// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CancellationException;
import java.util.concurrent.CompletionException;

import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.CompressBatch;

import test.Ice.ami.Test.CloseMode;
import test.Ice.ami.Test.TestIntfPrx;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Ice.ami.Test.TestIntfException;
import test.Util.Application;

public class AMI
{
    private static void test(boolean b)
    {
        if(!b)
        {
            new Throwable().printStackTrace();
            //
            // Exceptions raised by callbacks are swallowed by CompletableFuture.
            //
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static class SentCallback extends Callback
    {
        SentCallback()
        {
            _thread = Thread.currentThread().getId();
        }

        public void sent(boolean ss)
        {
            //
            // The documented semantics for the sent callback states that the callback will be
            // invoked by the calling thread if the request was sent synchronously, otherwise
            // it will be invoked by a thread pool thread.
            //
            test(ss && _thread == Thread.currentThread().getId() ||
                 !ss && _thread != Thread.currentThread().getId());
            called();
        }

        long _thread;
    }

    static class SentAsyncCallback extends Callback
    {
        SentAsyncCallback(long thread)
        {
            _thread = thread;
        }

        public void sent(boolean ss)
        {
            //
            // For whenSentAsync(), the callback is always invoked from an Executor thread.
            //
            test(_thread == Thread.currentThread().getId());
            called();
        }

        long _thread;
    }

    public static void run(Application app, com.zeroc.Ice.Communicator communicator, boolean collocated, TestIntfPrx p,
                           TestIntfControllerPrx testController)
    {
        PrintWriter out = app.getWriter();

        out.print("testing begin/end invocation... ");
        out.flush();
        {
            java.util.Map<String, String> ctx = new java.util.HashMap<>();

            test(p.ice_isAAsync("::Test::TestIntf").join());
            test(p.ice_isAAsync("::Test::TestIntf", ctx).join());

            p.ice_pingAsync().join();
            p.ice_pingAsync(ctx).join();

            test(p.ice_idAsync().join().equals("::Test::TestIntf"));
            test(p.ice_idAsync(ctx).join().equals("::Test::TestIntf"));

            test(p.ice_idsAsync().join().length == 2);
            test(p.ice_idsAsync(ctx).join().length == 2);

            if(!collocated)
            {
                test(p.ice_getConnectionAsync().join() != null);
            }

            p.opAsync().join();
            p.opAsync(ctx).join();

            test(p.opWithResultAsync().join() == 15);
            test(p.opWithResultAsync(ctx).join() == 15);

            p.opWithUEAsync().whenComplete((result, ex) -> { test(ex != null && ex instanceof TestIntfException); });
            p.opWithUEAsync(ctx).whenComplete((result, ex) -> { test(ex != null && ex instanceof TestIntfException); });

            if(p.supportsFunctionalTests())
            {
                test(p.opBoolAsync(true).join());

                test(p.opByteAsync((byte)0xff).join() == (byte)0xff);

                test(p.opShortAsync(Short.MIN_VALUE).join() == Short.MIN_VALUE);

                test(p.opIntAsync(Integer.MIN_VALUE).join() == Integer.MIN_VALUE);

                test(p.opLongAsync(Long.MIN_VALUE).join() == Long.MIN_VALUE);

                test(p.opFloatAsync(3.14f).join() == 3.14f);

                test(p.opDoubleAsync(1.1E10).join() == 1.1E10);
            }
        }
        out.println("ok");

        out.print("testing local exceptions... ");
        out.flush();
        {
            TestIntfPrx indirect = p.ice_adapterId("dummy");

            indirect.opAsync().whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof com.zeroc.Ice.NoEndpointException);
                });

            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            if(p.ice_getConnection() != null)
            {
                com.zeroc.Ice.InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                com.zeroc.Ice.Communicator ic = app.initialize(initData);
                com.zeroc.Ice.ObjectPrx o = ic.stringToProxy(p.toString());
                TestIntfPrx p2 = TestIntfPrx.checkedCast(o);
                ic.destroy();

                try
                {
                    p2.opAsync();
                    test(false);
                }
                catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
                {
                    // Expected.
                }
            }
        }
        out.println("ok");

        out.print("testing sent callback... ");
        out.flush();
        {
            final SentCallback cb = new SentCallback();

            {
                CompletableFuture<Boolean> r = p.ice_isAAsync("");
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
            }

            {
                CompletableFuture<Void> r = p.ice_pingAsync();
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
            }

            {
                CompletableFuture<String> r = p.ice_idAsync();
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
            }

            {
                CompletableFuture<String[]> r = p.ice_idsAsync();
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
            }
        }

        out.print("testing sent async callback... ");
        out.flush();
        {
            //
            // Create an executor to use for dispatching completed futures.
            //
            java.util.concurrent.ExecutorService executor = java.util.concurrent.Executors.newSingleThreadExecutor();

            //
            // Determine the id of the executor's thread.
            //
            long executorThread = 0;
            try
            {
                executorThread = executor.submit(() -> { return Thread.currentThread().getId(); }).get();
            }
            catch(Exception ex)
            {
                test(false);
            }

            final SentAsyncCallback cb = new SentAsyncCallback(executorThread);

            {
                CompletableFuture<Boolean> r = p.ice_isAAsync("");
                Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    }, executor);
                cb.check();
            }

            {
                CompletableFuture<Void> r = p.ice_pingAsync();
                Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    }, executor);
                cb.check();
            }

            {
                CompletableFuture<String> r = p.ice_idAsync();
                Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    }, executor);
                cb.check();
            }

            {
                CompletableFuture<String[]> r = p.ice_idsAsync();
                Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    }, executor);
                cb.check();
            }

            executor.shutdown();
        }
        out.println("ok");

        out.print("testing batch requests with proxy... ");
        out.flush();
        {
            {
                CompletableFuture<Void> r = p.ice_batchOneway().ice_flushBatchRequestsAsync();
                test(Util.getInvocationFuture(r).sentSynchronously());
                test(Util.getInvocationFuture(r).isSent());
                test(Util.getInvocationFuture(r).isDone());
            }

            {
                final SentCallback cb = new SentCallback();
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = p.ice_batchOneway();
                b1.opBatch();
                b1.opBatch();
                CompletableFuture<Void> r = b1.ice_flushBatchRequestsAsync();
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
                test(Util.getInvocationFuture(r).isSent());
                test(r.isDone());
                test(p.waitForBatch(2));
            }

            if(p.ice_getConnection() != null)
            {
                final SentCallback cb = new SentCallback();
                test(p.opBatchCount() == 0);
                TestIntfPrx b1 = p.ice_batchOneway();
                b1.opBatch();
                b1.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                CompletableFuture<Void> r = b1.ice_flushBatchRequestsAsync();
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.sent(sentSynchronously);
                    });
                cb.check();
                test(Util.getInvocationFuture(r).isSent());
                test(r.isDone());
                test(p.waitForBatch(1));
            }
        }
        out.println("ok");

        if(p.ice_getConnection() != null)
        {
            out.print("testing batch requests with connection... ");
            out.flush();
            {
                {
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity())).
                        ice_batchOneway();
                    b1.opBatch();
                    b1.opBatch();
                    CompletableFuture<Void> r =
                        b1.ice_getConnection().flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent());
                    test(r.isDone());
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Exception.
                    //
                    final Callback cb = new Callback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity())).
                        ice_batchOneway();
                    b1.opBatch();
                    b1.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                    CompletableFuture<Void> r =
                        b1.ice_getConnection().flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex != null);
                            cb.called();
                        });
                    cb.check();
                    test(!Util.getInvocationFuture(r).isSent());
                    test(r.isDone());
                    test(p.opBatchCount() == 0);
                }
            }
            out.println("ok");

            out.print("testing batch requests with communicator... ");
            out.flush();
            {
                {
                    //
                    // 1 connection.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity())).
                        ice_batchOneway();
                    b1.opBatch();
                    b1.opBatch();
                    CompletableFuture<Void> r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent());
                    test(r.isDone());
                    test(p.waitForBatch(2));
                }

                {
                    //
                    // Exception - 1 connection.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(p.ice_getConnection().createProxy(p.ice_getIdentity())).
                        ice_batchOneway();
                    b1.opBatch();
                    b1.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                    CompletableFuture<Void> r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    test(r.isDone());
                    test(p.opBatchCount() == 0);
                }

                {
                    //
                    // 2 connections.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    TestIntfPrx b2 = TestIntfPrx.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b1.opBatch();
                    b2.opBatch();
                    b2.opBatch();
                    CompletableFuture<Void> r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    test(r.isDone());
                    test(p.waitForBatch(4));
                }

                {
                    //
                    // Exception - 2 connections - 1 failure.
                    //
                    // All connections should be flushed even if there are failures on some connections.
                    // Exceptions should not be reported.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    TestIntfPrx b2 = TestIntfPrx.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                    CompletableFuture<Void> r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    test(r.isDone());
                    test(p.waitForBatch(1));
                }

                {
                    //
                    // Exception - 2 connections - 2 failures.
                    //
                    // The sent callback should be invoked even if all connections fail.
                    //
                    final SentCallback cb = new SentCallback();
                    test(p.opBatchCount() == 0);
                    TestIntfPrx b1 = TestIntfPrx.uncheckedCast(
                        p.ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    TestIntfPrx b2 = TestIntfPrx.uncheckedCast(
                        p.ice_connectionId("2").ice_getConnection().createProxy(p.ice_getIdentity())).ice_batchOneway();
                    b2.ice_getConnection(); // Ensure connection is established.
                    b1.opBatch();
                    b2.opBatch();
                    b1.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                    b2.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
                    CompletableFuture<Void> r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                    Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                        {
                            test(ex == null);
                            cb.sent(sentSynchronously);
                        });
                    cb.check();
                    test(Util.getInvocationFuture(r).isSent()); // Exceptions are ignored!
                    test(r.isDone());
                    test(p.opBatchCount() == 0);
                }
            }
            out.println("ok");
        }

        out.print("testing future operations... ");
        out.flush();
        {
            {
                TestIntfPrx indirect = p.ice_adapterId("dummy");
                CompletableFuture<Void> r = indirect.opAsync();
                Util.getInvocationFuture(r).waitForCompleted();
                try
                {
                    r.join();
                    test(false);
                }
                catch(CompletionException ex)
                {
                    test(ex.getCause() instanceof com.zeroc.Ice.NoEndpointException);
                }

                testController.holdAdapter();
                InvocationFuture<Void> r1;
                InvocationFuture<Void> r2;
                try
                {
                    r1 = Util.getInvocationFuture(p.opAsync());
                    byte[] seq = new byte[10024];
                    while(true)
                    {
                        r2 = Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                        if(!r2.sentSynchronously())
                        {
                            break;
                        }
                    }

                    if(p.ice_getConnection() != null)
                    {
                        test(r1.sentSynchronously() && r1.isSent() && !r1.isDone() ||
                             !r1.sentSynchronously() && !r1.isDone());

                        test(!r2.sentSynchronously() && !r2.isDone());
                    }
                }
                finally
                {
                    testController.resumeAdapter();
                }

                r1.waitForSent();
                test(r1.isSent());

                r2.waitForSent();
                test(r2.isSent());

                r1.waitForCompleted();
                test(r1.isDone());

                r2.waitForCompleted();
                test(r2.isDone());

                test(r1.getOperation().equals("op"));
                test(r2.getOperation().equals("opWithPayload"));
            }

            {
                {
                    //
                    // Twoway
                    //
                    InvocationFuture<Void> r = Util.getInvocationFuture(p.ice_pingAsync());
                    test(r.getOperation().equals("ice_ping"));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p);
                    r.join();
                }

                {
                    //
                    // Oneway
                    //
                    TestIntfPrx p2 = p.ice_oneway();
                    InvocationFuture<Void> r = Util.getInvocationFuture(p2.ice_pingAsync());
                    test(r.getOperation().equals("ice_ping"));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p2);
                }

                {
                    //
                    // Batch request via proxy
                    //
                    TestIntfPrx p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    InvocationFuture<Void> r = Util.getInvocationFuture(p2.ice_flushBatchRequestsAsync());
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == p2);
                    r.join();
                }

                if(p.ice_getConnection() != null)
                {
                    //
                    // Batch request via connection
                    //
                    com.zeroc.Ice.Connection con = p.ice_getConnection();
                    TestIntfPrx p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    InvocationFuture<Void> r =
                        Util.getInvocationFuture(con.flushBatchRequestsAsync(CompressBatch.BasedOnProxy));
                    test(r.getConnection() == con);
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    r.join();

                    //
                    // Batch request via communicator
                    //
                    p2 = p.ice_batchOneway();
                    p2.ice_ping();
                    r = Util.getInvocationFuture(communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy));
                    test(r.getConnection() == null); // Expected
                    test(r.getCommunicator() == communicator);
                    test(r.getProxy() == null); // Expected
                    r.join();
                }
            }

            if(p.ice_getConnection() != null)
            {
                InvocationFuture<Void> r1 = null;
                InvocationFuture<String> r2 = null;

                testController.holdAdapter();
                try
                {
                    InvocationFuture<Void> r = null;
                    byte[] seq = new byte[10024];
                    for(int i = 0; i < 200; ++i) // 2MB
                    {
                        r = Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                    }

                    test(!r.isSent());

                    r1 = Util.getInvocationFuture(p.ice_pingAsync());
                    r2 = Util.getInvocationFuture(p.ice_idAsync());
                    r1.cancel(false);
                    r2.cancel(false);
                    try
                    {
                        r1.join();
                        test(false);
                    }
                    catch(CancellationException ex)
                    {
                    }
                    try
                    {
                        r2.join();
                        test(false);
                    }
                    catch(CancellationException ex)
                    {
                    }
                }
                finally
                {
                    testController.resumeAdapter();
                }
                p.ice_ping();
                //test(!r1.isSent() && r1.isDone());
                test(!r1.isSent());
                test(r1.isDone());
                test(!r2.isSent() && r2.isDone());

                testController.holdAdapter();
                try
                {
                    r1 = Util.getInvocationFuture(p.opAsync());
                    r2 = Util.getInvocationFuture(p.ice_idAsync());
                    r1.waitForSent();
                    r2.waitForSent();
                    r1.cancel(false);
                    r2.cancel(false);
                    try
                    {
                        r1.join();
                        test(false);
                    }
                    catch(CancellationException ex)
                    {
                    }
                    try
                    {
                        r2.join();
                        test(false);
                    }
                    catch(CancellationException ex)
                    {
                    }
                }
                finally
                {
                    testController.resumeAdapter();
                }
            }
        }
        out.println("ok");

        if(p.ice_getConnection() != null && p.supportsAMD())
        {
            out.print("testing graceful close connection with wait... ");
            out.flush();
            {
                //
                // Local case: begin a request, close the connection gracefully, and make sure it waits
                // for the request to complete.
                //
                com.zeroc.Ice.Connection con = p.ice_getConnection();
                Callback cb = new Callback();
                con.setCloseCallback(c -> cb.called());
                CompletableFuture<Void> f = p.sleepAsync(100);
                con.close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait); // Blocks until the request completes.
                try
                {
                    f.join(); // Should complete successfully.
                }
                catch(Throwable ex)
                {
                    test(false);
                }
                cb.check();
            }
            {
                //
                // Remote case.
                //
                byte[] seq = new byte[1024 * 10];

                //
                // Send multiple opWithPayload, followed by a close and followed by multiple opWithPaylod.
                // The goal is to make sure that none of the opWithPayload fail even if the server closes
                // the connection gracefully in between.
                //
                int maxQueue = 2;
                boolean done = false;
                while(!done && maxQueue < 50)
                {
                    done = true;
                    p.ice_ping();
                    java.util.List<InvocationFuture<Void>> results = new java.util.ArrayList<>();
                    for(int i = 0; i < maxQueue; ++i)
                    {
                        results.add(Util.getInvocationFuture(p.opWithPayloadAsync(seq)));
                    }
                    if(!Util.getInvocationFuture(p.closeAsync(CloseMode.GracefullyWithWait)).isSent())
                    {
                        for(int i = 0; i < maxQueue; i++)
                        {
                            InvocationFuture<Void> r = Util.getInvocationFuture(p.opWithPayloadAsync(seq));
                            results.add(r);
                            if(r.isSent())
                            {
                                done = false;
                                maxQueue *= 2;
                                break;
                            }
                        }
                    }
                    else
                    {
                        maxQueue *= 2;
                        done = false;
                    }
                    for(InvocationFuture<Void> q : results)
                    {
                        q.join();
                    }
                }
            }
            out.println("ok");

            out.print("testing graceful close connection without wait... ");
            out.flush();
            {
                //
                // Local case: start an operation and then close the connection gracefully on the client side
                // without waiting for the pending invocation to complete. There will be no retry and we expect the
                // invocation to fail with ConnectionManuallyClosedException.
                //
                p = p.ice_connectionId("CloseGracefully"); // Start with a new connection.
                com.zeroc.Ice.Connection con = p.ice_getConnection();
                CompletableFuture<Void> f = p.startDispatchAsync();
                Util.getInvocationFuture(f).waitForSent(); // Ensure the request was sent before we close the connection
                con.close(com.zeroc.Ice.ConnectionClose.Gracefully);
                try
                {
                    f.join();
                    test(false);
                }
                catch(CompletionException ex)
                {
                    test(ex.getCause() instanceof com.zeroc.Ice.ConnectionManuallyClosedException);
                    test(((com.zeroc.Ice.ConnectionManuallyClosedException)ex.getCause()).graceful);
                }
                catch(Throwable ex)
                {
                    test(false);
                }
                p.finishDispatch();

                //
                // Remote case: the server closes the connection gracefully, which means the connection
                // will not be closed until all pending dispatched requests have completed.
                //
                con = p.ice_getConnection();
                Callback cb = new Callback();
                con.setCloseCallback(c -> cb.called());
                f = p.sleepAsync(100);
                p.close(CloseMode.Gracefully); // Close is delayed until sleep completes.
                cb.check();
                f.join();
            }
            out.println("ok");

            out.print("testing forceful close connection... ");
            out.flush();
            {
                //
                // Local case: start an operation and then close the connection forcefully on the client side.
                // There will be no retry and we expect the invocation to fail with ConnectionManuallyClosedException.
                //
                p.ice_ping();
                com.zeroc.Ice.Connection con = p.ice_getConnection();
                CompletableFuture<Void> f = p.startDispatchAsync();
                Util.getInvocationFuture(f).waitForSent(); // Ensure the request was sent before we close the connection
                con.close(com.zeroc.Ice.ConnectionClose.Forcefully);
                try
                {
                    f.join();
                    test(false);
                }
                catch(CompletionException ex)
                {
                    test(ex.getCause() instanceof com.zeroc.Ice.ConnectionManuallyClosedException);
                    test(!((com.zeroc.Ice.ConnectionManuallyClosedException)ex.getCause()).graceful);
                }
                catch(Throwable ex)
                {
                    test(false);
                }
                p.finishDispatch();

                //
                // Remote case: the server closes the connection forcefully. This causes the request to fail
                // with a ConnectionLostException. Since the close() operation is not idempotent, the client
                // will not retry.
                //
                try
                {
                    p.close(CloseMode.Forcefully);
                    test(false);
                }
                catch(com.zeroc.Ice.ConnectionLostException ex)
                {
                    // Expected.
                }
            }
            out.println("ok");
        }
    }
}
