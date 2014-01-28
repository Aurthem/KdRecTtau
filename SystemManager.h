#ifndef au_sysmanager
#define au_sysmanager

#include "KDB/kdb.h"
#include "VDDCRec/kdcvd.h"
#include "VDDCRec/ksettrgdb.h"	//1
#include "VDDCRec/kglobparam.h"	//2
#include "VDDCRec/kedr_trigger.h"	//0
#include "ReadNat/kedr_read_trg.h"	//0
#include "ReadNat/read_nat_c.h"
#include "VDDCRec/kdcswitches.h"
#include "VDDCRec/kdcpar.h"
#include "KEmcRec/emc_system.h"
#include "KrToF/tof_system.h"
#include "KsToF/tofsim.h"	//tof_sim_real
#include "KrMu/mu_system.h"
#include "KrMu/mu_event.h"
#include "KrMu/mu_res.h"

#include "typedefs.h"
#include "io_apps.h"

class SystemManager {
public:
	SystemManager(void);
	virtual ~SystemManager();
private:
	class System {
	public:
		System(void) { };
		virtual ~System(void) { };
		
		virtual bool init(void)=0;
		virtual bool set_run(int run)=0;
		virtual bool process_event(void)=0;
	};
public:
	class TriggerSystem: public System {
	public:
		TriggerSystem(void);
		
		virtual bool init(void);
		virtual bool set_run(int run);
		virtual bool process_event(void);
		
		bool read(void);
		
		int PT_passed, ST_passed, KlukvaErr;
		float LKr_scale, CsI_scale;
	private:
	};
	class VDDCsystem: public System {
	public:
		virtual bool init(void);
		virtual bool set_run(int run);
		virtual bool process_event(void);
	private:
	};
	class EMCsystem: public System {
	public:
		virtual bool init(void);	//false if system was already loaded
		virtual bool set_run(int run);
		virtual bool process_event(void);
	private:
	};
	class ToFsystem: public System {
	public:
		virtual bool init(void);
		virtual bool set_run(int run);
		virtual bool process_event(void);
		
		T_vd times_track_barrel;
		T_vd times_emc_barrel;
		T_vd times_emc_endcap;
	private:
	};
	class MuSystem: public System {
	public:
		MuSystem(void);
		
		virtual bool init(void);
		virtual bool set_run(int run);
		virtual bool process_event(void);
		
		bool cleanup(void);
		
		unsigned int result_hits;
		unsigned short result_type;
	private:
	};

	static TriggerSystem trigger;
	static VDDCsystem vddc;
	static EMCsystem emc;
	static ToFsystem tof;
	static MuSystem mu;
};
#endif	//au_sysmanager
