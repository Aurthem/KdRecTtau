#ifndef au_io_apps
#define au_io_apps

#include "typedefs.h"
#include "slurper.h"

#include "TRandom3.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include "TH1D.h"

//-----------output_classes-----------
class ProgramOptions {
public:
	ProgramOptions(int argc, char *argv[]);
	~ProgramOptions(void);
	
	bool isGiven(const T_s& str) const;
	const T_s* getValue(const T_s& str) const;		//can't return more than one value, fix it
	static std::ostream& print(std::ostream& output=std::cout);
	
	bool addOption(const T_s& opt);		//full name of the option only: --<name>=<opt1> <opt2> ... etc
	
	T_s getPath(void) const { return path; }
private:
	T_vs args;
	class Option {
	public:
		Option(const T_s& nm, unsigned short int vf, const T_s& ds)
			: name(nm),short_name(0),value_fields(vf),description(ds) { }
		Option(const Option& opt);
		Option& operator=(Option opt);
		~Option(void) { if(short_name) delete short_name; }
		
		static std::pair<short int,T_s> get_option(const T_s& str);	//0 - short options are extracted, -1 - failed
		bool hasName(const T_s& str) const { return str==name; }
		bool hasShort(char chr) const { return (short_name ? chr==*short_name : false); }
		unsigned short int GetFields(void) const { return value_fields; }
		
		void SetShName(char snm);
		std::ostream& print(std::ostream& output=std::cout) const;
		static std::ostream& print_usage(std::ostream& output=std::cout);
	private:
		T_s name;
		char* short_name;
		unsigned short int value_fields;
		T_s description;

		static const T_vs option_flags;
		static const char short_option_flag;
	};
	static const std::vector<Option> options;
	static std::vector<Option> generate_options(void);
	static std::vector<Option>::const_iterator find_option(const T_s& str);
	static std::vector<std::vector<Option>::const_iterator> find_short_options(const T_s& str);
	
	std::map<std::vector<Option>::const_iterator,T_s*> given_options;
	
	T_s path;		//program path, if not overridden with an option
};

namespace IOglobals {
	extern ProgramOptions* popts;
}

class Parameters {	//for Run only
public:
	Parameters(void);
	Parameters& fill(const T_pmssmsli& dummy);
	Parameters& clear(void);
	
	bool StringExists(const T_s& key) const;	//checks if this key exists in the map
	bool ListExists(const T_s& key) const;
	bool CheckStrings(const T_vs& key_list) const;
	bool CheckLists(const T_vs& key_list) const;

	const T_li* GetList(const T_s& key) const;
	const T_li* SetList(const T_s& key, const T_li& value);

	const T_s* GetString(const T_s& key) const;
	const T_s* SetString(const T_s& key, const T_s& value);

	char GetChar(const T_s& key, const std::size_t pos, const char def_value='0') const;
	const T_s* SetChar(const T_s& key, const std::size_t pos, const char value, const char def_value='0');
	
	T_s PrintList(const T_s& key) const;
private:
	T_mss strings;
	T_msli lists;
	
	double value;

	friend std::ostream& operator<< (std::ostream&,const Parameters&);
};

//-----------input_classes-----------
class CrsSlurper : public Slurper {		//rewrite these slurpers to generalize and simplify format
public:
	CrsSlurper(const T_s& filename) : Slurper(filename) { }
	T_msvpdd get(const T_s& target);	//get all the useful stuff from target block
private:
//	T_pdd extract_pair(const T_s& str);	//extracts limits from innermost {"..."}
};

class PrmSlurper : public Slurper {
public:
	PrmSlurper(const T_s& filename) : Slurper(filename) { }
	T_pmssmsli get(const T_s& target);
private:
//	T_li extract_list(const T_s& str);
//	T_li generate_list(int val1, int val2);
};

class CssSlurper : public Slurper {
public:
	CssSlurper(const T_s& filename) : Slurper(filename) { }
	T_mss get(const T_s& target) const;	//customize some more later
	T_ls get_blocks(const T_s& target="") const;
private:
	void do_nothing(void) const;
};

class RsSlurper : public Slurper {
public:
	RsSlurper(const T_s& filename) : Slurper(filename) { readout(filename); }
	void readout(const T_s& filename);
	
	struct Status {
		double energy;
		T_li LKr_ring;
		T_li LKr_bad;
		T_li CsI_ring;
		T_li CsI_bad;
	};
	std::map<T_li,Status>* get(void) { return &contents; }
	void print(std::ostream& output=std::cout) const;
	void clear(void) { contents.clear(); }
private:
	std::map<T_li,Status> contents;
};

class LumReader {
public:
	LumReader(void) : path("/home/boroden/current/KdRecTtau") { }
	LumReader(const std::map<int,double>& ext_data) : data(ext_data),path("/home/boroden/current/KdRecTtau") { }
	
	void setPath(const T_s& ext_path) { path=ext_path; }
	
	void read(const T_s& from);
	void write(const T_s& where) const;
	
	void append(const std::map<int,double>& ext_data) { data.insert(ext_data.begin(),ext_data.end()); }
	void clear(void) { data.clear(); }
	
	std::map<int,double> get_multipliers(const T_li& run_list, TRandom3& root_rand) const;
private:
	std::map<int,double> data;
	T_s path;
};

class LogRoot {
public:
	LogRoot(const T_s& path, const T_s& logname, const T_s& label="");
	~LogRoot(void);
	void init(const T_s& label);
	
	bool SetLabel(const T_s& label);
	
	void Add(TObject* obj) { data->Add(obj); }	//could use <Option_t* opt> maybe
	TObject* Get(const T_s& obj_name) const { return data->FindObject(obj_name.c_str()); }
	TObject* getNext(void) { return data_itr->Next(); }
	void reset(void) { data_itr->Reset(); }
	
	bool Write(void) const;
	void Clear(void);	//clears all objects in the file
	void Purge(void);	//recreates file
	
	static void ParseLogs(const T_s& ext_path);
private:
	TList* data;
	TIter* data_itr;
	
	T_s base_name;
	T_s name;	//just current name
	T_s cur_label;	//current data label
	
	LogRoot(const LogRoot& lr) { std::cerr<<"LogRoot: copy constructor is not allowed."<<std::endl; }
};
#endif	//au_io_apps
