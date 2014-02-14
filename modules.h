//possibly move it somewhere
//#include <stdio.h>
//#include <map>
#include "KDB/kdb.h"
#include "VDDCRec/kdcvd.h"

#include "VDDCRec/ksettrgdb.h"	//1
#include "VDDCRec/kglobparam.h"	//2
#include "VDDCRec/kedr_trigger.h"	//0
#include "ReadNat/kedr_read_trg.h"	//0
#include "KDisplay/kdisplay_event.h"
//#include "VDDCRec/kevnum.h"	//arcane debugging

#include "io_apps.h"
#include "criteria.h"

#include "sep_beams.h"
#include "VDDCRec/ktracks.h"		//ktrrec_, ktrrec_h_

#include "DataManager.h"
#include "SystemManager.h"

#ifndef aurthem_run
#define aurthem_run

extern "C" {
	void kedrsepbeams_(int* NrepArg, int* c1, int* c2, int* c3, int* c4, int* c5, int* c6, int* c7, int* c8);
	void kedrsepbeams_report_(void);
	void kedrsepbeams_hist_(int* pawcsize, int* IDarg, float* val, int* Nbins, float* vmin, float* vmax);
}

class Run { //run.cpp
public:
    Run(const T_s& target);
    ~Run(void);
	void clear(void);	//clear all data

	//cycles through runs and events, calling user function on each event
	void all( bool(*primary_trigger_function)(), bool(*event_function)() );
	void check( void(*event_function)() );	//only check runs with user function (don't cycle through events)

	//class to contain information about all runs
	//from kdb_runinfo and also some user info (amount of events selected, cpu time used, etc.)
	class GInfo {	//global info
	public:
		class GRun {	//global info about one run
		public:
			int nrun;			//run number
			int status;			//run existence
			KDBruninfo kdbInfo;	//DB information
			char date[100];
			int nread;			//amount of events read
			int nlkr, nlkr_raw;
			int ncsi, ncsi_raw;
			int nemc, nemc_raw;
			float cputime;

			GRun();
			virtual ~GRun();
		} *current;				//current run
		KDBconn *conn;			//pointer to the database
		std::map<int, GRun*> runs;	//map of all runs

		GInfo();
		GInfo(const std::list<int>& ls);
		virtual ~GInfo();

		GRun* GetRun(int nrun);	//get run from the database, creates GRun
		void SetRun(int nrun);	//set run as current; create it, if it's not in runs
		void SetRunList(const std::list<int>& ls);	//add run list to the container
		void PrintList() const;
		void clear();
	} *info;

	Parameters params;
private:
	static void switch_off(RsSlurper* bad_channels, int run_number);
};
#endif

/*/checks if lkrData, csiData, emcRec are filled exactly as
 structures semc_data, semc for an event
 (they are not)
/*/
namespace emcCheck { //emc_check.cpp
    bool event(const bool all); //bails out at first error if !all
}

/*/
draws energy resolution of electromagnetic calorimeters from bhabha scattering events
/*/
namespace Bhabha { //bhabha.cpp
	void bhabha(void);
	
	void channel_info(void);
}
