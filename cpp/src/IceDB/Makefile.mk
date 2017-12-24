# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    = IceDB

IceDB_targetdir         := $(libdir)
IceDB_dependencies      := Ice
IceDB_libs              := lmdb
IceDB_cppflags          := -DICE_DB_API_EXPORTS
IceDB_devinstall        := no

projects += $(project)
