
#include "criteria.h"

Criteria::Criteria() {
}

Criteria& Criteria::fill(const T_msvpdd& dummy) {
	thresholds.insert(dummy.begin(),dummy.end());
	return *this;
}

bool Criteria::check(const T_s& key, const double& value, const T_s& si) const {
	double tmp=value*SUconverter::get_multiplier(si);
	if(thresholds.count(key)) {
		bool result=false;
		T_msvpdd::const_iterator found=thresholds.find(key);
		for(T_vpdd::const_iterator itr=found->second.begin();itr!=found->second.end();++itr) {
			result|=(itr->first<tmp && tmp<itr->second);
		}
		return result;
	}
	else return true;
}

std::ostream& operator<< (std::ostream& out, const Criteria& cr) {
	T_msvpdd::const_iterator itr;
	out<<"{\n";
	for(itr=cr.thresholds.begin();itr!=cr.thresholds.end();itr++) {
		out<<"\t"<<itr->first<<"="<<SUconverter::print(itr->second,"")<<";\n";
	}
	out<<"}\n";
	return out;
}
