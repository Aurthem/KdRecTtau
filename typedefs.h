#ifndef __CINT__

//you can just replace those back, if you so desire
#ifndef aurthem_typedefs
#define aurthem_typedefs

#include <vector>
#include <string>
#include <map>
#include <list>
#include "TObject.h"
#include <sstream>

namespace {	//stl types used throughout
	typedef std::string T_s;
	typedef std::vector<std::string> T_vs;
	typedef std::pair<int,int> T_pii;
	typedef std::map<std::string,double> T_msd;
	typedef std::map<std::string,std::string> T_mss;
	typedef std::vector<std::map<std::string,std::string> > T_vmss;
	typedef std::pair<double,double> T_pdd;
	typedef std::pair<std::string,std::string> T_pss;
//	typedef std::map<std::string,std::pair<double,double> > T_mspdd;
//	typedef std::pair<std::string,std::pair<double,double> > T_pspdd;
	typedef std::vector<std::pair<double,double> > T_vpdd;
	typedef std::map<std::string,std::vector<std::pair<double,double> > > T_msvpdd;
	typedef std::pair<std::string,std::vector<std::pair<double,double> > > T_psvpdd;
	typedef std::list<int> T_li;
	typedef std::map<std::string,std::list<int> > T_msli;
	typedef std::pair<std::map<std::string,std::string>,std::map<std::string,std::list<int> > > T_pmssmsli;
	typedef std::map<std::string,std::pair<int,int> > T_mspii;
	typedef std::pair<std::map<std::string,std::string>,std::map<std::string,std::pair<int,int> > > T_pmssmspii;

	typedef std::list<std::string> T_ls;
	typedef std::pair<double,std::string> T_pds;
	typedef std::map<std::string,std::pair<double,std::string> > T_mspds;
	typedef std::map<std::string,std::map<std::string,std::pair<double,std::string> > > T_msmspds;
	typedef std::map<std::string,std::map<std::string,std::pair<std::vector<double>,std::string> > > T_msmspvds;
	typedef std::vector<double> T_vd;
	typedef std::vector<int> T_vi;
	typedef std::pair<std::vector<double>,std::string> T_pvds;

	typedef std::map<std::string,std::map<std::string,std::string> > T_msmss;
	typedef std::map<std::string,std::list<std::string> > T_msls;
	typedef std::pair<unsigned int,unsigned int> T_puu;
	typedef std::map<std::string,std::pair<unsigned int,unsigned int> > T_mspuu;

	typedef std::pair<std::string,double> T_psd;
}

#endif	//aurthem_typedefs
#endif	//__CINT__
