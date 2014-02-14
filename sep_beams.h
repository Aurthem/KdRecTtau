#ifndef au_kedrfortran
#define au_kedrfortran

//#include "ReadNat/re_def.inc"
#include "ReadNat/re_def.h"		//kedrraw_
//#include "ReadNat/rr_def.inc"
//#include "ReadNat/lm_def.inc"	//no raw arrays in kedrlumcb_
#include "ReadNat/lm_def.h"		//kedrlumcb_
//#include "ReadNat/kedr_read_nat.inc"

#include <cstring>
#include <stdio.h>
#include <iostream>
#include <algorithm>

#define _DBG 1
#define _BGLIST

namespace KedrFortran {
//Estimation of the background at separated beams
	//Up to 8 different selection conditions is foreseen:
	void SepBeams(int NrepArg, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8);
	void SepBeams_report(void);
	//Histogram filling after this call:
	void SepBeams_hist(int pawcsize, int IDarg, double val, int Nbins, double vmin, double vmax);
	//pawcsize - zero or HLIMIT parameter, IDarg - histogram ID base (colliding beams ID+1:ID+8,separated beams ID+11:ID+18)
	//val - value for current event, Nbins - number of bins, vmin - histogram min, vmax - histogram max
	
	void SepBeams_name(char* FileArg);		//set histogram filename
	void SepBeams_bgfile(char* FileArg);	//set background filename
	
	void SepBeams_nextRun(void);
	void SepBeams_nextFrame(void);
	
	const int Mcond=8;
#ifdef _BGLIST
	const int Mlist=50;
#endif
	const int MhistEnt=100;
	
	struct kedr_read_nat_cb1_ext {
		int NrecRead,NrErrors;
		int NrecWrite[5],NwErrors[5];
		int NevLost,NevRead;
		int MCfile,RunNumber,LastSeqNum;
		int NerrMsg,ErrMsgMax;
		int NevWrite[5];
		float FileLumE,FileLumP;
		int nDCVDrec,nDCVDvecOverflow,nDCVDhitOverflow,nDCVDtrkOverflow;
		int nDCVVDtxGarbage,nDCVVDtzGarbage,nDCVDcuts;
		float EbeamRN;
		int NevReadTot;
		int ThisRunIsBad;
	};
	extern "C" {
		extern struct kedr_read_nat_cb1_ext kedr_read_nat_cb1_;
	};
	
	struct OutVars {
		int TotalCount[3][Mcond+1];	//event total counters
		int Count[Mcond+1];			//event counters for the cadr
		double Qtot[3],Ltot[3];		//total charge and luminosity
		double Tliv[3];
		int Separation1;			//at the cadr beginning (unknown)
		int SoR;					//Start of Run flag
		int NevRead0;				//at start of run
		int NevRead1;				//event number from ReadNat
		int NevReadS;
		int Ncalls,NinRun,Nreport;
		int HistBooked,NhistEnt;
		int L1int,L2int;
		
		int RunTime;
		int RunTime1;
		short HistCond[MhistEnt][Mcond], Cond[Mcond];
		double HistVal[MhistEnt];
		double Lum;
		int ID;
		
	#ifdef _BGLIST
		int BgCond;
		int Nlist;
		int BgRun,BgList[Mlist][2];
		char BgFile[80];
	#endif
		char HistFile[80];
		
		OutVars(void);
	};
}

#endif //au_kedrfortran
