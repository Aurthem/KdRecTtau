{
if (!TClassTable::GetDict("DataClass")) {
	gSystem->Load("~/current/lib/libDataClass.so");
}
#include <map>
	TFile* hfile = new TFile("data.root");
	TTree* tree = (TTree*)hfile->Get("T_data");
	TBranch* branch = tree->GetBranch("data");
	DataClass* dc = new DataClass();// dc=0;
	branch->SetAddress(&dc);
	
	cout<<"Tree with "<<tree->GetEntries()<<" entries read."<<endl;
	cout<<"Example read:"<<endl;
	int nEntry=0;
	const T_pds* ds=0;
//	const TObject* dob=0;
	for(;!ds;nEntry++) {
		branch->GetEntry(nEntry);
	//	dob=dc->Get("emc_bhabha::CsI.invMass");
		const DataClass::Double_data* dummy=dynamic_cast<const DataClass::Double_data*>(dc->Get("emc_bhabha::CsI.invMass"));
		ds=dummy->get();
	}
	if(nEntry!=tree->GetEntries()) {
		cout<<"\tpair from event "<<(nEntry-1)<<" is\n\t  ";
//	T->GetEntry(nEntry);
//	ds = dc->Get("emc_bhabha.lkrTheta");
		cout<<ds->first<<" "<<ds->second<<endl;
	}
//	T->StartViewer();
}
