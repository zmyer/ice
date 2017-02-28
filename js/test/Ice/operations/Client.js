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

    var Twoways = require("Twoways").Twoways;
    var Oneways = require("Oneways").Oneways;
    var BatchOneways = require("BatchOneways").BatchOneways;

    var allTests = function(out, communicator, Test, bidir)
    {
        var ref, base, cl, derived;

        return Ice.Promise.try(() =>
            {
                out.write("testing twoway operations... ");
                ref = "test:default -p 12010";
                base = communicator.stringToProxy(ref);
                return Test.MyClassPrx.checkedCast(base);
            }
        ).then(prx =>
            {
                cl = prx;
                return Test.MyDerivedClassPrx.checkedCast(cl);
            }
        ).then(prx =>
            {
                derived = prx;
                return Twoways.run(communicator, cl, Test, bidir);
            }
        ).then(() => Twoways.run(communicator, derived, Test, bidir)
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing oneway operations... ");
                return Oneways.run(communicator, cl, Test, bidir);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                out.write("testing batch oneway operations... ");
                return BatchOneways.run(communicator, cl, Test, bidir);
            }
        ).then(() =>
            {
                out.writeLine("ok");
                return cl;
            });
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        var c = Ice.initialize(id);
        return Ice.Promise.try(() => allTests(out, c, Test, false)
            ).then(cl => cl.shutdown()
            ).finally(() => c.destroy().then(() => c.destroy()).then(() => c.destroy())); // Test multiple destroy calls
    };
    exports._test = run;
    exports._clientAllTests = allTests;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
