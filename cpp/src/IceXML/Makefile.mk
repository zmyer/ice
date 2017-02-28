# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= IceXML

IceXML_targetdir	:= $(libdir)
IceXML_dependencies	:= Ice
IceXML_cppflags  	:= -DICE_XML_API_EXPORTS
IceXML_libs 		:= expat

projects += $(project)
