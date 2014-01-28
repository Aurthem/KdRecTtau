
#include "slurper.h"

//---------------
//----Slurper----
//---------------
const T_mss Slurper::delimiters=build_map_from_list<T_s,T_s>("={",";")("=",";")("=\"","\";");
//delimiters' firsts should not be the same
const T_mss Slurper::comments=build_map_from_list<T_s,T_s>("//","\n")("/*","*/");
const T_mss Slurper::strings=build_map_from_list<T_s,T_s>("\"","\"")("'","'");
const T_s Slurper::whitespace=" \t\n";
const T_pss Slurper::block_delimiters=T_pss("{","}");

void Slurper::readout(const T_s& filename) {
	blocks.clear();	connections.clear();
	
	T_mss::const_iterator cm_status=comments.end(), str_status=strings.end();
	std::ifstream file(filename.c_str());
	T_s buf, contents;
	if(file.good()) while(!file.eof()) {
		std::getline(file,buf);
		buf.push_back('\n');
		contents+=cut(buf,cm_status,str_status);	//call cut() consecutively and once
	} else {
		std::cerr<<"Couldn't open file \""<<filename
			<<"\" correctly, contents will be blank."<<std::endl;
	}
	file.close();
	
//std::cout<<"Parsing contents:\n"<<contents<<std::endl;
	parse(contents);
//std::cout<<"Result:\n"; print(); std::cout<<std::endl;
	if(connections.merge(blocks)) {
		std::cerr<<"Slurper detected blocks with same names in same scopes. They were merged."<<std::endl;
	}
	if(!connections.unique()) {	//probably sort is needed as well for it to trigger
		std::cerr<<"Warning! Slurper found errors in inner logic. "
			<<"They were deleted, but don't expect anything to work correctly."<<std::endl;
	}
//std::cout<<"Result:\n"; print(); std::cout<<std::endl;
}
/*/---------//
void Slurper::AddBlock(const T_s& block_name, const T_mss& block_contents) {
	T_ls tmp;
	T_s name=connections.GetSimpleName(block_name,tmp);
	std::multimap<T_s,T_mss>::iterator itr_end=blocks.insert(std::make_pair(name,block_contents));
	const std::multimap<T_s,T_mss>::iterator* previous=NULL;
	for(T_ls::const_iterator itr=tmp.begin();itr!=tmp.end();++itr) {
		std::multimap<T_s,T_mss>::iterator itr_m=blocks.insert(std::make_pair(*itr,T_mss()));
		previous=connections.AddChild(previous,&itr_m);	//returns const pointer to the actual, copied into child, iterator
	}
	connections.AddChild(previous,&itr_end);
}
//---------/*/
void Slurper::get(const T_s& name, T_mss& fields) const {
	fields.clear();
	connections.FillFields(name,fields);
}
void Slurper::get_blocks(T_ls& names, const T_s& parent) const {
	names.clear();
	connections.FillBlockNames(parent,names);
}

T_s& Slurper::cut(T_s& result, T_mss::const_iterator& cm_status, T_mss::const_iterator& str_status) {
	std::size_t pos=0, pos1, pos2, pos3;	//positions in the string
//std::cout<<"Cut:\""<<result<<"\""<<std::endl;
	while(pos!=T_s::npos) {
		if(cm_status==comments.end() && str_status==strings.end()) {
			pos1=find_first_in(result,pos,comments,cm_status);
			pos2=find_first_in(result,pos,strings,str_status);
			pos3=result.find_first_of(whitespace,pos);
			if(pos3<pos1 && pos3<pos2) {	//whitespaces
				pos=pos3;
				result.erase(pos,result.find_first_not_of(whitespace,pos)-pos);
				cm_status=comments.end(); str_status=strings.end();
//std::cout<<"\terased whitespaces"<<std::endl;
			} else if(pos2<pos1) {			//strings
				pos=pos2+(str_status->first).size();	//so that first won't interfere
				cm_status=comments.end();
//std::cout<<"\tfound strings: \""<<str_status->first<<"\""<<std::endl;
			} else if(pos1!=T_s::npos) {	//comments
				pos=pos1;
				result.erase(pos,cm_status->first.size());		//so that first won't interfere
				str_status=strings.end();
//std::cout<<"\tfound comments: \""<<cm_status->first<<"\""<<std::endl;
			} else {						//nothing
				pos=T_s::npos;
//std::cout<<"\tfound nothing"<<std::endl;
			}
		} else if(str_status!=strings.end()) {
			pos2=result.find(str_status->second,pos);
			if(pos2!=T_s::npos) {
				pos=pos2+(cm_status->second).size();
				str_status=strings.end();
//std::cout<<"\t  finished string"<<std::endl;
			} else {
				pos=T_s::npos;	//ignored all
//std::cout<<"\t  continuing string"<<std::endl;
			}
		} else {	//cm_status!=comments.end()
			pos1=result.find(cm_status->second,pos);
			if(pos1!=T_s::npos) {
				result.erase(pos,pos1-pos+(cm_status->second).size());
				cm_status=comments.end();
//std::cout<<"\t  finished comment"<<std::endl;
			} else {
				result.erase(pos);
				pos=T_s::npos;	//erased all
//std::cout<<"\t  continuing comment"<<std::endl;
			}
		}
	}
//std::cout<<"Result:\""<<result<<"\""<<std::endl;
	return result;
}

std::size_t Slurper::skip_to_unopened(const T_s& str, const std::size_t pos, const T_pss& pr) {
	std::size_t pos1=pos, pos2, pos3;
	while(pos1!=T_s::npos) {
		pos2=str.find(pr.first,pos1);
		pos3=str.find(pr.second,pos1);
		if(pos3<pos2) {
			return pos3;
		} else if(pos2==T_s::npos || pos2==str.size()-pr.first.size()) {
			//if(first.find(second)==0) first takes precedence
			if(pr.first==pr.second) return pos3;
			else return T_s::npos;
		} else {
			pos3=skip_to_unopened(str,pos2+pr.first.size(),pr);
			if(pos3==T_s::npos || pos3==str.size()-pr.second.size()) {
				return T_s::npos;
			} else {
				pos1=pos3+pr.second.size();
			}
		}
	}
	return pos1;
}

std::size_t Slurper::find_first_in(const T_s& str, const std::size_t at_pos,
  const T_mss& from_this, T_mss::const_iterator& what_found) {
	std::size_t result=T_s::npos, tmp;
	for(T_mss::const_iterator itr=from_this.begin();itr!=from_this.end();itr++) {
		tmp=str.find(itr->first,at_pos);
		if(tmp<result) {
			result=tmp;
			what_found=itr;
		}
	}
	return result;
}

void Slurper::parse(const T_s& from, const std::multimap<T_s,T_mss>::iterator* parent) {
	T_mss::const_iterator dm_status=delimiters.end(); //delimiter status
	std::size_t pos=0, pos1, pos2, pos3;
	while(pos!=T_s::npos && pos!=from.size()) {
//std::cout<<"Position="<<pos<<"; size="<<from.size()<<std::endl;
//std::cout<<"\tparsing step:\n"<<from.substr(pos)<<std::endl;
		dm_status=delimiters.end();
		pos1=T_s::npos; pos2=T_s::npos;
		for(T_mss::const_iterator itr=delimiters.begin();itr!=delimiters.end();++itr) {
			pos2=from.find(itr->first,pos);
			if(pos2<pos1) { //assuming that delimiters' firsts are never equal
				pos1=pos2;
				dm_status=itr;
			} else if(pos1!=T_s::npos && pos1==pos2) {
				if(dm_status->first.size()<itr->first.size()) {	//the bigger delimiter, the better
					dm_status=itr;
				}
			}
		}
		pos3=from.find(block_delimiters.first,pos);
		if(pos3<pos1) {	//subblock was found, parse it recursively
			T_s block_name=from.substr(pos,pos3-pos);
//std::cout<<"\tparsing new block:\n"<<block_name<<std::endl;
			if(pos3==from.size()-block_delimiters.first.size()) {
				std::multimap<T_s,T_mss>::iterator child=blocks.insert(std::make_pair(block_name,T_mss()));
//std::cout<<"inserted \""<<block_name<<"\"=\"\""<<std::endl;
				connections.AddChild(parent,&child);
				return;	//pos=T_s::npos;
			} else {
				pos2=skip_to_unopened(from,pos1=pos3+block_delimiters.first.size(),block_delimiters);
//std::cout<<block_name<<": skip_to_unopened returned ";
//if(pos2==T_s::npos) std::cout<<"npos"; else std::cout<<pos2;
//std::cout<<" from "<<pos1<<std::endl;
				std::multimap<T_s,T_mss>::iterator child=blocks.insert(std::make_pair(block_name,T_mss()));
//std::cout<<"inserted \""<<block_name<<"\"=\"\""<<"; size="<<child->second.size()<<std::endl;
				parse(from.substr(pos1,pos2-pos1),connections.AddChild(parent,&child));
				if(pos2==T_s::npos || pos2==from.size()-block_delimiters.second.size()) {
					return;	//pos=T_s::npos;
				} else {
					pos=pos2+block_delimiters.second.size();
//std::cout<<"added position="<<pos<<" from "<<pos2<<std::endl;
				}
			}
			continue;
		}
		if(parent && dm_status==delimiters.end()) {	//none found
			(*parent)->second.insert( std::make_pair(from.substr(pos),T_s()) );	//allowed empty values
//std::cout<<"inserted to parent: \""<<from.substr(pos)<<"\"=\"\""<<std::endl;
//std::cout<<"Result:"<<std::endl; print((*parent)->first);
			return;
		}
		if(parent && pos1!=pos && pos1!=from.size()-dm_status->first.size()) {	//empty keys are ignored
			T_s key=from.substr(pos,pos1-pos);
			pos2=from.find(dm_status->second,pos1+dm_status->first.size()); //allow first delimiter to remain in value
			if(pos2!=T_s::npos) {
				(*parent)->second.insert( std::make_pair(key,from.substr(pos1,pos2-pos1)) );
//std::cout<<"inserted to parent: \""<<key<<"\"=\""<<from.substr(pos1,pos2-pos1)<<"\""<<std::endl;
				pos=pos2+dm_status->second.size();
			} else {
				(*parent)->second.insert( std::make_pair(key,from.substr(pos1)) );
//std::cout<<"Result:"<<std::endl; print((*parent)->first);
				return;	//pos=T_s::npos;
			}
		} else if(parent && pos1==from.size()-dm_status->first.size()) {
			(*parent)->second.insert( std::make_pair(from.substr(pos),T_s()) );	//allowed empty values
//std::cout<<"Result:"<<std::endl; print((*parent)->first);
			return;	//pos=T_s::npos;
		} else {
			pos=pos1+dm_status->first.size(); //if pos==pos1 move forward
//std::cout<<"added position="<<pos<<" from "<<pos1<<std::endl;
		}
	}
//std::cout<<"Result:"<<std::endl; if(parent) print((*parent)->first); else std::cout<<"No parent"<<std::endl;
}

//-------------------
//----Connections----
//-------------------
void Slurper::Connections::clear(void) {
	for(std::list<tree_node*>::iterator itr=heads.begin();itr!=heads.end();++itr) {
		delete *itr;
	}
	heads.clear();
	current=NULL;
}
void Slurper::Connections::print(const T_s& str, std::ostream& output) const {
	if(str.empty()) {
		for(std::list<tree_node*>::const_iterator itr=heads.begin();itr!=heads.end();++itr) {
			(*itr)->print(0,output);
		}
	} else {
		tree_node* tmp=find_name(str);
		if(tmp) tmp->print(0,output);
	}
}

bool Slurper::Connections::unique(void) {	//look at boost for 'for'->'for_each' conversion
	bool result=true;
	for(std::list<tree_node*>::iterator itr=heads.begin();itr!=heads.end();++itr) {
		result&=(*itr)->unique();
	}
	for(std::list<tree_node*>::iterator itr=heads.begin();itr!=heads.end();++itr) {
		std::list<tree_node*>::iterator itr2=itr;
		while(++itr2!=heads.end()) {
			result&=(*itr)->unique_values(*itr2);
		}
	}
	return result;
}

bool Slurper::Connections::merge(std::multimap<T_s,T_mss>& blocks) {
	bool result=false;
	for(std::list<tree_node*>::iterator itr=heads.begin();itr!=heads.end();++itr) {
		std::list<tree_node*>::iterator itr2=itr;
		for(++itr2;itr2!=heads.end();) {
			if((*itr)->get()->first == (*itr2)->get()->first) {
				result=true;
				(*itr)->get()->second.insert((*itr2)->get()->second.begin(),(*itr2)->get()->second.end());
				blocks.erase((*itr2)->get());
				(*itr)->merge_with(*itr2);
				itr2=heads.erase(itr2);
			} else {
				++itr2;
			}
		}
	}
	for(std::list<tree_node*>::iterator itr=heads.begin();itr!=heads.end();++itr) {
		result|=(*itr)->merge(blocks);
	}
	return result;
}

const std::multimap<T_s,T_mss>::iterator* Slurper::Connections::AddChild(
  const std::multimap<T_s,T_mss>::iterator* parent, const std::multimap<T_s,T_mss>::iterator* child) {
	if(parent) {
		current=find(*parent);
		if(!current) {
			heads.push_back(current = new tree_node(*parent));
		}
		if(child) {
			tree_node* tmp = new tree_node(*child);
			current->add_child(tmp);
			return &(tmp->get());
		} else {
			return &(current->get());
		}
	} else if(child) {
		heads.push_back(current = new tree_node(*child));
		return &(current->get());
	} else {
		return NULL;
	}
}
T_s Slurper::Connections::GetSimpleName(const T_s& name, T_ls& residue) {
	tree_node::cut_name(name,residue);
	T_s result=residue.back();	//should never be empty (if name's empty, it's still added to the list)
	residue.pop_back();
	return result;
}

void Slurper::Connections::FillFields(const T_s& full_name, T_mss& fields) const {
	tree_node* tmp=NULL;
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		if( (tmp=(*itr_h)->find_chain(full_name)) ) break;
	}
	if(!tmp) return;
	tmp->fill_fields(fields);
}
void Slurper::Connections::FillBlockNames(const T_s& full_parent_name, T_ls& children_names) const {
	if(full_parent_name.empty()) {
		for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
			children_names.push_back( (*itr_h)->get()->first );
		}
		return;
	}
	tree_node* tmp=NULL;
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		if( (tmp=(*itr_h)->find_chain(full_parent_name)) ) break;
	}
//std::cout<<"Searched for chain \""<<full_parent_name<<"\" and found = "<<tmp<<std::endl;
	if(!tmp) return;
	tmp->fill_children_names(children_names);
}

Slurper::Connections::tree_node* Slurper::Connections::find(std::multimap<T_s,T_mss>::iterator itr) const {
	if(current->equal(itr)) return current;
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		if((*itr_h)->equal(itr)) return current=*itr_h;
	}
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		tree_node* tmp=(*itr_h)->find(itr);
		if(tmp!=NULL) return current=tmp;
	}
	return NULL;
}
Slurper::Connections::tree_node* Slurper::Connections::find_name(const T_s& name) const {
	if(current->equal(name)) return current;
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		if((*itr_h)->equal(name)) return current=*itr_h;
	}
	for(std::list<tree_node*>::const_iterator itr_h=heads.begin();itr_h!=heads.end();++itr_h) {
		tree_node* tmp=(*itr_h)->find_name(name);
		if(tmp!=NULL) return current=tmp;
	}
	return NULL;
}

T_s Slurper::Connections::GetName(std::multimap<T_s,T_mss>::iterator itr) const {
	T_s result=(current=find(itr))->get()->first;
	current->append_ancestors(result);
	return result;
}

//-----------------
//----tree_node----
//-----------------
Slurper::Connections::tree_node::~tree_node(void) {
	for(std::list<tree_node*>::iterator itr=this->children.begin();itr!=this->children.end();++itr) {
		delete *itr;	// recursion through destructor calls
	}
	this->children.clear();
}

Slurper::Connections::tree_node* Slurper::Connections::tree_node::find(std::multimap<T_s,T_mss>::iterator itr) const {
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		if((*itr_b)->equal(itr)) return *itr_b;
	}	//first we search on one level, and only then go deeper
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		tree_node* tmp=(*itr_b)->find(itr);
		if(tmp!=NULL) return tmp;
	}
	return NULL;
}
Slurper::Connections::tree_node* Slurper::Connections::tree_node::find_name(const T_s& name) const {
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		if((*itr_b)->equal(name)) return *itr_b;
	}	//first we search on one level, and only then go deeper
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		tree_node* tmp=(*itr_b)->find_name(name);
		if(tmp!=NULL) return tmp;
	}
	return NULL;
}
void Slurper::Connections::tree_node::find_names(const T_s& name, std::list<tree_node*>& result) const {
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		if((*itr_b)->equal(name)) result.push_back(*itr_b);
	}	//first we search on one level, and only then go deeper
	for(std::list<tree_node*>::const_iterator itr_b=this->children.begin();itr_b!=this->children.end();++itr_b) {
		(*itr_b)->find_names(name,result);
	}
}

Slurper::Connections::tree_node* Slurper::Connections::tree_node::find_chain(const T_s& full_name) {
	T_ls chain;
	tree_node::cut_name(full_name,chain);
//std::cout<<"tree_node: cut_name \""<<full_name<<"\" gave size="<<chain.size()<<std::endl;
	if(chain.empty()) return NULL;

	std::list<tree_node*> name_list;
	T_ls::const_iterator itr_c=chain.begin();
	if(this->equal(*itr_c)) name_list.push_back(this);
	find_names(*(itr_c++),name_list);
//std::cout<<"tree_node: find_names gave size="<<name_list.size()<<std::endl;
	if(name_list.empty()) return NULL;	//didn't find the first link

	tree_node *result, *tmp;	//here somewhere you can implement name1.*.name2 and other regexps, if you want
	for(std::list<tree_node*>::const_iterator itr=name_list.begin();itr!=name_list.end();++itr) {
		result=*itr;
		for(T_ls::const_iterator itr_tmp=itr_c;;++itr_tmp) {	//itr_c=begin()+1
			if(itr_tmp==chain.end()) return result;
			tmp=result->find_name(*itr_tmp);
//std::cout<<"tree_node: searched for name "<<*itr_tmp<<" and found "<<tmp<<std::endl;
			if(tmp && tmp->parent==result) result=tmp;
			else break;
		}
	}
	return NULL;
}
void Slurper::Connections::tree_node::fill_fields(T_mss& fields, const T_s& to_append) const {
	T_s to_append_new=to_append;
	if(!to_append.empty()) to_append_new+=delimiters.front();
	for(T_mss::const_iterator itr=contents->second.begin();itr!=contents->second.end();++itr) {
		fields[to_append_new+itr->first]=itr->second;
	}
	for(std::list<tree_node*>::const_iterator itr_b=children.begin();itr_b!=children.end();++itr_b) {
		(*itr_b)->fill_fields(fields,to_append_new+(*itr_b)->contents->first);
	}
}
void Slurper::Connections::tree_node::fill_children_names(T_ls& result) const {
	for(std::list<tree_node*>::const_iterator itr_b=children.begin();itr_b!=children.end();++itr_b) {
		result.push_back( (*itr_b)->contents->first );
	}
}

void Slurper::Connections::tree_node::print(unsigned int tab, std::ostream& output) const {
	print_tab(tab,output);
	output<<contents->first<<" {\n";
	for(std::list<tree_node*>::const_iterator itr=children.begin();itr!=children.end();++itr) {
		(*itr)->print(tab+1,output);
	}
	for(T_mss::const_iterator itr=contents->second.begin();itr!=contents->second.end();++itr) {
		print_tab(tab+1,output);
		output<<itr->first<<itr->second;
		for(T_mss::const_iterator itr_d=Slurper::delimiters.begin();itr_d!=Slurper::delimiters.end();++itr_d) {
			if(!itr->second.find(itr_d->first)) {
				output<<itr_d->second;	//values don't contain second delimiters
				break;
			}
		}
		output<<"\n";
	}
	print_tab(tab,output);
	output<<"}"<<std::endl;
}

void Slurper::Connections::tree_node::add_child(tree_node* child) {
	if(child) {
		child->parent=this;	//(who's my daddy?)
		this->children.push_back(child);
	}
}

bool Slurper::Connections::tree_node::unique(void) {
	bool found=false;
	bool result=unique_value(contents,found);
	for(std::list<tree_node*>::iterator itr=children.begin();itr!=children.end();++itr) {
		result&=(*itr)->unique();
	}
	return result;
}
bool Slurper::Connections::tree_node::unique_value(std::multimap<T_s,T_mss>::iterator itr, bool& found) {
	bool result=true;	//found should be false on first step
	for(std::list<tree_node*>::iterator itr_b=children.begin();itr_b!=children.end();) {
		if((*itr_b)->equal(itr)) {
			if(found) {
				delete *itr_b;
				itr_b=children.erase(itr_b);
				result=false;
			} else {
				found=true;
				++itr_b;
			}
		} else {
			++itr_b;
		}
	}	//first we search on one level, and only then go deeper
	for(std::list<tree_node*>::const_iterator itr_b=children.begin();itr_b!=children.end();++itr_b) {
		result&=(*itr_b)->unique_value(itr,found);
	}
	return result;
}
bool Slurper::Connections::tree_node::unique_values(tree_node* base) {
	bool result=true;
	for(std::list<tree_node*>::iterator itr=children.begin();itr!=children.end();++itr) {
		bool found=true;
		result&=base->unique_value((*itr)->contents,found);
	}
	for(std::list<tree_node*>::iterator itr=children.begin();itr!=children.end();++itr) {
		result&=(*itr)->unique_values(base);
	}
	return result;
}

bool Slurper::Connections::tree_node::merge(std::multimap<T_s,T_mss>& blocks) {
	bool result=false;
	for(std::list<tree_node*>::iterator itr=children.begin();itr!=children.end();++itr) {
		std::list<tree_node*>::iterator itr2=itr;
		for(++itr2;itr2!=children.end();) {
			if((*itr)->contents->first == (*itr2)->contents->first) {
				result=true;
				(*itr)->contents->second.insert((*itr2)->contents->second.begin(),(*itr2)->contents->second.end());
				blocks.erase((*itr2)->contents);
				(*itr)->merge_with(*itr2);
				itr2=children.erase(itr2);
			} else {
				++itr2;
			}
		}
	}
	for(std::list<tree_node*>::iterator itr=children.begin();itr!=children.end();++itr) {
		result|=(*itr)->merge(blocks);
	}
	return result;
}
void Slurper::Connections::tree_node::merge_with(tree_node* base) {
	if(!base) return;
	for(std::list<tree_node*>::iterator itr=base->children.begin();itr!=base->children.end();++itr) {
		this->add_child(*itr);
	}
	base->children.clear();
	delete base;
}
//namespace{
const char* options_tmp2[]={".","::"};	//don't forget to count and put there \|/
const T_ls Slurper::Connections::tree_node::delimiters=T_ls(options_tmp2,options_tmp2+2);
//}
void Slurper::Connections::tree_node::append_ancestors(T_s& result) const {
	if(parent) {
		result=parent->contents->first+delimiters.front()+result;
		parent->append_ancestors(result);
	}
}

void Slurper::Connections::tree_node::cut_name(const T_s& name, T_ls& result) {
	T_ls::const_iterator delim=delimiters.end();
	std::size_t pos1=name.size(), pos2;
	for(T_ls::const_iterator itr=delimiters.begin();itr!=delimiters.end();++itr) {
		pos2=name.find(*itr);
		if(pos2<pos1) {
			pos1=pos2;
			delim=itr;
		}
	}
	if(delim!=delimiters.end()) {
		result.push_back(name.substr(0,pos1));	//follow the ordering!
		cut_name(name.substr(pos1+delim->size()),result);
	} else if(!name.empty()) {
		result.push_back(name);
	}
}
