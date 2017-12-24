# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [IceGridTestCase()], multihost=False)
