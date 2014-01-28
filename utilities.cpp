
#include "utilities.h"

#include <TMath.h>
//namespace{
//	double pi=TMath::Pi();
//	double pi=3.14159265358979323846;
//}

namespace SUconverter{	//but it's better like that, because there're no names of private members in the header
const T_msd time=
	build_map_from_list<T_s,double>("ps",1e-3)("ns",1.)("mks",1e+3)("ms",1e+6)("s",1e+9);
const T_msd distance=
	build_map_from_list<T_s,double>("nm",1e-7)("mkm",1e-4)("mm",1e-1)("cm",1.)("dm",1e+1)("m",1e+2);
const T_msd angle=
	build_map_from_list<T_s,double>("deg",TMath::Pi()/180.)("rad",1.);
const T_msd energy=
	build_map_from_list<T_s,double>("eV",1e-6)("keV",1e-3)("MeV",1.)("GeV",1e+3);
const T_msd magnetic_field=
	build_map_from_list<T_s,double>("Gauss",1.)("Gs",1.)("kGs",1e+3)("Tesla",1e+4)("Tl",1e+4);
const T_msd area=
	build_map_from_list<T_s,double>("cm^2",1.)("m^2",1e+4)("mm^2",1e-2)("fm^2",1e-26)("barn",1e-24)("b",1e-24)("kb",1e-21)("mkb",1e-30)("nb",1e-33);
const T_msd luminosity=	//integral luminosity
	build_map_from_list<T_s,double>("mkb^{-1}",1.)("nb^{-1}",1e+3)("b^{-1}",1e-6)("fm^{-2}",1e-4);

	T_s print(const T_pdd& pr, const T_s& format) {
		std::ostringstream result; result<<std::setprecision(5)<<std::scientific;
		double mtpl=get_multiplier(format);
		if(mtpl==0.) {	//this is never entered
			std::cerr<<"Wrong units entered, won't be used as a multiplier."<<std::endl;
			mtpl=1.;
		}
		if(pr.first!=-std::numeric_limits<double>::max() && pr.second!=std::numeric_limits<double>::max()) {
			result<<"{"<<pr.first/mtpl<<" % "<<pr.second/mtpl<<"}"<<format;
		} else if(pr.first!=-std::numeric_limits<double>::max()) {
			result<<"{>"<<pr.first/mtpl<<"}"<<format;
		} else if(pr.second!=std::numeric_limits<double>::max()) {
			result<<"{<"<<pr.second/mtpl<<"}"<<format;
		} else {
		    result<<"{ }"<<format;
		}
		return result.str();
	}
	T_s print(const T_vpdd& vpr, const T_s& format) {
		std::ostringstream result; result<<std::setprecision(5)<<std::scientific;
		double mtpl=get_multiplier(format);
		if(mtpl==0.) {	//this is never entered
			std::cerr<<"Wrong units entered, won't be used as a multiplier."<<std::endl;
			mtpl=1.;
		}
		result<<"{";
		for(T_vpdd::const_iterator itr=vpr.begin();itr!=vpr.end();++itr) {
			if(itr!=vpr.begin()) result<<",";
			if(itr->first!=-std::numeric_limits<double>::max() && itr->second!=std::numeric_limits<double>::max()) {
				result<<itr->first/mtpl<<"%"<<itr->second/mtpl;
			} else if(itr->first!=-std::numeric_limits<double>::max()) {
				result<<">"<<itr->first/mtpl;
			} else if(itr->second!=std::numeric_limits<double>::max()) {
				result<<"<"<<itr->second/mtpl;
			}
		}
		result<<"}"<<format;
		return result.str();
	}

	double get_multiplier(const T_s& from_unit, const T_s& to_unit) {
		double result=1.;
		if(!from_unit.empty()) {
			T_msd::const_iterator itr;
			if((itr= time.find(from_unit))!= time.end() || (itr=  distance.find(from_unit))!=  distance.end()
			 ||(itr=angle.find(from_unit))!=angle.end() || (itr=    energy.find(from_unit))!=    energy.end()
			 ||(itr= area.find(from_unit))!= area.end() || (itr=luminosity.find(from_unit))!=luminosity.end()
			 ||(itr=magnetic_field.find(from_unit))!=magnetic_field.end()) {
				result=itr->second;
			} else {
				std::cerr<<"Warning: \""<<from_unit<<"\" is not a valid unit, ignored."<<std::endl;
			}
		} else if(to_unit.empty()) {	//recursion ends
			return result;
		}
		return result/get_multiplier(to_unit);
	}
	SUtype get_type(const T_s& str) {
		if(time.find(str)		!=time.end())		return time_unit;
		if(distance.find(str)	!=distance.end())	return distance_unit;
		if(angle.find(str)		!=angle.end())		return angle_unit;
		if(energy.find(str)		!=energy.end())		return energy_unit;
		if(magnetic_field.find(str)!=magnetic_field.end())	return magnetic_field_unit;
		if(area.find(str)		!=area.end())		return area_unit;
		if(luminosity.find(str)	!=luminosity.end())	return luminosity_unit;
		return no_unit;
	}
}

namespace IOconverter {
	int get_int_warn(const T_s& str) {
		int result=0;
		if( str.empty() )
			std::cerr<<"List extraction from empty string."<<std::endl;
		else if( !( result=atoi(str.c_str()) ) )	//add &&(str!="0") maybe, so that only incorrect values trigger
			std::cerr<<"List extraction from \""<<str<<"\" gave possibly incorrect value (0)."<<std::endl;
		return result;
	}
	T_li extract_list(const T_s& str) {
		T_li result; int val1=0, val2; bool opened_range=false;
		std::size_t pos1=0, pos2;
		T_s dummy;
		while(pos1!=T_s::npos) {
			pos2=str.find_first_of("-,",pos1);
			if(!opened_range) {
				if(pos2!=T_s::npos) {
					if(pos2==str.size()-1) {
						if( !(dummy=str.substr(pos1,pos2-pos1)).empty() ) result.push_back(get_int_warn(dummy));
						pos1=T_s::npos;
					} else {
						if(str.at(pos2)==',') {
							if( !(dummy=str.substr(pos1,pos2-pos1)).empty() ) result.push_back(get_int_warn(dummy));
							pos1=pos2+1;
						} else {
							if( !(dummy=str.substr(pos1,pos2-pos1)).empty() ) {
								val1=get_int_warn(dummy); opened_range=true;
							}
							pos1=pos2+1;
						}
					}
				} else {
					if( !(dummy=str.substr(pos1)).empty() ) result.push_back(get_int_warn(dummy));
					pos1=T_s::npos;
				}
			} else {
				if(pos2!=T_s::npos) {
					if(pos2==str.size()-1) {
						if( !(dummy=str.substr(pos1,pos2-pos1)).empty() ) {
							val2=get_int_warn(dummy);
							T_li tmp=generate_list(val1,val2); opened_range=false;
							result.insert(result.end(),tmp.begin(),tmp.end());
						}
						pos1=T_s::npos;
					} else {
						if(str.at(pos2)==',') {
							if( !(dummy=str.substr(pos1,pos2-pos1)).empty() ) {
								val2=get_int_warn(dummy);
								T_li tmp=generate_list(val1,val2); opened_range=false;
								result.insert(result.end(),tmp.begin(),tmp.end());
							}
							pos1=pos2+1;
						} else {	//middle values in range are ignored (what're they supposed to do, anyway?)
							pos1=pos2+1;
						}
					}
				} else {
					if( !(dummy=str.substr(pos1)).empty() ) {
						val2=get_int_warn(dummy);
						T_li tmp=generate_list(val1,val2); opened_range=false;
						result.insert(result.end(),tmp.begin(),tmp.end());
					}
					pos1=T_s::npos;
				}
			}
		}
		result.sort(); result.unique();
		return result;
	}
	T_li generate_list(int val1, int val2) {
		T_li result;
		int min, max;
		if(val1<val2) { min=val1; max=val2; }
		else { min=val2; max=val1; }
		for(int i=min;i<=max;++i) {
			result.push_back(i);
		}
		return result;
	}

	T_pdd extract_pair(const T_s& str) {
		T_pdd result(-std::numeric_limits<double>::max(),std::numeric_limits<double>::max());	//for double min() is a min _positive_ value
		std::size_t pos=str.find_first_of("<>");
		if(pos!=T_s::npos && pos!=str.size()-1) {	//on incorrect values atof() gives 0.0
			if(str.at(pos)=='<') {
				result.second=atof(str.substr(pos+1).c_str());
			} else {
				result.first=atof(str.substr(pos+1).c_str());
			}
		} else {
			pos=str.find_first_of('%');
			if(pos!=T_s::npos && pos!=0 && pos!=str.size()-1) {
				result.first=atof(str.substr(0,pos).c_str());
				result.second=atof(str.substr(pos+1).c_str());
			}
		}
		if(result.first>=result.second) {
			std::cerr<<"Extract pair: limits leave no gap, default values are returned."<<std::endl;
			return T_pdd(-std::numeric_limits<double>::max(),std::numeric_limits<double>::max());
		}
		return result;
	}

	T_ls list_files(const T_s& dirname,const T_s& suffix,const T_s& prefix) {
		T_ls result;
		if(dirname.empty()) return result;		//just a precaution
		T_s workdir=gSystem->pwd();
		TSystemDirectory dir(dirname.c_str(),dirname.c_str());
		TList *files = dir.GetListOfFiles();	//uses new inside (!)
		gSystem->cd(workdir.c_str());	//bug fix for ROOT prior to 5.34
		
		if(files) {
			TIter TL_itr(files);
			TSystemFile *file;
			T_s filename;
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
	T_ls list_directories(const T_s& dirname) {
		T_ls result;
		if(dirname.empty()) return result;		//just a precaution
		T_s workdir=gSystem->pwd();
		TSystemDirectory dir(dirname.c_str(),dirname.c_str());
		TList *files = dir.GetListOfFiles();	//uses new inside (!)
		gSystem->cd(workdir.c_str());	//bug fix for ROOT prior to 5.34
		
		if(files) {
			TIter TL_itr(files);
			TSystemFile *file;
			T_s filename;
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
}
