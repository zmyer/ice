// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OBJECT_ADAPTER_H
#define ICEPY_OBJECT_ADAPTER_H

#include <Python.h>
#include <Ice/ObjectAdapterF.h>

namespace IcePy
{

extern PyTypeObject ObjectAdapterType;

bool initObjectAdapter(PyObject*);

PyObject* createObjectAdapter(const Ice::ObjectAdapterPtr&);

}

#endif
