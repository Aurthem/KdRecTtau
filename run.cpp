// KEDR
//#include "KEmcRec/EmcTypes.h"
#include "ReadNat/re_def.h"		//kedrraw_
#include "ReadNat/rr_def.h"		//kedrrun_cb_
// ROOT
#include "TBenchmark.h"

#include "modules.h"
//#include "utilities.h"

//inspired mostly by KEmcTest/complex_examples/std_run && GRunInfo

Run::Run(const T_s& target) : info(0) {
	if(!IOglobals::popts) { std::cerr<<"Program options are not available!"<<std::endl; exit(-2); }
	
	PrmSlurper prmS(IOglobals::popts->getPath()+"/input/run_params");
	params.clear();
	params.fill(prmS.get(target));
	if(IOglobals::popts->isGiven("run")) {
		T_li dummy_lst=IOconverter::extract_list(*IOglobals::popts->getValue("run"));
		dummy_lst.sort(); dummy_lst.unique();
		this->params.SetList((IOglobals::popts->isGiven("sim")?"sim_list":"run_list"),dummy_lst);
	} else {	//just add it anyway
		const T_li* run_lst=this->params.GetList(IOglobals::popts->isGiven("sim")?"sim_list":"run_list");
		if(run_lst) {
			IOglobals::popts->addOption(T_s("--run=")+IOconverter::print_list(run_lst));
		}
	}
}
Run::~Run(void) {
	if(info) delete info;
}
void Run::clear(void) {
   	if(info) info->clear();
	params.clear();
}

void Run::all( bool(*primary_trigger_function)(), void(*event_function)() ) {
	int err=0, nev=0, rec;
//	if(params.GetChar("sim_cosm_deb",0)=='0') dm.init("runs");	//will go here in case of multiple calls
	TBenchmark* my_bench = new TBenchmark();

	//initialization of reconstruction in systems
	if(!params.CheckStrings(build_vector_from_list<T_s>("sim_events")("exp_events")("sim_mask")("run_mask"))) {
		std::cerr<<"Run params error: string missing."<<std::endl;	return;
	}
	if(!params.CheckLists(build_vector_from_list<T_s>("sim_list")("run_list"))) {
		std::cerr<<"Run params error: list missing."<<std::endl;	return;
	}	//to deal with nulls (maybe rework this further later)

	T_vs Nev_s; std::vector<int> Nev_val;
	if(IOglobals::popts->isGiven("sim")) {
		IOconverter::tokenize(*params.GetString("sim_events"),Nev_s,",",true);
	} else {
		IOconverter::tokenize(*params.GetString("exp_events"),Nev_s,",",true);
	}
	for(T_vs::const_iterator itr=Nev_s.begin();itr!=Nev_s.end();itr++) {
		Nev_val.push_back(atoi(itr->c_str()));
	}	while(Nev_val.size()<3) Nev_val.push_back(0);

	T_s run_mask; T_li run_list;
	if(IOglobals::popts->isGiven("sim")) {
		run_mask=*params.GetString("sim_mask");
		run_list=*params.GetList("sim_list");
	} else {
		run_mask=*params.GetString("run_mask");
		run_list=*params.GetList("run_list");	//	run_list.sort() - already inside GetList();
	}

	SystemManager::trigger.init();
	SystemManager::emc.init();
	SystemManager::tof.init();
	SystemManager::mu.init();
	SystemManager::vddc.init();
//----for simulation----//
	std::map<int,int> Nev_interv;
	{	//filling it
		LumReader lumr; lumr.setPath(IOglobals::popts->getPath()); lumr.read("luminosity");
		std::vector<double> tmp_rnd(run_list.size(),0.);
		TRandom3 root_rand=TRandom3(0);
		std::map<int,double> lum_multipliers=lumr.get_multipliers(run_list,root_rand);
std::cout<<"Event numbers:\n";
		for(T_li::const_iterator itr=run_list.begin();itr!=run_list.end();++itr) {
			Nev_interv[*itr]=(lum_multipliers.count(*itr) ? static_cast<int>(Nev_val[1]*lum_multipliers[*itr]) : 0);
std::cout<<"run"<<*itr<<"\t=\t"<<Nev_interv[*itr]<<"\n";
		}
std::cout<<std::endl;
	}
//----------------------/*/

//	semc_cards.EMC_DEBUG=1;

//	int nr=0;
	//cycle through runs

	info = new GInfo(run_list);
	RsSlurper* bad_channels = new RsSlurper(IOglobals::popts->getPath()+"/input/run_status");
	
	if(IOglobals::popts->isGiven("sim")) {
		char nat_file[200];
		sprintf(nat_file,run_mask.c_str());
		kedr_open_nat(nat_file,&err);
		if(err) {
			printf("open error for %s\n",nat_file);
			exit(3);
		}
	}
	
	SystemManager::trigger.read();
	SystemManager::trigger.LKr_scale=2.; SystemManager::trigger.CsI_scale=2.;
	for(T_li::const_iterator itrr=run_list.begin();itrr!=run_list.end();itrr++) {
std::cout<<"\tSet run "<<*itrr<<std::endl;
		info->SetRun(*itrr);
		ksimreal(-10,*itrr,*itrr);	//modified
		
		// opening nat file
		if(!IOglobals::popts->isGiven("sim")) {
			char nat_file[200];
			sprintf(nat_file,run_mask.c_str(),*itrr);
			kedr_open_nat(nat_file,&err);
			if(err) {
				printf("open error for %s\n",nat_file);
				continue;
			}
			std::cout<<"\nFile \""<<nat_file<<"\""<<std::endl;
		}
		
		switch_off(bad_channels,*itrr);
		
		SystemManager::trigger.set_run(*itrr);	//run doesn't matter here	//ksetscaleE(LKr_scale,CsI_scale) inside
		SystemManager::emc.set_run(*itrr);		//empty
		SystemManager::tof.set_run(*itrr);
		SystemManager::mu.set_run(*itrr);
		SystemManager::vddc.set_run(*itrr);		//empty
	
		//cycle through events
		nev=-1;
		my_bench->Start("run_all");
		
		double lump=0.,lume=0.,lumpC=0.,lumeC=0.,timerun=0.;
		float lp=0.,le=0.,lpo=0.,leo=0.,lump_int=0.,lume_int=0.;
		float kpdc=0.,ttime=-1.,ttimeo=0.,dtime=0.;
		float ptrate,strate;
		int sep=0;
		int countL=0;
		
		int Nev_max=(IOglobals::popts->isGiven("sim") ? Nev_interv[*itrr] : Nev_val[1]);
		while(nev<Nev_max) {
			kedr_read_nat(&err); if(err) break;
			
			if(!IOglobals::popts->isGiven("sim")) {
				if(kedrraw_.Header.RunNumber!=*itrr) {
					std::cerr<<"Error in run number: "<<*itrr<<"!="<<kedrraw_.Header.RunNumber
						<<"\tevent="<<nev<<std::endl;
					continue;
				//	kedrraw_.Header.RunNumber=*itrr;
				}
				kedr_lumc_(&le,&lp); kedr_lum_(&lume_int,&lump_int);
				kedr_rate_(&ptrate,&strate);
				kedr_timeinfo_(&dtime,&ttime,&kpdc);
				kedr_separation_(&sep);
				bool pathSep=(sep==1 || (le>0.1||lp>0.1)) && (le>0.05||lp>0.05);
				if(ttime>ttimeo && (le>0||lp>0)) {
					double dummy=(pathSep ? (countL ? 0.5 : 1.) : (countL ? 0. : 0.5) );
					double dTtime=(ttime-ttimeo)*kpdc*dummy;
					dummy*=(dTtime>120. ? 10.*kpdc : dtime);
					if(pathSep) { leo=le; lpo=lp; countL=0; } else if(!countL) { countL=1; }
					lumeC+=leo*0.01*dummy;
					lumpC+=lpo*0.01*dummy;
					timerun+=dummy;
					ttimeo=ttime;
				}
				if(!pathSep) { ttimeo=ttime; countL=1; }
				if(ttimeo>ttime) { ttimeo=ttime; countL=1; }
				if(!pathSep) {
				//	std::cerr<<"Beams are separated\tevent="<<nev<<std::endl;
					continue;
				}
			}
			
			info->current->nread++;
			nev++;
			if(nev%1000==0) std::cout<<"\tevent: "<<nev<<std::endl;
			if(nev<Nev_val[0]) continue;

//std::cout<<"\t-->\tEvent "<<nev<<": ";
			SystemManager::trigger.process_event();	//trigger checks are inside bhabha_prim_trig();
			if(primary_trigger_function) if(!primary_trigger_function()) {
				continue;
			}
//std::cout<<"OK"<<std::endl;
			
			SystemManager::emc.process_event();		//empty
			if(!SystemManager::vddc.process_event()) continue;	//if it's empty, skip
			SystemManager::tof.process_event();
			SystemManager::mu.process_event();
			
			if(event_function) event_function();	//if function is not NULL, run it
		}//end of event loop
		
std::cout<<"kedrrun_cb_.Header.RunType="<<static_cast<int>(kedrrun_cb_.Header.RunType)
	<<"; kedrraw_.Header.RunNumber="<<kedrraw_.Header.RunNumber<<std::endl;
if(!IOglobals::popts->isGiven("sim")) std::cout<<"Luminosity: lumeC="<<lumeC*1000.<<" lumpC="<<lumpC*1000.<<"; runtime="<<timerun<<std::endl;

		// CPU usage
		my_bench->Stop("run_all");
		info->current->cputime = my_bench->GetCpuTime("run_all");
		my_bench->Reset();

		if(!IOglobals::popts->isGiven("sim")) { close_nat(); }
	} // end of loop over runs
	if(IOglobals::popts->isGiven("sim")) { close_nat(); }
	
	SystemManager::mu.cleanup();
	
	delete bad_channels; bad_channels=0;

	info->PrintList();
	delete info; info=0;
	
	delete my_bench;
}

void Run::check( void(*event_function)() ) {
	int err=0, nev=0, rec;

	//initialization of reconstruction in systems
	if(!params.StringExists("run_mask")) std::cerr<<"Warning! String \"run_mask\" is not present."<<std::endl;
	if(!params.ListExists("run_list")) std::cerr<<"Warning! List \"run_list\" is not present."<<std::endl;

	T_s run_mask=*params.GetString("run_mask");
	T_li run_list=*params.GetList("run_list");	//run_list.sort(); is already inside GetList();
	info = new GInfo(run_list);

	SystemManager::emc.init();
	SystemManager::tof.init();
	SystemManager::mu.init();
//	SystemManager::vddc.init();

//	int nr=0;
	//cycle through runs
	for(T_li::const_iterator itrr=run_list.begin();itrr!=run_list.end();itrr++) {
		info->SetRun(*itrr);
		
		emc_run(*itrr, 0);
		tof_run(*itrr);
		SystemManager::mu.set_run(*itrr);
		kdcvdrun(*itrr,&err);

		// opening nat file
		char nat_file[200];
		sprintf(nat_file,run_mask.c_str(),(*itrr));
		kedr_open_nat(nat_file,&err);
		if(err) {
			printf("open error for %s\n",nat_file);
			continue;
		}
		std::cout<<"\nFile \""<<nat_file<<"\""<<std::endl;

		if(event_function) event_function();	//if function is not NULL, run it
		
std::cout<<"kedrrun_cb_.Header.RunType="<<static_cast<int>(kedrrun_cb_.Header.RunType)
	<<"; kedrraw_.Header.RunNumber="<<kedrraw_.Header.RunNumber<<std::endl;

		close_nat();
	} // end of loop over runs

	SystemManager::mu.cleanup();

	//print runs' information
	info->PrintList();
	delete info; info=0;
}

void Run::switch_off(RsSlurper* bad_channels, int run_number) {
	const std::map<T_li,RsSlurper::Status>* tmp_bad=bad_channels->get();
	for(std::map<T_li,RsSlurper::Status>::const_iterator itr_m=tmp_bad->begin();itr_m!=tmp_bad->end();++itr_m) {
		T_li::const_iterator itr=std::find(itr_m->first.begin(),itr_m->first.end(),run_number);
		if(itr!=itr_m->first.end()) {
		//	if(itr_m->second.energy==0) continue;	//might be just DB error, so we won't do anything harsh with it
			T_li::const_iterator itr_b;
			for(itr_b=itr_m->second.LKr_ring.begin();itr_b!=itr_m->second.LKr_ring.end();++itr_b)
				emc_channel_off(kEMC_LKR,*itr_b);
			for(itr_b=itr_m->second.LKr_bad.begin();itr_b!=itr_m->second.LKr_bad.end();++itr_b)
				emc_channel_off(kEMC_LKR,*itr_b);
			for(itr_b=itr_m->second.CsI_ring.begin();itr_b!=itr_m->second.CsI_ring.end();++itr_b)
				emc_channel_off(kEMC_CSI,*itr_b);
			for(itr_b=itr_m->second.CsI_bad.begin();itr_b!=itr_m->second.CsI_bad.end();++itr_b)
				emc_channel_off(kEMC_CSI,*itr_b);
		}
	}
}

Run::GInfo::GRun::GRun():
	nrun(0),status(0),nread(0),
	nlkr(0),nlkr_raw(0),ncsi(0),ncsi_raw(0),nemc(0),nemc_raw(0) {
		bzero(&kdbInfo,sizeof(KDBruninfo));
}
Run::GInfo::GRun::~GRun() { }

Run::GInfo::GInfo(): current(0), conn(0) { }
Run::GInfo::GInfo(const std::list<int>& ls): current(0) {
	conn=kdb_open();
	if(!conn) std::cerr<<"Could not connect to database."<<std::endl;
	SetRunList(ls);
}
Run::GInfo::~GInfo() {
	clear();
	if(conn) { kdb_close(conn); conn=0; }
}

Run::GInfo::GRun* Run::GInfo::GetRun(int nrun) {
	struct tm *tms;
	if(!conn) {
		std::cerr<<"GInfo::GetRun -> Error, no DB opened"<<std::endl;
		return NULL;
	}
	GInfo::GRun* run = new GRun();
	run->nrun=nrun;
	run->status=kdb_run_check_existance(conn,nrun);
	if(run->status<1) run->kdbInfo.reserved=-1;
	if(!kdb_run_get_info(conn, nrun, 1, &(run->kdbInfo)) ) {
		std::cerr<<"GInfo::GetRun()->Could not get run information for nrun="<<nrun<<std::endl;
	}
	time_t time_begin = run->kdbInfo.time_begin;
	tms=localtime(&time_begin);
	strftime(run->date,30,"(%d/%m/%Y %a %H:%M)",tms);
	return run;
}

void Run::GInfo::SetRun(int nrun) {
	if(!runs.count(nrun)) {
		current=GetRun(nrun);
		runs[nrun]=current;
	} else current=runs[nrun];
//	kedrraw_.Header.RunNumber=nrun;		//patched for global connectivity (e.g. emc_all())
}

void Run::GInfo::SetRunList(const std::list<int>& ls) {
	if(!conn) conn=kdb_open();
	for(T_li::const_iterator itr=ls.begin();itr!=ls.end();itr++) {
		SetRun(*itr);
	}
//	if(conn) { kdb_close(conn); conn=0; }	//should not be here, kdb will be open through GInfo lifetime
	current=runs.begin()->second;	//first run in the map is set to be current one
}

void Run::GInfo::PrintList() const {
	if(runs.empty()) return;
	std::map<int, GInfo::GRun*>::const_iterator itr;
	double rate=0.;
	int indx=0;
	
	int tot_nread=0, tot_nwrite=0, tot_nlkr=0, tot_ncsi=0, tot_nemc=0;
	int tot_nlkr_raw=0, tot_ncsi_raw=0, tot_nemc_raw=0;
	float tot_cputime=0.;
	
	std::cout<<"------- Run statistics -------"<<std::endl;
	std::cout<<"  i  nrun  date                   r  nwrite  nread       rate  nlkr nlkr_raw  ncsi ncsi_raw  nemc nemc_raw"<<std::endl;
	for(itr=runs.begin();itr!=runs.end();++itr) {
		GInfo::GRun* run = itr->second;
		rate=0.;
		if(run->cputime) rate = static_cast<double>(static_cast<float>(run->nread)/run->cputime/1000.);
		printf("%3d %5d %s %2d %7d %6d    %4.3fkH %5d    %5d %5d    %5d %5d    %5d\n",
			indx,run->nrun,run->date,run->kdbInfo.reserved,run->kdbInfo.nwrite,run->nread,rate,
			run->nlkr,run->nlkr_raw,run->ncsi,run->ncsi_raw,run->nemc,run->nemc_raw);
		indx++;
		tot_nwrite+=run->kdbInfo.nwrite; tot_nread+=run->nread; tot_cputime+=run->cputime;
		tot_nlkr+=run->nlkr; tot_ncsi+=run->ncsi; tot_nemc+=run->nemc;
		tot_nlkr_raw+=run->nlkr_raw; tot_ncsi_raw+=run->ncsi_raw; tot_nemc_raw+=run->nemc_raw;
	}
	if(tot_cputime) rate = static_cast<double>(static_cast<float>(tot_nread)/tot_cputime/1000.);
	printf("::Total::   [runs %5d-%5d]    %9d %6d    %4.3fkH %5d    %5d %5d    %5d %5d    %5d\n",
		(runs.empty()?0:runs.begin()->second->nrun),(runs.empty()?0:runs.rbegin()->second->nrun),
		tot_nwrite,tot_nread,rate,tot_nlkr,tot_nlkr_raw,tot_ncsi,tot_ncsi_raw,tot_nemc,tot_nemc_raw);
	std::cout<<"------- ||| |||||||||| -------"<<std::endl;
}

void Run::GInfo::clear() {
	std::map<int, GRun*>::const_iterator itr;
	for(itr=runs.begin();itr!=runs.end();itr++) delete itr->second;
	runs.clear();
}
