## @file
# The root makefile of qin project
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
# Define the variables required by the standard rules - see the standard
# rules file (below) for details of these variables.

SUB_DEPTH = .

include config.mk

#########################################################################
# Export all variables.

#.EXPORT_ALL_VARIABLES:


.PHONY: all clean version

#########################################################################
# note! the following code required a unix shell.

all: $(OUT_DIR) version
	$(MAKE) all -C ./src

clean:
	$(MAKE) clean -C ./src
	-$(RM) -f out/*.$(.EXEC) out/*.$(.LIB) *.$(.DLIB) *.$(.SYSMOD) out/*.h

$(OUT_DIR):
	$(MKDIR) $@


#
# Generate version infomation
#
version: $(OUT_DIR)/version-generated.h

$(OUT_DIR)/version-generated.h:
	$(PYTHON) "$(GEN-VERSION)" \
       "$(appName)" "$(appNameShort)" \
       "$(appVersionMajor)" "$(appVersionMinor)" "$(appVersionBuild)" \
       $(OUT_DIR)/version-generated.h
