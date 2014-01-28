#--------------------------------------------------------------------------
#--next 3 lines are better left untouched
ifeq (,$(WORKDIR))
WORKDIR=..
endif

VERBOSEMAKE=""

#--compiler
#CC_LOCAL := gcc
#--linker
CXX_LOCAL := g++

#--additional options for C/C++ compiler
COPTS  = -I./ -I../VDRec/. -I../VDDCRec/.   -I../KrMH/.  `root-config --cflags` -I./
#apparently, rtti is used in TBuffer, so don't use -fno-rtti

#--additional options for Fortran compiler
FOPTS  = -I./ -I../VDRec/. -I../VDDCRec/.   -I../KrMH/.

#--additional options for linker
LDOPTS = -lg2c  `root-config --libs --new `  -lMinuit -g `root-config --new --libs` -lMinuit -Wl,-rpath -Wl,`root-config --libdir`

#--if ONLYBINARY is defined, library is absent from the package
#ONLYBINARY=""

#--if you need to add CERNLIB, better use this key
CERNLIBRARY = ""

#CERNLIBS = jetset74 mathlib graflib grafX11

#--where to put executables
#BINDIR := ./

#--additional libraries (inserted after libraries)
LIB_LOCAL= -lpq -lcrypt -lbz2 -lstdc++

#LIB_LOCAL+= `root-config --libs` -lGui -lMinuit -lg2c

#--define executables to compile
BINARIES = Screening
#BINARIES = Screening RecSimpleSampleCC

Screening_MODULES = Screening
Screening_LIBS = KDisplay VDDCRec KsTrg VDRec ReadNat KDB KdDCSim DchGeom KrAtc AppFramework KrToF KsToF KrMu KEmcRec LKrTools FitTools KdConvert KrObjects
Screening_LIBS+= DataClass

#RecSimpleSampleCC_MODULES = RecSimpleSampleCC
#RecSimpleSampleCC_LIBS = KDisplay VDDCRec KsTrg VDRec ReadNat KDB KdDCSim DchGeom KrAtc AppFramework KrToF KsToF KrMu KEmcRec LKrTools FitTools KdConvert KrObjects

#
#--next line is better left untouched
include $(WORKDIR)/KcReleaseTools/rules.mk

