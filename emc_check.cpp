//#include <vector>
//#include <string>
//#include <iomanip>

#include "KEmcRec/emc_system.h"

#include "modules.h"

namespace emcCheck {
	template <class T>
	bool compare(const T& first, const T& second, const std::string& what, int i=-1, int j=-1) {
		if(first!=second) {
			std::cerr<<std::setprecision(10);
			std::cerr<<"emcCheck::"<<what;
			if(i>=0) std::cerr<<"["<<i<<"]"; if(j>=0) std::cerr<<"["<<j<<"]";
			std::cerr<<"\tfailed at event "<<lkrData->getNEvent()
				<<":\n\t\t"<<first<<" != "<<second<<std::endl;
			return false;
		} else {
			return true;
		}
	}
	template <class K, class V>
	bool compareWL(const K& firstK, const K& secondK,
	  const V& firstV, const V& secondV, const std::string& what, int i=-1, int j=-1) {
		if(firstK!=secondK && firstV<secondV) { //with limits on second
			std::cerr<<std::setprecision(10);
			std::cerr<<"emcCheck::"<<what;
			if(i>=0) std::cerr<<"["<<i<<"]"; if(j>=0) std::cerr<<"["<<j<<"]";
			std::cerr<<"\tfailed at event "<<lkrData->getNEvent()
				<<":\n\t\t"<<firstK<<" != "<<secondK
				<<" && "<<firstV<<" < "<<secondV<<std::endl;
			return false;
		} else {
			return true;
		}
	}

	bool check_data(const bool all) {
		bool result=true;
		for(int i=0; i<NLKRSIZE; i++) {
	if(!(result&=compare(semc_data.glkr_energy[i],lkrData->energy[i],"data()\t-> glkr_energy",i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_type[i]  ,lkrData->type[i]  ,"data()\t-> glkr_type"  ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_theta[i] ,lkrData->theta[i] ,"data()\t-> glkr_theta" ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_phi[i]   ,lkrData->phi[i]   ,"data()\t-> glkr_phi"   ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_rho[i]   ,lkrData->fR[i]    ,"data()\t-> glkr_rho"   ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_dtheta[i],lkrData->dtheta[i],"data()\t-> glkr_dtheta",i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_dphi[i]  ,lkrData->dphi[i]  ,"data()\t-> glkr_dphi"  ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_drho[i]  ,lkrData->dR[i]    ,"data()\t-> glkr_drho"  ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_x[i]     ,lkrData->getX(i)  ,"data()\t-> glkr_x"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_y[i]     ,lkrData->getY(i)  ,"data()\t-> glkr_y"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_z[i]     ,lkrData->getZ(i)  ,"data()\t-> glkr_z"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_nl[i]    ,lkrData->getNl(i) ,"data()\t-> glkr_nl"    ,i)) && !all) return false;
	if(!(result&=compare(semc_data.glkr_clsr[i],lkrData->getCluster(i),"data()\t->glkr_clsr" ,i)) && !all) return false;
		}
		for(int i=0; i<NCSISIZE; i++) {
	if(!(result&=compare(semc_data.gcsi_energy[i] ,csiData->energy[i],"data()\t-> gcsi_energy",i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_type[i]   ,csiData->type[i]  ,"data()\t-> gcsi_type"  ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_theta[i]  ,csiData->theta[i] ,"data()\t-> gcsi_theta" ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_phi[i]    ,csiData->phi[i]   ,"data()\t-> gcsi_phi"   ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_rho[i]    ,csiData->fR[i]    ,"data()\t-> gcsi_rho"   ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_x[i]      ,csiData->getX(i)  ,"data()\t-> gcsi_x"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_y[i]      ,csiData->getY(i)  ,"data()\t-> gcsi_y"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_z[i]      ,csiData->getZ(i)  ,"data()\t-> gcsi_z"     ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_nl[i]     ,csiData->getNl(i) ,"data()\t-> gcsi_nl"    ,i)) && !all) return false;
	if(!(result&=compare(semc_data.gcsi_clsr[i],csiData->getCluster(i),"data()\t-> gcsi_clsr" ,i)) && !all) return false;
		}
		return result;
	}

	bool check_emc(const bool all) {
		bool result=true;
	if(!(result&=compareWL(semc.emc_ncls,int(emcRec->emcClusters.size()),
			semc.emc_ncls,NEMC_CLS,"emc()\t-> emc_ncls")) && !all) return false;
		for(int i=0; i<semc.emc_ncls; i++) {
			GClusterTower *cls = (GClusterTower *)emcRec->emcClusters[i];
	if(!(result&=compare(static_cast<int>(semc.emc_type[i]),cls->getType(),"emc()\t-> emc_type",i)) && !all) return false;
	if(!(result&=compareWL(static_cast<unsigned int>(semc.emc_ncells[i]),cls->getNCells(),
			static_cast<int>(semc.emc_ncells[i]),NEMC_CEL,"emc()\t-> emc_ncells",i)) && !all) return false;
			for(int j=0; j<semc.emc_ncells[i]; j++) {
				int gind=semc.emc_cells[i][j], shift=0;
				if(gind>=12001 && gind<=21600) {
					shift=12001;
				} else if(gind>=7001 && gind <= 8536) {
					shift=7001;
				} else {
	if(!(result&=compare(gind,7001,"emc()\t-> gind of emc_cells",i,j)) && !all) return false;
				}
				gind -= shift;
	if(!(result&=compare(gind,cls->Cells[j].getChannel(),"emc()\t-> emc_cells",i,j)) && !all) return false;
			}
	if(!(result&=compare(semc.emc_energy[i],cls->getEnergy(),"emc()\t-> emc_energy",i)) && !all) return false;
	if(!(result&=compare(semc.emc_x[i]     ,cls->getX()     ,"emc()\t-> emc_x"     ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_y[i]     ,cls->getY()     ,"emc()\t-> emc_y"     ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_z[i]     ,cls->getZ()     ,"emc()\t-> emc_z"     ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_vx[i]    ,cls->getVx()    ,"emc()\t-> emc_Vx"    ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_vy[i]    ,cls->getVy()    ,"emc()\t-> emc_Vy"    ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_vz[i]    ,cls->getVz()    ,"emc()\t-> emc_Vz"    ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_theta[i] ,cls->getTheta() ,"emc()\t-> emc_theta" ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_phi[i]   ,cls->getPhi()   ,"emc()\t-> emc_phi"   ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_rho[i]   ,cls->getRho()   ,"emc()\t-> emc_rho"   ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_dtheta[i],cls->getDtheta(),"emc()\t-> emc_dtheta",i)) && !all) return false;
	if(!(result&=compare(semc.emc_dphi[i]  ,cls->getDphi()  ,"emc()\t-> emc_dphi"  ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_drho[i]  ,cls->getDrho()  ,"emc()\t-> emc_drho"  ,i)) && !all) return false;
	if(!(result&=compare(semc.emc_theta_str[i],cls->getThetaStr(),"emc()\t-> emc_theta_str",i)) && !all) return false;
	if(!(result&=compare(semc.emc_phi_str[i],cls->getPhiStr(),"emc()\t-> emc_phi_str",i)) && !all) return false;
		}
		return result;
	}

	bool check_str(const bool all) {
		bool result=true; int ncls=0;
		for(int i=0; i<12; i++) ncls+=emcRec->strClusters[i].size();
	if(!(result&=compareWL(semc.str_ncls,ncls,
			semc.str_ncls,NSTR_CLS,"str()\t-> str_ncls")) && !all) return false;
		for(int nl=0; nl<12; nl++) {
			for(std::vector<GClusterStrip *>::iterator it=emcRec->strClusters[nl].begin();it!=emcRec->strClusters[nl].end();it++) {
				GClusterStrip *cls = *it;
				int i=cls->getIndexEmc()-1;
	if(!(result&=compareWL(static_cast<int>(semc.str_ncells[i]),cls->getNCells(),
			static_cast<int>(semc.str_ncells[i]),NSTR_CEL,"str()\t-> str_ncells",i)) && !all) return false;
				for(int j=0; j<semc.str_ncells[i]; j++) {		// cls->Cells[j].getChannel()
	if(!(result&=compare(semc.str_cells[i][j]-12001,cls->cells[j],"str()\t-> str_cells",i,j)) && !all) return false;
				}
	if(!(result&=compare(semc.str_energy[i],cls->getEnergy(),"str()\t-> str_energy",i)) && !all) return false;
	if(!(result&=compare(static_cast<int>(semc.str_type[i]),cls->getType(),"str()\t-> str_type",i)) && !all) return false;
	if(!(result&=compare(static_cast<int>(semc.str_nl[i]),cls->getNl()    ,"str()\t-> str_nl"  ,i)) && !all) return false;
	if(!(result&=compare(semc.str_rho[i]   ,cls->getRho()   ,"str()\t-> str_rho"   ,i)) && !all) return false;
	if(!(result&=compare(semc.str_xrec[i]  ,cls->getXrec()  ,"str()\t-> str_xrec"  ,i)) && !all) return false;
			}
		}
		return result;
	}

	bool check_str_track(const bool all) {
		bool result=true;
	if(!(result&=compareWL(semc.str_ntracks,int(emcRec->strTracks.size()),
			semc.str_ntracks,NSTR_TRK,"str_track()\t-> str_ntracks")) && !all) return false;
		if(result) for(int i=0; i<semc.str_ntracks; i++) {
			GTrack *track = emcRec->strTracks[i];
	if(!(result&=compare(static_cast<int>(semc.str_track_nvect[i]),track->nvect,"str_track()\t-> str_track_nvect",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_x0[i],track->x0,"str_track()\t-> str_track_x0",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_y0[i],track->y0,"str_track()\t-> str_track_y0",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_z0[i],track->z0,"str_track()\t-> str_track_z0",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_vx[i],track->Vx,"str_track()\t-> str_track_vx",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_vy[i],track->Vy,"str_track()\t-> str_track_vy",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_vz[i],track->Vz,"str_track()\t-> str_track_vz",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_t0[i],track->t0,"str_track()\t-> str_track_t0",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_t1[i],track->t1,"str_track()\t-> str_track_t1",i)) && !all) return false;
	if(!(result&=compare(semc.str_track_t2[i],track->t2,"str_track()\t-> str_track_t2",i)) && !all) return false;
		}
		return result;
	}

	bool check_emc2smb(const bool all) {
		bool result=true;
	// emc2str
		for(int i=0; i<semc.emc_ncls; i++) {
			GClusterTower *cls = (GClusterTower *)emcRec->emcClusters[i];
	if(!(result&=compareWL(static_cast<int>(semc.emc_str_ncls[i]),static_cast<int>(cls->strClusters.size()),
		static_cast<int>(semc.emc_str_ncls[i]),NEMC_CLS,"emc2smb()\t-> emc_str_ncls",i)) && !all) return false;
			if(result) for(int j=0; j<semc.emc_str_ncls[i]; j++) {
	if(!(result&=compare(static_cast<int>(semc.emc_str_cls[i][j]),cls->strClusters[j]->getIndexEmc(),"emc2smb()\t-> emc_str_cls",i,j)) && !all) return false;
			}
		}
	// emc2str_tracks
		for(int i=0; i<semc.emc_ncls; i++) {
			GClusterTower *cls = (GClusterTower *)emcRec->emcClusters[i];
	if(!(result&=compareWL(static_cast<int>(semc.emc_str_ntrk[i]),static_cast<int>(cls->strTracks.size()),
		static_cast<int>(semc.emc_str_ntrk[i]),NSTR_TRK,"emc2smb()\t-> emc_str_ntrk",i)) && !all) return false;
			if(result) for(int j=0; j<semc.emc_str_ntrk[i]; j++) {
	if(!(result&=compare(static_cast<int>(semc.emc_str_tracks[i][j]),cls->strTracks[j]->indx,"emc2smb()\t-> emc_str_tracks",i,j)) && !all) return false;
			}
		}
	// emc2dc_tracks
		for(int i=0; i<semc.emc_ncls; i++) {
			GClusterTower *cls = (GClusterTower *)emcRec->emcClusters[i];
	if(!(result&=compareWL(static_cast<int>(semc.emc_dc_ntrk[i]),static_cast<int>(cls->dctracks.size()),
		static_cast<int>(semc.emc_dc_ntrk[i]),NDCH_TRK,"emc2smb()\t-> emc_dc_ntrk",i)) && !all) return false;
			if(result) for(int j=0; j<semc.emc_dc_ntrk[i]; j++) {
	if(!(result&=compare(static_cast<int>(semc.emc_dc_tracks[i][j]),cls->dctracks[j],"emc2smb()\t-> emc_dc_tracks",i,j)) && !all) return false;
			}
		}
	// emc2emc
		for(int i=0; i<semc.emc_ncls; i++) {
			GClusterTower *cls = (GClusterTower *)emcRec->emcClusters[i];
	if(!(result&=compareWL(static_cast<int>(semc.emc_emc_ncls[i]),static_cast<int>(cls->Partners.size()),
		static_cast<int>(semc.emc_emc_ncls[i]),NEMC_CLS,"emc2smb()\t-> emc_csi_ncls",i)) && !all) return false;
			if(result) for(int j=0; j<semc.emc_emc_ncls[i]; j++) {
	if(!(result&=compare(static_cast<int>(semc.emc_emc_cls[i][j]),cls->Partners[j]->getIndexEmc(),"emc2smb()\t-> emc_csi_cls",i,j)) && !all) return false;
			}
		}
		return result;
	}

	bool event(const bool all) {
		bool result=true;
		if(!(result&=check_data(all)) && !all) return false;
		if(!(result&=check_emc(all))  && !all) return false;
		if(!(result&=check_str(all))  && !all) return false;
		if(!(result&=check_str_track(all)) &&!all) return false;
		if(!(result&=check_emc2smb(all)) && !all) return false;
		return result;
	}
}
