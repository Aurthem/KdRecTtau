#ifdef __CINT__

#ifndef aurthem_typedefs_cint
#define aurthem_typedefs_cint

#include <vector>
#include <string>
#include <map>
#include <list>
#include <sstream>

//rootcint doesn't like "std::" and merges this namespace with default one
using namespace std;	//this will be ignored anyway
	typedef string T_s;
	typedef vector<string> T_vs;
	typedef pair<int,int> T_pii;
	typedef map<string,double> T_msd;
	typedef map<string,string> T_mss;
	typedef vector<map<string,string> > T_vmss;
	typedef pair<double,double> T_pdd;
	typedef pair<string,string> T_pss;
//	typedef map<string,pair<double,double> > T_mspdd;
//	typedef pair<string,pair<double,double> > T_pspdd;
	typedef vector<pair<double,double> > T_vpdd;
	typedef map<string,vector<pair<double,double> > > T_msvpdd;
	typedef pair<string,vector<pair<double,double> > > T_psvpdd;
	typedef list<int> T_li;
	typedef map<string,list<int> > T_msli;
	typedef pair<map<string,string>,map<string,list<int> > > T_pmssmsli;
	typedef map<string,pair<int,int> > T_mspii;
	typedef pair<map<string,string>,map<string,pair<int,int> > > T_pmssmspii;

	typedef list<string> T_ls;
	typedef pair<double,string> T_pds;
	typedef map<string,pair<double,string> > T_mspds;
	typedef map<string,map<string,pair<double,string> > > T_msmspds;
	typedef map<string,map<string,pair<vector<double>,string> > > T_msmspvds;
	typedef vector<double> T_vd;
	typedef vector<int> T_vi;
	typedef pair<vector<double>,string> T_pvds;
	
	typedef map<string,map<string,string> > T_msmss;
	typedef map<string,list<string> > T_msls;

	typedef pair<string,double> T_psd;
	
	typedef typename T_mspds::iterator T_mspds_itr;
	typedef typename T_mspds::const_iterator T_mspds_citr;
	typedef typename T_msmspds::const_iterator T_msmspds_citr;
	typedef typename list<T_mspds_citr>::const_iterator T_lmspds_citr_citr;
	
	typedef typename T_vd::const_iterator T_vd_citr;
	typedef typename T_vi::const_iterator T_vi_citr;
	typedef typename T_vd::size_type T_vd_st;
	typedef typename T_vi::size_type T_vi_st;

#endif	//aurthem_typedefs_cint
#endif	//__CINT__
