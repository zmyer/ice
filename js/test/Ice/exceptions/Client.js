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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    async function allTests(out, communicator, Test)
    {
        class EmptyI extends Test.Empty
        {
        }

        class ServantLocatorI
        {
            locate(curr, cookie)
            {
                return null;
            }

            finished(curr, servant, cookie)
            {
            }

            deactivate(category)
            {
            }
        }

        function test(value, ex)
        {
            if(!value)
            {
                let message = "test failed";
                if(ex)
                {
                    message += "\n" + ex.toString();
                }
                throw new Error(message);
            }
        }

        out.write("testing object adapter registration exceptions... ");
        try
        {
            await communicator.createObjectAdapter("TestAdapter0");
            test(false)
        }
        catch(ex)
        {
            test(ex instanceof Ice.InitializationException, ex); // Expected
        }

        try
        {
            await communicator.createObjectAdapterWithEndpoints("TestAdapter0", "default");
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.FeatureNotSupportedException, ex); // Expected
        }
        out.writeLine("ok");

        out.write("testing servant registration exceptions... ");
        {
            let adapter = await communicator.createObjectAdapter("");
            adapter.add(new EmptyI(), Ice.stringToIdentity("x"));
            try
            {
                adapter.add(new EmptyI(), Ice.stringToIdentity("x"));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.AlreadyRegisteredException, ex);
            }

            try
            {
                adapter.add(new EmptyI(), Ice.stringToIdentity(""));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.IllegalIdentityException, ex);
                test(ex.id.name === "");
            }

            try
            {
                adapter.add(null, Ice.stringToIdentity("x"));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.IllegalServantException, ex);
            }

            adapter.remove(Ice.stringToIdentity("x"));
            try
            {
                adapter.remove(Ice.stringToIdentity("x"));
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.NotRegisteredException, ex);
            }
            adapter.deactivate();
        }
        out.writeLine("ok");

        out.write("testing servant locator registration exceptions... ");
        {
            let adapter = await communicator.createObjectAdapter("");
            adapter.addServantLocator(new ServantLocatorI(), "x");
            try
            {
                adapter.addServantLocator(new ServantLocatorI(), "x");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.AlreadyRegisteredException, ex);
            }
            adapter.deactivate();
            out.writeLine("ok");

            out.write("testing value factory registration exception... ");
            communicator.getValueFactoryManager().add(() => null, "::x");
            try
            {
                communicator.getValueFactoryManager().add(() => null, "::x");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.AlreadyRegisteredException, ex);
            }
        }
        out.writeLine("ok");

        out.write("testing stringToProxy... ");
        let ref = "thrower:default -p 12010";
        let base = communicator.stringToProxy(ref);
        test(base !== null);
        out.writeLine("ok");

        out.write("testing checked cast... ");
        let thrower = await Test.ThrowerPrx.checkedCast(base);
        test(thrower !== null);
        test(thrower.equals(base));
        out.writeLine("ok");

        out.write("catching exact types... ");
        try
        {
            await thrower.throwAasA(1);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.A, ex);
            test(ex.aMem === 1);
        }

        try
        {
            await thrower.throwAorDasAorD(1);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.A, ex);
            test(ex.aMem === 1);
        }

        try
        {
            await thrower.throwAorDasAorD(-1);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.D, ex);
            test(ex.dMem === -1);
        }

        try
        {
            await thrower.throwBasB(1, 2);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.B, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }

        try
        {
            await thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.C, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        out.writeLine("ok");

        out.write("catching base types... ");
        try
        {
            await thrower.throwBasB(1, 2);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.A, ex);
            test(ex.aMem == 1);
        }

        try
        {
            await thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.B, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        out.writeLine("ok");

        out.write("catching derived types... ");
        try
        {
            await thrower.throwBasA(1, 2);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.B, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }

        try
        {
            await thrower.throwCasA(1, 2, 3);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.C, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }

        try
        {
            await thrower.throwCasB(1, 2, 3);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.C, ex);
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        out.writeLine("ok");

        if(await thrower.supportsUndeclaredExceptions())
        {
            out.write("catching unknown user exception... ");
            try
            {
                await thrower.throwUndeclaredA(1);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnknownUserException, ex);
            }

            try
            {
                await thrower.throwUndeclaredB(1, 2);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnknownUserException, ex);
            }

            try
            {
                await thrower.throwUndeclaredC(1, 2, 3);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnknownUserException, ex);
            }
            out.writeLine("ok");
        }

        if(await thrower.supportsAssertException())
        {
            out.write("testing assert in the server... ");
            try
            {
                await thrower.throwAssertException();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ConnectionLostException ||
                     ex instanceof Ice.UnknownException, ex);
            }
            out.writeLine("ok");
        }

        out.write("testing memory limit marshal exception...");
        try
        {
            await thrower.throwMemoryLimitException(null);
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.MemoryLimitException, ex);
        }

        try
        {
            await thrower.throwMemoryLimitException(new Uint8Array(20 * 1024));
            test(false);
        }
        catch(ex)
        {
            test(ex.toString().indexOf("ConnectionLostException") > 0, ex);
        }
        out.writeLine("ok");

        let retries = 5;
        while(--retries > 0)
        {
            // The above test can cause a close connection between the echo server and
            // bidir server, we need to wait until the bidir server has reopen the
            // connection with the echo server.

            try
            {
                await thrower.ice_ping();
                break;
            }
            catch(ex)
            {
                if(ex instanceof Ice.ObjectNotExistException && retries > 0)
                {
                    await Ice.Promise.delay(20);
                }
                else
                {
                    throw ex;
                }
            }
        }

        out.write("catching object not exist exception... ");
        try
        {
            let thrower2 = Test.ThrowerPrx.uncheckedCast(
                thrower.ice_identity(Ice.stringToIdentity("does not exist")));
            await thrower2.ice_ping();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.ObjectNotExistException, ex);
            test(ex.id.equals(Ice.stringToIdentity("does not exist")));
        }
        out.writeLine("ok");

        out.write("catching facet not exist exception... ");
        try
        {
            let thrower2 = Test.ThrowerPrx.uncheckedCast(thrower, "no such facet");
            await thrower2.ice_ping();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.FacetNotExistException, ex);
            test(ex.facet == "no such facet");
        }
        out.writeLine("ok");

        out.write("catching operation not exist exception... ");
        try
        {
            let thrower2 = Test.WrongOperationPrx.uncheckedCast(thrower);
            await thrower2.noSuchOperation();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.OperationNotExistException, ex);
            test(ex.operation == "noSuchOperation");
        }
        out.writeLine("ok");

        out.write("catching unknown local exception... ");
        try
        {
            await thrower.throwLocalException();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.UnknownLocalException, ex);
        }

        try
        {
            await thrower.throwLocalExceptionIdempotent();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.UnknownLocalException ||
                 ex instanceof Ice.OperationNotExistException, ex);
        }
        out.writeLine("ok");

        out.write("catching unknown non-Ice exception... ");
        try
        {
            await thrower.throwNonIceException();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Ice.UnknownException, ex);
        }
        out.writeLine("ok");

        out.write("testing asynchronous exceptions... ");
        await thrower.throwAfterResponse();
        try
        {
            await thrower.throwAfterException();
            test(false);
        }
        catch(ex)
        {
            test(ex instanceof Test.A, ex);
        }
        out.writeLine("ok");

        await thrower.shutdown();
    }

    async function run(out, initData)
    {
        let communicator
        try
        {
            initData.properties.setProperty("Ice.MessageSizeMax", "10");
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            initData.properties.setProperty("Ice.PrintStackTraces", "1");
            communicator = Ice.initialize(initData);
            await allTests(out, communicator, Test);
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
