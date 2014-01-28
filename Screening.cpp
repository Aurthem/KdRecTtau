#include "modules.h"
//#include "criteria.h"
//#include "utilities.h"
#include "DataVisualizer.h"
#include "DataAnalizer.h"

int main(int argc, char *argv[]) {
	IOglobals::popts = new ProgramOptions(argc,argv);
/*/-----test-----//
	CrsSlurper crs("./input/criteria");
	Criteria emc;
	T_msvpdd tmp=crs.get("emc_bhabha");
	std::cout<<"Structure:\nemc_bhabha"<<emc.fill(tmp)<<std::endl;
	std::cout<<"Crs blocks:\n"; crs.print("emc_bhabha");
	PrmSlurper prmS("./input/run_params");
	Parameters prm;
	std::cout<<"bhabha"<<prm.fill(prmS.get("bhabha"))<<std::endl;
//-----test-----/*/
	//single action values:
	if(IOglobals::popts->isGiven("help")) { IOglobals::popts->print(); return 0; }
	if(IOglobals::popts->isGiven("parselog")) {
		const T_s* tmp_log_path=IOglobals::popts->getValue("parselog");
		T_s log_path=(tmp_log_path?*tmp_log_path:"/spool/users/boroden/remote");
		LogRoot::ParseLogs(log_path);
		return 0;
	}
	if(IOglobals::popts->isGiven("drawlog")) {
		DataVisualizer dviz(true,"data",IOglobals::popts->getPath()+"/input/log.style");
		dviz.Draw();
		return 0;
	}
	if(IOglobals::popts->isGiven("draw")) {
		T_s data_name="data";
		const T_s* data_app=IOglobals::popts->getValue("label");
		if(data_app) data_name+=*data_app;
		if(IOglobals::popts->isGiven("sim")) data_name+="_sim";
		
		DataVisualizer dviz(false,data_name,IOglobals::popts->getPath()+"/input/data.style");
		if(IOglobals::popts->isGiven("sim")) dviz.SetLabel("_sim");
		dviz.Draw();
		if(!IOglobals::popts->isGiven("batch")) {
			DataVisualizer dviz2(false,"bad_channels",IOglobals::popts->getPath()+"/input/runs.style");
			dviz2.Draw();
		}
		return 0;
	}
	if(IOglobals::popts->isGiven("remote")) {
		char* tmp_ch=0;	//can't initialize strings with null
		T_s gt_f=( (tmp_ch=getenv("GE_TASK_FIRST"))		? tmp_ch : "");
		T_s gt_l=( (tmp_ch=getenv("GE_TASK_LAST"))		? tmp_ch : "");
		T_s gt_s=( (tmp_ch=getenv("GE_TASK_STEPSIZE"))	? tmp_ch : "");
		T_s gt_id=((tmp_ch=getenv("GE_TASK_ID"))		? tmp_ch : "");
		std::cout<<"Remote array job parameters:\t"<<gt_f<<"-"<<gt_l<<":"<<gt_s<<"\t->\t"<<gt_id<<std::endl;
	}
	//single action end
	
	//complex actions
	if(IOglobals::popts->isGiven("bad") || IOglobals::popts->isGiven("badonly") || IOglobals::popts->isGiven("analize")) {
		if(!IOglobals::popts->isGiven("analize")) Bhabha::channel_info();
		
		DataAnalizer dlz(IOglobals::popts->getPath(),"bad_channels");
		dlz.SetGap(6);	//set 2 for lots of points
		dlz.analize();
		dlz.SetSkips(2);
		dlz.discard_previous();	//comment to allow persistence
		dlz.dump(IOglobals::popts->getPath()+"/input/run_status");
		
		if(!IOglobals::popts->isGiven("bad")) return 0;
	}
	
	Bhabha::bhabha();	//main action
	
	delete IOglobals::popts; IOglobals::popts=0;
	return 0;
}
