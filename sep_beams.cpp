
#include "sep_beams.h"

namespace KedrFortran {
	OutVars::OutVars(void) : TotalCount(),Count(),Qtot(),Ltot(),Tliv(),
	Separation1(2),SoR(2),NevRead0(0),NevRead1(0),NevReadS(0),Ncalls(0),NinRun(0),
	Nreport(0),HistBooked(0),NhistEnt(0),L1int(0),L2int(0)
#ifdef _BGLIST
	,BgCond(8)
	{	strcpy(BgFile,"BgList_test.txt");
#elif
	{
#endif
		strcpy(HistFile,"kedrsep_test.rz");
		std::fill(TotalCount[0]+0,TotalCount[2]+Mcond+1,0);
		std::fill(Count+0,Count+Mcond+1,0);
		std::fill(Qtot+0,Qtot+3,0); std::fill(Ltot+0,Ltot+3,0); std::fill(Tliv+0,Tliv+3,0);
	}
	
	OutVars out_vars;

	void SepBeams(int NrepArg, int c1, int c2, int c3, int c4, int c5, int c6, int c7, int c8) {
		++out_vars.Ncalls;
		out_vars.Nreport=NrepArg;
		
		if(kedr_read_nat_cb1_.MCfile==0) {
			if(kedrraw_.Header.RunNumber>=5187) {
				if(out_vars.Separation1==1) {
					if(kedrraw_.Header.Separation==0) out_vars.Separation1=2;		//unknown
					else if(kedrraw_.Header.Separation!=0) out_vars.Separation1=2;	//unknown
				}
			}
			if(kedrlumcb_.RunType>1) out_vars.Separation1=2;
		}
#if 000
		std::cout<<"sep_beams: Ncalls="<<out_vars.Ncalls<<" Separation1="<<out_vars.Separation1
			<<" Sep(ev)="<<kedrraw_.Header.Separation<<" Lum="<<out_vars.Lum
			<<" NevReadTot="<<kedr_read_nat_cb1_.NevReadTot<<" MCfile="<<kedr_read_nat_cb1_.MCfile<<std::endl;
#endif
	
		out_vars.Cond[0]=c1; out_vars.Cond[1]=c2;
		out_vars.Cond[2]=c3; out_vars.Cond[3]=c4;
		out_vars.Cond[4]=c5; out_vars.Cond[5]=c6;
		out_vars.Cond[6]=c7; out_vars.Cond[7]=c8;
		for(int i=0;i<Mcond;++i) {	//8th position is reserved for later
			if(out_vars.Cond[i]) ++out_vars.Count[i];
		}
		
#ifdef _BGLIST
		if(out_vars.Separation1==1) {
			if(	(out_vars.BgCond==1&&c1!=0) || (out_vars.BgCond==2&&c2!=0) ||
				(out_vars.BgCond==3&&c3!=0) || (out_vars.BgCond==4&&c4!=0) ||
				(out_vars.BgCond==5&&c5!=0) || (out_vars.BgCond==6&&c6!=0) ||
				(out_vars.BgCond==7&&c7!=0) || (out_vars.BgCond==8&&c8!=0) ) {
				if(out_vars.Nlist<Mlist) {
					++out_vars.Nlist;
					out_vars.BgRun=kedrraw_.Header.RunNumber;
					out_vars.BgList[out_vars.Nlist][0]=kedrraw_.Header.Number;
					out_vars.BgList[out_vars.Nlist][1]=out_vars.NinRun;
				}
			}
		}
#endif
#if 666
		return;
#else
		if(out_vars.Nreport==0 || (out_vars.Ncalls%out_vars.Nreport)!=0) return;
#endif
	}
	//------------
	void SepBeams_report(void) {
		double rTotal0=out_vars.TotalCount[0][Mcond];
		double rTotal1=out_vars.TotalCount[1][Mcond];
		double rTotal2=out_vars.TotalCount[2][Mcond];
#ifdef _DBG
		std::cout<<"sep_beams: read="; for(int i=0;i<3;++i) std::cout<<(i?",":"")<<out_vars.TotalCount[i][Mcond];
		std::cout<<" called="<<out_vars.Ncalls<<" NevReadTot="<<kedr_read_nat_cb1_.NevReadTot<<std::endl;
		std::cout<<" Q= ";	for(int i=0;i<3;++i) std::cout<<(i?",\t":"")<<out_vars.Qtot[i];
		std::cout<<"\n T= ";	for(int i=0;i<3;++i) std::cout<<(i?",\t":"")<<out_vars.Tliv[i];
		std::cout<<"\n L= ";	for(int i=0;i<3;++i) std::cout<<(i?",\t":"")<<out_vars.Ltot[i];
		std::cout<<std::endl;
#endif
		if(rTotal1<10.) return;
		double fN=(rTotal1 ? rTotal0/rTotal1 : 0.);
		double fQ=(out_vars.Qtot[1] ? out_vars.Qtot[0]/out_vars.Qtot[1] : 0.);
		double fT=(out_vars.Tliv[1] ? out_vars.Tliv[0]/out_vars.Tliv[1] : 0.);
#ifdef _DBG
		std::cout<<"fN="<<fN<<"\tfQ="<<fQ<<"\tfT="<<fT<<std::endl;
#endif
		for(int i=0;i<Mcond;++i) {
			double bg=out_vars.TotalCount[1][i];
			if(out_vars.Ltot[1]>0.) bg-=(out_vars.Ltot[0]*out_vars.TotalCount[0][i] ? 
				out_vars.Ltot[1]/out_vars.Ltot[0]*out_vars.TotalCount[0][i] : 0.);
			if(out_vars.TotalCount[0][i]>0 || out_vars.TotalCount[1][i]>0) {
				std::cout<<" "<<i<<": "<<out_vars.TotalCount[0][i]<<"\tbg="<<static_cast<int>(bg*fN+0.5)<<" "
				<<static_cast<int>(bg*fQ+0.5)<<" "<<static_cast<int>(bg*fT+0.5)<<"\tNev="<<bg<<std::endl;
			}
		}
		if(out_vars.HistBooked!=0) {
		//	writes all histograms to out_vars.HistFile
//!		//	call HRPUT(0,out_vars.HistFile(:lenocc(out_vars.HistFile)),'N')
		}
		return;
	}

	void SepBeams_hist(int pawcsize, int IDarg, double val, int Nbins, double vmin, double vmax) {
		if(out_vars.HistBooked==0) {
			out_vars.ID=IDarg;
			out_vars.HistBooked=1;
			if(pawcsize>0) {
			//initializes zebra, sets limit for paw
//!			//	call HLIMIT(pawcsize);
			}
			for(int i=0;i<Mcond;++i) {
			//creates 1D histograms
//!			//	call HBOOK1(ID+1+i,'colliding beams',Nbins,vmin,vmax,0.)
//!			//	call HBOOK1(ID+11+i,'separated beams',Nbins,vmin,vmax,0.)
			}
			out_vars.NhistEnt=0;
		}
		out_vars.HistVal[out_vars.NhistEnt]=val;
		for(int i=0;i<Mcond;++i) {
			out_vars.HistCond[out_vars.NhistEnt][i]=out_vars.Cond[i];
		}
		if(out_vars.NhistEnt<MhistEnt) ++out_vars.NhistEnt;	//starts from 0 now
		return;
	}

//Following functions are to be called by kedr_read_nat
	void SepBeams_nextRun(void) {
		if(kedr_read_nat_cb1_.MCfile!=0) {
			out_vars.RunTime=kedrlumcb_.tm_tot;		//total run time, sec?
			out_vars.Separation1=2;		//separation unknown
			out_vars.RunTime1=out_vars.RunTime;
			out_vars.L1int=kedrlumcb_.lum_e_int;	//10^27 cm^-2=1 mb^-1
			out_vars.L2int=kedrlumcb_.lum_p_int;	//10^27 cm^-2=1 mb^-1
			out_vars.Lum=0.;
		} else {
			out_vars.RunTime=0;
			out_vars.Separation1=0;
			out_vars.RunTime1=out_vars.RunTime;
			out_vars.L1int=0;
			out_vars.L2int=0;
			out_vars.Lum=100.;
		}
		for(int i=0;i<=Mcond;++i) out_vars.Count[i]=0;
		out_vars.Nlist=0;
		out_vars.NinRun=0;
		out_vars.NevRead0=kedr_read_nat_cb1_.NevReadTot;
		out_vars.NevRead1=kedr_read_nat_cb1_.NevReadTot;
		out_vars.SoR=2;
#if defined(_DBG) && (_DBG>0)
		std::cout<<"sep_beams: SoR="<<out_vars.SoR<<" Ltot=";
		for(int i=0;i<3;++i) std::cout<<(i?",":"")<<out_vars.Ltot[i];
		std::cout<<" L1int="<<out_vars.L1int<<",L2int="<<out_vars.L2int<<std::endl;
#endif
		return;
	}
	void SepBeams_name(char* FileArg) {
		strcpy(out_vars.HistFile,FileArg);
		return;
	}
#ifdef _BGLIST
	void SepBeams_bgfile(char* FileArg) {
		strcpy(out_vars.BgFile,FileArg);
		return;
	}
#endif

	void SepBeams_nextFrame(void) {
		int Separation=2;			//at the cadr end
		int SkipCnt=0;
		double dL=0.,dQ=0.,dTlive=0.;

		out_vars.NinRun=kedr_read_nat_cb1_.NevReadTot-out_vars.NevRead0;
		out_vars.Count[Mcond]=kedr_read_nat_cb1_.NevReadTot-out_vars.NevRead1;
		if(kedr_read_nat_cb1_.MCfile!=0) {
			dL=0.1; dQ=0.1;
			++out_vars.RunTime;
			out_vars.SoR=0;
			out_vars.Separation1=0;
			Separation=0;
			out_vars.Lum=100.;
		} else {
			out_vars.RunTime=kedrlumcb_.tm_tot;
			if(kedrlumcb_.vp.swing==2) {	//separated beams
				Separation=1;				//separated beams
			} else if(kedrlumcb_.vp.swing==1) {
				Separation=0;				//colliding beams
			} else {
				Separation=2;				//unknown
			}
			if(kedrlumcb_.RunType>1) Separation=2;
			double Ie=(kedrlumcb_.vp.e_curr[0]+kedrlumcb_.vp.e_curr[1])/1000.;	//in mA
			double Ip=(kedrlumcb_.vp.p_curr[0]+kedrlumcb_.vp.p_curr[1])/1000.;	//in mA
			dTlive=kedrlumcb_.period*kedrlumcb_.tm_liv_tot/100.;
			out_vars.Lum=std::max(kedrlumcb_.lum_e,kedrlumcb_.lum_p)/1000.;
			double dL1=kedrlumcb_.lum_e_int-out_vars.L1int;
			double dL2=kedrlumcb_.lum_p_int-out_vars.L2int;
			dL=std::max(dL1,dL2)/1.e6;
			dQ=(Ie+Ip)*dTlive/1000.;
			if((out_vars.Lum>2 || dL>0.005) && Separation==1) Separation=2;
			if(out_vars.Lum<4 && Separation==0) Separation=2;
			
#if defined(_DBG) && (_DBG>0)
			std::cout<<"sep_beams: t="<<out_vars.RunTime<<","<<out_vars.RunTime1
			<<" sep="<<out_vars.Separation1<<","<<Separation<<" Ncalls="<<out_vars.Ncalls
			<<" Nread="<<kedr_read_nat_cb1_.NevReadTot<<" SkipCnt="<<SkipCnt<<","<<out_vars.Nreport<<std::endl;
			std::cout<<"dL="<<dL<<","<<dL1/1.e6<<","<<dL2/1.e6<<"\tdQ="<<dQ
			<<"\nLum="<<out_vars.Lum<<"\tdT="<<dTlive<<"\tSoR="<<out_vars.SoR<<std::endl;
			std::cout<<"old Ltot="; for(int i=0;i<3;++i) std::cout<<(i?",":"")<<out_vars.Ltot[i];
			std::cout<<"\tL1int="<<out_vars.L1int<<",L2int="<<out_vars.L2int<<std::endl;
#endif
			if(out_vars.SoR>0) {	//first LUMI record can be from wrong
				out_vars.Separation1=2;
				--out_vars.SoR;
			}
		}
		if(Separation==out_vars.Separation1 && Separation<2) {
			for(int i=0;i<=Mcond;++i) {
				out_vars.TotalCount[Separation][i]+=out_vars.Count[i];
			}
			out_vars.Qtot[Separation]+=dQ;
			out_vars.Ltot[Separation]+=dL;
			out_vars.Tliv[Separation]+=dTlive;
#if defined(_DBG) && (_DBG>0)
			std::cout<<"sep_beams: ID="<<out_vars.ID<<" Nent="<<out_vars.NhistEnt
			<<" Ncalls="<<out_vars.Ncalls<<std::endl;
#endif
			if(out_vars.NhistEnt>0) {
				int IDs=out_vars.ID+1+10*Separation;	//+1 in case ID=0 (which is illegal)
#if 000
				std::cout<<"sep_beams: IDs="<<IDs<<" Nent="<<out_vars.NhistEnt
				<<" Sep="<<out_vars.Separation1<<","<<Separation<<std::endl;
#endif
				for(int i=0;i<out_vars.NhistEnt;++i) {
					for(int j=0;j<Mcond;++j) {
#if defined(_DBG) && (_DBG>0)
						std::cout<<"\tent="<<i<<" val="<<out_vars.HistVal[i]
						<<" cond="<<out_vars.HistCond[i][j]<<std::endl;
#endif
						if(out_vars.HistCond[i][j]!=0) {
							//fills histogram
//!						//	call HF1(IDs+i,out_vars.HistVal[i],1.)
						}
					}
				}
			}
#ifdef _BGLIST
	//!!!Arcane writing!!! Change
			char chLine[80];
			if(Separation==1) {
				//open file
			//!	open(43,file=out_vars.BgFile,status='unknown');
				//get to the end of file by repeated reading
			//!	while(read(43,'(a)') chLine);
				for(int i=0;i<out_vars.Nlist;++i) {
					//write to the file
				//!	write(43,*) out_vars.BgRun,out_vars.BgList[i][0],out_vars.BgList[i][1];
				}
				//close file
			//!	close(43);
			}
#endif
		} else {
			for(int i=0;i<=Mcond;++i) {
				out_vars.TotalCount[2][i]+=out_vars.Count[i];
			}
			out_vars.Qtot[2]+=dQ;
			out_vars.Ltot[2]+=dL;
			out_vars.Tliv[2]+=dTlive;
		}
		out_vars.NhistEnt=0;
		for(int i=0;i<=Mcond;++i) out_vars.Count[i]=0;
		out_vars.RunTime1=out_vars.RunTime;
		out_vars.Separation1=Separation;
		out_vars.NevRead1=kedr_read_nat_cb1_.NevReadTot;
		if(kedr_read_nat_cb1_.MCfile==0) {
			out_vars.L1int=kedrlumcb_.lum_e_int;
			out_vars.L2int=kedrlumcb_.lum_p_int;
		}
#ifdef _BGLIST
		out_vars.Nlist=0;
#endif
#if defined(_DBG) && (_DBG>0)
		std::cout<<"new Ltot="; for(int i=0;i<3;++i) std::cout<<(i?",":"")<<out_vars.Ltot[i];
		std::cout<<"\tL1int="<<out_vars.L1int<<",L2int="<<out_vars.L2int<<std::endl;
#endif
#if 666
		if(out_vars.Nreport>0) {
			SkipCnt+=kedr_read_nat_cb1_.NevReadTot-out_vars.NevReadS;
			if(SkipCnt>=out_vars.Nreport || SkipCnt<0) {	//just in case...
				SkipCnt=0;
				out_vars.NevReadS=kedr_read_nat_cb1_.NevReadTot;
				SepBeams_report();
			}
		}
#endif
		return;
	}
}
