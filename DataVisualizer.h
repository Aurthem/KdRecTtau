#ifndef au_visualizer
#define au_visualizer

#include "typedefs.h"
#include "criteria.h"
#include "DataManager.h"
#include "io_apps.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TProfile.h"
#include "TPaveText.h"
#include "TText.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TF1.h"

//#include "RTypes.h"	//colors (no such thing in root 5.14)
#include "TColor.h"

//class with non-trivial destructor should have copy constructor and assignment operator to avoid deletion while copying
//Manager(const Manager&)	Manager& operator=(const Manager&)

class DataVisualizer {
public:
	DataVisualizer(bool raw_ext, const T_s& dataname, const T_s& cssname);
	void init(const T_s& dataname, const T_s& cssname);
	virtual ~DataVisualizer();
	
//	void AddHist(const T_s& name, const T_s& title, int bins_number, const T_pdd& minmax);
//	void AddProfile(const T_s& name, const T_s& title, int xbins_number, const T_pdd& x_minmax, const T_pdd& y_minmax);
//	void FillHist(const T_s& name, double value);
	
	TH1D* GetHist(const T_s& name) const;
	TProfile* GetProfile(const T_s& name) const;
	
	void Draw(void) const;
	void SetLabel(const T_s& lbl="") { label=lbl; }
	T_s GetPath(void) const { return path; }
private:
	bool raw;
	DataManager* dm;
	CssSlurper csl;
	T_s label;	//to append to filenames
	T_s path;
	LogRoot log_root;
	
	std::list<TH1D*> histograms;	//1D histograms
	std::list<TProfile*> profiles;	//profiles y(x) with standard calculated errors
	//names should be different in hists and profiles combined
	
	void fill_all(void);
	void fill_all_raw(const T_s* label=0);
	void clear_all(void);
	
	void readout_wireframes(void);
	
	class Wireframe;
	friend class Wireframe;	//lots of private stuff should be kept private
	std::vector<Wireframe*> wireframes;	//we need this so it won't delete canvas on insertion
	
	static Double_t fitf_lognormal(const Double_t *value, const Double_t *params);	//asymmetric gauss for bhabha
	static Double_t fitf_bhabha_xsection(const Double_t *value, const Double_t *params);
	static void fit_gauslog(TH1 *hh, Double_t &bb_mean, Double_t &bb_sigma, Double_t &ev_tail);
	static void fit_bb_xsection(TH1 *hh, Double_t &norm, Double_t &res);
};

class DataVisualizer::Wireframe {
public:
	Wireframe(DataVisualizer* prt);
	~Wireframe(void);
	
	void	setup(		const T_s& name, const T_mss& params, const T_mss& global_params);
	TH1D*	add_hist(	const T_s& name, const T_mss& params, const T_mss& global_params);
	TProfile* add_prof(	const T_s& name, const T_mss& params, const T_mss& global_params);
	
	void Draw(void) const;

	TStyle* ts;	//style object, global one is gStyle
	//call ts->cd() to set gStyle to ts; call gROOT->SetStyle("Plain") to set gStyle to plain
	TCanvas* cv;
	TPad* pad;
	TPaveText* pt;
	TLegend* leg;
	TText* tt;	//temp pointer to apply styles
	
	struct Binds {
		T_puu position;
		T_s data;
		T_s fit;
		T_s draw_options;
		
		T_s xunit;
		T_s yunit;
		
		bool logx, logy, norm, err;
		Binds(void) : position(T_puu(0,0)),
			logx(false),logy(false),norm(false),err(false) { }	//should be by default
	};
//	std::map<void*,T_puu> binds;	//binds hists with subpads positions (they start from 1,1)
//	std::map<void*,T_s> names;	//hist or profile vs name to search
//	std::map<void*,T_s> fit_names;
	std::map<void*,Binds> binds;
private:
	DataVisualizer* parent;	//for parent methods (in Java it's done automatically)
	TH1D* get_hist(const void* ptr) const;
	TProfile* get_profile(const void* ptr) const;

	static bool get_param(const T_s& name, const T_mss& params, const T_mss& global_params, T_s& result);
	static short int extract_color(const T_s& str);
	static std::map<T_s,short int> colors;
};
#endif	//au_visualizer
