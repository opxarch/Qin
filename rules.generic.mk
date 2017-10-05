## @file
# Rules - generic
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
# Include generic part

include $(SUB_DEPTH)/config.mk
include $(SUB_DEPTH)/rules.mk


#########################################################################
# Define the generic rules

CC := $(GCC)
CXX := $(GXX)

DEFS += IN_RING3
CFLAGS += -O1 -Wall -Wstrict-prototypes -Wno-unused-variable
CXXFLAGS += -O1 -Wall -Wno-unused-variable
LDFLAGS += -Wl,-subsystem,console -lstdc++
ARFLAGS +=

CFLAGS   += $(addprefix -I, $(CINCS) $(CXXINCS) $(INCS)) $(addprefix -D, $(DEFS))
CXXFLAGS += $(addprefix -I, $(CINCS) $(CXXINCS) $(INCS)) $(addprefix -D, $(DEFS))
LDFLAGS  += $(addprefix -L, $(LIB_DIR)) $(addprefix -l, $(LIBS))


#########################################################################
# building files

%.s.o: %.s
	$(call run-command,$(CC) $(CFLAGS) -c $< -o $@,"  CC        $@")

%.s.d: %.s
	$(call run-command,$(CC) $(CFLAGS) -MM -MF $@ $<,"  DEP       $@")
	-@sed -i 's,\($(notdir $*)\)\.o[ :]*,$<.o: ,g' '$@'

%.c.o: %.c
	$(call run-command,$(CC) $(CFLAGS) -c $< -o $@,"  CC        $@")

%.c.d: %.c
	$(call run-command,$(CC) $(CFLAGS) -MM -MF $@ $<,"  DEP       $@")
	-@sed -i 's,\($(notdir $*)\)\.o[ :]*,$<.o: ,g' '$@'

%.cpp.o: %.cpp
	$(call run-command,$(CXX) $(CXXFLAGS) -c $< -o $@,"  CXX       $@")

%.cpp.d: %.cpp
	$(call run-command,$(CXX) $(CXXFLAGS) -MM -MF $@ $<,"  DEP      $@")
	-@sed -i 's,\($(notdir $*)\)\.o[ :]*,$<.o: ,g' '$@'

%.$(.EXEC):
	$(call run-command,$(CC) $(filter %.o %.a,$^) $(LDFLAGS) -o $@,"  LINK      $@")
	@$(MV) $@ $(SUB_DEPTH)/$(OUT_DIR)/$@

%.$(.LIB):
	$(call run-command,$(AR) $(ARFLAGS) $@ $(filter %.o,$^) > /dev/null,"  AR        $@")
	$(RANLIB) $@
	@$(MV) $@ $(SUB_DEPTH)/$(OUT_DIR)/$@
