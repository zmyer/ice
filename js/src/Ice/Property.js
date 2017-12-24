// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

Ice.Property = class
{
    constructor(pattern, deprecated, deprecatedBy)
    {
        this._pattern = pattern;
        this._deprecated = deprecated;
        this._deprecatedBy = deprecatedBy;
    }

    get pattern()
    {
        return this._pattern;
    }

    get deprecated()
    {
        return this._deprecated;
    }

    get deprecatedBy()
    {
        return this._deprecatedBy;
    }
};

module.exports.Ice = Ice;
