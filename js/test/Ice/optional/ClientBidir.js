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
    var InitialI = require("InitialI").InitialI;
    var AMDInitialI = require("AMDInitialI").AMDInitialI;
    var Client = require("Client");

    var ArrayUtil = Ice.ArrayUtil;

    var allTests = function(out, communicator, amd)
    {
        var base;
        return Ice.Promise.try(() =>
            {
                base = communicator.stringToProxy("initial:default -p 12010");
                return communicator.createObjectAdapter("");
            }
        ).then(adapter =>
            {
                if(amd)
                {
                    adapter.add(new AMDInitialI(), Ice.stringToIdentity("initial"));
                }
                else
                {
                    adapter.add(new InitialI(), Ice.stringToIdentity("initial"));
                }
                return base.ice_getConnection().then(conn =>
                    {
                        conn.setAdapter(adapter);
                        return Client._clientAllTests(out, communicator, Test);
                    });
            });
    };

    var run = function(out, id)
    {
        var communicator = null;
        return Ice.Promise.try(() =>
            {
                communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with synchronous dispatch...");
                return allTests(out, communicator, false);
            }
        ).then(() => communicator.destroy()
        ).then(() =>
            {
                communicator = Ice.initialize(id);
                out.writeLine("testing bidir callbacks with asynchronous dispatch...");
                return allTests(out, communicator, true);
            }
        ).then(() => communicator.destroy()
        ).then(() =>
            {
                communicator = Ice.initialize(id);
                var base = communicator.stringToProxy("__echo:default -p 12010");
                return Test.EchoPrx.checkedCast(base);
            }
        ).then(prx => prx.shutdown()
        ).finally(() =>
            {
                if(communicator)
                {
                    return communicator.destroy();
                }
            });
    };
    exports._testBidir = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
