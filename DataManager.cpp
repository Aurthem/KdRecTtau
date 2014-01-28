
#include <iostream>
#include "DataManager.h"

DataManager::DataManager(const T_s& ext_path, const T_s& treename, bool update)
: path(ext_path), hfile(0),tree(0),branch(0), skimstream(0) {
	data = new DataClass();
	if(path==T_s("/home/boroden/current/KdRecTtau")) path="/spool/users/boroden/trees";
	init(treename,update);
}
DataManager::~DataManager() {
	delete data;
	if(tree) tree->Print();
	init();	//deletes all root related stuff
//std::cout<<"DataManager destructor activated."<<std::endl;
}
void DataManager::init(const T_s& treename, bool update) {
	base_name = treename; read_status=false; skimstream=0;
	buffer.clear();
	if(tree) { delete tree; tree=0; }
	if(hfile) { delete hfile; hfile=0; }
	if(treename.empty()) return;
	
	name=base_name;
	if(batch.isActive()) {
		name+=batch.GetLabel();
	}
	
	hfile = new TFile( (path+(batch.isActive()?"/batch/":"/")+name+".root").c_str(),
		(update?"UPDATE":"RECREATE"),"TTree run data ROOT file");
	hfile->SetCompressionLevel(1);	//compressed
	tree = new TTree( (T_s("T_")+name).c_str(),(name+" data").c_str());
	tree->SetAutoSave(10000000); // autosave when 10 Mbyte written
	tree->SetCacheSize(10000000);  // set a 10 MBytes cache (useless when writing local files)
	TTree::SetBranchStyle(1);
	branch = tree->Branch(name.c_str(), &data, 16000,1);	//no custom streamer
//	branch->SetAutoDelete(kFALSE);
	branch->SetAutoDelete(kTRUE);
	tree->BranchRef();
}
void DataManager::init_skim(const T_s& headname) {
	init();
	nat_all_records(1);						//to handle all records
	skimstream = write_nat(headname.c_str(),mRunFiles);		//use mSingleFile to write into one file
	if(!skimstream) return;
	nat_filter(skimstream,fLuminosity);		//to keep luminosity records
}
bool DataManager::read(const T_s& treename) {
	base_name=treename; read_status=true; skimstream=0;
	buffer.clear();
	if(tree) { delete tree; tree=0; }
	if(hfile) { delete hfile; hfile=0; }
	if(treename.empty()) return false;
	
	if (!TClassTable::GetDict("DataClass")) {
		gSystem->Load("~/current/lib/libDataClass.so");
	}
	
	name=base_name;
	if(batch.isActive()) {
		name+=batch.GetLabel();
		T_ls file_list=IOconverter::list_files(path+"/batch/",".root",name);
		for(T_ls::const_iterator itr=file_list.begin();itr!=file_list.end();++itr) {
			if(batch.parseLabel(*itr)) {
				name=itr->substr(0,itr->size()-T_s(".root").size());
				break;
			}
		}	//if none were true, file will be made zombie
	}
	
	hfile = new TFile( (path+(batch.isActive()?"/batch/":"/")+name+".root").c_str() );
std::cout<<"Reading file \""<<hfile->GetName()<<"\""<<std::endl;
	if(hfile->IsZombie()) return false;	//failed to load
	tree = dynamic_cast<TTree*>(hfile->Get( (T_s("T_")+name).c_str() ));
	if(!tree) return false;
	branch = tree->GetBranch(name.c_str());
	if(!branch) return false;

//if(data) delete data; data=0; //patch for 0-initialization of branch address (?)
	branch->SetAddress(&data);
	
	centry=0;
	nentry=tree->GetEntries();
	return true;
}

bool DataManager::write(void) {
	if(!tree) return false;
	hfile = tree->GetCurrentFile(); //just in case we switched to a new file (might be a bad idea)
	if( hfile->Write(name.c_str(),TObject::kWriteDelete) ) return true;
	else return false;
}
bool DataManager::next(void) {
	if(tree) {
//std::cout<<"DataManager: read_status="<<read_status<<" old c|n="<<centry<<"|"<<nentry<<std::endl;
		if(!read_status) {
			if(tree->Fill()<0) return false;
			else return true;
		} else {
			if(centry<nentry) {
//if(data) delete data; data=0; //patch for 0-initialization of branch address (?)
				if(tree->GetEntry(centry++)<0) return false;	//when i/o error happens, don't continue reading
//				if(branch->GetEntry(centry++)<0) return false;	//when i/o error happens, don't continue reading
				else return true;
			} else {
				return false;
			}
		}
//std::cout<<"\tnew c|n="<<centry<<"|"<<nentry<<std::endl;
	} else if(skimstream) {
		if(nat_record_id()!=1) {
			std::cout<<"Found auxiliary record with id="<<nat_record_id()<<std::endl;
		} else {
			put_event(skimstream);
		}
		return true;	//should break out of run when fails to read next event
	}
	return false;
}

bool DataManager::batchNext(void) {
	if(batch.next()) {
		if(read_status) return read(base_name);
		init(base_name); return true;
	}
	return false;
}

void DataManager::flush(void) {
	for(std::map<std::pair<T_s,T_s>,std::pair<double,T_s> >::const_iterator itr=buffer.begin();itr!=buffer.end();++itr) {
		data->Add(itr->first.first,itr->first.second,itr->second.first,itr->second.second);
	}
	buffer.clear();
}
bool DataManager::BuffCheck(const Criteria& cr, const T_s& glabel, double value, const T_s& ulabel) {
	T_pss tmp=data->cut_labels(glabel);
//std::cout<<"DataManager AddCheck: \""<<glabel<<"\"="<<value<<" "<<ulabel<<"(";
	if(cr.check(tmp.second,value,ulabel)) {
		buffer[tmp]=std::make_pair(value,ulabel);
//std::cout<<"true)"<<std::endl;
		return true;
	} else {
//std::cout<<"false)"<<std::endl;
		return false;
	}
}

bool DataManager::Check(const Criteria& cr, const T_s& glabel) const {
	T_pss tmp=data->cut_labels(glabel);
	return Check(cr,tmp.first,tmp.second);
}
bool DataManager::Check(const Criteria& cr, const T_s& blabel, const T_s& flabel) const {
	const TObject* tmp=data->Get(blabel,flabel);
	if(!tmp) return true;
	bool result=true;
	{	double tmp_value;
		if(ExtractData<double>(tmp,tmp_value)) result&=cr.check(flabel,tmp_value);
	}{	int tmp_value;
		if(ExtractData<int>(tmp,tmp_value)) result&=cr.check(flabel,tmp_value);
	}{	T_vd tmp_value;
		if(ExtractDataV<T_vd>(tmp,tmp_value))
			for(T_vd::const_iterator itr=tmp_value.begin();itr!=tmp_value.end();++itr) {
				result&=cr.check(flabel,*itr);
			}
	}{	T_vi tmp_value;
		if(ExtractDataV<T_vi>(tmp,tmp_value))
			for(T_vi::const_iterator itr=tmp_value.begin();itr!=tmp_value.end();++itr) {
				result&=cr.check(flabel,*itr);
			}
	}
	return result;//cr.check(flabel,tmp->first,tmp->second);
}
bool DataManager::CheckinList(const Criteria& cr, const T_s& blabel, const T_ls& ls) const {
	for(T_ls::const_iterator itr=ls.begin();itr!=ls.end();itr++) {
		if(!Check(cr,blabel,*itr)) return false;
	}
	return true;
}

void DataManager::Batch::init(const T_s& b_str, const T_li* run_list) {
	std::size_t pos=b_str.find(':');	//first colon separates base from active list
	base=static_cast<unsigned int>( std::atoi(b_str.substr(0,pos).c_str()) );
	if(pos!=T_s::npos && ++pos!=T_s::npos) {
		active=IOconverter::convert_list<unsigned int>( IOconverter::extract_list(b_str.substr(pos)) );
	} else {
		std::cerr<<"Warning: no active parts in batch mode."<<std::endl;
	}
	current=active.begin();
	
	if(run_list && !run_list->empty()) {
		T_li tmp_runs=*run_list; tmp_runs.sort(); tmp_runs.unique();
		if(base) {
			unsigned int basket=tmp_runs.size()/base;
			unsigned int overflow=tmp_runs.size()%base;
			T_li::const_iterator tmp=tmp_runs.begin();
			for(unsigned int idx=1;idx<=base;++idx) {
				unsigned int ii=((overflow ? overflow-->0 : false) ? basket+1 : basket);
				for(unsigned int i=0;i<ii;++i) {
					runs[idx].push_back(*tmp++);
					if(tmp==tmp_runs.end()) return;
				}
			}
		} else {
			runs[0]=tmp_runs;	//dump all to 0 cell
		}
	}
}

T_s DataManager::Batch::GetLabel(void) const {
	std::ostringstream result;
	result<<"-b"<<base<<"_";
	if(current!=active.end()) {
		result<<*current<<(sim?"-s":"-r");
		if(runs.count(*current)) result<<IOconverter::print_list(GetRuns());
	}
	return result.str();
}
const T_li* DataManager::Batch::GetRuns(void) const {
	std::map<unsigned int,std::list<int> >::const_iterator itr;
	if(current!=active.end() && (itr=runs.find(*current))!=runs.end())
		return &itr->second;
//	if(current!=active.end() && runs.count(*current)) return &runs[*current];
	return NULL;
}
T_li DataManager::Batch::GetTotalRuns(void) const {
	T_li result;
	std::map<unsigned int,std::list<int> >::const_iterator itr_r;
	for(std::list<unsigned int>::const_iterator itr=active.begin();itr!=active.end();++itr) {
		if( (itr_r=runs.find(*itr))!=runs.end() )
			result.insert(result.end(),itr_r->second.begin(),itr_r->second.end());
	}
	result.sort(); result.unique();
	return result;
}

bool DataManager::Batch::parseLabel(const T_s& str) {
	std::size_t pos1=str.find("-b"), pos2;
	if(pos1==T_s::npos || (pos1+=2)==T_s::npos) return false;
	pos2=str.find('_',pos1);
	if(base!=static_cast<unsigned int>( std::atoi(str.substr(pos1,pos2-pos1).c_str()) )) return false;
	if(pos2==T_s::npos || (pos1=pos2+1)==T_s::npos) return false;
	pos2=str.find((sim?"-s":"-r"),pos1);
	unsigned int cur=static_cast<unsigned int>( std::atoi(str.substr(pos1,pos2-pos1).c_str()) );
	if(std::find(active.begin(),active.end(),cur)==active.end()) return false;
	if(pos2==T_s::npos || (pos1=pos2+2)==T_s::npos) return false;
	runs[cur]=IOconverter::extract_list(str.substr(pos1));
	return true;	//added value to runs
}
