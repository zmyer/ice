// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBT;

import java.util.UUID;

final class ConnectorI implements IceInternal.Connector
{
    @Override
    public IceInternal.Transceiver connect()
    {
        return new TransceiverI(_instance, _addr, _uuid, _connectionId);
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String toString()
    {
        StringBuffer buf = new StringBuffer();
        if(!_addr.isEmpty())
        {
            buf.append(_addr);
        }
        if(_uuid != null)
        {
            if(!_addr.isEmpty())
            {
                buf.append(';');
            }
            buf.append(_uuid.toString());
        }
        return buf.toString();
    }

    @Override
    public int hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, String addr, UUID uuid, int timeout, String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _uuid = uuid;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _uuid);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _timeout);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    @Override
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof ConnectorI))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        ConnectorI p = (ConnectorI)obj;
        if(!_uuid.equals(p._uuid))
        {
            return false;
        }

        if(_timeout != p._timeout)
        {
            return false;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return false;
        }

        return _addr.equals(p._addr);
    }

    private Instance _instance;
    private String _addr;
    private UUID _uuid;
    private int _timeout;
    private String _connectionId;
    private int _hashCode;
}
