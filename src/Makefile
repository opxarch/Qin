## @file
# The makefile for openwsp
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

SUB_DEPTH = ./..

include $(SUB_DEPTH)/autoconf.mk

#########################################################################
# Targets.

OBJS =  util/error.cpp.o                    \
		util/assert.cpp.o					\
        util/string.cpp.o                   \
        util/timer.cpp.o					\
		audiosys/audiosys_null.cpp.o		\
		audiosys/audiosys_dsound.cpp.o		\
		audiosys/audiosystem.cpp.o			\
		audiosys/audioformat.cpp.o			\
		mididev/mididev_winmm.cpp.o			\
		mididev/mididev.cpp.o				\
		mixer/mixer.cpp.o					\
		mixer/resampler.cpp.o				\
		wavetable/wavetable.cpp.o			\
		midi/note.cpp.o						\
		midi/mapping.cpp.o					\
		midi/ports.cpp.o					\
		midi/message.cpp.o					\
		memory/mmu.cpp.o					\
		effect/adsr.cpp.o					\
		effect/amplifier.cpp.o				\
		effect/filter.cpp.o					\
		effect/delay.cpp.o					\
		effect/inverter.cpp.o				\
		effect/effectors.cpp.o				\
        main.cpp.o							\
        
#LIBS += winmm

.PHONY: all clean subs qin2 subs_clean

all: subs qin2

clean: subs_clean generic_clean

#########################################################################
# Standard rules.

include $(SUB_DEPTH)/rules.generic.mk


#########################################################################
# Main rules.

qin2: qin2.$(.EXEC)

#
# openwsp console
#
qin2.$(.EXEC): $(OBJS)

#
# Generate the status code descriptors.
#
util/error.cpp.o: $(ROOT_DIR)/$(OUT_DIR)/errors-generated.h

$(ROOT_DIR)/$(OUT_DIR)/errors-generated.h: \
		util/errors.sed \
		$(ROOT_DIR)/src/include/util/error.h
        
	$(call run-command,$(SED) -f $< $(filter %.h,$^) > $@,"  SED      $@")

#
# Sub items
#
subs:

subs_clean:

