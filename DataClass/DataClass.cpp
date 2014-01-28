
#include <iostream>
#include "DataClass.h"

using namespace std;

ClassImp(DataClass)	//some macro of TObject

DataClass::~DataClass() {
	Clear();
}
void DataClass::Clear(Option_t *option) {
	if(option) {
		if(option[0]==' ') clear();
		else cout<<"DataClass::Clear -> wrong option."<<endl;
	} else {
		cout<<"DataClass::Clear -> option is null."<<endl;
	}
}
void DataClass::Fill() {
}

std::size_t DataClass::size(void) const {
	std::size_t result=0;
#ifndef __CINT__
	map<T_s,map<T_s,TObject*> >::const_iterator itr;
#else
	T_msmsT_citr itr;
#endif
	for(itr=data.begin();itr!=data.end();++itr) {
		result+=itr->second.size();
	}
	return result;
}

const TObject* DataClass::Get(const T_s& glabel) const {
	T_pss tmp=cut_labels(glabel);
	return Get(tmp.first,tmp.second);
}

bool DataClass::Remove(const T_s& glabel) {
	T_pss tmp=cut_labels(glabel);
	return Remove(tmp.first,tmp.second);
}
bool DataClass::Remove(const T_s& blabel, const T_s& flabel) {
	if(!data.count(blabel)) return false;
	bool result=false;
	if(data[blabel].count(flabel)) {
		delete data[blabel][flabel];
		result=data[blabel].erase(flabel);
	}
	if(data[blabel].empty()) result|=data.erase(blabel);
	return result;
}

//glabel format: use either blabel.flabel or blabel::flabel (don't mix in one label)
T_pss DataClass::cut_labels(const T_s& glabel) {
	T_pss result;
	size_t pos1=glabel.find("."), pos2=glabel.find("::");
	if(pos1<pos2 && pos1!=glabel.size()-1) {
		result.first=glabel.substr(0,pos1);
		result.second=glabel.substr(pos1+1);
	} else if(pos2!=T_s::npos && pos2!=glabel.size()-2) {
		result.first=glabel.substr(0,pos2);
		result.second=glabel.substr(pos2+2);
	} else {
		cerr<<"Warning: Attempt to insert empty field label."<<endl;
		result.first=glabel;	//will contain '.' or "::" at the end (easy to indicate errors)
	}
	return result;
}

#ifndef __CINT__
void DataClass::clear(void) {
	map<T_s,map<T_s,TObject*> >::const_iterator itr_b;
	for(itr_b=data.begin();itr_b!=data.end();++itr_b) {
		map<T_s,TObject*>::const_iterator itr_f;
		for(itr_f=itr_b->second.begin();itr_f!=itr_b->second.end();++itr_f) {
			delete itr_f->second;
		}
	}
	data.clear();
}
const TObject* DataClass::Get(const T_s& blabel, const T_s& flabel) const {
	map<T_s,map<T_s,TObject*> >::const_iterator itr_b=data.find(blabel);
	if(itr_b==data.end()) return NULL;
	map<T_s,TObject*>::const_iterator itr_f=itr_b->second.find(flabel);
	if(itr_f==itr_b->second.end()) return NULL;
	return (itr_f->second);
}
void DataClass::print() {
	cout<<"DataClass {\n";
	for(map<T_s,map<T_s,TObject*> >::const_iterator itr_b=data.begin();itr_b!=data.end();itr_b++) {
		cout<<"\t"<<itr_b->first<<" {\n";
		for(map<T_s,TObject*>::const_iterator itr_f=itr_b->second.begin();itr_f!=itr_b->second.end();itr_f++) {
			cout<<"\t\t"<<itr_f->first<<"=";
			itr_f->second->Print();
			cout<<";\n";
		} cout<<"\t}\n";
	} cout<<"}";
	cout<<endl;
}
#else
void DataClass::clear(void) {
	for(T_msmsT_citr itr_b=data.begin();itr_b!=data.end();++itr_b) {
		for(T_msT_citr itr_f=itr_b->second.begin();itr_f!=itr_b->second.end();++itr_f) {
			delete itr_f->second;
		}
	}
	data.clear();
}
const TObject* DataClass::Get(const T_s& blabel, const T_s& flabel) const {
	T_msmsT_citr itr_b=data.find(blabel);
	if(itr_b==data.end()) return NULL;
	T_msT_citr itr_f=itr_b->second.find(flabel);
	if(itr_f==itr_b->second.end()) return NULL;
	return (itr_f->second);
}
void DataClass::print() {
	cout<<"DataClass {\n";
	for(T_msmsT_citr itr_b=data.begin();itr_b!=data.end();itr_b++) {
		cout<<"\t"<<itr_b->first<<" {\n";
		for(T_msT_citr itr_f=itr_b->second.begin();itr_f!=itr_b->second.end();itr_f++) {
			cout<<"\t\t"<<itr_f->first<<"="<<itr_f->second<<";\n";
		} cout<<"\t}\n";
	} cout<<"}";
	cout<<endl;
}
#endif
