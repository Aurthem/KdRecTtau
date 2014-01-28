{
if (!TClassTable::GetDict("DataClass")) {
	gSystem->Load("~/current/lib/libDataClass.so");
}	//gSystem->Load("/usr/local/root-5.30.00/lib/root/libCore.so");
#include <map>
	TFile* hfile = new TFile("runs.root");
	TTree* tree = (TTree*)hfile->Get("T_runs");
	TBranch* branch = tree->GetBranch("runs");
	DataClass* dc = new DataClass();// dc=0;
	branch->SetAddress(&dc);
	
	cout<<"Tree with "<<tree->GetEntries()<<" entries read."<<endl;
	cout<<"Example read:"<<endl;
	int nEntry=0;
	const T_pvds* ds=0;
	const TObject* dtb=0;
	for(;!ds;nEntry++) {
		branch->GetEntry(nEntry);
		(dc->Get("all::CsI.notgood")); dtb=(dc->Get("all::CsI.notgood"));	//you need to repeat it, or root won't get it
		const DataClass::VDouble_data* dummy=dynamic_cast<const DataClass::VDouble_data*>(dtb);
	//	const DataClass::VDouble_data* dummy=dynamic_cast<const DataClass::VDouble_data*>(dc->Get("all::CsI.notgood"));
		ds=dummy->get();
	}
	if(nEntry!=tree->GetEntries()) {
		cout<<"\tpair from event "<<(nEntry-1)<<" is\n\t  ";
		cout<<"{";
		for(T_vd::const_iterator itr=ds->first.begin();itr!=ds->first.end();++itr) {
			if(itr!=ds->first.begin()) cout<<", ";
			cout<<*itr;
		} cout<<"} "<<ds->second<<endl;
	}
}
