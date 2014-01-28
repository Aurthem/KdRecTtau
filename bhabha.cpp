
#include "LKrTools/glkr_tools.h"
#include "KEmcRec/emc_system.h"		//semc, semc_data
#include "VDDCRec/ktracks.h"		//ktrrec_, ktrrec_h_
#include "VDDCRec/kvectors.h"		//kverec_
#include "VDDCRec/khits.h"			//kdcrec_
#include "KrMu/mu_system.h"
#include "KrMu/mu_event.h"
#include "KrToF/tof_system.h"		//sc_hits.h: kschit_; scb_hits.h: kscBhit_

//#include "Minuit2/FCNBase.h"
//#include "TFitterMinuit.h"
#include "TMinuit.h"
#include "TMatrixT.h"
#include "TVectorT.h"

#include "modules.h"

namespace Bhabha {
	DataManager* dm=0;

	Run* run=0;
	Criteria emc, vddc, all_cr;
	RsSlurper* run_status=0;

	void bhabha_event_all(void);
	void bhabha_event_lkr(void);	//forward declarations
	void bhabha_event_csi(void);
	void bhabha_event_emc(void);
	void bhabha_event_vddc(void);
	void bhabha_event_mu(void);
	void bhabha_event_tof(void);

	void bhabha_event(void) {
		dm->clear();
		bhabha_event_all();
	//	const DataClass::Data<double>* tmp_lkr=dynamic_cast<const DataClass::Data<double>*>(dm->data->Get("all_bhabha::vddc.LKr.sphericity"));
	//	const double* tmp_lkr_val=(tmp_lkr?tmp_lkr->get():0);
	//	if(tmp_lkr_val) {
	//		dm->data->print();
	//		kdisplay_event();
	//	//	std::cout<<"Debug:\tEvNum="<<kdcenum_.EvNum<<"\tDAQ"<<kdcenum_.EvNumDAQ<<std::endl;
	//	}
/*/-----separate_systems-----//
		bhabha_event_lkr();
		bhabha_event_csi(); //finds bhabha in CsI
		bhabha_event_emc(); //finds bhabha in LKr&CsI combined
		bhabha_event_vddc();
		bhabha_event_mu();
		bhabha_event_tof();		//raw
//-----separate_systems-----/*/

		if(!dm->data->empty()) {
		//	dm->data->print();
		//	kdisplay_event();
			dm->next();	//fills tree
		}
	}

	bool bhabha_prim_trig(void) {
		bool result=false;
		dm->clear_buf();
		
		float E_CsI[2], E_LKr;
		kemc_energy(E_CsI,&E_LKr);
		
		if(dm->BuffCheck(emc,"emc_bhabha::emc_primary.total_energy",static_cast<double>(E_CsI[0]+E_CsI[1]+E_LKr),"MeV")) {
			dm->flush();
			result=true;
		}
		return (result && SystemManager::trigger.PT_passed && SystemManager::trigger.ST_passed);
	}

	void bhabha(void) {	//histograms created
	//	kdcenum_.EvDBG=25059;	//debugging, reconstruction and raw event numbers do not match
		dm=new DataManager(IOglobals::popts->getPath());
		run=new Run("bhabha");	//block name inside run_params

		CrsSlurper crs(IOglobals::popts->getPath()+"/input/criteria");
		emc.fill(crs.get("emc_bhabha"));	//this string can be generalized, DataManager checks are suitable for this
		vddc.fill(crs.get("vddc_bhabha"));
		all_cr.fill(crs.get("all_bhabha"));
		run_status = new RsSlurper(IOglobals::popts->getPath()+"/input/run_status");

		if(IOglobals::popts->isGiven("sim")) {	//simulation
			dm->batch.SetSim(true);
			run->params.SetChar("sim_cosm_deb",0,'1');	//these lines should go
		} else {	//experiment - default
			dm->batch.SetSim(false);
			run->params.SetChar("sim_cosm_deb",0,'0');
		}
		if(IOglobals::popts->isGiven("skim")) {
			dm->init_skim("/spool/users/boroden/skim/skim_ee");
			dm->clear();
			run->all(bhabha_prim_trig,bhabha_event);	//cycles through runs and events, calling user function on each event
		} else {
			if(IOglobals::popts->isGiven("batch")) {
				dm->batch.init(*IOglobals::popts->getValue("batch"),run->params.GetList(IOglobals::popts->isGiven("sim")?"sim_list":"run_list"));
			}

			T_s base_dm="data";
			const T_s* app_dm=0;
			if( (app_dm=IOglobals::popts->getValue("label")) ) base_dm+=*app_dm;
			if( IOglobals::popts->isGiven("sim") ) base_dm+="_sim";
			dm->init(base_dm);

			do{	dm->clear();
				const T_li* dummy=dm->batch.GetRuns();
				if(dummy) {
					if(!dummy->empty()) run->params.SetList((IOglobals::popts->isGiven("sim")?"sim_list":"run_list"),*dummy);
					else continue;
				}
				run->all(bhabha_prim_trig,bhabha_event);	//cycles through runs and events, calling user function on each event
				dm->write();
			} while(dm->batchNext());
		}

		delete run_status; run_status=0;
		delete run; run=0;
		delete dm; dm=0;
	}

	void run_channel_info(void);
	void channel_info(void) {
		dm=new DataManager(IOglobals::popts->getPath(),"bad_channels");
		run=new Run("bhabha");	//block name inside run_params
		dm->clear();
		run->check(run_channel_info);	//cycles through runs and events, calling user function on each event
		dm->write();
		delete run; run=0;
		delete dm; dm=0;
	}

//-------|---------implementation---------|-------//
//------\|/-----aka_private_methods------\|/------//

//finds two collinear clusters with max energy
//clusters with bad cells>10 or max cell on the edge are ignored
	float semc_getTheta(int cls_num) {
		float result=semc.emc_theta_str[cls_num];
		if(result<1000.) return result;
		return semc.emc_theta[cls_num];
	}
	float semc_getPhi(int cls_num) {
		float result=semc.emc_phi_str[cls_num];
		if(result<1000.) return result;
		return semc.emc_phi[cls_num];
	}
	double getAngle(float theta1, float phi1, float theta2, float phi2) {
		double dummy=SUconverter::get_multiplier("deg","rad");
		double cos_dphi=cos(dummy*(phi2-phi1));
		return acos(0.5*( (1.+cos_dphi)*cos(dummy*(theta2-theta1)) + (1.-cos_dphi)*cos(dummy*(theta2+theta1)) ))*
			SUconverter::get_multiplier("rad","deg");
	}
	double semc_getAngle(int cls_n1, int cls_n2) {
		return getAngle(semc_getTheta(cls_n1),semc_getPhi(cls_n1),semc_getTheta(cls_n2),semc_getPhi(cls_n2));
	}
	
	class JetStructure {
	private:
		class MyFCN {
		public:
			static void function_sph(Int_t& npar, Double_t* grad, Double_t& fval, Double_t* par, Int_t flag) {
				if(npar<2) { fval=-1.; return; }
				double pt2_sum=0., p2_sum=0.;
				double nz=cos(par[0]), nx=sin(par[0])*cos(par[1]), ny=sin(par[0])*sin(par[1]);
				for(int idx=0;idx<ktrrec_.NTRAK;++idx) {
					double tmp=nx*ktrrec_.VxTRAK[idx]+ny*ktrrec_.VyTRAK[idx]+nz*ktrrec_.VzTRAK[idx];
					pt2_sum+=ktrrec_.PTRAK[idx]*ktrrec_.PTRAK[idx]*(1-tmp*tmp);
					p2_sum+=ktrrec_.PTRAK[idx]*ktrrec_.PTRAK[idx];
				}
				fval=(p2_sum ? 1.5*pt2_sum/p2_sum : 0.);
			}
			static void function_thr(Int_t& npar, Double_t* grad, Double_t& fval, Double_t* par, Int_t flag) {
				if(npar<2) { fval=-1.; return; }
				double pn_sum=0., p_sum=0.;
				double nz=cos(par[0]), nx=sin(par[0])*cos(par[1]), ny=sin(par[0])*sin(par[1]);
				for(int idx=0;idx<ktrrec_.NTRAK;++idx) {
					double tmp=nx*ktrrec_.VxTRAK[idx]+ny*ktrrec_.VyTRAK[idx]+nz*ktrrec_.VzTRAK[idx];
					pn_sum+=fabs(ktrrec_.PTRAK[idx]*tmp);
					p_sum+=ktrrec_.PTRAK[idx];
				}
				fval=(p_sum ? pn_sum/p_sum : 0.);
			}
		private:
		};
		static MyFCN fcn;
		
		TMinuit* minuit;
		bool minimize(void) {
			minuit->SetPrintLevel(-1);
			double arglist[10]={}; Int_t ierflg=0;
			minuit->mnexcm("SET NOWarnings",0,0,ierflg);	//no warnings
			arglist[0]=2;
			minuit->mnexcm("SET STRategy",arglist,1,ierflg);	//try to improve minimum
			minuit->SetErrorDef(0.5);	//likelihood errors (set 1 for chi2)
			minuit->SetMaxIterations(1000);
			
			minuit->DefineParameter(0,	"theta",1.,0.001,0.,TMath::Pi());
			minuit->DefineParameter(1,	"phi",	0.,0.001,-TMath::Pi(),TMath::Pi());
			int iret=minuit->Migrad();
			return !iret;
		}
	public:
		JetStructure(void) {
			minuit = new TMinuit(2);	//2 parameters (direction)
		}
		~JetStructure(void) { delete minuit; }
		
		double getSphericity(void) {
			minuit->SetFCN(&(fcn.function_sph));
			if(!minimize()) return -1.;
			double tmp;
			if(minuit->Eval(2,0,tmp,&get_params()[0],0)) return -1.;	//flag=1-init,2-derivatives,3-after fit,else - function
			return tmp;
		}
		double getThrust(void) {
			minuit->SetFCN(&(fcn.function_thr));
			if(!minimize()) return -1.;
			double tmp;
			if(minuit->Eval(2,0,tmp,&get_params()[0],0)) return -1.;	//flag=1-init,2-derivatives,3-after fit,else - function
			return tmp;
		}
		T_vd get_params(void) const {
			T_vd result;
			double tmp_val, tmp_err;
			minuit->GetParameter(0,tmp_val,tmp_err); result.push_back(tmp_val);
			minuit->GetParameter(1,tmp_val,tmp_err); result.push_back(tmp_val);
			return result;
		}
		T_pdd getSphericityM(void) const {		//without minuit, returns sphericity and aplanarity
			TMatrixT<double> Mpij(3,3);
			double norm=0.;
			for(int idx=0;idx<ktrrec_.NTRAK;++idx) {
				double p2=ktrrec_.PTRAK[idx]*ktrrec_.PTRAK[idx];
				double pn[3]={ ktrrec_.VxTRAK[idx], ktrrec_.VyTRAK[idx], ktrrec_.VzTRAK[idx] };
				for(int i=0;i<3;++i) for(int j=0;j<3;++j) Mpij[i][j]+=p2*pn[i]*pn[j];
				norm+=p2;
			}
			TVectorT<double> eigenv(3);
			Mpij.EigenVectors(eigenv);
			//(eigenv[0]+eigenv[1]+eigenv[2])/norm==1.;
			return std::make_pair(
				1.5*(1.-std::max(eigenv[0],std::max(eigenv[1],eigenv[2]))/norm),
				1.5*std::min(eigenv[0],std::min(eigenv[1],eigenv[2]))/norm	);
		}
	};
	JetStructure::MyFCN JetStructure::fcn=JetStructure::MyFCN();

	void bhabha_event_all() {
		dm->clear_buf();
		
		if(	!dm->BuffCheck(all_cr,"all_bhabha::mu.hit_type",static_cast<int>(SystemManager::mu.result_type))	||
			!dm->BuffCheck(all_cr,"all_bhabha::vddc.op_tracks",ktrrec_.NtrackIP)	) return;
		
		double total_energy_lkr=0., total_energy_csi=0.;
		for(int idx=0;idx<semc.emc_ncls;++idx) {
			if(semc.emc_type[idx]==1) total_energy_lkr+=semc.emc_energy[idx];		//LKr
			if(semc.emc_type[idx]==2) total_energy_csi+=semc.emc_energy[idx];		//CsI
		}
		double tmp_high_energy_lkr=0., tmp_high_energy_csi=0.;
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==1) {
		if(semc.emc_energy[idx1]>tmp_high_energy_lkr) {
			if( !dm->BuffCheck(all_cr,"all_bhabha::LKr.high.cluster_energy",semc.emc_energy[idx1],"MeV")	||
				!dm->BuffCheck(all_cr,"all_bhabha::LKr.high.theta",semc_getTheta(idx1),"deg")	) {
					continue;
			}
			tmp_high_energy_lkr=semc.emc_energy[idx1];
			
			for(int idx2=0;idx2<semc.emc_ncls;++idx2)
			if(idx2!=idx1 && semc.emc_type[idx2]==1 && semc.emc_energy[idx2]<=semc.emc_energy[idx1]) {
				if(	!dm->BuffCheck(all_cr,"all_bhabha::LKr.low.cluster_energy",semc.emc_energy[idx2],"MeV")||
					!dm->BuffCheck(all_cr,"all_bhabha::LKr.low.theta",semc_getTheta(idx2),"deg")			||
					!dm->BuffCheck(all_cr,"all_bhabha::LKr.energy_diff",semc.emc_energy[idx1]-semc.emc_energy[idx2],"MeV")) {
						continue;
				}

				double angle=semc_getAngle(idx1,idx2);
				if(dm->BuffCheck(all_cr,"all_bhabha::LKr.rel_angle",fabs(180.-angle),"deg")								&&
				dm->BuffCheck(all_cr,"all_bhabha::LKr.rel_theta",fabs(180.-semc_getTheta(idx1)-semc_getTheta(idx2)),"deg")	&&
				dm->BuffCheck(all_cr,"all_bhabha::LKr.rel_phi",fabs(180.-fabs(semc_getPhi(idx1)-semc_getPhi(idx2))),"deg")) {

					double high_track=0., low_track=0.; int high_track_idx=-1, low_track_idx=-1;
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx1];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx1][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>high_track) {
							high_track=ktrrec_.PTRAK[tmp_idx];
							high_track_idx=tmp_idx;
						}
					}
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx2];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx2][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>low_track) {
							low_track=ktrrec_.PTRAK[tmp_idx];
							low_track_idx=tmp_idx;
						}
					}
					if(high_track && low_track && high_track<low_track) {
						if(	!dm->BuffCheck(all_cr,"all_bhabha::LKr.high.cluster_energy",semc.emc_energy[idx2],"MeV")	||
							!dm->BuffCheck(all_cr,"all_bhabha::LKr.high.theta",semc_getTheta(idx2),"deg")				||
							!dm->BuffCheck(all_cr,"all_bhabha::LKr.low.cluster_energy",semc.emc_energy[idx1],"MeV")	||
							!dm->BuffCheck(all_cr,"all_bhabha::LKr.low.theta",semc_getTheta(idx1),"deg")	) {
								continue;
						}
						double tmp=low_track; low_track=high_track; high_track=tmp;
					}
					int vd_hits=0;
					for(int idx=0;idx<ktrrec_.NTRAK;++idx) {
						if(ktrrec_.KvdTRAK[idx]>vd_hits) vd_hits=ktrrec_.KvdTRAK[idx];
					}
					if( !(high_track || low_track) && !dm->BuffCheck(all_cr,"all_bhabha::vddc.LKr.vd_hits",vd_hits) )
						continue;	//there should be at least one track or at least some hits (_or_ is && in inverted logic)
					
					int residue40=0, residue80=0;
					for(int idx=0;idx<semc.emc_ncls;++idx) {
						if(idx==idx1 || idx==idx2) continue;
						if(semc.emc_energy[idx]>80.) ++residue80;
						else if(semc.emc_energy[idx]>40.) ++residue40;
					}
					if(	!dm->BuffCheck(all_cr,"all_bhabha::LKr.residue40",residue80)	||
						!dm->BuffCheck(all_cr,"all_bhabha::LKr.residue80",residue40)	) continue;
					
					if(total_energy_lkr!=0. && !dm->BuffCheck(all_cr,"all_bhabha::LKr.energy_div_total",static_cast<double>((semc.emc_energy[idx1]+semc.emc_energy[idx2])/total_energy_lkr)) ) continue;
					
					if(!dm->BuffCheck(all_cr,"all_bhabha::vddc.LKr.r_tracks",ktrrec_.NtrackR)) continue;
					JetStructure jstruct;
					double sph_tmp=jstruct.getSphericityM().first;	//matrix definition should calculate quicker and more precise
					if(!(sph_tmp<0) && !dm->BuffCheck(all_cr,"all_bhabha::vddc.LKr.sphericity",sph_tmp)) continue;
					
					if(SystemManager::tof.times_track_barrel.empty() && SystemManager::tof.times_emc_barrel.empty() && SystemManager::tof.times_emc_endcap.empty()) continue;
					
					dm->flush();
					dm->data->Add("all_bhabha::tof.barrel.track",SystemManager::tof.times_track_barrel,"ns");
					dm->data->Add("all_bhabha::tof.barrel.emc",SystemManager::tof.times_emc_barrel,"ns");
					dm->data->Add("all_bhabha::tof.endcap.emc",SystemManager::tof.times_emc_endcap,"ns");

					if(high_track) dm->data->Add("all_bhabha::LKr.high.track_momentum",high_track,"MeV");
					if(low_track) dm->data->Add("all_bhabha::LKr.low.track_momentum",low_track,"MeV");
					
					int ntracks=0; if(high_track) ++ntracks; if(low_track) ++ntracks;
					dm->data->Add("all_bhabha::LKr.ntracks",ntracks);
					
					dm->data->Add("all_bhabha::LKr.invMass",static_cast<double>(sqrt(2.*semc.emc_energy[idx1]*semc.emc_energy[idx2]*(1.-cos(angle*SUconverter::get_multiplier("deg","rad"))))),"MeV");
					dm->data->Add("all_bhabha::vddc.track.high.theta",acos(ktrrec_.VzTRAK[idx1])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.low.theta",acos(ktrrec_.VzTRAK[idx2])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.high.phi",(ktrrec_.VyTRAK[idx1]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx1]/sqrt(1.-ktrrec_.VzTRAK[idx1]*ktrrec_.VzTRAK[idx1]))*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.low.phi",(ktrrec_.VyTRAK[idx2]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx2]/sqrt(1.-ktrrec_.VzTRAK[idx2]*ktrrec_.VzTRAK[idx2]))*SUconverter::get_multiplier("rad","deg"),"deg");
					
					break;
				}
			}
		} }
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==2) {	//CsI
		if(semc.emc_energy[idx1]>tmp_high_energy_csi) {
			if(	!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.cluster_energy",semc.emc_energy[idx1],"MeV")	||
				!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.theta",semc_getTheta(idx1),"deg")				||
				!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.rho",semc.emc_rho[idx1],"cm")	) {
					continue;
			}
			tmp_high_energy_csi=semc.emc_energy[idx1];

			for(int idx2=0;idx2<semc.emc_ncls;++idx2)
			if(idx2!=idx1 && semc.emc_type[idx2]==2 && semc.emc_energy[idx2]<=semc.emc_energy[idx1]) {
				if(	!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.cluster_energy",semc.emc_energy[idx2],"MeV")||
					!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.theta",semc_getTheta(idx2),"deg")			||
					!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.rho",semc.emc_rho[idx2],"cm")				||
					!dm->BuffCheck(all_cr,"all_bhabha::CsI.energy_diff",fabs(semc.emc_energy[idx1]-semc.emc_energy[idx2]),"MeV")) {
						continue;
				}
				double angle=semc_getAngle(idx1,idx2);
				if(dm->BuffCheck(all_cr,"all_bhabha::CsI.rel_angle",fabs(180.-angle),"deg")								&&
				dm->BuffCheck(all_cr,"all_bhabha::CsI.rel_theta",fabs(180.-semc_getTheta(idx1)-semc_getTheta(idx2)),"deg")	&&
				dm->BuffCheck(all_cr,"all_bhabha::CsI.rel_phi",fabs(180.-fabs(semc_getPhi(idx1)-semc_getPhi(idx2))),"deg")) {
					double high_track=0., low_track=0.; int high_track_idx=-1, low_track_idx=-1;
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx1];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx1][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>high_track) {
							high_track=ktrrec_.PTRAK[tmp_idx];
							high_track_idx=tmp_idx;
						}
					}
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx2];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx2][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>low_track) {
							low_track=ktrrec_.PTRAK[tmp_idx];
							low_track_idx=tmp_idx;
						}
					}
					if(high_track && low_track && high_track<low_track) {
						if(	!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.cluster_energy",semc.emc_energy[idx2],"MeV")||
							!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.theta",semc_getTheta(idx2),"deg")			||
							!dm->BuffCheck(all_cr,"all_bhabha::CsI.high.rho",semc.emc_rho[idx2],"cm")				||
							!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.cluster_energy",semc.emc_energy[idx1],"MeV")	||
							!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.theta",semc_getTheta(idx1),"deg")			||	
							!dm->BuffCheck(all_cr,"all_bhabha::CsI.low.rho",semc.emc_rho[idx1],"cm")	) {
								continue;
						}
						double tmp=low_track; low_track=high_track; high_track=tmp;
					}
					int vd_hits=0;
					for(int idx=0;idx<ktrrec_.NTRAK;++idx) {
						if(ktrrec_.KvdTRAK[idx]>vd_hits) vd_hits=ktrrec_.KvdTRAK[idx];
					}
					if( !(high_track || low_track) && !dm->BuffCheck(all_cr,"all_bhabha::vddc.CsI.vd_hits",vd_hits) )
						continue;	//there should be at least one track or at least some hits (_or_ is && in inverted logic)
					
					int residue40=0, residue80=0;
					for(int idx=0;idx<semc.emc_ncls;++idx) {
						if(idx==idx1 || idx==idx2) continue;
						if(semc.emc_energy[idx]>80.) ++residue80;
						else if(semc.emc_energy[idx]>40.) ++residue40;
					}
					if(	!dm->BuffCheck(all_cr,"all_bhabha::CsI.residue40",residue80)	||
						!dm->BuffCheck(all_cr,"all_bhabha::CsI.residue80",residue40)	) continue;
					
					if(total_energy_csi!=0. && !dm->BuffCheck(all_cr,"all_bhabha::CsI.energy_div_total",static_cast<double>((semc.emc_energy[idx1]+semc.emc_energy[idx2])/total_energy_csi)) ) continue;
					
					int tof_nodata=0;
					if(SystemManager::tof.times_track_barrel.empty()) ++tof_nodata;
					if(SystemManager::tof.times_emc_barrel.empty()) ++tof_nodata;
					if(SystemManager::tof.times_emc_endcap.empty()) ++tof_nodata;
					if(!dm->BuffCheck(all_cr,"all_bhabha::tof.no_data",tof_nodata)) continue;
					
					dm->flush();
					dm->data->Add("all_bhabha::tof.barrel.track",SystemManager::tof.times_track_barrel,"ns");
					dm->data->Add("all_bhabha::tof.barrel.emc",SystemManager::tof.times_emc_barrel,"ns");
					dm->data->Add("all_bhabha::tof.endcap.emc",SystemManager::tof.times_emc_endcap,"ns");
					
					if(high_track) dm->data->Add("all_bhabha::CsI.high.track_momentum",high_track,"MeV");
					if(low_track)  dm->data->Add("all_bhabha::CsI.low.track_momentum",low_track,"MeV");
					
					if(semc.emc_y[idx1]*semc.emc_z[idx1]<0.)
						dm->data->Add("all_bhabha::CsI.high.up_cluster",static_cast<double>(semc.emc_energy[idx1]),"MeV");
					else
						dm->data->Add("all_bhabha::CsI.high.down_cluster",static_cast<double>(semc.emc_energy[idx1]),"MeV");
					if(semc.emc_y[idx2]*semc.emc_z[idx2]<0.)
						dm->data->Add("all_bhabha::CsI.low.up_cluster",static_cast<double>(semc.emc_energy[idx2]),"MeV");
					else
						dm->data->Add("all_bhabha::CsI.low.down_cluster",static_cast<double>(semc.emc_energy[idx2]),"MeV");
					
					int ntracks=0; if(high_track) ++ntracks; if(low_track) ++ntracks;
					dm->data->Add("all_bhabha::CsI.ntracks",ntracks);
					
					dm->data->Add("all_bhabha::CsI.invMass",static_cast<double>(sqrt(2.*semc.emc_energy[idx1]*semc.emc_energy[idx2]*(1.-cos(angle*SUconverter::get_multiplier("deg","rad"))))),"MeV");
					dm->data->Add("all_bhabha::vddc.track.high.theta",acos(ktrrec_.VzTRAK[idx1])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.low.theta",acos(ktrrec_.VzTRAK[idx2])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.high.phi",(ktrrec_.VyTRAK[idx1]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx1]/sqrt(1.-ktrrec_.VzTRAK[idx1]*ktrrec_.VzTRAK[idx1]))*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("all_bhabha::vddc.track.low.phi",(ktrrec_.VyTRAK[idx2]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx2]/sqrt(1.-ktrrec_.VzTRAK[idx2]*ktrrec_.VzTRAK[idx2]))*SUconverter::get_multiplier("rad","deg"),"deg");
					
					break;
				}
			}
		} }
	}
	void bhabha_event_lkr() {
		dm->clear_buf();
		double tmp_high_energy=0.;
		double total_energy=0.;
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==1) {	//LKr
			total_energy+=semc.emc_energy[idx1];
		}
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==1) {	//LKr
			run->info->current->nlkr_raw++;
		if(semc.emc_energy[idx1]>tmp_high_energy) {
			if(	!dm->BuffCheck(emc,"emc_bhabha::LKr.high.edge_maxcells",semc.emc_qlty[idx1])			||
				!dm->BuffCheck(emc,"emc_bhabha::LKr.high.bad_cells",semc.emc_ncells_bad[idx1])			||
				!dm->BuffCheck(emc,"emc_bhabha::LKr.high.cluster_energy",semc.emc_energy[idx1],"MeV")	||
				!dm->BuffCheck(emc,"emc_bhabha::LKr.high.theta",semc_getTheta(idx1),"deg")	) {
					continue;
			}
			tmp_high_energy=semc.emc_energy[idx1];

			for(int idx2=0;idx2<semc.emc_ncls;++idx2)
			if(idx2!=idx1 && semc.emc_type[idx2]==1 && semc.emc_energy[idx2]<=semc.emc_energy[idx1]) {
				if(	!dm->BuffCheck(emc,"emc_bhabha::LKr.low.edge_maxcells",semc.emc_qlty[idx2])			||
					!dm->BuffCheck(emc,"emc_bhabha::LKr.low.bad_cells",semc.emc_ncells_bad[idx2])		||
					!dm->BuffCheck(emc,"emc_bhabha::LKr.low.cluster_energy",semc.emc_energy[idx2],"MeV")||
					!dm->BuffCheck(emc,"emc_bhabha::LKr.low.theta",semc_getTheta(idx2),"deg")			||
					!dm->BuffCheck(emc,"emc_bhabha::LKr.energy_diff",semc.emc_energy[idx1]-semc.emc_energy[idx2],"MeV")) {
						continue;
				}

				double angle=semc_getAngle(idx1,idx2);
				if(dm->BuffCheck(emc,"emc_bhabha::LKr.rel_angle",fabs(180.-angle),"deg")								&&
				dm->BuffCheck(emc,"emc_bhabha::LKr.rel_theta",fabs(180.-semc_getTheta(idx1)-semc_getTheta(idx2)),"deg")	&&
				dm->BuffCheck(emc,"emc_bhabha::LKr.rel_phi",fabs(180.-fabs(semc_getPhi(idx1)-semc_getPhi(idx2))),"deg")) {

					double high_track=0., low_track=0.; int high_track_idx=-1, low_track_idx=-1;
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx1];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx1][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>high_track) {
							high_track=ktrrec_.PTRAK[tmp_idx];
							high_track_idx=tmp_idx;
						}
					}
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx2];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx2][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>low_track) {
							low_track=ktrrec_.PTRAK[tmp_idx];
							low_track_idx=tmp_idx;
						}
					}
					if(high_track && low_track && high_track<low_track) {
						if(	!dm->BuffCheck(emc,"emc_bhabha::LKr.high.cluster_energy",semc.emc_energy[idx2],"MeV")	||
							!dm->BuffCheck(emc,"emc_bhabha::LKr.high.theta",semc_getTheta(idx2),"deg")				||
							!dm->BuffCheck(emc,"emc_bhabha::LKr.low.cluster_energy",semc.emc_energy[idx1],"MeV")	||
							!dm->BuffCheck(emc,"emc_bhabha::LKr.low.theta",semc_getTheta(idx1),"deg")	) {
								continue;
						}
						double tmp=low_track; low_track=high_track; high_track=tmp;
					}
					if(!high_track && !low_track) continue;	//there should be at least one track

					dm->flush();

					if(high_track) dm->data->Add("emc_bhabha::LKr.high.track_momentum",high_track,"MeV");
					if(low_track)  dm->data->Add("emc_bhabha::LKr.low.track_momentum",low_track,"MeV");

					dm->data->Add("emc_bhabha::LKr.invMass",static_cast<double>(sqrt(2.*semc.emc_energy[idx1]*semc.emc_energy[idx2]*(1.-cos(angle*SUconverter::get_multiplier("deg","rad"))))),"MeV");
					if(total_energy!=0.)
						dm->data->Add("emc_bhabha::LKr.energy_div_total",static_cast<double>((semc.emc_energy[idx1]+semc.emc_energy[idx2])/total_energy));
					run->info->current->nlkr+=2;	//both clusters are counted
				//	return;	//should finish searching, probably
				//	idx1=idx2+1; break;
					break;
				}
			}
		} }
	}

//finds two collinear clusters with max energy
//clusters with bad cells>10 or max cell on the edge are ignored
	void bhabha_event_csi() {
		dm->clear_buf();
		double tmp_high_energy=0.;
		double total_energy=0.;
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==2) {	//CsI
			total_energy+=semc.emc_energy[idx1];
		}
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) if(semc.emc_type[idx1]==2) {	//CsI
			run->info->current->ncsi_raw++;
		if(semc.emc_energy[idx1]>tmp_high_energy) {
			if(	!dm->BuffCheck(emc,"emc_bhabha::CsI.high.edge_maxcells",semc.emc_qlty[idx1])			||
				!dm->BuffCheck(emc,"emc_bhabha::CsI.high.bad_cells",semc.emc_ncells_bad[idx1])			||
				!dm->BuffCheck(emc,"emc_bhabha::CsI.high.cluster_energy",semc.emc_energy[idx1],"MeV")	||
				!dm->BuffCheck(emc,"emc_bhabha::CsI.high.theta",semc_getTheta(idx1),"deg")				||
				!dm->BuffCheck(emc,"emc_bhabha::CsI.high.rho",semc.emc_rho[idx1],"cm")	) {
					continue;
			}
			tmp_high_energy=semc.emc_energy[idx1];

			for(int idx2=0;idx2<semc.emc_ncls;++idx2)
			if(idx2!=idx1 && semc.emc_type[idx2]==2 && semc.emc_energy[idx2]<=semc.emc_energy[idx1]) {
				if(	!dm->BuffCheck(emc,"emc_bhabha::CsI.low.edge_maxcells",semc.emc_qlty[idx2])			||
					!dm->BuffCheck(emc,"emc_bhabha::CsI.low.bad_cells",semc.emc_ncells_bad[idx2])		||
					!dm->BuffCheck(emc,"emc_bhabha::CsI.low.cluster_energy",semc.emc_energy[idx2],"MeV")||
					!dm->BuffCheck(emc,"emc_bhabha::CsI.low.theta",semc_getTheta(idx2),"deg")			||
					!dm->BuffCheck(emc,"emc_bhabha::CsI.low.rho",semc.emc_rho[idx2],"cm")				||
					!dm->BuffCheck(emc,"emc_bhabha::CsI.energy_diff",fabs(semc.emc_energy[idx1]-semc.emc_energy[idx2]),"MeV")) {
						continue;
				}
				double angle=semc_getAngle(idx1,idx2);
				if(dm->BuffCheck(emc,"emc_bhabha::CsI.rel_angle",fabs(180.-angle),"deg")								&&
				dm->BuffCheck(emc,"emc_bhabha::CsI.rel_theta",fabs(180.-semc_getTheta(idx1)-semc_getTheta(idx2)),"deg")	&&
				dm->BuffCheck(emc,"emc_bhabha::CsI.rel_phi",fabs(180.-fabs(semc_getPhi(idx1)-semc_getPhi(idx2))),"deg")) {

					double high_track=0., low_track=0.; int high_track_idx=-1, low_track_idx=-1;
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx1];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx1][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>high_track) {
							high_track=ktrrec_.PTRAK[tmp_idx];
							high_track_idx=tmp_idx;
						}
					}
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx2];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx2][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>low_track) {
							low_track=ktrrec_.PTRAK[tmp_idx];
							low_track_idx=tmp_idx;
						}
					}
					if(high_track && low_track && high_track<low_track) {
						if(	!dm->BuffCheck(emc,"emc_bhabha::CsI.high.cluster_energy",semc.emc_energy[idx2],"MeV")	||
							!dm->BuffCheck(emc,"emc_bhabha::CsI.high.theta",semc_getTheta(idx2),"deg")				||
							!dm->BuffCheck(emc,"emc_bhabha::CsI.high.rho",semc.emc_rho[idx2],"cm")					||
							!dm->BuffCheck(emc,"emc_bhabha::CsI.low.cluster_energy",semc.emc_energy[idx1],"MeV")	||
							!dm->BuffCheck(emc,"emc_bhabha::CsI.low.theta",semc_getTheta(idx1),"deg")				||
							!dm->BuffCheck(emc,"emc_bhabha::CsI.low.rho",semc.emc_rho[idx1],"cm")	) {
								continue;
						}
						double tmp=low_track; low_track=high_track; high_track=tmp;
					}
					if(!high_track && !low_track) continue;	//there should be at least one track

					dm->flush();

					if(high_track) dm->data->Add("emc_bhabha::CsI.high.track_momentum",high_track,"MeV");
					if(low_track)  dm->data->Add("emc_bhabha::CsI.low.track_momentum",low_track,"MeV");

					dm->data->Add("emc_bhabha::CsI.invMass",static_cast<double>(sqrt(2.*semc.emc_energy[idx1]*semc.emc_energy[idx2]*(1.-cos(angle*SUconverter::get_multiplier("deg","rad"))))),"MeV");
					if(total_energy!=0.)
						dm->data->Add("emc_bhabha::CsI.energy_div_total",static_cast<double>((semc.emc_energy[idx1]+semc.emc_energy[idx2])/total_energy));
					if(semc.emc_y[idx1]*semc.emc_z[idx1]<0.)
						dm->data->Add("emc_bhabha::CsI.high.up_cluster",static_cast<double>(semc.emc_energy[idx1]),"MeV");
					else
						dm->data->Add("emc_bhabha::CsI.high.down_cluster",static_cast<double>(semc.emc_energy[idx1]),"MeV");
					if(semc.emc_y[idx2]*semc.emc_z[idx2]<0.)
						dm->data->Add("emc_bhabha::CsI.low.up_cluster",static_cast<double>(semc.emc_energy[idx2]),"MeV");
					else
						dm->data->Add("emc_bhabha::CsI.low.down_cluster",static_cast<double>(semc.emc_energy[idx2]),"MeV");

					run->info->current->ncsi+=2;		//both clusters
				//	return;	//should finish searching, probably
				//	idx1=idx2+1; break;
					break;
				}
			}
		} }
	}

//forms full list of emc clusters, which at first contains all LKr&CsI clusters
//it is searched for bhabha, energy dependence on theta is drawn
//on the second go-through, the list contains all LKr clusters and CsI clusters, which do not have partners in LKr
//for LKr clusters with partners parameters are recalculated (not)
	void bhabha_event_emc() {
		double angle, theta1, theta2;
	//LKr+CsI	(C++ classes do not repopulate with recalculated clusters with partners, but C structure does)
		std::vector<GCluster*> emcClusters;
		std::vector<GCluster*>::const_iterator itr1, itr2;
		GClusterTower *cls1, *cls2;
		for(itr1=emcRec->lkrClusters.begin();itr1!=emcRec->lkrClusters.end();++itr1)
			emcClusters.push_back( dynamic_cast<GClusterTower*>(*itr1) );
		for(itr2=emcRec->csiClusters.begin();itr2!=emcRec->csiClusters.end();++itr2)
			emcClusters.push_back( dynamic_cast<GClusterTower*>(*itr2) );
		//search for bhabha
		dm->clear_buf();
		double tmp_high_energy=0., total_energy=0.;
		for(itr1=emcClusters.begin();itr1!=emcClusters.end();++itr1) { cls1=dynamic_cast<GClusterTower*>(*itr1);
			total_energy+=cls1->getEnergy();
		}
		for(itr1=emcClusters.begin();itr1!=emcClusters.end();++itr1) { cls1=dynamic_cast<GClusterTower*>(*itr1);
		if(cls1->getEnergy()>tmp_high_energy) {
			if(	!dm->BuffCheck(emc,"emc_bhabha::emc.high.energy",cls1->getEnergy(),"MeV")	||
				!dm->BuffCheck(emc,"emc_bhabha::emc.high.theta",static_cast<double>(cls1->getTheta2()),"deg")
			) continue;
			tmp_high_energy=cls1->getEnergy();

			for(itr2=emcClusters.begin();itr2!=emcClusters.end();++itr2) if(itr2!=itr1) { cls2=dynamic_cast<GClusterTower*>(*itr2);
			if(cls2->getEnergy()<=cls1->getEnergy()) {
				if(	!dm->BuffCheck(emc,"emc_bhabha::emc.low.energy",cls2->getEnergy(),"MeV")	||
					!dm->BuffCheck(emc,"emc_bhabha::emc.low.theta",static_cast<double>(cls2->getTheta2()),"deg")
				) continue;
				angle=get_angle(cls1->getTheta2(), cls1->getPhi2(), cls2->getTheta2(), cls2->getPhi2());
				if(dm->BuffCheck(emc,"emc_bhabha::emc.collinearity",fabs(180.-angle),"deg")								&&
				dm->BuffCheck(emc,"emc_bhabha::emc.coll_theta",fabs(180.-cls1->getTheta2()-cls2->getTheta2()),"deg")	&&
				dm->BuffCheck(emc,"emc_bhabha::emc.coll_phi",fabs(180.-fabs(cls1->getPhi2()-cls2->getPhi2())),"deg")) {
					dm->flush();
					if(total_energy!=0.)
						dm->data->Add("emc_bhabha::emc.energy_div_total",static_cast<double>((cls1->getEnergy()+cls2->getEnergy())/total_energy));
				//	return;	//should finish searching, probably
				//	itr1=++itr2; break;
					break;
				}
			} }
		} }
		emcClusters.clear();

	//Emc
	//there should be some procedure to search clusters and recalculate if they have partners (supposedly kdcvdrec does it already)
		bool energy_written=false;
		double run_energy=0.;
		//search for bhabha
		dm->clear_buf();
		tmp_high_energy=0., total_energy=0.;
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) { total_energy+=semc.emc_energy[idx1]; }
		for(int idx1=0;idx1<semc.emc_ncls;++idx1) { run->info->current->nemc_raw++;
		if(semc.emc_energy[idx1]>tmp_high_energy) {
			if(	!dm->BuffCheck(emc,"emc_bhabha::emc.high.merged_energy",semc.emc_energy[idx1],"MeV")	||
				!dm->BuffCheck(emc,"emc_bhabha::emc.high.merged_theta",semc_getTheta(idx1),"deg") ) {
				continue;
			}
			tmp_high_energy=semc.emc_energy[idx1];

			for(int idx2=0;idx2<semc.emc_ncls;++idx2)
			if(idx2!=idx1 && semc.emc_energy[idx2]<=semc.emc_energy[idx1]) {
				if(	!dm->BuffCheck(emc,"emc_bhabha::emc.low.merged_energy",semc.emc_energy[idx2],"MeV")	||
					!dm->BuffCheck(emc,"emc_bhabha::emc.low.merged_theta",semc_getTheta(idx2),"deg") ) {
					continue;
				}
				if(dm->BuffCheck(emc,"emc_bhabha::emc.merged_collinearity",fabs(180.-semc_getAngle(idx1,idx2)),"deg")	&&
				dm->BuffCheck(emc,"emc_bhabha::emc.merged_coll_theta",fabs(180.-semc_getTheta(idx1)-semc_getTheta(idx2)),"deg")	&&
				dm->BuffCheck(emc,"emc_bhabha::emc.merged_coll_phi",fabs(180.-fabs(semc_getPhi(idx1)-semc_getPhi(idx2))),"deg")) {

					double high_track=0., low_track=0.; int high_track_idx=-1, low_track_idx=-1;
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx1];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx1][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>high_track) {
							high_track=ktrrec_.PTRAK[tmp_idx];
							high_track_idx=tmp_idx;
						}
					}
					for(short idx_tr=0;idx_tr<semc.emc_dc_ntrk[idx2];++idx_tr) {
						int tmp_idx=semc.emc_dc_tracks[idx2][idx_tr]-1;		//starts from 1 there
						if(tmp_idx<0) { tmp_idx=0; std::cerr<<"Track index error"<<std::endl; }
						if(ktrrec_.PTRAK[tmp_idx]>low_track) {
							low_track=ktrrec_.PTRAK[tmp_idx];
							low_track_idx=tmp_idx;
						}
					}
					if(high_track && low_track && high_track<low_track) {
						if(	!dm->BuffCheck(emc,"emc_bhabha::emc.high.merged_energy",semc.emc_energy[idx2],"MeV")	||
							!dm->BuffCheck(emc,"emc_bhabha::emc.high.merged_theta",semc_getTheta(idx2),"deg")		||
							!dm->BuffCheck(emc,"emc_bhabha::emc.low.merged_energy",semc.emc_energy[idx1],"MeV")		||
							!dm->BuffCheck(emc,"emc_bhabha::emc.low.merged_theta",semc_getTheta(idx1),"deg")	) {
								continue;
						}
						double tmp=low_track; low_track=high_track; high_track=tmp;
					}
					if(!high_track && !low_track) continue;	//there should be at least one track

					dm->flush();

					if(high_track) dm->data->Add("emc_bhabha::emc.high.track_momentum",high_track,"MeV");
					if(low_track)  dm->data->Add("emc_bhabha::emc.low.track_momentum",low_track,"MeV");

		if(!energy_written) { energy_written=true;
			dm->data->Add("emc_bhabha::emc.total_energy",total_energy,"MeV");

			const std::map<T_li,RsSlurper::Status>* tmp_run_st=run_status->get();
			for(std::map<T_li,RsSlurper::Status>::const_iterator itr_m=tmp_run_st->begin();itr_m!=tmp_run_st->end();++itr_m) {
				T_li::const_iterator itr=std::find(itr_m->first.begin(),itr_m->first.end(),run->info->current->nrun);
				if(itr!=itr_m->first.end()) {
					run_energy=itr_m->second.energy;
					break;	//first found goes
				}
			}
			if(run_energy!=0.) {
				dm->data->Add("emc_bhabha::emc.run_energy",run_energy,"MeV");
				dm->data->Add("emc_bhabha::emc.total_energy_rel",total_energy*0.5/run_energy);
			}
		}
		dm->data->Add("emc_bhabha::emc.pair_energy",static_cast<double>(semc.emc_energy[idx1]+semc.emc_energy[idx2]),"MeV");
		if(run_energy!=0.) {
			dm->data->Add("emc_bhabha::emc.pair_energy_rel",static_cast<double>(semc.emc_energy[idx1]+semc.emc_energy[idx2])*0.5/run_energy);
			dm->data->Add("emc_bhabha::emc.high.merged_energy_rel",static_cast<double>(semc.emc_energy[idx1])/run_energy);
			dm->data->Add("emc_bhabha::emc.low.merged_energy_rel",static_cast<double>(semc.emc_energy[idx2])/run_energy);
		}
					if(total_energy!=0.)
						dm->data->Add("emc_bhabha::emc.merged_energy_div_total",static_cast<double>((semc.emc_energy[idx1]+semc.emc_energy[idx2])/total_energy));

					run->info->current->nemc+=2;		//both
					//	return;	//should finish searching, probably
				//	idx1=idx2+1; break;
					break;
				}
			}
		} }
	}
	double vddc_getAngle(int tr_n1, int tr_n2) {
		return acos(ktrrec_.VxTRAK[tr_n1]*ktrrec_.VxTRAK[tr_n2]+ktrrec_.VyTRAK[tr_n1]*ktrrec_.VyTRAK[tr_n2]+
			ktrrec_.VzTRAK[tr_n1]*ktrrec_.VzTRAK[tr_n2]) * SUconverter::get_multiplier("rad","deg");
	}
	void bhabha_event_vddc() {
		dm->clear_buf();
		double tmp_high_momentum=0.;
		if(	!dm->BuffCheck(vddc,"vddc_bhabha::op_tracks",ktrrec_.NtrackIP)	||
			!dm->BuffCheck(vddc,"vddc_bhabha::all_tracks",ktrrec_.NTRAK)	) {
				return;
		}
		for(int idx1=0;idx1<ktrrec_.NTRAK;++idx1) {	//LKr
		if(ktrrec_.PTRAK[idx1]>tmp_high_momentum) {
			if(	!dm->BuffCheck(vddc,"vddc_bhabha::track.high.momentum",ktrrec_.PTRAK[idx1],"MeV")	||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_x",ktrrec_.X0RelIP[idx1],"cm")		||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_y",ktrrec_.Y0RelIP[idx1],"cm")		||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_z",ktrrec_.Z0RelIP[idx1],"cm")		||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.vd_hits",ktrrec_.KvdTRAK[idx1])		||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.charge",ktrrec_.CHTRAK[idx1])			||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_nx",ktrrec_.VxTRAK[idx1])			||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_ny",ktrrec_.VyTRAK[idx1])			||
				!dm->BuffCheck(vddc,"vddc_bhabha::track.high.op_nz",ktrrec_.VzTRAK[idx1])	) {
					continue;
			}
			tmp_high_momentum=ktrrec_.PTRAK[idx1];

			for(int idx2=0;idx2<ktrrec_.NTRAK;++idx2)
			if(idx2!=idx1 && ktrrec_.PTRAK[idx2]<=ktrrec_.PTRAK[idx1]) {
				if(	!dm->BuffCheck(vddc,"vddc_bhabha::track.low.momentum",ktrrec_.PTRAK[idx2],"MeV")||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_x",ktrrec_.X0RelIP[idx2],"cm")	||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_y",ktrrec_.Y0RelIP[idx2],"cm")	||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_z",ktrrec_.Z0RelIP[idx2],"cm")	||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.vd_hits",ktrrec_.KvdTRAK[idx2])		||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.charge",ktrrec_.CHTRAK[idx2])		||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_nx",ktrrec_.VxTRAK[idx2])		||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_ny",ktrrec_.VyTRAK[idx2])		||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.low.op_nz",ktrrec_.VzTRAK[idx2])		||
					!dm->BuffCheck(vddc,"vddc_bhabha::track.momentum_diff",ktrrec_.PTRAK[idx1]-ktrrec_.PTRAK[idx2],"MeV")) {
						continue;
				}
				double angle=vddc_getAngle(idx1,idx2);
				if(dm->BuffCheck(vddc,"vddc_bhabha::track.rel_angle",fabs(180.-angle),"deg")) {
					dm->flush();
					dm->data->Add("vddc_bhabha::track.high.theta",acos(ktrrec_.VzTRAK[idx1])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("vddc_bhabha::track.low.theta",acos(ktrrec_.VzTRAK[idx2])*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("vddc_bhabha::track.high.phi",(ktrrec_.VyTRAK[idx1]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx1]/sqrt(1.-ktrrec_.VzTRAK[idx1]*ktrrec_.VzTRAK[idx1]))*SUconverter::get_multiplier("rad","deg"),"deg");
					dm->data->Add("vddc_bhabha::track.low.phi",(ktrrec_.VyTRAK[idx2]>0?1.:-1.)*
						acos(ktrrec_.VxTRAK[idx2]/sqrt(1.-ktrrec_.VzTRAK[idx2]*ktrrec_.VzTRAK[idx2]))*SUconverter::get_multiplier("rad","deg"),"deg");
					break;
				}
			}
		} }
	}
	void bhabha_event_mu() {
		if(SystemManager::mu.result_hits) dm->data->Add("mu_bhabha::hits",log(static_cast<double>(SystemManager::mu.result_hits)));
		if(SystemManager::mu.result_type) dm->data->Add("mu_bhabha::type",static_cast<int>(SystemManager::mu.result_type));
	}
	void bhabha_event_tof(void) {
		dm->data->Add("tof_bhabha::barrel.track",SystemManager::tof.times_track_barrel,"ns");
		dm->data->Add("tof_bhabha::barrel.emc",SystemManager::tof.times_emc_barrel,"ns");
		dm->data->Add("tof_bhabha::endcap.emc",SystemManager::tof.times_emc_endcap,"ns");
	}
	
	void run_channel_info(void) {
		dm->clear(); if(!run->info) return;
		int status, ierr=0;
		status=kdb_run_get_info(run->info->conn,run->info->current->nrun,1,&(run->info->current->kdbInfo));
		//RunStatus>=1 for good runs
		if(status<=0) {
			std::cerr<<"Error while reading from database."<<std::endl;
			ierr=1;
		//	return;
		}

		dm->data->Add("channels::nrun",run->info->current->nrun);
		dm->data->Add("channels::elum_int",static_cast<double>(run->info->current->kdbInfo.intelum),"mkb^{-1}");
		dm->data->Add("channels::plum_int",static_cast<double>(run->info->current->kdbInfo.intplum),"mkb^{-1}");	//==1e30 cm-2
		dm->data->Add("channels::init_lum",static_cast<double>(run->info->current->kdbInfo.iluminosity),"10 b^{-1}");	//==1e25 cm-2
		dm->data->Add("channels::setE",static_cast<double>(run->info->current->kdbInfo.E_setup),"keV");
		dm->data->Add("channels::corE",static_cast<double>(run->info->current->kdbInfo.E_corr),"keV");
		dm->data->Add("channels::numee_cand",run->info->current->kdbInfo.nee);
		dm->data->Add("channels::runtime",static_cast<double>(run->info->current->kdbInfo.runsec),"sec");
		dm->data->Add("channels::magfield",static_cast<double>(run->info->current->kdbInfo.magfield),"Gauss");

		T_vi tmp_notgood, tmp_ring, tmp_bad;
		for(unsigned short i=0;i<NLKRSIZE;++i) {
			if(semc_data.glkr_bad[i]!=kGOOD) tmp_notgood.push_back(i);
			if(semc_data.glkr_bad[i]==kRING) tmp_ring.push_back(i);
			if(semc_data.glkr_bad[i]>2) tmp_bad.push_back(i);
		}
		dm->data->Add("channels::LKr.notgood",tmp_notgood);
		dm->data->Add("channels::LKr.ring",tmp_ring);
		dm->data->Add("channels::LKr.bad",tmp_bad);
std::cout<<"Filled LKr, sizes="<<tmp_notgood.size()<<", "<<tmp_ring.size()<<", "<<tmp_bad.size()<<std::endl;
		tmp_notgood.clear(); tmp_ring.clear(); tmp_bad.clear();
		for(unsigned short i=0;i<NCSISIZE;++i) {
			if(semc_data.gcsi_bad[i]!=kGOOD) tmp_notgood.push_back(i);
			if(semc_data.gcsi_bad[i]==kRING) tmp_ring.push_back(i);
			if(semc_data.gcsi_bad[i]>2) tmp_bad.push_back(i);
		}
		dm->data->Add("channels::CsI.notgood",tmp_notgood);
		dm->data->Add("channels::CsI.ring",tmp_ring);
		dm->data->Add("channels::CsI.bad",tmp_bad);
std::cout<<"Filled CsI, sizes="<<tmp_notgood.size()<<", "<<tmp_ring.size()<<", "<<tmp_bad.size()<<std::endl;

		double res_lum=0., res_lum_csi=0.;
		{	double LumIntE_csi, LumIntP_csi, LumIntE, LumIntP;
			LumIntE=run->info->current->kdbInfo.intelum*1.e-3;	//RunInfo(OdbIntLe)*1.e-3;
			LumIntP=run->info->current->kdbInfo.intplum*1.e-3;	//RunInfo(OdbIntLp)*1.e-3;
			int run_type=run->info->current->kdbInfo.reserved;	//RunInfo(OdbType);

			int data_lcsi[6];
			status=kdb_read_for_run(run->info->conn,1525,run->info->current->nrun,data_lcsi,6);
			//itbdb_lcsi=1525, ltbdb_lcsi=6; ilcsi_lum=2  - from VDDCRec/kdbcsilum.inc
			if(status<=0) std::cerr<<"Error: unable to get CsI_lum table from DB."<<std::endl;
			LumIntE_csi=data_lcsi[2]*1.e-3;
			LumIntP_csi=data_lcsi[2]*1.e-3;

			res_lum=std::max(LumIntE,LumIntP);
			res_lum_csi=std::max(LumIntE_csi,LumIntP_csi);
			if(run_type==0) {
				if(res_lum<=0.01) res_lum=0.;
				if(res_lum_csi<=0.01) res_lum_csi=0.;
			} else if(run_type<=2) {
				if(run_type>0) {
					int num_events=run->info->current->kdbInfo.nread;	//RunInfo(OdbNread);
					if(res_lum<=0.01) res_lum=num_events/1.e5;
					if(res_lum_csi<=0.01) res_lum_csi=num_events/1.e5;
				}
			}
			if(ierr || (run_type==0 && !(res_lum && res_lum_csi))) {
				std::cerr<<"Error while getting information from DB, run "<<
					run->info->current->nrun<<" is excluded from simulation"<<std::endl;
			}
		}
		dm->data->Add("channels::legacy_ilum",res_lum,"nb^{-1}");
		dm->data->Add("channels::legacy_ilum_csi",res_lum_csi,"nb^{-1}");

		dm->data->Add("channels::int_luminosity",//static_cast<double>((run.info->current->kdbInfo.intelum+run.info->current->kdbInfo.intplum)/2),"mkb^{-1}");
			std::max(static_cast<double>(run->info->current->kdbInfo.intelum),static_cast<double>(run->info->current->kdbInfo.intplum)),"mkb^{-1}");
		dm->data->Add("kdb::int_luminosity",static_cast<double>(kdb_run_get_luminosity(run->info->current->nrun)),"nb^{-1}");

		int table_id=kdb_get_id(run->info->conn,"runenergy");
		unsigned int table_length=kdb_get_length(run->info->conn,table_id);
		int32_t* table_buffer=static_cast<int32_t*>( malloc( sizeof(int32_t)*table_length ) );
		unsigned short int kdb_version=1;
		kdb_read_for_run_ver(run->info->conn,table_id, run->info->current->nrun,kdb_version, table_buffer,table_length);
		if(table_buffer[3]==run->info->current->nrun) {	//we check if record exists for this run
			dm->data->Add("kdb::energy",static_cast<double>(table_buffer[1]),"eV");
			dm->data->Add("kdb::diff_energy",static_cast<double>(table_buffer[2]),"eV");
			dm->data->Add("kdb::ndf",static_cast<int>(table_buffer[4]));
			dm->data->Add("kdb::chi2",static_cast<double>(table_buffer[5])/1e3);	//you need to divide it like that
			dm->data->Add("kdb::sigma",static_cast<double>(table_buffer[6]),"eV");
		}
		free(table_buffer);

		if(!dm->data->empty()) dm->next();
	}
}

