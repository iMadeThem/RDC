#
# Makefile for 'CmdDLSR'.
#
# Type 'make' or 'make CmdDLSR' to create the binary.
# Type 'make clean' or 'make clear' to delete all temporaries.
# Type 'make run' to execute the binary.
# Type 'make debug' to debug the binary using gdb(1).
#

# build target specs
CC = g++
CFLAGS = -Wall -W
INCPATH = -I. -I/System/Library/Frameworks/ImageCaptureCore.framework/Headers
OUT_DIR = release_build
LIBS = -framework ImageCaptureCore

SOURCES = PTPCamera.cpp \
	PTPUtil.cpp

OBJECTS =
# first target entry is the target invoked when typing 'make'
default: CmdDLSR
	@test -d $(OUT_DIR) || mkdir -p $(OUT_DIR)

CmdDLSR: $(OUT_DIR)/PTPCamera.cpp.o $(OUT_DIR)/PTPUtil.cpp.o $(OUT_DIR)/RDCCamera.cpp.o
	@echo -n 'Linking CmdDLSR... '
	@$(CC) $(CFLAGS) $(INCPATH) -o CmdDLSR $(OUT_DIR)/PTPCamera.cpp.o $(OUT_DIR)/PTPUtil.cpp.o $(OUT_DIR)/RDCCamera.cpp.o $(LIBS)
	@echo Done.

$(OUT_DIR)/PTPCamera.cpp.o: PTPCamera.cpp PTPCamera.h PTPUtil.h RDCCamera.h
	@echo -n 'Compiling PTPCamera.cpp... '
	@$(CC) $(CFLAGS) $(INCPATH) -o $(OUT_DIR)/PTPCamera.cpp.o -c PTPCamera.cpp
	@echo Done.

$(OUT_DIR)/PTPUtil.cpp.o: PTPUtil.cpp PTPUtil.h
	@echo -n 'Compiling PTPUtil.cpp... '
	@$(CC) $(CFLAGS) $(INCPATH) -o $(OUT_DIR)/PTPUtil.cpp.o -c PTPUtil.cpp
	@echo Done.

$(OUT_DIR)/RDCCamera.cpp.o: RDCCamera.cpp RDCCamera.h MacICACamera.h PTPCamera.h
	@echo -n 'Compiling RDCCamera.cpp... '
	@$(CC) $(CFLAGS) $(INCPATH) -o $(OUT_DIR)/RDCCamera.cpp.o -c RDCCamera.cpp
	@echo Done.

run:
	./CmdDLSR

debug:
	gdb ./CmdDLSR

clean:
	@echo -n 'Removing all temporary binaries... '
	@rm -f CmdDLSR $(OUT_DIR)/*.o
	@echo Done.

clear:
	@echo -n 'Removing all temporary binaries... '
	@rm -f CmdDLSR $(OUT_DIR)/*.o
	@echo Done.

