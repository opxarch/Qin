## @file
# rules definitions
#

#
#  Qin instrument (synthesizer)
#
#  Copyleft (C) 2017, The first Middle School in Yongsheng, Lijiang, China.
#  please contact with <diyer175@hotmail.com> if you have any problems.
#
#  This project is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public License(GPL)
#  as published by the Free Software Foundation; either version 2.1
#  of the License, or (at your option) any later version.
#
#  This project is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#

#########################################################################
# The following parameters should be set:
#       SUB_DEPTH:     The depth of sub dir.
# Optional:
#       OBJS           Specify the mid-stage object files.
#       DEPS           Specify the extra dependency rules files.
#
#       else consts from config.mak.
#


#########################################################################
# Sub dirs relative addressing

ROOT_DIR := $(SUB_DEPTH)


#########################################################################
# How to run a shell command
# @param    $(1)   Command string.
# @param    $(2)   Infomation to display if it's quiet.
# @return          GENERATED code.
#
ifeq ($(MAK_CONFIG_QUIET),y)
  define run-command
    @echo $(2)
    @$(1)
  endef
else
  define run-command
    $(1)
  endef
endif # !defined(MAK_CONFIG_QUIET)


#########################################################################
# Define the generic parametrs

INCS += $(SUB_DEPTH)/src ./ ./include
INCS += $(SUB_DEPTH)/$(OUT_DIR)
INCS += $(SUB_DEPTH) $(SUB_DEPTH)/include
DEFS +=
LIBS +=
LIB_DIR += $(SUB_DEPTH)/$(OUT_DIR)


#########################################################################
# Platform dependency

DEFS.x86     = ARCH_X86 ARCH_BITS=32
DEFS.amd64   = ARCH_AMD64 ARCH_BITS=64
DEFS.win32   = OS_WIN32
DEFS.linux   = OS_LINUX _FILE_OFFSET_BITS=64
DEFS.openbsd = OS_OPENBSD

DEFS += $(DEFS.$(CONFIG_TARGET_ARCH)) $(DEFS.$(CONFIG_TARGET_OS))


#########################################################################
# Dependency rules

DEPS += $(patsubst %.o,%.d,$(filter %.o,$(OBJS)))

-include *.d
-include $(DEPS)


#########################################################################
# Generic cleaning rules

.PHONY: generic_clean

generic_clean:
	-$(RM) -f *.o $(OBJS) *.d $(DEPS) *.$(.EXEC) *.$(.LIB) *.$(.DLIB) *.$(.SYSMOD)
