#include <list>
#include <string>
#include <sstream>
#include <fstream>

list<string> list_directories(const string& dirname) {
	list<string> result;
	if(dirname.empty()) return result;		//just a precaution
	string workdir=gSystem->pwd();
	TSystemDirectory dir(dirname.c_str(),dirname.c_str());
	TList *files = dir.GetListOfFiles();	//uses new inside (!)
	gSystem->cd(workdir.c_str());	//bug fix for ROOT prior to 5.34
	
	if(files) {
		TIter TL_itr(files);
		TSystemFile *file;
		string filename;
		while(( file=dynamic_cast<TSystemFile*>(TL_itr.Next()) )) {
			filename = file->GetName();
			if(file->IsDirectory()) {
				result.push_back(filename);
			}
		}
	}
	delete files;
	return result;
}
list<string> list_files(const string& dirname,const string& suffix,const string& prefix) {
	list<string> result;
	if(dirname.empty()) return result;		//just a precaution
	string workdir=gSystem->pwd();
	TSystemDirectory dir(dirname.c_str(),dirname.c_str());
	TList *files = dir.GetListOfFiles();	//uses new inside (!)
	gSystem->cd(workdir.c_str());	//bug fix for ROOT prior to 5.34
	
	if(files) {
		TIter TL_itr(files);
		TSystemFile *file;
		string filename;
		while(( file=dynamic_cast<TSystemFile*>(TL_itr.Next()) )) {
			filename = file->GetName();
			if(	!(file->IsDirectory()) && (prefix.empty() || filename.find(prefix)==0) &&
				(suffix.empty() || filename.rfind(suffix)==filename.size()-suffix.size()) ) {
					result.push_back(filename);
			}
		}
	}
	delete files;
	return result;
}

void parse(const string& path) {
	list<string> dir_list=list_directories(path);
string workdir=gSystem->pwd();
	ostringstream exp_buf, sim_buf;
	for(list<string>::const_iterator itr=dir_list.begin();itr!=dir_list.end();++itr) {
		if(*itr==string(".") || *itr==string("..")) continue;	//ignore current and up
		cout<<"Opened \""<<*itr<<"\" to search for log.root"<<endl;
		gSystem->cd((path+*itr).c_str());
		TFile* file=new TFile("log.root","read");
	if(file->IsZombie()) { cerr<<"\tcould not find log.root"<<endl;
	} else {
		list<string> tmp_lst_exp=list_files(gSystem->pwd(),".root","data");
		list<string> tmp_lst_sim=list_files(gSystem->pwd(),"_sim.root","data");
		list<string> lst_exp, lst_sim;
		for(list<string>::iterator itr_sim=tmp_lst_sim.begin();itr_sim!=tmp_lst_sim.end();++itr_sim) {
			for(list<string>::iterator itr_exp=tmp_lst_exp.begin();itr_exp!=tmp_lst_exp.end();) {
				if(*itr_sim==*itr_exp) {
					itr_exp=tmp_lst_exp.erase(itr_exp);
				} else {
				//	*itr_exp=itr_exp->substr(0,itr_exp->find(".root"));		//bugs out with cint
				//	string tmp_str(*itr_exp);
				//	size_t tmp_pos=tmp_str.find(".root");
				//	string tmp_str2(tmp_str.c_str(),tmp_pos);
				//	lst_exp.push_back(tmp_str2);
					lst_exp.push_back(*itr_exp);
					++itr_exp;
				}
			}
		//	*itr_sim=itr_sim->substr(0,itr_sim->find(".root"));
		//	string tmp_str3(*itr_sim);
		//	size_t tmp_pos2=tmp_str.find(".root");
		//	string tmp_str4(tmp_str3.c_str(),tmp_pos2);
		//	lst_sim.push_back(tmp_str4);
			lst_sim.push_back(*itr_sim);
		}
		for(list<string>::iterator itr_exp=lst_exp.begin();itr_exp!=lst_exp.end();++itr_exp) {
			TList* exp_data=dynamic_cast<TList*>(file->Get( (string("hp_")+*itr_exp).c_str() ));
			TH1D* LKr_energy_exp=0, *CsI_energy_exp=0, *CsI1_exp=0, *CsI2_exp=0;
			if(exp_data) {
				LKr_energy_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy1"));
				CsI_energy_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy2"));
				CsI1_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.up_cluster"));
				CsI2_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.down_cluster"));
			}
			exp_buf<<*itr<<"\t\t"<<(LKr_energy_exp?LKr_energy_exp->GetEntries():0)<<"\t"
				<<(CsI_energy_exp?CsI_energy_exp->GetEntries():0)<<"\t"
				<<(CsI1_exp?CsI1_exp->GetEntries():0)<<"\t"<<(CsI2_exp?CsI2_exp->GetEntries():0)<<"\n";
		}
		for(list<string>::iterator itr_sim=lst_sim.begin();itr_sim!=lst_sim.end();++itr_sim) {
			TList* sim_data=dynamic_cast<TList*>(file->Get( (string("hp_")+*itr_sim).c_str() ));
			TH1D* LKr_energy_sim=0, *CsI_energy_sim=0, *CsI1_sim=0, *CsI2_sim=0;
			if(sim_data) {
				LKr_energy_sim=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy1"));
				CsI_energy_sim=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy2"));
				CsI1_sim=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.up_cluster"));
				CsI2_sim=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.down_cluster"));
			}
			sim_buf<<*itr<<"\t\t"<<(LKr_energy_sim?LKr_energy_sim->GetEntries():0)<<"\t"
				<<(CsI_energy_sim?CsI_energy_sim->GetEntries():0)<<"\t"
				<<(CsI1_sim?CsI1_sim->GetEntries():0)<<"\t"<<(CsI2_sim?CsI2_sim->GetEntries():0)<<"\n";
		}
	}
		if(file->IsOpen()) file->Close();
		delete file;
	}
	ofstream result_exp((workdir+"/parsed_entries_exp").c_str(),ofstream::trunc);
	result_exp.write(exp_buf.str().c_str(),exp_buf.str().size());
	result_exp.close();
	ofstream result_sim((workdir+"/parsed_entries_sim").c_str(),ofstream::trunc);
	result_sim.write(sim_buf.str().c_str(),sim_buf.str().size());
	result_sim.close();
gSystem->cd(workdir.c_str());
}
