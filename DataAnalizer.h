#ifndef au_analizer
#define au_analizer

#include "typedefs.h"
#include "DataManager.h"
#include "io_apps.h"

//class with non-trivial destructor should have copy constructor and assignment operator to avoid deletion while copying
//Manager(const Manager&)	Manager& operator=(const Manager&)

class DataAnalizer {
public:
	DataAnalizer(const T_s& ext_path, const T_s& dataname);
	virtual ~DataAnalizer();
	
	void analize(void);
	void dump(const T_s& where);
	void discard_previous(void) { run_status->clear(); }
	void SetSkips(unsigned int number=1) { skip_counter=number; }
	void SetGap(int number=1) { gap=number; }
private:
	DataManager dm;
	RsSlurper* run_status;
	
	T_s path;
	unsigned int skip_counter;
	int gap;
	
	static const T_pds energy_delta;
	
	std::map<int,double> energy;
	std::list<std::pair<double,T_li> > points;	//0. is for bad runs
	
	std::map<int,double> lum_runs; //mkb^{-1}=10^30 cm^{-2}
	
	struct Broken{
		T_li LKr_ring, LKr_bad;
		T_li CsI_ring, CsI_bad;
	};
	std::map<int,Broken> bad_channels;	//no run is a bad run; LKr, CsI
	
	void fill_all(void);
//	static T_vd::iterator find_energy(T_vd::iterator begin, T_vd::iterator end, const double value);
	std::list<std::pair<double,T_li> >::iterator find_point(const double value, const std::list<std::pair<double,T_li> >::iterator position);
};

#endif	//au_analizer
