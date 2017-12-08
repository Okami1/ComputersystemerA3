
# Directories

INCDIR = inc
OBJDIR = obj
LIBDIR = lib
SRCDIR = src

# Compiler defs

CC=gcc
CFLAGS=-I$(INCDIR) -L$(LIBDIR) $(LIBS)
# LIBS=-lm  # Uncomment if <math.h> used

# List common header files here

_DEPS = errors.h radio.h alarm.h simsec.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

# List implementation object files here

# For radio layer
_RAD_OBJS = radio.o
RAD_OBJS = $(patsubst %,$(OBJDIR)/%,$(_RAD_OBJS))

# For SimSec protocol layer
_SIMSEC_OBJS = simsec.o alarm.o
SIMSEC_OBJS = $(patsubst %,$(OBJDIR)/%,$(_SIMSEC_OBJS)) $(RAD_OBJS)

_ECG_OBJS = ecg.o alarm.o
ECG_OBJS = $(patsubst %,$(OBJDIR)/%,$(_ECG_OBJS)) $(RAD_OBJS)


# Rules

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Targets

all: radio_test simsec_test ecg_test

radio_test: $(OBJDIR)/radio_test.o $(RAD_OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

simsec_test: $(OBJDIR)/simsec_test.o $(SIMSEC_OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

ecg_test: $(OBJDIR)/ecg_test.o $(ECG_OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean clean-targets

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*~ core $(INCDIR)/*~ *~ *.exe radio_test simsec_test ecg_test

clean-targets:
	rm -f radio_test simsec_test ecg_test
