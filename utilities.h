
#include "typedefs.h"

#ifndef aurthem_utilities
#define aurthem_utilities

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <typeinfo>

#include "TSystem.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "TList.h"

namespace { //useful stuff
    // Copyright 2003-2006 Thorsten Ottosen (somewhat fragment of boost library)
	template<class K, class V>	//loose copy of boost::assign::map_list_of
	struct map_list_of_type {	//another way of doing so is to just define some create_map() method
		typedef std::map<K, V> T_mKV;
		T_mKV data;
		map_list_of_type(K k, V v) { data[k] = v; }
		map_list_of_type& operator()(K k, V v) { data[k] = v; return *this; }
		operator T_mKV const&() const { return data; }
	};
	template<class K, class V>
	map_list_of_type<K, V> build_map_from_list(K k, V v) {
		return map_list_of_type<K, V>(k, v);
	}

	template<class K>
	struct vector_list_of_type {
		typedef std::vector<K> T_vK;
		T_vK data;
		vector_list_of_type(K k) { data.push_back(k); }
		vector_list_of_type& operator()(K k) { data.push_back(k); return *this; }
		operator T_vK const&() const { return data; }
	};
	template<class K>
	vector_list_of_type<K> build_vector_from_list(K k) {
		return vector_list_of_type<K>(k);
	}
	
	//---------
//	typedef typename ContainerT::value_type Value_type;	//these two lines are not needed in VS, but necessary in g++
//	typedef typename Value_type::size_type Size_type;	//basically, they say how compiler should interpret double casting
	template<class ContainerK, class ContainerT>
	bool convert_container(ContainerK& result, const ContainerT* from, const typename ContainerK::value_type multiplier=1) {
		if(!from) return false;
		result.clear();
		typedef typename ContainerK::value_type Value_type;
		typedef typename ContainerT::const_iterator Itr;
		for(Itr itr=from->begin();itr!=from->end();++itr) {
			result.push_back(multiplier*static_cast<Value_type>(*itr));
		}
		return true;
	}
	template<typename Res, typename Type>
	bool convert_value(Res& result, const Type* from, const Res multiplier=1) {
		if(!from) return false;
		result=multiplier*static_cast<Res>(*from);
		return true;
	}
}

namespace SUconverter {	//system of units converter (in Java it would've been a static class)
	T_s print(const T_pdd& pr, const T_s& format);	//prints pair values in preferred format
	T_s print(const T_vpdd& vpr, const T_s& format);	//prints pair values in preferred format
	double get_multiplier(const T_s& from_unit, const T_s& to_unit="");
	enum SUtype {time_unit,distance_unit,angle_unit,energy_unit,magnetic_field_unit,area_unit,luminosity_unit,no_unit};
	SUtype get_type(const T_s& str);
}

namespace IOconverter {	// input/output converter (from/to string)
	template<class ContainerT>	//works with ContainerT=std::vector<int> or std::list<int>
	T_s print_list(const ContainerT* lt) {
		std::stringstream result; bool opened_range=false;
		if(!lt) return result.str();
		if(lt->empty()) return result.str();
		typedef typename ContainerT::value_type Value_type;
		typename ContainerT::const_iterator tmp; Value_type diff=0;
		for(typename ContainerT::const_iterator itr=lt->begin();itr!=lt->end();) {
			if(!opened_range) {
				result<<(*itr);
				tmp=itr++;
				if(itr!=lt->end()) {
					if(abs(diff=(*tmp)-(*itr))==1) {
						opened_range=true;
						result<<"-";
					} else {
						opened_range=false;
						result<<",";
					}
				}
			} else {
				tmp=itr++;
				if(itr!=lt->end()) {
					if(diff!=(*tmp)-(*itr)) {
						opened_range=false;
						result<<(*tmp)<<",";
					}
				} else {
					opened_range=false;
					result<<(*tmp);
				}
			}
		}
		return result.str();
	}
	T_li extract_list(const T_s& str);
	template<typename Type>
	std::list<Type> convert_list(const T_li& lst) {
		std::list<Type> result;
		for(T_li::const_iterator itr=lst.begin();itr!=lst.end();++itr) {
			result.push_back( static_cast<Type>(*itr) );
		}
		return result;
	}
	T_li generate_list(int val1, int val2);

	T_pdd extract_pair(const T_s& str);
	
	template<class ContainerT> //works with ContainerT=std::vector<std::string> or std::list<std::string>
	void tokenize(const std::string& str, ContainerT& tokens,
			const std::string& delimiters=" ", const bool trimEmpty=false) {
		std::string::size_type pos1=0, pos2;
		typedef typename ContainerT::value_type Value_type;	//these two lines are not needed in VS, but necessary in g++
		typedef typename Value_type::size_type Size_type;	//basically, they say how compiler should interpret double casting
		while(true) {
			pos2=str.find_first_of(delimiters,pos1);
			if(pos2!=pos1 || !trimEmpty)
				tokens.push_back(	//if pos2==npos, all is well (in this constructor)
					Value_type(str,pos1,static_cast<Size_type>(pos2-pos1))
				);
			if(pos2==std::string::npos) return;
			else pos1=pos2+1;
		}
	}
	
	template<class ContainerT, class ContainerD> //works with Container=std::vector<std::string> or std::list<std::string>
	void tokenize_strings(const std::string& source, const ContainerD& delimiters, ContainerT& tokens,
			std::vector<std::vector<typename ContainerD::const_iterator> >& placement,	//one element bigger than tokens
			const bool trimEmpty=false) {
		std::string::size_type pos1=0, pos2, pos3;
		typedef typename ContainerT::value_type Value_typeT;	//these two lines are not needed in VS, but necessary in g++
		typedef typename Value_typeT::size_type Size_typeT;		//basically, they say how compiler should interpret double casting
	//	typedef typename ContainerD::value_type Value_typeD;
	//	typedef typename Value_typeD::size_type Size_typeD;
		typedef typename ContainerD::const_iterator CitrD;
		placement.clear(); placement.push_back(std::vector<CitrD>());
		while(true) {
			pos2=std::string::npos;
			CitrD delim_status=delimiters.end();
			for(CitrD itr=delimiters.begin();itr!=delimiters.end();++itr) {
				pos3=source.find(*itr,pos1);
				if(pos3<pos2 || (pos3==pos2 && delim_status!=delimiters.end() && itr->size()>delim_status->size()) ) {
					pos2=pos3; delim_status=itr;
				}
			}
			if(pos2!=pos1 || !trimEmpty) {
				tokens.push_back(
					Value_typeT(source,pos1,static_cast<Size_typeT>(pos2-pos1))
				);
				placement.push_back(std::vector<CitrD>());
			}
			if(pos2==std::string::npos) {
				return;
			} else {
				pos1=pos2+delim_status->size();
				placement.back().push_back(delim_status);
			}
		}
	}

	template<typename Type>
	class Parser {	//parses strings as arithmetic expressions (incomplete)
	public:
		Parser(const T_s& source) {
			tokenize_strings(source,operators, values,operator_placement, true);
			if(	typeid(Type)==typeid(int) 		|| typeid(Type)==typeid(unsigned int) ||
				typeid(Type)==typeid(short int)	|| typeid(Type)==typeid(unsigned short int) ||
				typeid(Type)==typeid(long int)	|| typeid(Type)==typeid(unsigned long int) )			  
					type_type=int_type;
			else if(typeid(Type)==typeid(double) || typeid(Type)==typeid(float) || typeid(Type)==typeid(long double))
				type_type=double_type;
			else type_type=string_type;
		}
		Type evaluate(const std::map<T_s,Type>& variables) const {	//should be rewritten for more operators later
			Type result=Type();
			typedef typename std::map<T_s,Type> T_msType;
			typedef typename T_msType::const_iterator T_msType_Citr;
			T_msType_Citr itr_m;
			typedef typename T_vs::const_iterator T_vs_Citr;
			typedef typename std::vector<T_vs_Citr> T_v_vs_Citr;
			typedef typename std::vector<T_v_vs_Citr> T_vv_vs_Citr;
			typedef typename T_vv_vs_Citr::const_iterator T_vv_vs_Citr_Citr;
			T_vv_vs_Citr_Citr current=operator_placement.begin();
			for(T_vs::const_iterator itr=values.begin();itr!=values.end();++itr) {
				Type dummy;
				if( (itr_m=variables.find(*itr)) !=variables.end()) {
					dummy=itr_m->second;
				} else {
					dummy=extract_value(*itr);
				}
				if(!current->empty() && *(current->back())==T_s("-") ) result-=dummy;
				else result+=dummy;
				++current;
			}
			return result;
		}
	private:
		static const T_vs operators;	//allowed operators
		enum Type_type { int_type, double_type, string_type };
		Type_type type_type;
		T_vs values;
		std::vector<std::vector<T_vs::const_iterator> > operator_placement;
		
		Type extract_value(const T_s& str) const {
			if(type_type==int_type) return static_cast<Type>( atoi(str.c_str()) );
			else if(type_type==double_type) return static_cast<Type>( atof(str.c_str()) );
			else return 0;//static_cast<Type>(str);
		//	switch(type_type) {
		//	case int_type:	return static_cast<Type>( atoi(str.c_str()) );
		//	case double_type:	return static_cast<Type>( atof(str.c_str()) );
		//	case string_type:
		//	default:	return static_cast<Type>(str);
		//	}
		}
	};
	
//	const char* operators_tmp;
//	const unsigned int operators_tmp_size=2;
	template<typename Type>
	const T_vs Parser<Type>::operators=build_vector_from_list<T_s>("+")("-");
	
	T_ls list_files(const T_s& dirname="./",const T_s& suffix=".root",const T_s& prefix="");
	T_ls list_directories(const T_s& dirname="./");
}
#endif	//aurthem_utilities
