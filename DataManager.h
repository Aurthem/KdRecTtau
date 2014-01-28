#ifndef au_dataManager
#define au_dataManager

#include "typedefs.h"
#include "criteria.h"

#include "DataClass/DataClass.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TClassTable.h"
#include "TSystem.h"

#include "ReadNat/read_nat_c.h"
#include "ReadNat/re_def.h"

class DataManager {
public:
	DataManager(const T_s& ext_path="/spool/users/boroden/trees", const T_s& treename="", bool update=false);
	virtual ~DataManager();
	void clear(void) { clear_buf(); data->clear(); reset(); }
	void clear_buf(void) { buffer.clear(); }
	void init(const T_s& treename="", bool update=false);
	void init_skim(const T_s& headname="/spool/users/boroden/skim/skim_ee");
	bool read(const T_s& treename="");
	void reset(void) { if(read_status) centry=0; }
	void flush(void);	//adds buffer to data (buffer is cleared)

	bool write(void);	//true on success
	bool next(void);	//fills tree and readies for (or reads) next event
	bool batchNext(void);	//prepares next file
//	void print();
	
	bool BuffCheck(const Criteria& cr, const T_s& glabel, double value, const T_s& ulabel="");	//if false, value is not added
	
	bool Check(const Criteria& cr, const T_s& glabel) const;
	bool Check(const Criteria& cr, const T_s& blabel, const T_s& flabel) const;
	bool CheckinList(const Criteria& cr, const T_s& blabel, const T_ls& ls) const;

	DataClass* data;
	
	template<typename Type, typename Res>
	static bool ExtractData(const TObject* origin, Res& result, const T_s& res_unit="") {
		const DataClass::Data<Type>* tmp_data=dynamic_cast<const DataClass::Data<Type>*>(origin);
		return (tmp_data && convert_value(result,tmp_data->get(),
			static_cast<Res>(SUconverter::get_multiplier(tmp_data->get_unit(),res_unit))) );
	}	//static_cast here can hide some errors in units, be careful
	template<typename Type, typename Res>		//double template, possibly
	static bool ExtractDataV(const TObject* origin, Res& result, const T_s& res_unit="") {
		const DataClass::Data<Type>* tmp_data=dynamic_cast<const DataClass::Data<Type>*>(origin);
		return (tmp_data && convert_container(result,tmp_data->get(),
			static_cast<typename Res::value_type>(SUconverter::get_multiplier(tmp_data->get_unit(),res_unit))) );
	}
	
	class Batch {
	public:
		Batch(void) : base(0),current(active.begin()),sim(false) { }
		Batch(const T_s& b_str, const T_li* run_list=NULL): sim(false) { init(b_str,run_list); }
		void init(const T_s& b_str, const T_li* run_list=NULL);
		bool next(void) { if(current!=active.end()) return ++current!=active.end(); return false; }
		void reset(void) { current=active.begin(); }
		
		bool isActive(void) const { return current!=active.end(); }
		
		T_s GetLabel(void) const;
		const T_li* GetRuns(void) const;
		T_li GetTotalRuns(void) const;	//returns all active runs
		void SetSim(bool sm) { sim=sm; }
		
		bool parseLabel(const T_s& str);
	private:
		unsigned int base;
		std::list<unsigned int> active;
		std::list<unsigned int>::const_iterator current;
		
		bool sim;	//indicates simulation
		std::map<unsigned int,std::list<int> > runs;
	} batch;
private:
	std::map<std::pair<T_s,T_s>,std::pair<double,T_s> > buffer;
	
	T_s path;	//main path

	TFile* hfile;
	TTree* tree;
	TBranch *branch;	//TBranchSTL?
	T_s base_name;
	T_s name;	//just current name
	bool read_status;
	Long64_t centry;	//current entry number
	Long64_t nentry;	//max entry
	
	int skimstream;
};
#endif
