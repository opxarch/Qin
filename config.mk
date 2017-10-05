## @file
# Config of Qin2
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

include $(SUB_DEPTH)/autoconf.mk

#########################################################################
# Project name and version

appName = Qin 2 v2.1.0 (instrument/synthesizer)
appNameShort = Qin2
appVersionMajor = 0
appVersionMinor = 0
appVersionBuild = 1

#########################################################################
# Tools

GEN-VERSION := $(SUB_DEPTH)/scripts/gen-version.py

#########################################################################
# Project dirs

OUT_DIR = ./out
OUT_APIS_DIR = ./out/apis

#########################################################################
# Toolchain

ifeq ($(CONFIG_BUILD_HOST),y)
  GCC       := $(HOST_GCC)
  GXX       := $(HOST_GXX)
  AR        := $(HOST_AR)
  RANLIB    := $(HOST_RANLIB)
else # BUILD TARGET
  GCC       := $(TARGET_GCC)
  GXX       := $(TARGET_GXX)
  AR        := $(TARGET_AR)
  RANLIB    := $(TARGET_RANLIB)
endif

#########################################################################
# Platform librarys

ifeq ($(CONFIG_TARGET_OS),win32)
  LIBS      += ws2_32
  LIBS      += winmm
  LIBS      += kernel32
endif

#########################################################################
# SDK - SDL

ifneq ($(SDK_SDL_DIR),)
  INCS += $(SDK_SDL_DIR)/include
  LIB_DIR += $(SDK_SDL_DIR)/lib
endif
