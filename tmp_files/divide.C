{	gROOT->Reset();
	TFile* file=new TFile("log.root");
	file->ls();
	TList* lst1=(TList*)(file->Get("hp_data_ntr"));
	lst1->Print();
	TProfile* pr1=(TProfile*)(lst1->FindObject("theta_resolution.p.theta_emc"));
	TList* lst2=(TList*)(file->Get("hp_data_ntr_sim"));
	TProfile* pr2=(TProfile*)(lst2->FindObject("theta_resolution.p.theta_emc"));
	TH1D* hexp=pr1->ProjectionX("_pxexp","e");	//for normal hist, use "e" to retain profile errors
	TH1D* hsim=pr2->ProjectionX("_pxsim","e");
	TProfile* pr_div=(TProfile*)(pr1->Clone("theta_resolution.rel"));
	TCanvas* cv1=new TCanvas("pr_exp_div_sim","pr_eds",800,600);
	pr_div->Divide(hsim);
	pr_div->SetAxisRange(0.8,1.2,"y");
	pr_div->Draw();
	cv1->Print((string(cv1->GetName())+".eps").c_str());

	TCanvas* cv2=new TCanvas("pr_sim_div_exp","pr_sde",800,600);
	TProfile* pr_div2=(TProfile*)(pr2->Clone("theta_resolution.rel2"));
	pr_div2->Divide(hexp);
	pr_div2->SetAxisRange(0.8,1.2,"y");
	pr_div2->Draw();
	cv2->Print((string(cv2->GetName())+".eps").c_str());
}
