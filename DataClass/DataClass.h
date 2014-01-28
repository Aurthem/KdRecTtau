#ifndef au_dataClass
#define au_dataClass

#include <iostream>

#ifndef __CINT__
#include "typedefs.h"
#else
#include "typedefs_cint.h"
	typedef typename map<T_s,TObject*>::iterator T_msT_itr;
	typedef typename map<T_s,TObject*>::const_iterator T_msT_citr;
	typedef typename map<T_s,map<T_s,TObject*> >::const_iterator T_msmsT_citr;
#endif

#include "TObject.h"
#include "TBuffer.h"


#ifndef __CINT__
#include <typeinfo>
namespace IOconverter {
	class Eraser {
	public:
		Eraser(void) : storage(NULL) { }
		Eraser(Eraser const& other) : storage(other.storage->clone()) { }
		template<class Tp> Eraser(Tp const& dt) : storage(new Object<Tp>(dt)) { }
		~Eraser(void) { Clear(); }
		Eraser& operator=(Eraser const& other) {
			Clear();
			storage=other.storage->clone();
			return *this;
		}
		template<class Tp> Eraser& operator=(Tp const& dt) {
			Clear();
			storage = new Object<Tp>(dt);
			return *this;
		}
		void Clear(void) { if(storage) delete storage; }
		template<class Tp> const Tp& Cast(void) const { return (static_cast<Object<Tp>*>(this->storage))->data; }
		//due to parsing complications, use it like this: ers.template Cast<T_vd>();
		//template keyword is necessary
		bool isContainer(void) const { return storage->isContainer(); }
	private:
		struct Interface {
			virtual ~Interface(void) { }
			virtual Interface* clone(void) const = 0;
			virtual bool isContainer(void) const = 0;
		};
		template<class Tp> struct Object : Interface {
			Object(void) : data() { }
			Object(Tp const& dt) : data(dt) { }
			virtual ~Object(void) { }
			virtual Interface* clone(void) const { return new Object<Tp>(*this); }
			virtual bool isContainer(void) const { return false; }
			Tp data;
		};
		Interface* storage;
	};
	template <> bool Eraser::Object<T_vd>::isContainer(void) const { return true; }
	template <> bool Eraser::Object<T_vi>::isContainer(void) const { return true; }
//	template<typename T,typename Alloc>		//this specialization doesn't work
//	bool Eraser::Object< std::vector<T,Alloc> >::isContainer(void) const { return true; }
}
//#include "prettyprint98.hpp"	//does not work on gcc 3.4.0 and lower
#endif

class DataClass : public TObject {
public:
	DataClass() { }
	DataClass(DataClass* dc) { *this=*dc; }
	DataClass(const DataClass& dc) : TObject(dc),data(dc.data) { }//,memory(dc.memory) { }
	virtual ~DataClass();
	void Clear(Option_t *option =" ");
	void print(void);
	bool empty(void) const { return data.empty(); }
	std::size_t size(void) const;
	void Fill();
	
	template<typename Type>
	void Add(const T_s& blabel, const T_s& flabel, const Type& value, const T_s& ulabel="");
	template<typename Type>
	void Add(const T_s& glabel, const Type& value, const T_s& ulabel="") {
		T_pss tmp=cut_labels(glabel);
		Add(tmp.first,tmp.second,value,ulabel);
	}
	
	template<typename Type>
	class Data : public TObject {	//non-virtual destructor, so don't inherit from this class
	public:
		Data(void) : inited(false) { }
		~Data(void) { }
		
#ifndef __CINT__
		virtual void Print(Option_t* option="") const {
			const Type* tmp=get();
			if(tmp) {
				IOconverter::Eraser ers(*tmp);
				if(ers.isContainer()) {
					T_vd tmp_v=ers.template Cast<T_vd>();
					std::cout<<"{";
					for(T_vd::const_iterator itr=tmp_v.begin();itr!=tmp_v.end();++itr) {
						if(itr!=tmp_v.begin()) std::cout<<", ";
						std::cout<<*itr;
					}	std::cout<<"}";
				} else {
					std::cout<<ers.template Cast<double>();
				}
			//	std::cout<<*tmp;
			} else std::cout<<"!init¡";
			std::cout<<"|"<<get_unit()<<std::flush;
		}
#endif
		
		void set(Type value, const T_s& ulabel="") { data=value; unit=ulabel; inited=true; }
		const Type* get(void) const { return (inited ? &data : NULL); }
		const T_s get_unit(void) const { return unit; }
	private:
		Type data;
		T_s unit;
		bool inited;
		
		ClassDef(Data,1)
	};

	const TObject* Get(const T_s& glabel) const;
	const TObject* Get(const T_s& blabel, const T_s& flabel) const;
	
	bool Remove(const T_s& glabel);
	bool Remove(const T_s& blabel, const T_s& flabel);
	void clear(void);

	static T_pss cut_labels(const T_s& glabel);
	
private:
#ifndef __CINT__
	std::map<T_s,std::map<T_s,TObject*> > data;
#else
	map<T_s,map<T_s,TObject*> > data;
#endif
	
	ClassDef(DataClass,1)	//some macro of TObject (short version=1 goes there)
};

template<typename Type>
void DataClass::Add(const T_s& blabel, const T_s& flabel, const Type& value, const T_s& ulabel) {
#ifndef __CINT__
	std::map<T_s,TObject*>::iterator itr=data[blabel].find(flabel);
#else
	T_msT_itr itr=data[blabel].find(flabel);
#endif
	if(itr==data[blabel].end()) {
		Data<Type>* tmp = new Data<Type>(); tmp->set(value,ulabel);
#ifndef __CINT__
		std::pair<T_s,TObject*> pr=make_pair(flabel,tmp);
#else
		pair<T_s,TObject*> pr=make_pair(flabel,tmp);
#endif
//cout<<"Made pair=\""<<flabel<<"\"|"<<tmp<<endl;
		data[blabel].insert(pr);
	} else {
		Data<Type>* tmp=dynamic_cast<Data<Type>*>(itr->second);
		if(tmp) {
			tmp->set(value,ulabel);
		} else {
			if(itr->second) delete itr->second;
			tmp = new Data<Type>(); tmp->set(value,ulabel);
			itr->second=tmp;
		}
	}
}

templateClassImp(DataClass::Data)

#endif
