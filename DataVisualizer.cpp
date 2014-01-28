
#include "DataVisualizer.h"

#include <algorithm>
DataVisualizer::DataVisualizer(bool raw_ext, const T_s& dataname, const T_s& cssname)
 : raw(raw_ext), dm(0),csl(CssSlurper(cssname)),path(IOglobals::popts->getPath()),log_root(IOglobals::popts->getPath(),"log") {
	if(raw) {
		readout_wireframes();
		fill_all_raw(IOglobals::popts->getValue("label"));
	} else {
		dm=new DataManager(IOglobals::popts->getPath());
		if(!log_root.SetLabel(T_s("hp_")+dataname)) log_root.init(T_s("hp_")+dataname);		//a bit too clunky, change later
		init(dataname,cssname);
	}
}
void DataVisualizer::init(const T_s& dataname, const T_s& cssname) {
	if(IOglobals::popts->isGiven("batch")) {
		dm->batch.init(*IOglobals::popts->getValue("batch"));
	}
	if(!dm->read(dataname)) {
		std::cerr<<"Failed to open file \""<<path<<"/"<<dataname<<"\""<<std::endl;
		exit(-1);
	}
//std::ostringstream csl_print; csl.print(T_s(),csl_print);
//log_dm.data->Add(cssname,csl_print);
	readout_wireframes();
	
	do{	fill_all(); } while(dm->batchNext());
log_root.Write();
log_root.Purge();
}

void DataVisualizer::readout_wireframes(void) {
	T_ls cv_lst=csl.get_blocks();
	T_ls::iterator itr_global=std::find(cv_lst.begin(),cv_lst.end(),"global");	//global is not a canvas
	if(itr_global!=cv_lst.end()) cv_lst.erase(itr_global);
	
	TH1::AddDirectory(kFALSE);	//changed defaults so that hists won't be added to any open file
	//	hist->SetDirectory(0);	//use this one to remove single hist
	
	for(T_ls::const_iterator itr=cv_lst.begin();itr!=cv_lst.end();++itr) {
		Wireframe* wfr = new Wireframe(this);
		wfr->setup(*itr,csl.get(*itr),csl.get("global"));

		T_ls tmp_lst;
		tmp_lst=csl.get_blocks(*itr+".histograms");
		T_mss leg_map=csl.get(*itr+".legend.entries");
std::cout<<"Got blocks \""<<*itr+".histograms"<<"\" size="<<tmp_lst.size()<<std::endl;
		for(T_ls::const_iterator itr_h=tmp_lst.begin();itr_h!=tmp_lst.end();++itr_h) {	//ensure that hist names are unique
			TH1D* hist = wfr->add_hist(*itr+".h."+*itr_h,csl.get(*itr+".histograms."+*itr_h),csl.get("global.histograms"));
			T_mss::const_iterator itr_leg=leg_map.find(*itr_h);
			if(itr_leg!=leg_map.end()) {
				T_vs tmp; IOconverter::tokenize(itr_leg->second,tmp,",\"",true);
				T_s label, option;
				if(!tmp.empty()) {
					label=tmp[0];
					if(tmp.size()>=2) option=tmp[1];
				}
				wfr->leg->AddEntry(hist,label.c_str(),option.c_str());
			}
		}
		tmp_lst=csl.get_blocks(*itr+".profiles");
		for(T_ls::const_iterator itr_p=tmp_lst.begin();itr_p!=tmp_lst.end();++itr_p) {
			TProfile* prof = wfr->add_prof(*itr+".p."+*itr_p,csl.get(*itr+".profiles."+*itr_p),csl.get("global.profiles"));
			T_mss::const_iterator itr_leg=leg_map.find(*itr_p);
			if(itr_leg!=leg_map.end()) {
				T_vs tmp; IOconverter::tokenize(itr_leg->second,tmp,",\"",true);
				T_s label, option;
				if(!tmp.empty()) {
					label=tmp[0];
					if(tmp.size()>=2) option=tmp[1];
				}
				wfr->leg->AddEntry(prof,label.c_str(),option.c_str());
			}
		}
		
		wireframes.push_back(wfr);
	}
}

DataVisualizer::~DataVisualizer() {
/*/----//maybe this will be needed
	for(std::list<TH1D*>::iterator itr=histograms.begin();itr!=histograms.end();) {
		delete *itr; itr=histograms.erase(itr);
	}
	for(std::list<TProfile*>::iterator itr=profiles.begin();itr!=profiles.end();) {
		delete *itr; itr=profiles.erase(itr);
	}
//----/*/
	for(std::vector<Wireframe*>::const_iterator itr=wireframes.begin();itr!=wireframes.end();++itr) {
		delete *itr;
	}	wireframes.clear();
	delete dm;
}
/*
void DataVisualizer::AddHist(const T_s& name, const T_s& title, int bins_number, const T_pdd& minmax) {
	histograms.push_back(new TH1D(name.c_str(),title.c_str(),bins_number,minmax.first,minmax.second));
	names.push_back(name);
}
void DataVisualizer::AddProfile(const T_s& name, const T_s& title, int xbins_number, const T_pdd& x_minmax, const T_pdd& y_minmax) {	//there's also Option_t* option to vary errors
	profiles.push_back(new TProfile(name.c_str(),title.c_str(),xbins_number,
		x_minmax.first,x_minmax.second,y_minmax.first,y_minmax.second));
	names.push_back(name);
}*/
TH1D* DataVisualizer::GetHist(const T_s& name) const {
	std::list<TH1D*>::const_iterator itr=histograms.begin();
	while(itr!=histograms.end()) {
//		if( name==T_s( (*itr)->GetName() ) ) break;	// <- lots of strings generated
		if( !strcmp(name.c_str(),(*itr)->GetName()) ) break;
		++itr;
	}
	if(itr!=histograms.end()) return *itr;
	else return NULL;
}
TProfile* DataVisualizer::GetProfile(const T_s& name) const {
	std::list<TProfile*>::const_iterator itr=profiles.begin();
	while(itr!=profiles.end()) {
		if( !strcmp(name.c_str(),(*itr)->GetName()) ) break;
		++itr;
	}
	if(itr!=profiles.end()) return *itr;
	else return NULL;
}

void DataVisualizer::Draw(void) const {
	for(std::vector<Wireframe*>::const_iterator itr=wireframes.begin();itr!=wireframes.end();++itr) {
		(*itr)->Draw();
	}
}

void DataVisualizer::fill_all(void) {
	dm->reset();
	T_vs tmp_vs;
	
	while(dm->next()) {	//fills all histograms, etc.
		for(std::vector<Wireframe*>::const_iterator itr=wireframes.begin();itr!=wireframes.end();++itr) {
//std::cout<<"Filling wireframe "<<(*itr)->cv->GetName()<<std::endl;
			for(std::list<TH1D*>::const_iterator itr_h=histograms.begin();itr_h!=histograms.end();++itr_h) {
				std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_h) );
				if(itr_d!=(*itr)->binds.end()) {
					tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);	//fixed for " left in strings
					for(T_vs::const_iterator itr_vs=tmp_vs.begin();itr_vs!=tmp_vs.end();++itr_vs) {
						const TObject* tmp=dm->data->Get(*itr_vs);
						if(!tmp) continue;
						double tmp_value; T_vd tmp_value_v;
	if(	DataManager::ExtractData<double>(tmp,tmp_value,itr_d->second.xunit) ||
		DataManager::ExtractData<int>(tmp,tmp_value) )
			(*itr_h)->Fill(tmp_value);
	else if(DataManager::ExtractDataV<T_vd>(tmp,tmp_value_v,itr_d->second.xunit) ||
		DataManager::ExtractDataV<T_vi>(tmp,tmp_value_v) )
			for(T_vd::const_iterator itr=tmp_value_v.begin();itr!=tmp_value_v.end();++itr) {
				(*itr_h)->Fill(*itr);
			}
//std::cout<<"Filling "<<(*itr_h)->GetName()<<" with "<<tmp_value<<" entries="<<(*itr_h)->GetEntries()<<std::endl;
					}
				}
			}
			for(std::list<TProfile*>::const_iterator itr_p=profiles.begin();itr_p!=profiles.end();++itr_p) {
				std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_p) );
				if(itr_d!=(*itr)->binds.end()) {
					tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);	//fixed for " left in strings
					std::vector<T_vs::const_iterator> pairs;
					T_vs::const_iterator itr_vs=tmp_vs.begin(), tmp_vs_end=tmp_vs.end();
					//strangely, g++ doesn't understand when end() should return const_iterator inside std::find
					while( (itr_vs=std::find(itr_vs,tmp_vs_end,":"))!=tmp_vs.end() ) {
						if(itr_vs!=tmp_vs.begin()) {
							pairs.push_back(itr_vs++);
							if(itr_vs==tmp_vs.end()) pairs.pop_back();
						} else {
							++itr_vs;
						}
					}
					for(std::vector<T_vs::const_iterator>::const_iterator itr_pr=pairs.begin();itr_pr!=pairs.end();++itr_pr) {
//std::cout<<"Pairs size="<<pairs.size()<<", strings size="<<tmp_vs.size()<<" : ("<<(*itr_pr-1-tmp_vs.begin())<<")|("<<(*itr_pr+1-tmp_vs.begin())<<")"<<std::endl;
//std::cout<<"\t\t : \""<<*(*itr_pr-1)<<"\"|"<<*(*itr_pr)<<"|\""<<*(*itr_pr+1)<<"\""<<std::endl;
						const TObject* tmpx=dm->data->Get(*(*itr_pr-1)), *tmpy=dm->data->Get(*(*itr_pr+1));
						if(!tmpx || !tmpy) continue;
						double tmpx_value, tmpy_value; T_vd tmpx_value_v, tmpy_value_v;
						tmpx_value=tmpy_value=std::numeric_limits<double>::max();	//defaults
	if(	DataManager::ExtractData<double>(tmpx,tmpx_value,itr_d->second.xunit) ||
		DataManager::ExtractData<int>(tmpx,tmpx_value) ||
		DataManager::ExtractDataV<T_vd>(tmpx,tmpx_value_v,itr_d->second.xunit) ||
		DataManager::ExtractDataV<T_vi>(tmpx,tmpx_value_v) ) { }
	if(	DataManager::ExtractData<double>(tmpy,tmpy_value,itr_d->second.yunit) ||
		DataManager::ExtractData<int>(tmpy,tmpy_value) ||
		DataManager::ExtractDataV<T_vd>(tmpy,tmpy_value_v,itr_d->second.yunit) ||
		DataManager::ExtractDataV<T_vi>(tmpy,tmpy_value_v) ) { }
						if(tmpx_value!=std::numeric_limits<double>::max()) {
							if(tmpy_value!=std::numeric_limits<double>::max()) {
								(*itr_p)->Fill(tmpx_value,tmpy_value);
							} else {
								for(T_vd::const_iterator itr2=tmpy_value_v.begin();itr2!=tmpy_value_v.end();++itr2) {
									(*itr_p)->Fill(tmpx_value,*itr2);
								}
							}
						} else {
							if(tmpy_value!=std::numeric_limits<double>::max()) {
								for(T_vd::const_iterator itr1=tmpx_value_v.begin();itr1!=tmpx_value_v.end();++itr1) {
									(*itr_p)->Fill(*itr1,tmpy_value);
								}
							} else {
								for(T_vd::const_iterator itr1=tmpx_value_v.begin(), itr2=tmpy_value_v.begin();
									itr1!=tmpx_value_v.end() && itr2!=tmpy_value_v.end();++itr1,++itr2) {
										(*itr_p)->Fill(*itr1,*itr2);
								}
							}
						}
					}
				}
			}
		}
	}
	
	for(std::list<TH1D*>::const_iterator itr_h=histograms.begin();itr_h!=histograms.end();++itr_h) {
std::cout<<"Adding "<<(*itr_h)->GetName()<<" to log.root"<<std::endl;
		log_root.Add(*itr_h);
	}
	for(std::list<TProfile*>::const_iterator itr_p=profiles.begin();itr_p!=profiles.end();++itr_p) {
std::cout<<"Adding "<<(*itr_p)->GetName()<<" to log.root"<<std::endl;
		log_root.Add(*itr_p);
	}
}
void DataVisualizer::fill_all_raw(const T_s* label) {
	TFile* hfile = new TFile( (path+"/log.root").c_str() );
	if(hfile->IsZombie()) return;	//failed to load
	
	T_vs tmp_vs;
	T_s data_name="data";
	if(label) data_name+=*label;
	TList* exp_data=dynamic_cast<TList*>(hfile->Get( (T_s("hp_")+data_name).c_str() ));
	for(std::vector<Wireframe*>::const_iterator itr=wireframes.begin();itr!=wireframes.end();++itr) {
		for(std::list<TH1D*>::const_iterator itr_h=histograms.begin();itr_h!=histograms.end();++itr_h) {
			std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_h) );
			if(itr_d!=(*itr)->binds.end()) {
				tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);
				for(T_vs::const_iterator itr_vs=tmp_vs.begin();itr_vs!=tmp_vs.end();++itr_vs)
				if(*itr_vs=="exp:" && ++itr_vs!=tmp_vs.end()) {
					const TH1D* tmp=dynamic_cast<const TH1D*>(exp_data->FindObject( itr_vs->c_str() ));
					if(!tmp) continue;
					(*itr_h)->Add(tmp);
				}
			}
		}
		for(std::list<TProfile*>::const_iterator itr_p=profiles.begin();itr_p!=profiles.end();++itr_p) {
			std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_p) );
			if(itr_d!=(*itr)->binds.end()) {
				tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);
				for(T_vs::const_iterator itr_vs=tmp_vs.begin();itr_vs!=tmp_vs.end();++itr_vs)
				if(*itr_vs=="exp:" && ++itr_vs!=tmp_vs.end()) {
					const TProfile* tmp=dynamic_cast<const TProfile*>(exp_data->FindObject( itr_vs->c_str() ));
					if(!tmp) continue;
					tmp->Copy(*(*itr_p));	//not all options from log.style will be in effect
				}
			}
		}
	}
	TList* sim_data=dynamic_cast<TList*>(hfile->Get( (T_s("hp_")+data_name+"_sim").c_str() ));
	for(std::vector<Wireframe*>::const_iterator itr=wireframes.begin();itr!=wireframes.end();++itr) {
		for(std::list<TH1D*>::const_iterator itr_h=histograms.begin();itr_h!=histograms.end();++itr_h) {
			std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_h) );
			if(itr_d!=(*itr)->binds.end()) {
				tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);
				for(T_vs::const_iterator itr_vs=tmp_vs.begin();itr_vs!=tmp_vs.end();++itr_vs)
				if(*itr_vs=="sim:" && ++itr_vs!=tmp_vs.end()) {
					const TH1D* tmp=dynamic_cast<const TH1D*>(sim_data->FindObject( itr_vs->c_str() ));
					if(!tmp) continue;
					(*itr_h)->Add(tmp);
				}
			}
		}
		for(std::list<TProfile*>::const_iterator itr_p=profiles.begin();itr_p!=profiles.end();++itr_p) {
			std::map<void*,Wireframe::Binds>::const_iterator itr_d=(*itr)->binds.find( static_cast<void*>(*itr_p) );
			if(itr_d!=(*itr)->binds.end()) {
				tmp_vs.clear(); IOconverter::tokenize(itr_d->second.data,tmp_vs,",\"",true);
				for(T_vs::const_iterator itr_vs=tmp_vs.begin();itr_vs!=tmp_vs.end();++itr_vs)
				if(*itr_vs=="sim:" && ++itr_vs!=tmp_vs.end()) {
					const TProfile* tmp=dynamic_cast<const TProfile*>(sim_data->FindObject( itr_vs->c_str() ));
					if(!tmp) continue;
					tmp->Copy(*(*itr_p));	//not all options from log.style will be in effect
				}
			}
		}
	}
}
void DataVisualizer::clear_all(void) {
	dm->reset();
	while(dm->next()) {	//clears all histograms
		for(std::list<TH1D*>::const_iterator itr=histograms.begin();itr!=histograms.end();++itr) {
			(*itr)->Reset();
		}
		for(std::list<TProfile*>::const_iterator itr=profiles.begin();itr!=profiles.end();++itr) {
			(*itr)->Reset();
		}
	}
}

void DataVisualizer::fit_gauslog(TH1 *hh, Double_t &bb_mean, Double_t &bb_sigma, Double_t &ev_tail) {
	unsigned int nev=static_cast<unsigned int>(hh->GetEntries());	//it returns Double_t
	if(nev<50) return;
	Double_t mean=hh->GetMean(), rms=hh->GetRMS();
	int nbin=hh->GetNbinsX();
	Double_t h_xmin = hh->GetBinCenter(1) - hh->GetBinWidth(1)*0.5;
	Double_t h_xmax = hh->GetBinCenter(nbin) + hh->GetBinWidth(nbin)*0.5;

	TF1 *flognorm = new TF1("flognorm",fitf_lognormal, h_xmin, h_xmax, 4);	//should not be Cloned
	flognorm->SetParNames("Amax","xmax","delta","sigma");
	Double_t perr[4]={1., 1., 1., 0.1}; flognorm->SetParErrors(perr);
	flognorm->SetParLimits(0, 0.,hh->GetBinContent(hh->GetMaximumBin())*1.5);
	flognorm->SetParLimits(1, 0.5*(3.*h_xmin-h_xmax),0.5*(3.*h_xmax-h_xmin));
	flognorm->SetParLimits(2, 0.5*(h_xmin-h_xmax),0.5*(h_xmax-h_xmin));
	flognorm->SetParLimits(3, 0.,2.5*rms);

	Double_t params[4]={0.,0.,0.,0.};
//	Double_t xmin_fit = mean - 3.0*rms; // 3.5
//	Double_t xmax_fit = mean + 1.5*rms; // 1.5
	int xmin_fitn=1, xmax_fitn=nbin;
	{	//hh->FindFirstBinAbove(), hh->FindLastBinAbove()
		double threshold=hh->GetBinContent(hh->GetMaximumBin())*0.1;
		for(xmin_fitn=hh->GetMaximumBin();xmin_fitn>=1;--xmin_fitn) {
			if(hh->GetBinContent(xmin_fitn)<threshold) break;
		}
		for(xmax_fitn=hh->GetMaximumBin();xmax_fitn<=nbin;++xmax_fitn) {
			if(hh->GetBinContent(xmax_fitn)<threshold) break;
		}
	}
	Double_t xmin_fit = hh->GetBinCenter(xmin_fitn);
	Double_t xmax_fit = hh->GetBinCenter(xmax_fitn);
	params[0] = hh->GetBinContent(hh->GetMaximumBin());
	params[1] = hh->GetBinCenter(hh->GetMaximumBin());
	Double_t tmp=(mean-params[1])/rms;
	if(!tmp) {
		params[2]=0.; params[3]=std::numeric_limits<double>::max();
	} else {
		if(tmp*tmp<(3./8)) {
			Double_t dummy=sqrt(1-8.*tmp*tmp/3);
			if(tmp<0) {
				params[2]=(mean-params[1])*(1.+dummy)/(1.-dummy);
				params[3]=-0.5*(1.-dummy)/tmp;
			} else {
				params[2]=(mean-params[1])*(1.-dummy)/(1.+dummy);
				params[3]=0.5*(1.+dummy)/tmp;
			}
		} else {	//sloppy approximation, works for big sigmas
			params[2] = tmp*tmp*tmp*fabs(mean-params[1]);
			tmp*=tmp;
			if(tmp>1.) params[3]=std::numeric_limits<double>::min();
			else params[3] = sqrt(log(1/tmp));
		}
	}
	
	const char *options[2]={"q0",""};
//	flognorm->FixParameter(0,params[0]);
//	flognorm->ReleaseParameter(0);
//	flognorm->SetParLimits(0, 0.,hh->GetBinContent(hh->GetMaximumBin())*1.5);
	for(int i=0; i<4; i++) { //consecutive approximations
		printf("-> %d xmin_fit:%6.3f xmax_fit:%6.3f Amax:%6.3f xmax:%6.3f delta:%6.3f sigma:%6.3f->\n",
			i,xmin_fit,xmax_fit, params[0],params[1],params[2],params[3]);
		flognorm->SetParameters(params);
		hh->Fit("flognorm",options[0],"", xmin_fit,xmax_fit);
		flognorm->GetParameters(params);
	//	tmp=exp(0.5*params[3]*params[3]);
	//	bb_mean=tmp*tmp*tmp*params[2];
	//	bb_sigma=sqrt(tmp*tmp-1)*fabs(bb_mean);
	//	bb_mean+=params[1]-params[2];	//shift
	//	xmin_fit = bb_mean - bb_sigma;
	//	xmax_fit = bb_mean + bb_sigma;
	}
	//events outside of approximation interval
	int nb1, nb2, nev_out;
	nb1=static_cast<int>( (xmin_fit-h_xmin)*nbin/(h_xmax-h_xmin) );
	nb2=static_cast<int>( (xmax_fit-h_xmin)*nbin/(h_xmax-h_xmin) );
	if(nb1<1) nb1=1; if(nb2>nbin) nb2=nbin;
//	nb1=hh->FindFixBin(xmin_fit); nb2=hh->FindFixBin(xmax_fit);
	nev_out=0;
	for(int i=1; i<nb1; i++) {
		nev_out+=static_cast<int>( hh->GetBinContent(i) );
	}
	for(int i=nb2+1; i<=nbin; i++) {
		nev_out+=static_cast<int>( hh->GetBinContent(i) );
	}

	ev_tail=static_cast<Double_t>(nev_out)/hh->GetEntries()*100.;

	const Int_t kNotDraw = 1<<9;
	TF1 *f=hh->GetFunction("flognorm");
	if(f) f->ResetBit(kNotDraw);

	tmp=exp(0.5*params[3]*params[3]);
	bb_mean=tmp*tmp*tmp*params[2];
	bb_sigma=sqrt(tmp*tmp-1)*fabs(bb_mean);
	bb_mean+=params[1]-params[2];	//shift
//	bb_mu=log(fabs(params[2]))+params[3]*params[3];
//	bb_sigma=params[3];
	
	delete flognorm;
	Double_t bb_res=bb_sigma/bb_mean*100.;
	printf("%10s mu:%7.2f  sigma:%6.2f  res:%6.3f  t:%6.3f\n",
		hh->GetName(),bb_mean, bb_sigma, bb_res, ev_tail);
}
Double_t DataVisualizer::fitf_lognormal(const Double_t *value, const Double_t *params) {
	double x=value[0], max=params[0], xp=params[1], delta=params[2], sigma=params[3];
	if(!delta) return 0.;
	double tmp=(x-xp)/delta, s2=sigma*sigma, result;
	if(tmp<=-1.) return 0.;
	if( fabs(tmp)<pow(10.,(log10(80.*s2*s2*s2/(28.*s2*s2-35.*s2+5.))-16.)/7.) ) {	//series expansion
		result=1.-0.5*tmp*tmp*(1.-tmp)/s2+tmp*tmp*tmp*tmp*(0.5*(3.-11.*s2)+(5.*s2-3.)*tmp)/(12.*s2*s2)-
			tmp*tmp*tmp*tmp*tmp*tmp*(274.*s2*s2-255.*s2+15.)/(720.*s2*s2*s2);
	} else {
		tmp=log(1.+tmp)/sigma;
		result=exp(-0.5*tmp*tmp);
	}
	return max*result;
}

void DataVisualizer::fit_bb_xsection(TH1 *hh, Double_t &norm, Double_t &res) {
	unsigned int nev=static_cast<unsigned int>(hh->GetEntries());	//it returns Double_t
	if(nev<50) return;
	Double_t mean=hh->GetMean(), rms=hh->GetRMS();
	int nbin=hh->GetNbinsX();
	Double_t h_xmin = hh->GetBinCenter(1) - hh->GetBinWidth(1)*0.5;
	Double_t h_xmax = hh->GetBinCenter(nbin) + hh->GetBinWidth(nbin)*0.5;

	TF1 *bbxsect = new TF1("bbxsect",fitf_bhabha_xsection, h_xmin, h_xmax, 2);	//should not be Cloned
	bbxsect->SetParName(0,"Norm"); bbxsect->SetParError(0,1.);
	bbxsect->SetParLimits(0, 0.,hh->GetBinContent(hh->GetMaximumBin()));
	bbxsect->SetParName(1,"Residue"); bbxsect->SetParError(1,1.);
	bbxsect->SetParLimits(1, -0.5*hh->GetBinContent(hh->GetMaximumBin()),0.5*hh->GetBinContent(hh->GetMaximumBin()));

	Double_t param1=hh->GetBinContent(nbin/2), param2=0.;
	Double_t xmin_fit=h_xmin, xmax_fit=h_xmax;
	double threshold=hh->GetBinContent(hh->GetMaximumBin())*0.1;
	int xmin_fitn,xmax_fitn;
	for(xmin_fitn=1;xmin_fitn<=nbin;++xmin_fitn) {
		if(hh->GetBinContent(xmin_fitn)>threshold) break;
	}
	for(xmax_fitn=nbin;xmax_fitn>=1;--xmax_fitn) {
		if(hh->GetBinContent(xmax_fitn)>threshold) break;
	}
	xmin_fit=hh->GetBinCenter(xmin_fitn); xmax_fit=hh->GetBinCenter(xmax_fitn);
	
	for(int i=0; i<4; i++) { //consecutive approximations
		printf("-> %d xmin_fit:%6.3f xmax_fit:%6.3f Norm:%6.3f Res:%6.3f->\n",
			i,xmin_fit,xmax_fit, param1, param2);
		bbxsect->SetParameter(0,param1); bbxsect->SetParameter(1,param2);
		hh->Fit("bbxsect","","", xmin_fit,xmax_fit);
		param1=bbxsect->GetParameter(0); param2=bbxsect->GetParameter(1);
	}

	const Int_t kNotDraw = 1<<9;
	TF1 *f=hh->GetFunction("bbxsect");
	if(f) f->ResetBit(kNotDraw);

	delete bbxsect;
}
Double_t DataVisualizer::fitf_bhabha_xsection(const Double_t *value, const Double_t *params) {
	double theta=value[0], norm=params[0], res=params[1];
	double tmp, result=0.;
	if(fabs(theta)<=0.01) theta=0.01;	//to avoid 1/0
	else if(fabs(theta-180.)<=0.01) theta=180.-0.01;
	theta*=SUconverter::get_multiplier("deg","rad");
	
	tmp=sin(theta*0.5); tmp*=tmp;
	double tmp_add=4.*tmp*(1-tmp);	//residue=sin^2(theta)
	tmp=1./tmp+tmp-1;
	result+=tmp*tmp;
	tmp=sin((TMath::Pi()-theta)*0.5); tmp*=tmp; tmp=1./tmp+tmp-1;
	result+=tmp*tmp;	//symmetric distribution
	return norm*result+res*tmp_add;
}

DataVisualizer::Wireframe::Wireframe(DataVisualizer* prt) : pt(0),leg(0), parent(prt) {
	std::size_t counter=parent->wireframes.size()+1;
	std::ostringstream ostr("cv"); ostr<<counter;
	ts = new TStyle((ostr.str()+"_style").c_str(),(ostr.str()+" style").c_str());
	cv = new TCanvas(ostr.str().c_str(),(ostr.str()+" title").c_str(),100,100, 800,600);
	ostr.str("pad"); ostr<<counter;
	pad = new TPad((ostr.str()+"_pad").c_str(),(ostr.str()+" title").c_str(),0.02,0.02,0.98,0.98,10,0,0);
	pad->SetCanvas(cv);
//	leg = new TLegend(0.2,0.2,0.5,0.4);
//	leg->AddEntry((TObject*)0,"No entry 1","l");
//	leg->AddEntry((TObject*)0,"No entry 2","l");
}
DataVisualizer::Wireframe::~Wireframe(void) {
	parent=0;
//std::cout<<"Wireframe destructor activated."<<std::endl;
	if(leg) delete leg;	if(pt) delete pt;
	if(pad) delete pad; if(cv) delete cv; if(ts) delete ts;
	//make sure that the names of objects are unique
//	leg=0; pt=0; pad=0; cv=0; ts=0;
}

void DataVisualizer::Wireframe::setup(const T_s& name, const T_mss& params, const T_mss& global_params) {
	T_s prm;	T_vs tmp;
	if(get_param("style.opt_fit",params,global_params,prm)) ts->SetOptFit( atoi(prm.c_str()) );
	if(get_param("style.opt_stat",params,global_params,prm)) ts->SetOptStat( atoi(prm.c_str()) );
	if(get_param("style.gridx",params,global_params,prm)) ts->SetPadGridX(prm=="true");
	if(get_param("style.gridy",params,global_params,prm)) ts->SetPadGridY(prm=="true");
	ts->SetName((name+"_style").c_str()); cv->SetName(name.c_str());	//unique names and such
	ts->cd(); cv->cd();
	if(get_param("canvas.title",params,global_params,prm)) cv->SetTitle(prm.c_str());
	if(get_param("canvas.position",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		int posx=0,posy=0;
		if(!tmp.empty()) {
			posx=atoi(tmp[0].c_str());
			if(tmp.size()>=2) posy=atoi(tmp[1].c_str());
		}
		cv->SetWindowPosition(posx,posy);
	}
	if(get_param("canvas.size",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		unsigned int szx=0,szy=0;
		if(!tmp.empty()) {
			szx=static_cast<unsigned int>( atoi(tmp[0].c_str()) );
			if(tmp.size()>=2) szy=static_cast<unsigned int>( atoi(tmp[1].c_str()) );
		}
		cv->SetCanvasSize(szx,szy);
	}
	pad->SetName((name+"_pad").c_str());
	if(get_param("pad.title",params,global_params,prm)) pad->SetTitle(prm.c_str());
	if(get_param("pad.corners",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		double bottom_left_x=0.,bottom_left_y=0.,top_right_x=0.,top_right_y=0.;
		if(!tmp.empty()) {
			bottom_left_x=atof(tmp[0].c_str());
			if(tmp.size()>=2) bottom_left_y=atof(tmp[1].c_str());
			if(tmp.size()>=3) top_right_x=atof(tmp[2].c_str());
			if(tmp.size()>=4) top_right_y=atof(tmp[3].c_str());
		}
		pad->SetPad(bottom_left_x,bottom_left_y,top_right_x,top_right_y);
	}
	if(get_param("pad.color",params,global_params,prm)) pad->SetFillColor( extract_color(prm) );
	if(get_param("pad.border",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		short border_size=0, border_mode=0;
		if(!tmp.empty()) {
			border_size=static_cast<short>( atoi(tmp[0].c_str()) );
			if(tmp.size()>=2) border_mode=static_cast<short>( atoi(tmp[1].c_str()) );
		}
		pad->SetBorderSize(border_size);
		pad->SetBorderMode(border_mode);
	}
{
//	pt = new TPaveText();
	double bottom_left_x=0.,bottom_left_y=0.,top_right_x=0.,top_right_y=0.;
	T_s option="br";
	if(get_param("header.body",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		if(!tmp.empty()) {
			bottom_left_x=atof(tmp[0].c_str());
			if(tmp.size()>=2) bottom_left_y=atof(tmp[1].c_str());
			if(tmp.size()>=3) top_right_x=atof(tmp[2].c_str());
			if(tmp.size()>=4) top_right_y=atof(tmp[3].c_str());
			if(tmp.size()>=5) option=tmp[4];
		}
	//	pt->SetX1NDC(bottom_left_x); pt->SetY1NDC(bottom_left_y);
	//	pt->SetX2NDC(top_right_x); pt->SetY2NDC(top_right_y);
	//	pt->SetOption(option.c_str());
	}
	pt = new TPaveText(bottom_left_x,bottom_left_y,top_right_x,top_right_y,option.c_str());
	if(get_param("header.fill_color",params,global_params,prm)) pt->SetFillColor( extract_color(prm) );
	if(get_param("header.color",params,global_params,prm)) pt->SetTextColor( extract_color(prm) );
	if(get_param("header.font",params,global_params,prm)) pt->SetTextFont( atoi(prm.c_str()) );
	if(get_param("header.align",params,global_params,prm)) pt->SetTextAlign( atoi(prm.c_str()) );
	if(get_param("header.lines",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",\"",true);	//fix for " left in strings
		for(T_vs::const_iterator itr=tmp.begin();itr!=tmp.end();++itr) {
		//	if(itr!=tmp.begin() && str.find("\"")==0) str.erase(0,1);	//fix for " left in strings
		//	if(tmp.end()-itr!=1 && str.rfind("\"")==str.size()-1) str.erase(str.size()-1,1);
			pt->AddText(itr->c_str());
		}
	}
}
{
//	leg = new TLegend();	//entries are added in the DataVisualizer constructor
	double bottom_left_x=0.,bottom_left_y=0.,top_right_x=0.,top_right_y=0.;
	if(get_param("legend.corners",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		if(!tmp.empty()) {
			bottom_left_x=atof(tmp[0].c_str());
			if(tmp.size()>=2) bottom_left_y=atof(tmp[1].c_str());
			if(tmp.size()>=3) top_right_x=atof(tmp[2].c_str());
			if(tmp.size()>=4) top_right_y=atof(tmp[3].c_str());
		}
//		leg->SetX1NDC(bottom_left_x); leg->SetY1NDC(bottom_left_y);
//		leg->SetX2NDC(top_right_x); leg->SetY2NDC(top_right_y);
	}
	leg = new TLegend(bottom_left_x,bottom_left_y,top_right_x,top_right_y);
	if(get_param("legend.option",params,global_params,prm)) leg->SetOption(prm.c_str());
	if(get_param("legend.title",params,global_params,prm)) if(!prm.empty()) leg->SetHeader(prm.c_str());
}

}
TH1D* DataVisualizer::Wireframe::add_hist(const T_s& name, const T_mss& params, const T_mss& global_params) {
	T_s prm;	T_vs tmp;
	TH1D* hist;//= new TH1D();	hist->SetName(name.c_str());
	int nbins=100;	//we need to create hist after we know nbins (important)
	if(get_param("nbins",params,global_params,prm)) nbins=atoi(prm.c_str());
	if(get_param("range",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		double xmin=0.,xmax=100.;
		if(!tmp.empty()) {
			xmin=atof(tmp[0].c_str());
			if(tmp.size()>=2) xmax=atof(tmp[1].c_str());
		}
	//	hist->GetXaxis()->Set(nbins,xmin,xmax);	//this instruction cannot (!) change bin number, look at hist->Rebin() if you want that
		hist = new TH1D(name.c_str(),"temp",nbins,xmin,xmax);
	} else {
	//	hist->GetXaxis()->Set(nbins,0.,100.);
		hist = new TH1D(name.c_str(),"temp",nbins,0.,100.);
	}
	if(get_param("title",params,global_params,prm)) hist->SetTitle(prm.c_str());

	std::map<void*,Binds>::iterator itr=binds.find(dynamic_cast<void*>(hist));
	if(itr==binds.end()) itr=binds.insert(std::make_pair(dynamic_cast<void*>(hist),Binds())).first;
	
	if(get_param("xunit",params,global_params,prm)) itr->second.xunit=prm;
	if(get_param("xtitle",params,global_params,prm)) {	//fancy title
		hist->SetXTitle( (itr->second.xunit.empty() ? prm : prm+", "+itr->second.xunit).c_str() );
	} else {
		hist->SetXTitle(itr->second.xunit.c_str());
	}
	if(get_param("fill_color",params,global_params,prm)) hist->SetFillColor( extract_color(prm) );
	if(get_param("line_color",params,global_params,prm)) hist->SetLineColor( extract_color(prm) );
	if(get_param("marker_color",params,global_params,prm)) hist->SetMarkerColor( extract_color(prm) );
	if(get_param("marker_style",params,global_params,prm)) hist->SetMarkerStyle( atoi(prm.c_str()) );

	if(get_param("data",params,global_params,prm)) itr->second.data=prm;
	if(get_param("position",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		unsigned int posx=0, posy=0;
		if(!tmp.empty()) {
			posx=static_cast<unsigned int>( atoi(tmp[0].c_str()) );
			if(tmp.size()>=2) posy=static_cast<unsigned int>( atoi(tmp[1].c_str()) );
		}
		itr->second.position=std::make_pair(posx,posy);
	}
	if(get_param("fit",params,global_params,prm)) itr->second.fit=prm;
	if(get_param("draw_options",params,global_params,prm)) itr->second.draw_options=prm;
	
	if(get_param("special",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",\"",true);
		for(T_vs::const_iterator itr_tmp=tmp.begin();itr_tmp!=tmp.end();++itr_tmp) {
			if(*itr_tmp=="norm") itr->second.norm=true;
			if(*itr_tmp=="logx") itr->second.logx=true;
			if(*itr_tmp=="logy") itr->second.logy=true;
			if(*itr_tmp=="err") itr->second.err=true;
		}
	}

	parent->histograms.push_back(hist);
	return hist;
}
TProfile* DataVisualizer::Wireframe::add_prof(const T_s& name, const T_mss& params, const T_mss& global_params) {
	T_s prm;	T_vs tmp;
	TProfile* prof;
	int nbins=100;	//we need to create prof after we know nbins (important)
	double xmin=0., xmax=100., ymin=0., ymax=100.;
	if(get_param("nbins",params,global_params,prm)) nbins=atoi(prm.c_str());
	if(get_param("xrange",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		if(!tmp.empty()) {
			xmin=atof(tmp[0].c_str());
			if(tmp.size()>=2) xmax=atof(tmp[1].c_str());
		}
	}
	if(get_param("yrange",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		if(!tmp.empty()) {
			ymin=atof(tmp[0].c_str());
			if(tmp.size()>=2) ymax=atof(tmp[1].c_str());
		}
	}
	prof = new TProfile(name.c_str(),"temp",nbins,xmin,xmax,ymin,ymax);
	prof->SetMinimum(ymin); prof->SetMaximum(ymax);	//should not be necessary
	if(get_param("title",params,global_params,prm)) prof->SetTitle(prm.c_str());

	std::map<void*,Binds>::iterator itr=binds.find(dynamic_cast<void*>(prof));
	if(itr==binds.end()) itr=binds.insert(std::make_pair(dynamic_cast<void*>(prof),Binds())).first;

	if(get_param("xunit",params,global_params,prm)) itr->second.xunit=prm;
	if(get_param("xtitle",params,global_params,prm)) {	//fancy title
		prof->SetXTitle( (itr->second.xunit.empty() ? prm : prm+", "+itr->second.xunit).c_str() );
	} else {
		prof->SetXTitle(itr->second.xunit.c_str());
	}
	if(get_param("yunit",params,global_params,prm)) itr->second.yunit=prm;
	if(get_param("ytitle",params,global_params,prm)) {	//fancy title
		prof->SetYTitle( (itr->second.yunit.empty() ? prm : prm+", "+itr->second.yunit).c_str() );
	} else {
		prof->SetYTitle(itr->second.yunit.c_str());
	}

	if(get_param("data",params,global_params,prm)) itr->second.data=prm;
	if(get_param("position",params,global_params,prm)) {
		tmp.clear(); IOconverter::tokenize(prm,tmp,",",true);
		unsigned int posx=0, posy=0;
		if(!tmp.empty()) {
			posx=static_cast<unsigned int>( atoi(tmp[0].c_str()) );
			if(tmp.size()>=2) posy=static_cast<unsigned int>( atoi(tmp[1].c_str()) );
		}
		itr->second.position=std::make_pair(posx,posy);
	}
	if(get_param("draw_options",params,global_params,prm)) itr->second.draw_options=prm;
	
	if(get_param("marker_color",params,global_params,prm)) prof->SetMarkerColor( extract_color(prm) );
	if(get_param("line_color",params,global_params,prm)) prof->SetLineColor( extract_color(prm) );

	parent->profiles.push_back(prof);
	return prof;
}

void DataVisualizer::Wireframe::Draw(void) const {
	if(!ts || !cv || !pad) return;
	ts->cd();
	cv->cd();	//equivalent to cv->cd(0); canvas is zero pad
	cv->Draw();
	if(pt)	 pt->Draw();
	pad->Draw();
	if(leg)	leg->Draw();
	
	unsigned int padx=0, pady=0;	//numeration starts from 1,1; left->right, up->down
	std::map<void*,Binds> tmp_map=binds;	//to draw with "same" option
	std::vector<void*> repeated;
	std::map<T_puu,bool> occupied;	//row,column:0/1
	unsigned int duplicates=0;
	for(std::map<void*,Binds>::iterator itr=tmp_map.begin();itr!=tmp_map.end();) {
std::cout<<"Positions = "<<itr->second.position.first<<" | "<<itr->second.position.second<<std::endl;
		if(itr->second.position.first && itr->second.position.second) {
			if(itr->second.position.first>padx) padx=itr->second.position.first;
			if(itr->second.position.second>pady) pady=itr->second.position.second;
			occupied[itr->second.position]=true;
		}
		std::map<void*,Binds>::iterator itr_tmp=itr;
		for(++itr_tmp;itr_tmp!=tmp_map.end();) {
			if(itr_tmp->second.position==itr->second.position) {
				repeated.push_back(itr_tmp->first);
				tmp_map.erase(itr_tmp++);
				++duplicates;
			} else {
				++itr_tmp;
			}
		}
		tmp_map.erase(itr++);
	}
	if(!tmp_map.empty()) std::cerr<<"Warning! DataVisualizer's Wireframe::Draw had an error with binds."<<std::endl;
	
	if(binds.size()>occupied.size()+duplicates) while(binds.size()>padx*pady) padx<pady ? ++padx : ++pady;	//grow if there's not enough space	
	pad->Divide(pady,padx);	//inverted to match intuitive definitions
std::cout<<"Divided pad into "<<padx<<"|"<<pady<<" with "<<duplicates<<" duplicates"<<std::endl;
	for(unsigned int padx_tmp=1;padx_tmp<=padx;++padx_tmp) {	//filling blanks
		for(unsigned int pady_tmp=1;pady_tmp<=pady;++pady_tmp) {
			occupied.insert(std::make_pair(std::make_pair(padx_tmp,pady_tmp),false));
		}
	}
	for(std::map<void*,Binds>::const_iterator itr=binds.begin();itr!=binds.end();++itr) {
		if(!itr->second.position.first || !itr->second.position.second) {	//find free position
			unsigned int posx=0, posy=0;
			std::map<T_puu,bool>::iterator itr_tmp;
			for(itr_tmp=occupied.begin();itr_tmp!=occupied.end();++itr_tmp) if(!itr_tmp->second) break;
			if(itr_tmp!=occupied.end()) {
				posx=itr_tmp->first.first; posy=itr_tmp->first.second;
				itr_tmp->second=true;
			} else {
				std::cerr<<"Wireframe::Draw couldn't assign position correctly."<<std::endl;
			}
			pad->cd( pady*(posx-1) + posy );
		} else {
			pad->cd( pady*(itr->second.position.first-1) + itr->second.position.second );	//this numeration starts from 1
		}
		T_s option; bool invisible_pad=false;
		if(!repeated.empty() && std::find(repeated.begin(),repeated.end(),itr->first)!=repeated.end()) {
			option="same";
			invisible_pad=true;
		}
		
		if(itr->second.logx) gPad->SetLogx(1);
		if(itr->second.logy) gPad->SetLogy(1);
		
		TH1D* hist; TProfile* prof;
		if( (hist=get_hist(itr->first)) ) {
			if(itr->second.err) hist->Sumw2();	//error bars are displayed, look into disabling them later
			if(itr->second.norm) {
				double integral=hist->Integral();
				if(integral) hist->Scale(1./integral);
			}
			TPad* inv_pad=0; TVirtualPad *cur_pad=0;
			if(invisible_pad) {
				inv_pad=dynamic_cast<TPad*>(gPad->Clone());
				inv_pad->SetFillStyle(4000);
				cur_pad=gPad;
				pad->cd();
				inv_pad->Draw(); inv_pad->cd();
				option="][sames";
			}
		//	std::map<void*,T_s>::const_iterator itr_f=fit_names.find(itr->first);
			if(!itr->second.fit.empty()) {
				if(itr->second.fit=="gauslog") {
					Double_t bb_mean=1., bb_sigma=0., bb_tail=0.;	//need defaults in case fit prematurely returns
					DataVisualizer::fit_gauslog(hist, bb_mean, bb_sigma, bb_tail);
					char str[150]; TLatex lx;
					sprintf(str,"#frac{s.d.}{mean}=%4.2f%%",bb_sigma*100./bb_mean);
					hist->Draw((itr->second.draw_options+option).c_str());
					double xtmp;
					if(hist->GetNbinsX()>2*hist->GetMaximumBin())
						xtmp=0.7*hist->GetBinCenter(hist->GetNbinsX())+0.3*hist->GetBinCenter(1);
					else xtmp=0.1*hist->GetBinCenter(hist->GetNbinsX())+0.9*hist->GetBinCenter(1);
					lx.DrawLatex(xtmp, 0.3*hist->GetMaximum(),str);
				} else if(itr->second.fit=="bb_xsect") {
					Double_t bb_norm=1., bb_res=0.;
					DataVisualizer::fit_bb_xsection(hist, bb_norm, bb_res);
				} else
					hist->Draw((itr->second.draw_options+option).c_str());
			} else
				hist->Draw((itr->second.draw_options+option).c_str());
			
			if(invisible_pad) {
				cur_pad->cd();
			}
		}
		if( (prof=get_profile(itr->first)) ) prof->Draw((itr->second.draw_options+option).c_str());
	}
	
//	cv->Draw();
	T_s pic_path=parent->GetPath();
	if(pic_path==T_s("/home/boroden/current/KdRecTtau")) pic_path="/spool/users/boroden/trees/pics";
	cv->Print( (pic_path+"/"+cv->GetName()+parent->label+".eps").c_str() );
}

TH1D* DataVisualizer::Wireframe::get_hist(const void* ptr) const {
	if(!parent) return NULL;
	std::list<TH1D*>::const_iterator itr=parent->histograms.begin();
	while(itr!=parent->histograms.end()) {
		if( ptr==dynamic_cast<void*>(*itr) ) break;
		++itr;
	}
	if(itr!=parent->histograms.end()) {
		return *itr;
	}
	else return NULL;
}
TProfile* DataVisualizer::Wireframe::get_profile(const void* ptr) const {
	if(!parent) return NULL;
	std::list<TProfile*>::const_iterator itr=parent->profiles.begin();
	while(itr!=parent->profiles.end()) {
		if( ptr==dynamic_cast<void*>(*itr) ) break;
		++itr;
	}
	if(itr!=parent->profiles.end()) return *itr;
	else return NULL;
}

bool DataVisualizer::Wireframe::get_param(const T_s& name, const T_mss& params, const T_mss& global_params, T_s& result) {
	T_mss::const_iterator itr;
	if( (itr=params.find(name)) !=params.end()) { result=itr->second; return true; }
	if( (itr=global_params.find(name)) !=global_params.end()) { result=itr->second; return true; }
	return false;
}

//commented colors are not supported in root 5.14
std::map<T_s,short> DataVisualizer::Wireframe::colors=build_map_from_list<T_s,short int>("kWhite",kWhite)("kBlack",kBlack)//("kGray",kGray)
	("kRed",kRed)("kGreen",kGreen)("kBlue",kBlue)("kYellow",kYellow)("kMagenta",kMagenta)("kCyan",kCyan);
//	("kOrange",kOrange)("kSpring",kSpring)("kTeal",kTeal)("kAzure",kAzure)("kViolet",kViolet)("kPink",kPink);
short int DataVisualizer::Wireframe::extract_color(const T_s& str) {	//see enum EColor in Rtypes.h
	if(!str.empty() && str.at(0)=='#') {
std::cout<<"Hex color="<<str<<" is "<<TColor::GetColor(str.c_str())<<std::endl;
		return TColor::GetColor(str.c_str());	// #rrggbb
	}
	return IOconverter::Parser<short int>(str).evaluate(colors);
}
