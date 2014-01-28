
#include "SystemManager.h"

SystemManager::TriggerSystem SystemManager::trigger=SystemManager::TriggerSystem();
SystemManager::VDDCsystem SystemManager::vddc=SystemManager::VDDCsystem();
SystemManager::EMCsystem SystemManager::emc=SystemManager::EMCsystem();
SystemManager::ToFsystem SystemManager::tof=SystemManager::ToFsystem();
SystemManager::MuSystem SystemManager::mu=SystemManager::MuSystem();

SystemManager::TriggerSystem::TriggerSystem(void) :
	PT_passed(0), ST_passed(0), KlukvaErr(0), LKr_scale(1.), CsI_scale(1.) {
}
SystemManager::MuSystem::MuSystem(void) : result_hits(0), result_type(0) {
}

bool SystemManager::TriggerSystem::init(void) {
/*/-----1------//
	int TriggerRun=*run_list.begin();
	if(ksettrgdb(&TriggerRun)) std::cerr<<"Failed to load trigger!"<<std::endl;
//-----2------//
	UserTRGFlag=1;
//------3------/*/

	KDBtext dbtrg[2];	//trg_text (id=1904) - table with pt&st
	KDBconn *conn=kdb_open();
	if(!conn) { std::cerr<<"Database connection failed."<<std::endl; return false; }

	T_li run_list;
	if(IOglobals::popts->isGiven("run")) {
		run_list=IOconverter::extract_list(*IOglobals::popts->getValue("run"));
		kdb_read_for_run(conn,1904,*run_list.begin(),dbtrg,2);
	} else {
		std::cerr<<"\tRun is not found."<<std::endl;
		return false;
	}
	
	std::ostringstream tmp_trg;
	tmp_trg<<"PT="<<dbtrg[0].textref<<"\n\nST="<<dbtrg[1].textref<<std::endl;
	T_s tmp_trg_str=tmp_trg.str();
	size_t tmp_pos=0;
	while((tmp_pos=tmp_trg_str.find("+",tmp_pos))!=T_s::npos) {
		tmp_trg_str.insert(++tmp_pos,"\n   ");
		tmp_pos+=4;
	}
	
	//file line should be less than 132 characters! (fortran stuff)
	std::ofstream file((IOglobals::popts->getPath()+"/input/trgformula_"+IOconverter::print_list(&run_list)+".dat").c_str(),std::ios_base::trunc);
	file<<tmp_trg_str<<std::endl;
	file.close();
	
	if(conn) kdb_close(conn);
	
	return true;
/*/------0------//
	T_s trg_str="/home/boroden/current/trgformula12aNoABG.dat";
	int trgerr;
	char *trg_str_c=new char[trg_str.size()+1];
	std::strcpy(trg_str_c,trg_str.c_str());
	kedr_read_trg_(trg_str_c,&trgerr,trg_str.size());
	delete trg_str_c;
	int PT_passed, ST_passed, KlukvaErr;
	kedr_trigger(trgFdefault+trgFsc1+trgFsc2,&PT_passed,&ST_passed,&KlukvaErr);
//trgFdefault=0,trgFse=1,trgFrnd=2,trgFlm=4,trgFabg=8,trgFdcv=16,trgFsc1=32,trgFsc2=64,trgFComb=128,trgFst2n=256
//------------/*/
}
bool SystemManager::TriggerSystem::read(void) {
	T_s trg_str;
	if(IOglobals::popts->isGiven("run")) {
		trg_str=IOglobals::popts->getPath()+"/input/trgformula_"+*IOglobals::popts->getValue("run")+".dat";
	} else {
		trg_str=IOglobals::popts->getPath()+"/input/trgformula_7806-7916.dat";
	}
	int trgerr;
	char *trg_str_c=new char[trg_str.size()+1];
	std::strcpy(trg_str_c,trg_str.c_str());
	kedr_read_trg_(trg_str_c,&trgerr,trg_str.size());
	delete trg_str_c;
	
	if(trgerr) return false;
	else return true;
}

bool SystemManager::VDDCsystem::init(void) {
//	kdcswitches_.KemcAllowed=1;		//information from strips is used in reconstruction and also initial search for tracks
//	kdcswitches_.KtofAllowed=-1;	//do not require amplitude or time when assigning strips to tracks
//	kdcpar1_.DeleteTracksGhost=1;	//delete ghosting tracks
	kdcpar1_.UseEMCSeparation=1;	//electron/meson criterion is used to set reconstruction parameters for strips
	if(IOglobals::popts->isGiven("sim")) {
	//	kdcsimxt();
		kdcsimsigma();
	} else {
		kdcvdinit(0);
	}
	if(IOglobals::popts->isGiven("cosmic")) kdcvdcosmic();
	else kdcvdnocosmic();
	
	return true;
}
bool SystemManager::EMCsystem::init(void) {
	return !( emc_init_spec((IOglobals::popts->getPath()+"/input/param.emc").c_str()) < 0 );
//	emc_init();
}
bool SystemManager::ToFsystem::init(void) {
	tof_init();
	return true;	//no fail states
}
bool SystemManager::MuSystem::init(void) {
	if(mu_default_init(1)) std::cerr<<"Mu init error"<<std::endl;
	if(mu_get_db_status(0)<0) {
		std::cerr<<"Mu status db information couldn't be accessed."<<std::endl;
		if(mu_init_status()) std::cerr<<"Mu status memory allocation error, unpredictable behaviour."<<std::endl;
	}
	if(IOglobals::popts->isGiven("sim")) {
		mu_set_sim_clbr();
	}
	return true;
}

bool SystemManager::TriggerSystem::set_run(int run) {
	ksetscaleE(LKr_scale,CsI_scale);	//trg scale
	return true;
}
bool SystemManager::VDDCsystem::set_run(int run) {
	int err=0;
//	if(!opts->isGiven("sim")) {
//		kdcvdrun(run,&err); //get DC and VD calibrations for current run
//	}
	return !err;
}
bool SystemManager::EMCsystem::set_run(int run) {
//	emc_run(run,opts->isGiven("sim")); //emc calibrations
	return true;
}
bool SystemManager::ToFsystem::set_run(int run) {
	tof_run(run);
	if(IOglobals::popts->isGiven("sim")) {
	//	tof_sim();   //call for simulation runs
		tof_sim_real(run,2,0);
	}
	if(IOglobals::popts->isGiven("cosmic")) {
		tof_cosmic(0); //call for cosmic runs (unsure about par value)
	}
	return true;
}
bool SystemManager::MuSystem::set_run(int run) {
	if(IOglobals::popts->isGiven("sim")) {
		mu_simrun(run,0);
	} else if(mu_get_db_status(run)<0) {
		std::cerr<<"Mu (run) status error";
		if(mu_get_db_status(run+1)) std::cerr<<"+";
		else if(mu_get_db_status(run-1)) std::cerr<<"- (skip this run)";
		else std::cerr<<"0";
		std::cerr<<std::endl;
	} else {
		if(mu_get_db_clbr_for_run(run)<0) std::cerr<<"Mu (run) calibration error"<<std::endl;
		if(mu_get_db_resolution(run)<0) std::cerr<<"Mu (run) resolution error"<<std::endl;
	}
	return true;
}

bool SystemManager::TriggerSystem::process_event(void) {
	kedr_trigger(trgFdefault+trgFdcv,&PT_passed,&ST_passed,&KlukvaErr);
	return true;
}
bool SystemManager::VDDCsystem::process_event(void) {
	int rec=0;
	kdcvdrec(0,&rec);
	return !(rec<0);
}
bool SystemManager::EMCsystem::process_event(void) {
//	emc_all();		//it's included 
	return true;
}
bool SystemManager::ToFsystem::process_event(void) {
	tof_event();
	
	times_track_barrel.clear();
	times_emc_barrel.clear();
	times_emc_endcap.clear();
	
	short itofq[128]={};
	double RAmplToF=0.2,dTAvTr=2.85,dTAvCl=2.85,dTAvClEC=-0.075;

	for(int id_tr=0;id_tr<kscBhit_.nDCtracks;++id_tr)
	if(kscBhit_.ntof[id_tr]>0) for(int id_ht=0;id_ht<kscBhit_.ntof[id_tr];++id_ht) {
		int phmt_numl=static_cast<int>(kscBhit_.numPmt_l[id_tr][id_ht]);
		int phmt_numr=static_cast<int>(kscBhit_.numPmt_r[id_tr][id_ht]);
		double tof1l=0.,tof1r=0.;
		bool trl_ok=false,trr_ok=false;
		if(phmt_numl>0) { tof1l=static_cast<double>(kscBhit_.time_BL_ns[id_tr][id_ht]); }
		if(phmt_numr>0) { tof1r=static_cast<double>(kscBhit_.time_BR_ns[id_tr][id_ht]); }
		
		if(tof1l!=0.&&tof1l!=-700. && itofq[phmt_numl]==0) {
			itofq[phmt_numl]=1; trl_ok=true;
		}
		if(tof1r!=0.&&tof1r!=-700. && itofq[phmt_numr]==0) {
			itofq[phmt_numr]=1; trr_ok=true;
		}
		if(trl_ok && trr_ok) {
			if(fabs(tof1l-tof1r)<18.) {
				times_track_barrel.push_back(tof1l);
				times_track_barrel.push_back(tof1r);
			} else {
				double tmin=(fabs(tof1l-dTAvTr)<fabs(tof1r-dTAvTr) ? tof1l : tof1r);
				times_track_barrel.push_back(tmin);
				times_track_barrel.push_back(tmin);
			}
		} else {
			if(trl_ok) {
				times_track_barrel.push_back(tof1l);
				times_track_barrel.push_back(0.);
			} else if(trr_ok) {
				times_track_barrel.push_back(0.);
				times_track_barrel.push_back(tof1r);
			}
		}
	}
	
	bool sim=false;
	for(int id_ht=0;id_ht<kschit_.nScHits;++id_ht) if(itofq[id_ht]==0) {
		int phmt_num=static_cast<int>(kschit_.numPmt[id_ht]);
		if(itofq[phmt_num]==0) {
			short phmt_status=static_cast<short>(kschit_.status[id_ht]);
			double tof_phi=static_cast<double>(kschit_.phi[id_ht])*SUconverter::get_multiplier("rad","deg");
			while(tof_phi<0.) tof_phi+=360.;
			while(tof_phi>360.) tof_phi-=360.;
			double tof1=static_cast<double>(kschit_.time_ns[id_ht]);
			
			if(tof1!=0.&&tof1!=-700.) for(int cl=0;cl<semc.emc_ncls;++cl) {
				int emctype=semc.emc_type[cl];
				if(semc.emc_energy[cl]>1000.) {
					double zemc=semc.emc_z[cl];
					double Rampl1=kschit_.amp_mev[id_ht];
					double tof_dphi=fabs(semc.emc_phi[cl]-tof_phi);
					while(tof_dphi>360.) tof_dphi-=360.;
					if(tof_dphi>180.) tof_dphi=360.-tof_dphi;
					
					if(phmt_status==0 && kschit_.time_ch[id_ht]>0.	&&
					(Rampl1>RAmplToF || (sim && Rampl1>0.))		&&
					tof_dphi<15. && itofq[id_ht]==0	) {
						if(phmt_num>=32 && phmt_num<96 &&
						phmt_status==0 && emctype==1	) {
							itofq[phmt_num]=1;
							times_emc_barrel.push_back(tof1);
						} else if(phmt_status==0 &&
						(emctype==2 || (emctype==1 && fabs(zemc)>80.))	&&
						( (kschit_.numPmt[id_ht]<32 && zemc>0.)	||
						(kschit_.numPmt[id_ht]>=96 && zemc<0.)	)		) {
							double avT=kschit_.time_ns[id_ht];
							double Rampl=kschit_.amp_mev[id_ht];
							itofq[phmt_num]=1;
							if(avT && (Rampl>RAmplToF || (sim && Rampl>0.)) ) {
								times_emc_endcap.push_back(avT);
							}
						}
					}
				}
			}
		}
	}
	return true;
}
bool SystemManager::MuSystem::process_event(void) {
	result_hits=0; result_type=0;	//reset data to return

	short MuNhits=0, MuNhits1=0, MuNhits2=0, MuNhits3=0;
	int MuNhitsRaw=mu_next_event_good();
	if(MuNhitsRaw<=0) return false;

	double* mu_x=new double[MuNhitsRaw], *mu_y=new double[MuNhitsRaw];
	int* mu_st=new int[MuNhitsRaw];
	for(unsigned short idx=0;idx<MuNhitsRaw;++idx) mu_st[idx]=0;
	bool COSMICMU=false;
	double r1=0., r2=0.;
	short octants[8][4]={};		//inits to 0
	for(unsigned short idx=0;idx<MuNhitsRaw;++idx) {
		unsigned short channel=mu_event.ch[idx];
		unsigned short ch_idx=mu_event.ch[idx]-MU_FIRST_CHANNEL;

	//	unsigned short position=(ch_idx%256);
	//	unsigned short p_octant=(position<16*6?1:(position<16*6*2?2:0)); if(p_octant) p_octant+=(ch_idx/256)*2;
	//	if(position>=16*6) position-=16*6;
	//	unsigned short p_layer=(position<20?1:(position<42?2:(position<68?3:(position<16*6?4:0))));
	//	std::cout<<"\tCalculation: pos="<<(ch_idx%256)<<" p_oct="<<p_octant<<" p_lyr="<<p_layer<<std::endl;

		unsigned short statusMU=mu_channel_status(channel);
//		std::cout<<"\t\t\tchannel="<<channel<<"\tstatus="<<statusMU<<"\tr_status="<<(int)mu_event.r_status[idx]<<std::endl;
		if(statusMU==MU_STS_USABLE && ch_idx<MU_NUMBER_OF_CHANNELS) {
			unsigned short layer=0, octant=0;	//unsigned is always >=0
			layer=mu_channel[ch_idx].layer-1;
	//		std::cout<<"\tlayer="<<layer;
			if(layer<3) {
				octant=mu_channel[ch_idx].octant-1;
	//			std::cout<<"\toctant="<<octant<<std::endl;
				if(octant<8) {	++MuNhits;
					int Ntubes=mu_channel[ch_idx].ntubes;
					r1=0.; r2=0.;
					for(int tube_num=0;tube_num<Ntubes;++tube_num) r1=r1+mu_channel_tube_x(channel,tube_num);
					for(int tube_num=0;tube_num<Ntubes;++tube_num) r2=r2+mu_channel_tube_y(channel,tube_num);
					if(Ntubes>0) mu_x[idx]=r1/Ntubes;
					if(Ntubes>0) mu_y[idx]=r2/Ntubes;

					if(mu_x[idx] && mu_y[idx]) mu_st[idx]=1;
					if(layer==0) ++MuNhits1;
					else if(layer==1) ++MuNhits2;
					else if(layer==2) ++MuNhits3;
					octants[octant][layer]=1;
				}
			}
		}
	}
//	std::cout<<"\t\tMu result: hits="<<MuNhits<<"\tlayers={"<<MuNhits1<<","<<MuNhits2<<","<<MuNhits3<<"}"<<std::endl;
	
	unsigned short oct_hits[8]={};
	for(short idx=0;idx<8;++idx) for(short id_l=0;id_l<3;++id_l) {
		if(octants[idx][id_l]) ++oct_hits[idx];
	}
	for(unsigned short idx=0;idx<8;++idx) {
		result_hits+=(oct_hits[idx]<<(idx*2));
	}
	for(short idx=0;idx<8;++idx) {
		unsigned short hit_cross=0; short idx_cross=-1;
		for(short idx_add=3;idx_add<6;++idx_add) {	//+3,+4,+5
			if(oct_hits[(idx+idx_add)%8]>hit_cross) {
				idx_cross=(idx+idx_add)%8;
				hit_cross=oct_hits[idx_cross];
			}
		}
		unsigned short tmp_type=oct_hits[idx]*10+hit_cross;
		if(tmp_type>result_type) result_type=tmp_type;
	}
	
	delete[] mu_x; mu_x=0;
	delete[] mu_y; mu_y=0;
	delete[] mu_st; mu_st=0;
	
	return true;
}

bool SystemManager::MuSystem::cleanup(void) {
	mu_delete_clbr();
	mu_delete_geometry();
	return true;
}
