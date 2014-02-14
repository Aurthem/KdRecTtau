
#include "io_apps.h"

namespace IOglobals {
	ProgramOptions* popts=0;
}

//-----------output_classes-----------
const std::vector<ProgramOptions::Option> ProgramOptions::options=ProgramOptions::generate_options();

ProgramOptions::ProgramOptions(int argc, char *argv[]) {
	args=T_vs(argv,argv+argc);
	std::pair<short int,T_s> dummy;
	unsigned short int fields_to_extract=0;
	std::vector<std::vector<ProgramOptions::Option>::const_iterator>* short_list=0;
	for(T_vs::const_iterator itr=args.begin();itr!=args.end();++itr) {
		if(itr==args.begin()) continue;
		dummy=Option::get_option(*itr);
		if(dummy.first>0) {
			std::vector<ProgramOptions::Option>::const_iterator tmp=find_option(dummy.second);
			if(tmp!=options.end()) {
				if(!given_options.count(tmp)) given_options[tmp]=NULL;
				for(unsigned short int idx=0;idx<tmp->GetFields();++idx) {
					if(++itr==args.end()) return;
					if(given_options[tmp]) *given_options[tmp]+=","+*itr;
					else given_options[tmp]=new T_s(*itr);
				}
			}
		} else if(!dummy.first) {
			short_list=new std::vector<std::vector<ProgramOptions::Option>::const_iterator>(find_short_options(dummy.second));
			for(std::vector<std::vector<ProgramOptions::Option>::const_iterator>::const_iterator itr_s=short_list->begin();itr_s!=short_list->end();++itr_s) {
				if(!given_options.count(*itr_s)) given_options[*itr_s]=NULL;
				for(unsigned short int idx=0;idx<(*itr_s)->GetFields();++idx) {
					if(++itr==args.end()) return;
					if(given_options[*itr_s]) *given_options[*itr_s]+=","+*itr;
					else given_options[*itr_s]=new T_s(*itr);
				}
			//	if(itr==args.end()) break;
			}
			delete short_list; short_list=0;
		} else {
			print(); exit(1);
		}
	//	if(itr==args.end()) break;
	}
	
	const T_s* path_ptr=getValue("path");
	path=(path_ptr? *path_ptr : T_s("/spool/users/boroden/current/KdRecTtau"));
	while(path.rbegin()!=path.rend() && *(path.rbegin())=='/') path.erase(path.end()-1);	//patch for tab completion path
}
ProgramOptions::~ProgramOptions(void) {
	for(std::map<std::vector<Option>::const_iterator,T_s*>::const_iterator itr=given_options.begin();itr!=given_options.end();++itr) {
		if(itr->second) delete itr->second;
	}
	given_options.clear();
}

bool ProgramOptions::isGiven(const T_s& str) const {
	bool result=false;
	for(std::map<std::vector<Option>::const_iterator,T_s*>::const_iterator itr=given_options.begin();itr!=given_options.end();++itr) {
		result|=itr->first->hasName(str);
	}
	return result;
}
const T_s* ProgramOptions::getValue(const T_s& str) const {
	for(std::map<std::vector<Option>::const_iterator,T_s*>::const_iterator itr=given_options.begin();itr!=given_options.end();++itr) {
		if(itr->first->hasName(str)) return itr->second;
	}
	return NULL;
}

std::ostream& ProgramOptions::print(std::ostream& output) {
	Option::print_usage(output);
	output<<"Options:\n";
	for(std::vector<Option>::const_iterator itr=options.begin();itr!=options.end();) {
		output<<"  ";
		itr->print(output);
		if(++itr!=options.end()) output<<";\n";
		else output<<".";
	}
	output<<std::endl;
	return output;
}
bool ProgramOptions::addOption(const T_s& opt) {
	T_vs cut_opt;
	IOconverter::tokenize(opt,cut_opt,"= ",true);
	if(cut_opt.empty()) return false;
	std::pair<short int,T_s> dummy=Option::get_option(cut_opt.front());
	if(dummy.first>0) {
		std::vector<ProgramOptions::Option>::const_iterator tmp=find_option(dummy.second);
		if(tmp!=options.end()) {
			if(cut_opt.size()<static_cast<size_t>(tmp->GetFields()+1)) return false;
			if(!given_options.count(tmp)) given_options[tmp]=NULL;
			else given_options[tmp]->clear();
			for(unsigned short int idx=1;idx<=tmp->GetFields();++idx) {
				if(given_options[tmp]) *given_options[tmp]+=","+cut_opt[idx];
				else given_options[tmp]=new T_s(cut_opt[idx]);
			}
		}
		return true;
	}
	return false;
}

std::vector<ProgramOptions::Option> ProgramOptions::generate_options(void) {
	std::vector<Option> result;
	{ Option dummy("exp",	0,"set run status to experiment [default]");dummy.SetShName('e');	result.push_back(dummy); }
	{ Option dummy("sim",	0,"set run status to simulation");			dummy.SetShName('s');	result.push_back(dummy); }
	{ Option dummy("cosmic",0,"set run status to cosmic");										result.push_back(dummy); }
	{ Option dummy("bad",	0,"read and analize bad channels, then go into standard run");		result.push_back(dummy); }
	{ Option dummy("badonly",0,"only read and analize bad channels");	dummy.SetShName('b');	result.push_back(dummy); }
	{ Option dummy("analize",0,"only analize bad channels");			dummy.SetShName('a');	result.push_back(dummy); }
	{ Option dummy("draw",	0,"only draw pictures from existing data");	dummy.SetShName('d');	result.push_back(dummy); }
	{ Option dummy("drawlog",0,"draw pictures from existing log file");							result.push_back(dummy); }
	{ Option dummy("batch",	1,"batch mode: <parts_number>:<range_of_parts>");					result.push_back(dummy); }
	{ Option dummy("run",	1,"overrides run params: <list_of_runs>");	dummy.SetShName('r');	result.push_back(dummy); }
	{ Option dummy("skim",	0,"set screening for skim");										result.push_back(dummy); }
	{ Option dummy("label",	1,"label to append to generated filenames");dummy.SetShName('l');	result.push_back(dummy); }
	{ Option dummy("path",	1,"give the folder to read from and write to");						result.push_back(dummy); }
	{ Option dummy("parselog",1,"parse log.root files inside folders at the path");				result.push_back(dummy); }
	{ Option dummy("remote",0,"indicate that task is run by remote");							result.push_back(dummy); }
	{ Option dummy("help",	0,"print help");							dummy.SetShName('h');	result.push_back(dummy); }
	return result;
}
std::vector<ProgramOptions::Option>::const_iterator ProgramOptions::find_option(const T_s& str) {
	for(std::vector<ProgramOptions::Option>::const_iterator itr=options.begin();itr!=options.end();++itr) {
		if(itr->hasName(str)) return itr;
	}
	return options.end();
}
std::vector<std::vector<ProgramOptions::Option>::const_iterator> ProgramOptions::find_short_options(const T_s& str) {
	std::vector<std::vector<ProgramOptions::Option>::const_iterator> result;
	char cur;
	for(std::size_t pos=0;pos!=str.size();++pos) {
		cur=str[pos];
		for(std::vector<ProgramOptions::Option>::const_iterator itr=options.begin();itr!=options.end();++itr) {
			if(itr->hasShort(cur)) { result.push_back(itr); break; }
		}
	}
	return result;
}

const T_vs ProgramOptions::Option::option_flags=build_vector_from_list<T_s>("--");
const char ProgramOptions::Option::short_option_flag='-';

ProgramOptions::Option::Option(const Option& opt) : name(opt.name),value_fields(opt.value_fields),description(opt.description) {
	if(opt.short_name) short_name=new char(*opt.short_name); else short_name=0;
}
ProgramOptions::Option& ProgramOptions::Option::operator=(Option opt) {
	std::swap(name,opt.name);
	std::swap(value_fields,opt.value_fields);	//lots of copying, but still
	std::swap(description,opt.description);
	std::swap(short_name,opt.short_name);
	return *this;
}

std::pair<short int,T_s> ProgramOptions::Option::get_option(const T_s& str) {
	std::pair<short int,T_s> result;
	for(T_vs::const_iterator itr=option_flags.begin();itr!=option_flags.end();++itr) {
		if(str.find(*itr)==0) {
			result.first=1; result.second=str.substr(itr->size());
			return result;
		}
	}
	if(str.find(short_option_flag)==0) {
		result.first=0; result.second=str.substr(1);
		return result;
	}
	result.first=-1; return result;
}

void ProgramOptions::Option::SetShName(char snm) {
	if(short_name) delete short_name;
	short_name = new char(snm);
}
std::ostream& ProgramOptions::Option::print(std::ostream& output) const {
	output<<name;
	if(short_name) output<<", "<<*short_name;
	output<<"\t("<<value_fields<<" field"
		<<(value_fields!=1 ? "s)" : ") ")
		<<" - "<<description;
	return output;
}
std::ostream& ProgramOptions::Option::print_usage(std::ostream& output) {
	output<<"Usage: "; if(option_flags.size()>1) output<<"{";
	for(T_vs::const_iterator itr=option_flags.begin();itr!=option_flags.end();++itr) {
		if(itr!=option_flags.begin()) output<<"/";
		output<<*itr;
	}	if(option_flags.size()>1) output<<"}";
	output<<"<option_name> [option_values]\n  also short notation when applicable: ";
	output<<short_option_flag<<"<short_option_name> [option_values]"<<std::endl;
	return output;
}

Parameters::Parameters() : value(-1) {
}

bool Parameters::StringExists(const T_s& key) const {
	return strings.count(key);
}
bool Parameters::ListExists(const T_s& key) const {
	return lists.count(key);
}
bool Parameters::CheckStrings(const T_vs& key_list) const {
	bool result=true;
	for(T_vs::const_iterator itr=key_list.begin();itr!=key_list.end();++itr) {
		result&=StringExists(*itr);
	}
	return result;
}
bool Parameters::CheckLists(const T_vs& key_list) const {
	bool result=true;
	for(T_vs::const_iterator itr=key_list.begin();itr!=key_list.end();++itr) {
		result&=ListExists(*itr);
	}
	return result;
}

const T_li* Parameters::GetList(const T_s& key) const {
//	return lists[key];
	T_msli::const_iterator itr=lists.find(key);
	if(itr!=lists.end()) return &itr->second;
	else return NULL;	//empty if not found
}
const T_li* Parameters::SetList(const T_s& key, const T_li& value) {
	return &(lists[key]=value);
}

const T_s* Parameters::GetString(const T_s& key) const {
//	return strings[key];
	T_mss::const_iterator itr=strings.find(key);
	if(itr!=strings.end()) return &itr->second;
	else return NULL;	//empty if not found
}
const T_s* Parameters::SetString(const T_s& key, const T_s& value) {
	return &(strings[key]=value);
}

char Parameters::GetChar(const T_s& key, const std::size_t pos, const char def_value) const {
	const T_s* ret_str=GetString(key);
	if(ret_str) if(ret_str->size() > pos) return ret_str->at(pos);
	return def_value;
}
const T_s* Parameters::SetChar(const T_s& key, const std::size_t pos, const char value, const char def_value) {
	T_s* result=&strings[key];	//shouldn't fail now
	if(result->size() > pos) {
		result->at(pos)=value;
	} else {
		while(result->size()<pos) { result->push_back(def_value); };
		result->push_back(value);
	}
	return result;
}

T_s Parameters::PrintList(const T_s& key) const {
	return IOconverter::print_list(GetList(key));
}

Parameters& Parameters::fill(const T_pmssmsli& dummy) {
	strings.insert(dummy.first.begin(),dummy.first.end());
	lists.insert(dummy.second.begin(),dummy.second.end());
	for(T_mss::iterator itr=strings.begin();itr!=strings.end();) {
		if(itr->first=="value") {	//separation of values (example)
			value=atof(itr->second.c_str());
			strings.erase(itr++);	//map erase invalidates only the erased iterator, next one is ok
		} else {
			++itr;
		}
	}
	return *this;
}

Parameters& Parameters::clear(void) {
	strings.clear();
	lists.clear();
	return *this;
}

std::ostream& operator<< (std::ostream& out, const Parameters& prm) {
//	out<<"Parameters {\n";
	out<<"{\n";
	for(T_mss::const_iterator itr=prm.strings.begin();itr!=prm.strings.end();itr++) {
		out<<"\t"<<itr->first<<"=\""<<itr->second<<"\";\n";
	}
	for(T_msli::const_iterator itr=prm.lists.begin();itr!=prm.lists.end();itr++) {
		out<<"\t"<<itr->first<<"={"<<IOconverter::print_list(&itr->second)<<"};\n";
	}
	out<<"\tvalue="<<prm.value<<";\n";
	out<<"}\n";
	return out;
}

//-----------input_classes-----------
T_msvpdd CrsSlurper::get(const T_s& target) {
	T_msvpdd result; T_vpdd res_second; T_pdd pr;
	T_mss dummy;
	Slurper::get(target,dummy);
	for(T_mss::const_iterator itrd=delimiters.begin();itrd!=delimiters.end();++itrd) {
		if(itrd->first=="={")	//the only valid option
		for(T_mss::const_iterator itr=dummy.begin();itr!=dummy.end();++itr) {
			std::size_t pos=itr->second.find(itrd->first);
			if(pos!=T_s::npos && pos!=itr->second.size()-itrd->first.size()) {
				if(pos!=0) {
					std::cerr<<"CrsSlurper was unable to get values correctly"<<std::endl;
					continue;
				}
				std::size_t tmp_pos=pos+itrd->first.size();
				pos=itr->second.find("}",tmp_pos);
				if(pos!=T_s::npos) {
					T_vs tmp_list;
					IOconverter::tokenize(itr->second.substr(tmp_pos,pos-tmp_pos),tmp_list,",",true);
					tmp_pos=pos+1; pos=itr->second.size();
					double mp=SUconverter::get_multiplier(itr->second.substr(tmp_pos,pos-tmp_pos));
					res_second.clear();
					for(T_vs::const_iterator itr_l=tmp_list.begin();itr_l!=tmp_list.end();++itr_l) {
						pr=IOconverter::extract_pair(*itr_l);
						pr.first=(pr.first==-std::numeric_limits<double>::max() ? pr.first : pr.first*mp);
						pr.second=(pr.second==std::numeric_limits<double>::max() ? pr.second : pr.second*mp);
						res_second.push_back(pr);	//weird limits eliminated
					}
					result[itr->first]=res_second;
				}
			}
		}
	}
	return result;
}

T_pmssmsli PrmSlurper::get(const T_s& target) {
	T_pmssmsli result;
	T_mss dummy;
	Slurper::get(target,dummy);
	for(T_mss::const_iterator itrd=delimiters.begin();itrd!=delimiters.end();++itrd) {
		if(itrd->first=="=" || itrd->first=="=\"" || itrd->first=="={")	//all valid options
		for(T_mss::const_iterator itr=dummy.begin();itr!=dummy.end();++itr) {
			std::size_t pos=itr->second.find(itrd->first);
			if(pos!=T_s::npos && pos!=itr->second.size()-itrd->first.size()) {
				if(pos!=0) {
					std::cerr<<"PrmSlurper was unable to get values correctly"<<std::endl;
					continue;
				}
				if(itrd->first=="=" || itrd->first=="=\"") {	//actual differentiation happens here
					pos+=itrd->first.size();
					result.first[itr->first]=itr->second.substr(pos);
				} else if(itrd->first=="={") {
					std::size_t tmp_pos=pos+itrd->first.size();
					pos=itr->second.find("}",tmp_pos);
					if(pos!=T_s::npos) {
						result.second[itr->first]=IOconverter::extract_list(itr->second.substr(tmp_pos,pos-tmp_pos));
					}
				}
			}
		}
	}
	return result;
}

T_mss CssSlurper::get(const T_s& target) const {
	T_mss result;
	T_mss dummy;
	Slurper::get(target,dummy);
	for(T_mss::const_iterator itr=dummy.begin();itr!=dummy.end();++itr) {
		T_mss::const_iterator dm_status=delimiters.end();
		std::size_t pos1=T_s::npos;
		for(T_mss::const_iterator itrd=delimiters.begin();itrd!=delimiters.end();++itrd) {
			if(itrd->first=="=" || itrd->first=="=\"" || itrd->first=="={"	) {	//all valid options
				std::size_t pos2=itr->second.find(itrd->first);
				if(pos2<pos1) { //assuming that delimiters' firsts are never equal
					pos1=pos2;
					dm_status=itrd;
				} else if(pos1!=T_s::npos && pos1==pos2) {
					if(dm_status->first.size()<itrd->first.size()) {	//the bigger delimiter, the better
						dm_status=itrd;
					}
				}
			}
		}
		if(dm_status==delimiters.end()) continue;	//if it's not valid, it's not added
		if(dm_status->first=="=" || dm_status->first=="=\"") {	//actual differentiation happens here
			pos1+=dm_status->first.size();
			result[itr->first]=itr->second.substr(pos1);
		} else if(dm_status->first=="={") {
			std::size_t tmp_pos=pos1+dm_status->first.size();
			pos1=itr->second.find("}",tmp_pos);
			if(pos1!=T_s::npos) {
				result[itr->first]=itr->second.substr(tmp_pos,pos1-tmp_pos);
			}
		}
	}
	return result;
}
T_ls CssSlurper::get_blocks(const T_s& target) const {
	T_ls result;
	Slurper::get_blocks(result,target);
	return result;
}

void CssSlurper::do_nothing(void) const {
}

void RsSlurper::readout(const T_s& filename) {
	Slurper::readout(filename);
	clear();
	T_mss dummy;
	T_li result_list; Status result;
	std::map<T_li,Status>::iterator current;
	Slurper::get("points",dummy);
	for(T_mss::const_iterator itr=dummy.begin();itr!=dummy.end();++itr) {
		result_list.clear();
		std::size_t pos1, pos2;
		if(( pos1=itr->first.find_first_of('[') )!=T_s::npos)
			result_list=IOconverter::extract_list(
				itr->first.substr(++pos1, pos2=itr->first.find_first_of(']',pos1) )
			);
		current=contents.insert( std::make_pair(result_list,Status()) ).first;	//shouldn't be the same list there already
		if(( pos1=itr->second.find_first_of('=')+1 )!=T_s::npos)
			current->second.energy=atof( itr->second.substr(pos1).c_str() );
		
		T_mss temp;
		Slurper::get(itr->first,temp);
		for(T_mss::const_iterator itr_p=temp.begin();itr_p!=temp.end();++itr_p) {
			T_mss::const_iterator dm_status=delimiters.end();
			pos1=T_s::npos;
			for(T_mss::const_iterator itrd=delimiters.begin();itrd!=delimiters.end();++itrd) {
				if(itrd->first=="=" || itrd->first=="=\"" || itrd->first=="={"	) {	//all valid options
					std::size_t pos2=itr_p->second.find(itrd->first);
					if(pos2<pos1) { //assuming that delimiters' firsts are never equal
						pos1=pos2;
						dm_status=itrd;
					} else if(pos1!=T_s::npos && pos1==pos2) {
						if(dm_status->first.size()<itrd->first.size()) {	//the bigger delimiter, the better
							dm_status=itrd;
						}
					}
				}
			}
			if(dm_status==delimiters.end()) continue;	//if it's not valid, it's not added
			if(dm_status->first=="=" || dm_status->first=="=\"") {	//actual differentiation happens here
				pos1+=dm_status->first.size();	//nothing here for now
			//	result[itr_p->first]=itr_p->second.substr(pos1);
			} else if(dm_status->first=="={") {
				pos1+=dm_status->first.size();
				pos2=itr_p->second.find("}",pos1);
				if(pos1!=T_s::npos) {
					T_li tmp_list=IOconverter::extract_list( itr_p->second.substr(pos1,pos2-pos1) );
					if(itr_p->first=="LKr.ring") current->second.LKr_ring=tmp_list;
					else if(itr_p->first=="LKr.bad") current->second.LKr_bad=tmp_list;
					else if(itr_p->first=="CsI.ring") current->second.CsI_ring=tmp_list;
					else if(itr_p->first=="CsI.bad") current->second.CsI_bad=tmp_list;
				}
			}
		}
	}
}

void RsSlurper::print(std::ostream& output) const {
	output<<"/*/\nContains energy points and broken channels. Rewritten by DataAnalizer.\n/*/\n";
	output<<"points{\n";
	for(std::map<T_li,Status>::const_iterator itr=contents.begin();itr!=contents.end();++itr) {
		output<<"\trun["<<IOconverter::print_list(&itr->first)<<"]="<<itr->second.energy<<";\n";
	}	output<<"}\n\n";
	for(std::map<T_li,Status>::const_iterator itr=contents.begin();itr!=contents.end();++itr) {
		output<<"run["<<IOconverter::print_list(&itr->first)<<"] {\n"
			<<"\tLKr{\n"
			<<"\t\tring={"<<IOconverter::print_list(&itr->second.LKr_ring)<<"};\n"
			<<"\t\tbad={"<<IOconverter::print_list(&itr->second.LKr_bad)<<"};\n"
			<<"\t}\n\tCsI{\n"
			<<"\t\tring={"<<IOconverter::print_list(&itr->second.CsI_ring)<<"};\n"
			<<"\t\tbad={"<<IOconverter::print_list(&itr->second.CsI_bad)<<"};\n"
			<<"\t}\n}\n";
	}
}

void LumReader::read(const T_s& from) {
	std::ifstream file((path+"/input/"+from).c_str());
	int nrun; double lum;
	while(file>>nrun>>lum) {	//returns reference to a file which converted to bool gives stream fail state
		data[nrun]=lum;
	}
	file.close();
}
void LumReader::write(const T_s& where) const {
	std::ofstream file((path+"/input/"+where).c_str(),std::ios_base::trunc);
	file<<std::setprecision(8)<<std::fixed;		//to match default printout
	for(std::map<int,double>::const_iterator itr=data.begin();itr!=data.end();++itr) {
		file<<itr->first<<'\t'<<itr->second<<'\n';
	}
	file.close();
}

std::map<int,double> LumReader::get_multipliers(const T_li& run_list, TRandom3& root_rand) const {
	std::map<int,double> result;
	double sum=0.,interval=0.;
	std::map<double,int> assigner;
	std::map<int,double>::const_iterator data_itr;
	for(T_li::const_iterator itr=run_list.begin();itr!=run_list.end();++itr) if((data_itr=data.find(*itr))!=data.end()) {
		sum+=data_itr->second;
	}
	for(T_li::const_iterator itr=run_list.begin();itr!=run_list.end();++itr) if((data_itr=data.find(*itr))!=data.end()) {
		assigner[interval+=data_itr->second/sum]=*itr;
		result[*itr]=0.;	//for safety
	}
	double grain=0., total=0.;
	while(total<10000.) {
		result[assigner.lower_bound(root_rand.Rndm())->second]+=grain=root_rand.Rndm();
		total+=grain;
	}
	for(std::map<int,double>::iterator itr=result.begin();itr!=result.end();++itr) {
		itr->second/=total;
	}
/*/----old----//
	for(std::map<int,double>::const_iterator itr=data.begin();itr!=data.end();++itr) {
		sum+=itr->second;
	}
	if(!sum) return result;
	for(std::map<int,double>::const_iterator itr=data.begin();itr!=data.end();++itr) {
		assigner[interval+=itr->second/sum]=itr->first;
		result[itr->first]=0.;	//for safety
	}
	//int grain=18, total=0;
	double grain=0., total=0.;
	while(total<10000.) {
		result[assigner.lower_bound(root_rand.Rndm())->second]+=grain=root_rand.Rndm();
		total+=grain;
	}
	for(std::map<int,double>::iterator itr=result.begin();itr!=result.end();++itr) {
		itr->second/=total;
	}
//----old----/*/
	return result;
}

LogRoot::LogRoot(const T_s& path, const T_s& logname, const T_s& label) : data_itr(0), base_name(logname) {
	name=path+"/"+base_name+".root";
	
	if(label.empty()) {
		data=0;
	} else if(!SetLabel(label)) {
		init(label);
	}
}
LogRoot::~LogRoot(void) {
	if(data_itr) delete data_itr;
	if(data) {/* data->Print();*/ delete data; }
}
void LogRoot::init(const T_s& label) {
	data = new TList();
	data_itr=new TIter(data);
	cur_label=label;
}

bool LogRoot::SetLabel(const T_s& label) {
	TList* data_tmp=0;
	TFile* file = new TFile(name.c_str(),"update","log file");
//	if(file) data_tmp=dynamic_cast<TList*>(file->Get(label.c_str()));
//	if(!file->IsZombie()) file->GetObject(label.c_str(),data_tmp);
	if(!file->IsZombie()) data_tmp=dynamic_cast<TList*>(file->GetListOfKeys()->FindObject(label.c_str()));
	delete file;
	if(data_tmp) {
		if(data_itr) delete data_itr;
		if(data) delete data;
		cur_label=label;
		data=data_tmp; data_itr=new TIter(data);
		return true;
	} else {
		return false;
	}
}

bool LogRoot::Write(void) const {
	TDirectory* old_dir=gDirectory->GetDirectory(0);	//current directory
	TFile* file = new TFile(name.c_str(),"update","log file");

	TObject* to_del=file->GetListOfKeys()->FindObject(cur_label.c_str());	//no TFile::Remove in 5.14
	if(to_del) { file->GetListOfKeys()->Remove(to_del); delete to_del; }
	//	file->cd(); gDirectory->Delete((cur_label+";*").c_str());	//another version (don't call file->Delete())

	bool result=(file->cd())&&(data->Write(cur_label.c_str(),TObject::kSingleKey)!=0);
	if(!old_dir->cd()) std::cerr<<"LogRoot: failed to return to current dir."<<std::endl;
	file->Purge();	//to remove old cycles
//	file->Save();
	delete file;
	
	return result;
}
void LogRoot::Clear(void) {
	TDirectory* old_dir=gDirectory->GetDirectory(0);	//current directory
	TFile* file = new TFile(name.c_str(),"update","log file");
	file->Clear();
	delete file;
	if(!old_dir->cd()) std::cerr<<"LogRoot: failed to return to current dir."<<std::endl;
}

void LogRoot::Purge(void) {
	TDirectory* old_dir=gDirectory->GetDirectory(0);	//current directory
	TFile* file = new TFile(name.c_str(),"update","log file");
	file->Purge();
	TList* dummy_lst=new TList();
	TIter dummy_itr(file->GetListOfKeys());
	TObject* tmp; TKey* tmp_key;
	while( (tmp_key=dynamic_cast<TKey*>(dummy_itr.Next())) ) {
		tmp=tmp_key->ReadObj();	//to actually read object from file
		if(!tmp) continue;
		if(tmp->InheritsFrom(TCollection::Class())) {
			TList* tl_tmp=dynamic_cast<TList*>(tmp);
			if(tl_tmp) {
				tl_tmp->SetName(tmp_key->GetName());
				dummy_lst->Add(tl_tmp);
			}
		} else {
			dummy_lst->Add(tmp);	//name will be just class name
		}
	}
	delete file;
	
	file = new TFile(name.c_str(),"recreate","log file");
	dummy_itr=TIter(dummy_lst);
	file->cd();
	while( (tmp=dummy_itr.Next()) ) {
		if(tmp->InheritsFrom(TCollection::Class())) {
			TList* tl_tmp=dynamic_cast<TList*>(tmp);
			if(tl_tmp) tl_tmp->Write(tmp->GetName(),TObject::kSingleKey);
		} else {
			tmp->Write(tmp->GetName());
		}
	}
	delete file;
	delete dummy_lst;
	if(!old_dir->cd()) std::cerr<<"LogRoot: failed to return to current dir."<<std::endl;
}

void LogRoot::ParseLogs(const T_s& ext_path) {
	T_s path=ext_path;
	while(path.rbegin()!=path.rend() && *(path.rbegin())=='/') path.erase(path.end()-1);
	path+="/";
	
	T_ls dir_list=IOconverter::list_directories(path);
T_s workdir=gSystem->pwd();
	std::ostringstream exp_buf, sim_buf;
	for(T_ls::const_iterator itr=dir_list.begin();itr!=dir_list.end();++itr) {
		if(*itr==T_s(".") || *itr==T_s("..")) continue;	//ignore current and up
		std::cout<<"Opened \""<<*itr<<"\" to search for log.root"<<std::endl;
		gSystem->cd((path+*itr).c_str());
		TFile* file=new TFile("log.root","read");
	if(file->IsZombie()) { std::cerr<<"\tcould not find log.root"<<std::endl;
	} else {
		T_ls tmp_lst_exp=IOconverter::list_files(gSystem->pwd(),".root","data");
		T_ls tmp_lst_sim=IOconverter::list_files(gSystem->pwd(),"_sim.root","data");
		for(T_ls::iterator itr_sim=tmp_lst_sim.begin();itr_sim!=tmp_lst_sim.end();++itr_sim) {
			for(T_ls::iterator itr_exp=tmp_lst_exp.begin();itr_exp!=tmp_lst_exp.end();) {
				if(*itr_sim==*itr_exp) {
					itr_exp=tmp_lst_exp.erase(itr_exp);
				} else {
					*itr_exp=itr_exp->substr(0,itr_exp->find(".root"));		//bugs out with cint
					++itr_exp;
				}
			}
			*itr_sim=itr_sim->substr(0,itr_sim->find(".root"));
		}
		for(T_ls::const_iterator itr_exp=tmp_lst_exp.begin();itr_exp!=tmp_lst_exp.end();++itr_exp) {
			TList* exp_data=dynamic_cast<TList*>(file->Get( (T_s("hp_")+*itr_exp).c_str() ));
			TH1D* LKr_energy_exp=0, *CsI_energy_exp=0, *CsI1_exp=0, *CsI2_exp=0;
			TH1D* LKr_Eexp_new=0, *CsI_Eexp_new=0, *CsI1_exp_new=0, *CsI2_exp_new=0;
			if(exp_data) {
				LKr_energy_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy1"));
				CsI_energy_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_energy.h.energy2"));
				CsI1_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.up_cluster"));
				CsI2_exp=dynamic_cast<TH1D*>(exp_data->FindObject("cluster_updown.h.down_cluster"));
				
				LKr_Eexp_new=dynamic_cast<TH1D*>(exp_data->FindObject("_ac_cluster_energy.h.energy1"));
				CsI_Eexp_new=dynamic_cast<TH1D*>(exp_data->FindObject("_ac_cluster_energy.h.energy2"));
				CsI1_exp_new=dynamic_cast<TH1D*>(exp_data->FindObject("_ac_cluster_updown.h.up_cluster"));
				CsI2_exp_new=dynamic_cast<TH1D*>(exp_data->FindObject("_ac_cluster_updown.h.down_cluster"));
			}
			exp_buf<<*itr<<"\t\t"<<(LKr_energy_exp?LKr_energy_exp->GetEntries():0)<<"\t"
				<<(CsI_energy_exp?CsI_energy_exp->GetEntries():0)<<"\t"
				<<(CsI1_exp?CsI1_exp->GetEntries():0)<<"\t"<<(CsI2_exp?CsI2_exp->GetEntries():0)<<"\n";
			exp_buf<<*itr<<"_new\t\t"<<(LKr_Eexp_new?LKr_Eexp_new->GetEntries():0)<<"\t"
				<<(CsI_Eexp_new?CsI_Eexp_new->GetEntries():0)<<"\t"
				<<(CsI1_exp_new?CsI1_exp_new->GetEntries():0)<<"\t"<<(CsI2_exp_new?CsI2_exp_new->GetEntries():0)<<"\n";
		}
		for(T_ls::const_iterator itr_sim=tmp_lst_sim.begin();itr_sim!=tmp_lst_sim.end();++itr_sim) {
			TList* sim_data=dynamic_cast<TList*>(file->Get( (T_s("hp_")+*itr_sim).c_str() ));
			TH1D* LKr_energy_sim=0, *CsI_energy_sim=0, *CsI1_sim=0, *CsI2_sim=0;
			TH1D* LKr_Esim_new=0, *CsI_Esim_new=0, *CsI1_sim_new=0, *CsI2_sim_new=0;
			if(sim_data) {
				LKr_energy_sim=dynamic_cast<TH1D*>(sim_data->FindObject("cluster_energy.h.energy1"));
				CsI_energy_sim=dynamic_cast<TH1D*>(sim_data->FindObject("cluster_energy.h.energy2"));
				CsI1_sim=dynamic_cast<TH1D*>(sim_data->FindObject("cluster_updown.h.up_cluster"));
				CsI2_sim=dynamic_cast<TH1D*>(sim_data->FindObject("cluster_updown.h.down_cluster"));
				
				LKr_Esim_new=dynamic_cast<TH1D*>(sim_data->FindObject("_ac_cluster_energy.h.energy1"));
				CsI_Esim_new=dynamic_cast<TH1D*>(sim_data->FindObject("_ac_cluster_energy.h.energy2"));
				CsI1_sim_new=dynamic_cast<TH1D*>(sim_data->FindObject("_ac_cluster_updown.h.up_cluster"));
				CsI2_sim_new=dynamic_cast<TH1D*>(sim_data->FindObject("_ac_cluster_updown.h.down_cluster"));
			}
			sim_buf<<*itr<<"\t\t"<<(LKr_energy_sim?LKr_energy_sim->GetEntries():0)<<"\t"
				<<(CsI_energy_sim?CsI_energy_sim->GetEntries():0)<<"\t"
				<<(CsI1_sim?CsI1_sim->GetEntries():0)<<"\t"<<(CsI2_sim?CsI2_sim->GetEntries():0)<<"\n";
			sim_buf<<*itr<<"_new\t\t"<<(LKr_Esim_new?LKr_Esim_new->GetEntries():0)<<"\t"
				<<(CsI_Esim_new?CsI_Esim_new->GetEntries():0)<<"\t"
				<<(CsI1_sim_new?CsI1_sim_new->GetEntries():0)<<"\t"<<(CsI2_sim_new?CsI2_sim_new->GetEntries():0)<<"\n";
		}
	}
		if(file->IsOpen()) file->Close();
		delete file;
	}
	std::ofstream result_exp((path+"parsed_entries_exp").c_str(),std::ofstream::trunc);
	result_exp.write(exp_buf.str().c_str(),exp_buf.str().size());
	result_exp.close();
	std::ofstream result_sim((path+"parsed_entries_sim").c_str(),std::ofstream::trunc);
	result_sim.write(sim_buf.str().c_str(),sim_buf.str().size());
	result_sim.close();
gSystem->cd(workdir.c_str());

	{	T_s exp_name, sim_name; T_vs names_exp, names_sim;
		int lkr_exp,csi_exp,csi1_exp,csi2_exp; T_vi LKr_exp, CsI_exp, CsI1_exp, CsI2_exp;
		int lkr_sim,csi_sim,csi1_sim,csi2_sim; T_vi LKr_sim, CsI_sim, CsI1_sim, CsI2_sim;
		
		std::ifstream exp_num((path+"parsed_entries_exp").c_str());
		while(exp_num>>exp_name>>lkr_exp>>csi_exp>>csi1_exp>>csi2_exp) {
			names_exp.push_back(exp_name);
			LKr_exp.push_back(lkr_exp); CsI_exp.push_back(csi_exp);
			CsI1_exp.push_back(csi1_exp); CsI2_exp.push_back(csi2_exp);
		}
		std::ifstream sim_num((path+"parsed_entries_sim").c_str());
		while(sim_num>>sim_name>>lkr_sim>>csi_sim>>csi1_sim>>csi2_sim) {
			names_sim.push_back(sim_name);
			LKr_sim.push_back(lkr_sim); CsI_sim.push_back(csi_sim);
			CsI1_sim.push_back(csi1_sim); CsI2_sim.push_back(csi2_sim);
		}
		std::cout<<"Dividing values: vector sizes "
		<<names_exp.size()<<" "<<LKr_exp.size()<<" "<<CsI_exp.size()<<" "<<CsI1_exp.size()<<" "<<CsI2_exp.size()
		<<" and "<<names_sim.size()<<" "<<LKr_sim.size()<<" "<<CsI_sim.size()<<" "<<CsI1_sim.size()<<" "<<CsI2_sim.size()
		<<std::endl;
		
		T_vd res_LKr, res_CsI, res_CsI1, res_CsI2;
		std::ofstream result_div((path+"entries_div").c_str(),std::ofstream::trunc);
		for(T_vs::const_iterator itr=names_exp.begin();itr!=names_exp.end();++itr) {
			T_vs::const_iterator itr2=std::find(names_sim.begin(),names_sim.end(),*itr);
			if(itr2!=names_sim.end()) {
				unsigned int pos1=static_cast<unsigned int>(itr-names_exp.begin());
				unsigned int pos2=static_cast<unsigned int>(itr2-names_sim.begin());
				res_LKr.push_back( (LKr_sim[pos2] ? static_cast<double>(LKr_exp[pos1])/LKr_sim[pos2] : -1.) );
				res_CsI.push_back( (CsI_sim[pos2] ? static_cast<double>(CsI_exp[pos1])/CsI_sim[pos2] : -1.) );
				res_CsI1.push_back( (CsI1_sim[pos2] ? static_cast<double>(CsI1_exp[pos1])/CsI1_sim[pos2] : -1.) );
				res_CsI2.push_back( (CsI2_sim[pos2] ? static_cast<double>(CsI2_exp[pos1])/CsI2_sim[pos2] : -1.) );
				result_div<<*itr<<"\t\t"<<res_LKr.back()<<"\t"<<res_CsI.back()<<"\t"
					<<res_CsI1.back()<<"\t"<<res_CsI2.back()<<"\n";
			}
		}
		result_div.close();
	}
}

