#############################################################################
#  Makefile macros
#############################################################################
# project root directory
BASE    = /home/pi/gqgmc/gqgmc-code-5f7e768dfa770be6274f4e268141a09022f5bdfa

# executables directory
BIN     = $(BASE)/bin

# object directory
OBJ     = $(BASE)/obj

# gq library directory
LIBS = $(BASE)/libs

# include search paths
GQ_INC   = -I$(BASE)
INC_DIR  = $(GQ_INC)

# libraries search paths
GQ_LIBS  = -L$(BASE)/libs
LIBS_PTH = $(GQ_LIBS)

#libraries linked
GQ_LNK  = -lGQGMC
LIBS_LNK = $(GQ_LNK)

# C Compiler
CC      = gcc
LD      = gcc

CPP     = g++
LDCPP   = g++

DEFINES = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED

# By default set to compile/link for 64-bit Linux.
# For 32-bit, change to -m32.
CPUSIZE = -mbe32

#CFLAGS  = $(CPUSIZE) -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES) $(INC_DIR)
CFLAGS  = $(CPUSIZE) -pipe -Wall -D_REENTRANT $(DEFINES) $(INC_DIR)

LDFLAGS = $(CPUSIZE) -Wl,-O1 $(LIBS_PTH)

# MOC compiler
MOC     = /usr/bin/moc-qt4

# Use ar & ranlib to build libgqgmc library
AR      = ar
RANLIB  = ranlib