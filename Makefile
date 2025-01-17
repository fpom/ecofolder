##############################################################
# Makefile for Unfolder

TARGET = ecofolder mci2dot mci2dot_ev mci2dot_cpr mci2csv pr_encoding rs_complement llnet2dot mci2sat sateval
LIBS = 
FLAGS = 
INCLUDES =

BINDIR = "$$HOME/.local/stow/ecofolder-231004/bin"

##############################################################
#https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html
#-std=gnu18
DEBUG =
WARNINGS = -Wall -Wno-unused-result

CC = gcc $(DEBUG) $(INCLUDES) $(WARNINGS) $(FLAGS)
SHELL = /bin/sh

TMPFILES =

OBJECTS = main.o common.o readlib.o \
    netconv.o nodelist.o readpep.o mci.o \
    mci_cpr.o unfold.o marking.o pe.o order.o

OBJECTS_EV = mci2dot_ev.o

OBJECTS_CPR = mci2dot_cpr.o

OBJECTS_PR = pr_encoding.o common.o

OBJECTS_RS = rs_complement.o common.o

OBJECTS_LD = llnet2dot.o common.o

OBJECTS_SAT = mci2sat.o common.o
	  
default: $(TARGET)

install: default
	mkdir -p $(BINDIR)
	cp -a $(TARGET) $(BINDIR)

clean:
	rm -f $(OBJECTS) $(OBJECTS_EV) $(OBJECTS_CPR) $(OBJECTS_PR) $(OBJECTS_RS) $(OBJECTS_LD) $(OBJECTS_SAT) $(TMPFILES) $(TARGET) core* *.output *.d .deps gmon.out

ecofolder: $(OBJECTS)
	$(CC) $(OBJECTS) -o ecofolder

mci2dot_ev: $(OBJECTS_EV)
	$(CC) $(OBJECTS_EV) -o mci2dot_ev

mci2dot_cpr: $(OBJECTS_CPR)
	$(CC) $(OBJECTS_CPR) -o mci2dot_cpr

pr_encoding: $(OBJECTS_PR)
	$(CC) $(OBJECTS_PR) -o pr_encoding

rs_complement: $(OBJECTS_RS)
	$(CC) $(OBJECTS_RS) -o rs_complement

llnet2dot: $(OBJECTS_LD)
	$(CC) $(OBJECTS_LD) -o llnet2dot

mci2sat: $(OBJECTS_SAT)
	$(CC) $(OBJECTS_SAT) -o mci2sat

# Dependencies

%.d: %.c
	  $(SHELL) -ec '$(CC)  -MM $? | sed '\''s/$*\.o/& $@/g'\'' > $@'

DEPS = $(OBJECTS:%.o=%.d)

.deps: $(DEPS)
	echo " " $(DEPS) | \
	sed 's/[ 	][ 	]*/#include /g' | tr '#' '\012' > .deps

include .deps
