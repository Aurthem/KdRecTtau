#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedfunction;
#pragma link C++ nestedtypedef;

//#pragma link C++ typedef T_ls;

//#pragma link C++ function tmp_map_list_of<T_s,T_s>(T_s,T_s);
//#pragma link C++ class Criteria;
//#pragma link C++ function Criteria::check(const T_s&, const double&, const T_s&);

//#pragma link C++ class std::pair<double,std::string>;
//#pragma link C++ class std::map<std::string,std::pair<double,std::string> >;
//#pragma link C++ class std::pair<std::string,std::pair<double,std::string> >;
//#pragma link C++ class std::map<std::string,std::pair<double,std::string> >::iterator;
//#include <functional>
//#include <map>

#pragma link C++ class T_pds;
#pragma link C++ class T_pvds;
//#pragma link C++ class T_vi;	//precompiled
#pragma link C++ typedef T_pss;
#pragma link C++ typedef T_msT_itr;
#pragma link C++ typedef T_msT_citr;
#pragma link C++ typedef T_msmsT_citr;

#pragma link C++ class DataClass::Data<double>+;	//add this triplet for every used type
#pragma link C++ function DataClass::Add<double>(const T_s&, const T_s&, const double&, const T_s&);
#pragma link C++ function DataClass::Add<double>(const T_s&, const double&, const T_s&);
#pragma link C++ class DataClass::Data<int>+;
#pragma link C++ function DataClass::Add<int>(const T_s&, const T_s&, const int&, const T_s&);
#pragma link C++ function DataClass::Add<int>(const T_s&, const int&, const T_s&);
#pragma link C++ class DataClass::Data<T_vd>+;
#pragma link C++ function DataClass::Add<T_vd>(const T_s&, const T_s&, const T_vd&, const T_s&);
#pragma link C++ function DataClass::Add<T_vd>(const T_s&, const T_vd&, const T_s&);
#pragma link C++ class DataClass::Data<T_vi>+;
#pragma link C++ function DataClass::Add<T_vi>(const T_s&, const T_s&, const T_vi&, const T_s&);
#pragma link C++ function DataClass::Add<T_vi>(const T_s&, const T_vi&, const T_s&);

//#pragma link C++ class T_mspds::iterator;
//#pragma link C++ class std::list<T_mspds::const_iterator>;
//#pragma link C++ function std::map<std::string,std::pair<double,std::string> >::map<std::string,std::pair<double,std::string> >(const std::less<std::string>&, const std::allocator<std::pair<const std::string,T_pds> >&);	//const Compare&,const Allocator&
//less<Key>, class Allocator=allocator<pair <const Key, T> >
#pragma link C++ class DataClass+;
#endif
