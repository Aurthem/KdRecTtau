
#include "DataAnalizer.h"

#include <algorithm>

const T_pds DataAnalizer::energy_delta=std::make_pair(0.15,"MeV");

DataAnalizer::DataAnalizer(const T_s& ext_path, const T_s& dataname) : dm(ext_path),path(ext_path), skip_counter(1),gap(1) {
	if(!dm.read(dataname)) {
		std::cerr<<"Failed to open file \""<<path<<"/"<<dataname<<"\""<<std::endl;
		exit(-1);
	}
	fill_all();
	
	run_status = new RsSlurper(path+"/input/run_status");
}
DataAnalizer::~DataAnalizer() {
	delete run_status;
}

void DataAnalizer::fill_all(void) {
	dm.reset();
	T_vs tmp_vs;
	
	while(dm.next()) {	//fills all histograms, etc.
		const TObject* tmp; int nrun=0; double energy_value=0.;
		Broken channels;
		
		if(DataManager::ExtractData<int>(dm.data->Get("channels::nrun"),nrun) &&
			DataManager::ExtractData<double>(dm.data->Get("kdb::energy"),energy_value,energy_delta.second) &&
			DataManager::ExtractDataV<T_vi>(dm.data->Get("channels::LKr.bad"),channels.LKr_bad) &&
			DataManager::ExtractDataV<T_vi>(dm.data->Get("channels::LKr.ring"),channels.LKr_ring) &&
			DataManager::ExtractDataV<T_vi>(dm.data->Get("channels::CsI.bad"),channels.CsI_bad) &&
			DataManager::ExtractDataV<T_vi>(dm.data->Get("channels::CsI.ring"),channels.CsI_ring)	) {
				energy[nrun]=energy_value;
				channels.LKr_ring.sort(); channels.LKr_ring.unique();
				channels.LKr_bad.sort(); channels.LKr_bad.unique();
				channels.CsI_ring.sort(); channels.CsI_ring.unique();
				channels.CsI_bad.sort(); channels.CsI_bad.unique();
				bad_channels[nrun]=channels;
				
				double lum_tmp=0.;
			//	if(DataManager::ExtractData<double>(dm.data->Get("channels::int_luminosity"),lum_tmp,"mkb^{-1}")) {
				if(DataManager::ExtractData<double>(dm.data->Get("channels::legacy_ilum"),lum_tmp,"nb^{-1}")) {
					if(lum_tmp!=0. && !lum_runs.count(nrun)) lum_runs[nrun]=lum_tmp;
				}
		}
	}
}

void DataAnalizer::analize(void) {
	points.clear(); std::list<std::pair<double,T_li> >::iterator itr_p;
	for(std::map<int,double>::const_iterator itr=energy.begin();itr!=energy.end();++itr) {
		itr_p=points.begin();
		while( (itr_p=find_point(itr->second,itr_p)) != points.end() ) {
			//gap=2 - one run can be failed without generating a new point
			if( (itr_p->second.front()-itr->first)<=gap && (itr->first-itr_p->second.back())<=gap ) {
				itr_p->first = ( ( itr_p->first*(itr_p->second.size()) )+itr->second )/(itr_p->second.size()+1);
				itr_p->second.push_back(itr->first);
				itr_p->second.sort(); itr_p->second.unique();	//use std::set to avoid these lines
				break;
			}
			++itr_p;
		}
		if(itr_p==points.end()) points.push_back( std::make_pair(itr->second,T_li(1,itr->first)) );
	}
	for(std::list<std::pair<double,T_li> >::iterator itr=points.begin();itr!=points.end();++itr) {
		if(itr->first!=0) continue;		//so that only bad runs are merged
		for(std::list<std::pair<double,T_li> >::iterator itr_s=itr;itr_s!=points.end();) {
			if(itr_s==itr) { ++itr_s; continue; }
			if( fabs(itr->first-itr_s->first)<energy_delta.first ) {
				itr->first = ( itr->first*(itr->second.size())+itr_s->first*(itr_s->second.size()) )/(itr->second.size()+itr_s->second.size());
				itr->second.insert(itr->second.end(),itr_s->second.begin(),itr_s->second.end());
				itr->second.sort();
				itr_s=points.erase(itr_s);
			} else {
				++itr_s;
			}
		}
	}
	
std::cout<<"Got points:\n";
	int run_count=0; double mean_energy=0.;
	for(std::list<std::pair<double,T_li> >::const_iterator itr=points.begin();itr!=points.end();++itr) {
		std::cout<<"\t"<<itr->first<<" {"<<IOconverter::print_list(&itr->second)<<"}"<<std::endl;
		if(itr->first!=0) { run_count+=itr->second.size(); mean_energy+=itr->second.size()*(itr->first); }
	}
	if(run_count) mean_energy/=run_count;
	std::cout<<"\t\tRuns analized = "<<run_count<<", with mean energy = "<<mean_energy<<std::endl;
}

void DataAnalizer::dump(const T_s& where) {
	std::map<T_li,RsSlurper::Status>* dummy=run_status->get();
	std::map<T_li,RsSlurper::Status>::iterator current;
	for(std::list<std::pair<double,T_li> >::const_iterator itr=points.begin();itr!=points.end();++itr) {
		current=dummy->insert( std::make_pair(itr->second,RsSlurper::Status()) ).first;
		current->second.energy=itr->first;
		std::map<int,unsigned int> LKr_ring_counter, LKr_bad_counter, CsI_ring_counter, CsI_bad_counter;
		for(T_li::const_iterator itr_l=itr->second.begin();itr_l!=itr->second.end();++itr_l) {
			std::map<int,Broken>::const_iterator run=bad_channels.find(*itr_l);
			if(run!=bad_channels.end()) {
				T_li::const_iterator itr_b;
				unsigned int cur_skip_counter=	//to account for container size
					(itr->second.size()<skip_counter+1 ? (itr->second.size()-1) : skip_counter);
				for(itr_b=run->second.LKr_ring.begin();itr_b!=run->second.LKr_ring.end();++itr_b)
					if( (LKr_ring_counter.count(*itr_b) ? ++LKr_ring_counter[*itr_b] : LKr_ring_counter[*itr_b]=0)==cur_skip_counter )
						current->second.LKr_ring.push_back(*itr_b);
				for(itr_b=run->second.LKr_bad.begin();itr_b!=run->second.LKr_bad.end();++itr_b)
					if( (LKr_bad_counter.count(*itr_b) ? ++LKr_bad_counter[*itr_b] : LKr_bad_counter[*itr_b]=0)==cur_skip_counter )
						current->second.LKr_bad.push_back(*itr_b);
				for(itr_b=run->second.CsI_ring.begin();itr_b!=run->second.CsI_ring.end();++itr_b)
					if( (CsI_ring_counter.count(*itr_b) ? ++CsI_ring_counter[*itr_b] : CsI_ring_counter[*itr_b]=0)==cur_skip_counter )
						current->second.CsI_ring.push_back(*itr_b);
				for(itr_b=run->second.CsI_bad.begin();itr_b!=run->second.CsI_bad.end();++itr_b)
					if( (CsI_bad_counter.count(*itr_b) ? ++CsI_bad_counter[*itr_b] : CsI_bad_counter[*itr_b]=0)==cur_skip_counter )
						current->second.CsI_bad.push_back(*itr_b);
			}
		}
		current->second.LKr_ring.sort(); current->second.LKr_ring.unique();
		current->second.LKr_bad.sort(); current->second.LKr_bad.unique();
		current->second.CsI_ring.sort(); current->second.CsI_ring.unique();
		current->second.CsI_bad.sort(); current->second.CsI_bad.unique();
	}
	std::ofstream file(where.c_str(),std::ios_base::trunc);
	run_status->print(file);
	file.close();
	
	LumReader lumr(lum_runs); lumr.setPath(path); lumr.write("luminosity");
}

//T_vd::iterator DataAnalizer::find_energy(T_vd::iterator begin, T_vd::iterator end, const double value) {
//	for(T_vd::iterator itr=begin;itr!=end;++itr) {
//		if( fabs(value-*itr)<energy_delta.first ) return itr;
//	}
//	return end;
//}
std::list<std::pair<double,T_li> >::iterator DataAnalizer::find_point(const double value, const std::list<std::pair<double,T_li> >::iterator position) {
	bool skipped_once=false;
	for(std::list<std::pair<double,T_li> >::iterator itr=position;itr!=points.end();++itr) {
		if( fabs(value-itr->first)<energy_delta.first ) return itr;
	}
	return points.end();
}
