# PH-11 Test Makefile
# IEC 60730 Class B / MISRA-C:2012

CC = "/c/Program Files/LLVM/bin/clang"
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -DHOST_TEST
INCLUDES = -I../src -Iunity -I.
SRCDIR = ../src

# Source modules (excluding hal.c and main.c)
SRC_MODULES = \
	$(SRCDIR)/sensor_proc.c \
	$(SRCDIR)/safety_mon.c \
	$(SRCDIR)/failsafe.c \
	$(SRCDIR)/motor_ctrl.c \
	$(SRCDIR)/mem_test.c \
	$(SRCDIR)/wdt_drv.c \
	$(SRCDIR)/dem.c \
	$(SRCDIR)/diag_mgr.c \
	$(SRCDIR)/scheduler.c

SRC_UNITY = unity/unity.c
SRC_STUB = test_hal_stub.c
SRC_COMMON_TEST = test_common.c

SRC_ALL_COMMON = $(SRC_MODULES) $(SRC_UNITY) $(SRC_STUB) $(SRC_COMMON_TEST)

.PHONY: all clean

all: test_all.exe
	./test_all.exe

test_all.exe: test_main.c test_unit.c test_integration.c test_qualification.c $(SRC_ALL_COMMON)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ \
		test_main.c test_unit.c test_integration.c test_qualification.c \
		$(SRC_ALL_COMMON)

clean:
	rm -f *.exe *.o *.gcda *.gcno
