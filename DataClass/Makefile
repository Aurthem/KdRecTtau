#--next 3 lines are better left untouched
ifeq (,$(WORKDIR))
WORKDIR=../..
endif

VERBOSEMAKE=""

#--compiler
#CC_LOCAL := gcc
#--linker
CXX_LOCAL := g++

INCFILES = -I./ -I../

#--additional options for C/C++ compilator
COPTS  =  `root-config --cflags` $(INCFILES) 
CPPOPTS = -Wno-deprecated

#--additional options for Fortran compilator
#FOPTS  = -I/space/KEDR/current/KdDCSim -I/space/KEDR/current/KsCERNJetset -I/space/KEDR/current/KsCERNPythia -I/space/KEDR/current/ReadNat -I/space/KEDR/current/VDDCRec

#--additional options for linker
LDOPTS = -lg2c `root-config --new --libs` -lMinuit -Wl,-rpath -Wl,`root-config --libdir`

#--if defined, library will be absent from this package
#ONLYBINARY=""

#--if you need to add CERNLIB, use this key
CERNLIBRARY = ""

#--where to put executables
#BINDIR := ./

#--additional libraries (added after libs)
LIB_LOCAL = -lpq -lcrypt -lbz2 -lstdc++

#--defines which executables are made
#BINARIES = 

#--point out from which modules they are made
#--(these modules won't be added to the library)
#--and which libraries should be added upon compilation

#--do not touch this one
include $(WORKDIR)/KcReleaseTools/rules.mk

MAINCLASS := DataClass

POST_MAKE:	rootlib
#--shared library to be made
ROOTLIB := $(LIBDIR)/lib$(MAINCLASS).so

#--dictionary
DICTCXX := $(MAINCLASS)Dict.cpp
#--include file for the dictionary
DICTH := $(MAINCLASS)Dict.h
#--compiled dictionary
DICTOBJ := $(TEMPDIR)/$(MAINCLASS)Dict.o

#LIBFILE := -L /space/KEDR/stable/lib -lKEmcRec -lMu-compact -lKrToF
rootlib: $(ROOTLIB)

$(DICTCXX): $(MAINCLASS).h $(MAINCLASS)_LinkDef.h
	@echo "Generating dictionary $@..."
	rootcint -f $@ -c $(INCFILES) $^

$(DICTOBJ): $(DICTH)

#--compilation of shared library for usage in root-scripts
$(ROOTLIB): $(LIBFILE)  $(DICTOBJ)
	$(CMDPREFIX)g++ -shared -o $(ROOTLIB) $(DICTOBJ)  $(LIBFILE) -lpq -lcrypt
#-L/space/KEDR/stable/lib
# -lKEmcRec -lMuRec-compact -lKrToF -lVDDCRec -lKDB -lReadNat

clean:
	rm -f $(DICTCXX) $(DICTH)
