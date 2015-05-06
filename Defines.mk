#############################################################################
#  Makefile macros
#############################################################################
# project root directory
BASE	= /home/phil/Projects/gqgmc

# executables directory
BIN	= $(BASE)/bin

# object directory
OBJ	= $(BASE)/obj

# gq library directory
LIBS = $(BASE)/libs

# include search paths
GQ_INC   = -I$(BASE)
QT_INC   = -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4
X11_INC  = -I/usr/include/X11
INC_DIR	 = $(GQ_INC) $(QT_INC) $(X11_INC)

# libraries search paths
GQ_LIBS	 = -L$(BASE)/libs
GUI_LIBS = -L/usr/lib/x86_64-linux-gnu
LIBS_PTH = $(GQ_LIBS) $(GUI_LIBS)

#libraries linked
GQ_LNK  = -lGQGMC
QT_LNK  = -lQtGui -lQtCore -lpthread
X11_LNK = -lXext -lX11
LIBS_LNK = $(GQ_LNK) $(QT_LNK) $(X11_LNK)

# C Compiler
CC	= gcc
LD	= gcc

CPP	= g++
LDCPP	= g++

DEFINES = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED

# By default set to compile/link for 64-bit Linux.
# For 32-bit, change to -m32.
CPUSIZE = -m64

#CFLAGS  = $(CPUSIZE) -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES) $(INC_DIR)
CFLAGS  = $(CPUSIZE) -pipe -Wall -D_REENTRANT $(DEFINES) $(INC_DIR)

LDFLAGS = $(CPUSIZE) -Wl,-O1 $(LIBS_PTH)

# MOC compiler
MOC     = /usr/bin/moc-qt4

# Use ar & ranlib to build libgqgmc library
AR      = ar
RANLIB  = ranlib





