// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;

    var allTests = function(out, communicator, communicator2)
    {
        var ref, base1, base2, retry1, retry2;

        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        Ice.Promise.try(() =>
            {
                out.write("testing stringToProxy... ");
                ref = "retry:default -p 12010";
                base1 = communicator.stringToProxy(ref);
                test(base1 !== null);
                base2 = communicator.stringToProxy(ref);
                test(base2 !== null);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Test.RetryPrx.checkedCast(base1);
            }
        ).then(obj =>
            {
                retry1 = obj;
                test(retry1 !== null);
                test(retry1.equals(base1));
                return Test.RetryPrx.checkedCast(base2);
            }
        ).then(obj =>
            {
                retry2 = obj;
                test(retry2 !== null);
                test(retry2.equals(base2));
                out.writeLine("ok");
                out.write("calling regular operation with first proxy... ");
                return retry1.op(false);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("calling operation to kill connection with second proxy... ");
                return retry2.op(true);
            }
        ).then(() => test(false),
               ex =>
            {
                if(typeof(window) === 'undefined' && typeof(WorkerGlobalScope) === 'undefined') // Nodejs
                {
                    test(ex instanceof Ice.ConnectionLostException);
                }
                else // Browser
                {
                    test(ex instanceof Ice.SocketException);
                }
                out.writeLine("ok");
                out.write("calling regular operation with first proxy again... ");
                return retry1.op(false);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing idempotent operation... ");
                return retry1.opIdempotent(4);
            }
        ).then(count =>
            {
                test(count === 4);
                out.writeLine("ok");
                out.write("testing non-idempotent operation... ");
                return retry1.opNotIdempotent();
            }
        ).then(() => test(false),
               ex =>
            {
                out.writeLine("ok");
                out.write("testing invocation timeout and retries... ");
                retry2 = Test.RetryPrx.uncheckedCast(communicator2.stringToProxy(retry1.toString()));
                return retry2.ice_invocationTimeout(500).opIdempotent(4);
            }
        ).then(() => test(false),
            function(ex)
            {
                test(ex instanceof Ice.InvocationTimeoutException);
                return retry2.opIdempotent(-1);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                return retry1.shutdown();
            }
        ).then(p.resolve, p.reject);
        return p;
    };

    var run = function(out, id)
    {
        //
        // For this test, we want to disable retries.
        //
        id.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // We don't want connection warnings because of the timeout
        //
        id.properties.setProperty("Ice.Warn.Connections", "0");
        var c = Ice.initialize(id);

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        var id2 = new Ice.InitializationData();
        id2.properties = c.getProperties().clone();
        id2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
        var c2 = Ice.initialize(id2);

        return Ice.Promise.try(() => allTests(out, c, c2)).finally(
            () =>
            {
                c2.destroy();
                return c.destroy();
            });
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
