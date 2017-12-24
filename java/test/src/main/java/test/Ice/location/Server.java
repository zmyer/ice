// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

public class Server extends test.Util.Application
{
    private com.zeroc.Ice.InitializationData _initData;

    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();

        //
        // Register the server manager. The server manager creates a new
        // 'server' (a server isn't a different process, it's just a new
        // communicator and object adapter).
        //
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

        //
        // We also register a sample server locator which implements the
        // locator interface, this locator is used by the clients and the
        // 'servers' created with the server manager interface.
        //
        ServerLocatorRegistry registry = new ServerLocatorRegistry();
        registry.addObject(adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("ServerManager")), null);
        com.zeroc.Ice.Object object = new ServerManagerI(registry, _initData, this);
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("ServerManager"));

        com.zeroc.Ice.LocatorRegistryPrx registryPrx =
            com.zeroc.Ice.LocatorRegistryPrx.uncheckedCast(adapter.add(registry,
                com.zeroc.Ice.Util.stringToIdentity("registry")));

        ServerLocator locator = new ServerLocator(registry, registryPrx);
        adapter.add(locator, com.zeroc.Ice.Util.stringToIdentity("locator"));

        adapter.activate();

        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.location");
        initData.properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Server.SizeWarn", "0");
        initData.properties.setProperty("ServerManagerAdapter.Endpoints", getTestEndpoint(initData.properties, 0));

        _initData = initData;
        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
