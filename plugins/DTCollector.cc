#include "L1Trigger/L1IntegratedMuonTrigger/interface/DTCollector.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambPhContainer.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambThContainer.h"
#include "DataFormats/MuonDetId/interface/DTChamberId.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

using namespace L1ITMu;

DTCollector::DTCollector( const edm::ParameterSet& ps ):
  SubsystemCollector(ps) {
}

void DTCollector::extractPrimitives(const edm::Event& ev, 
				    const edm::EventSetup& es, 
				    std::vector<TriggerPrimitive>& out) const {
  edm::Handle<L1MuDTChambPhContainer> phiDigis;
  edm::Handle<L1MuDTChambThContainer> thetaDigis;
  ev.getByLabel(_src,phiDigis);
  ev.getByLabel(_src,thetaDigis);

  for( int wheel = -2; wheel <= 2 ; ++wheel ) {
    for( int station = 1; station <= 4; ++station ) {
      for( int sector = 1; sector <= 12; ++sector ) {
	for( int bx = bx_min; bx <= bx_max; ++bx) {	  
	  std::unique_ptr<L1MuDTChambPhDigi> phi_segm_1(
	    phiDigis->chPhiSegm1(wheel,station,sector,bx)
	    );
	  std::unique_ptr<L1MuDTChambPhDigi> phi_segm_2(
	    phiDigis->chPhiSegm2(wheel,station,sector,bx)
	    );
	  std::unique_ptr<L1MuDTChambThDigi> theta_segm(
	    thetaDigis->chThetaSegm(wheel,station,sector,bx)
	    );
	  
	  if( phi_segm_1 ) {
	    const int bti_group = findBTIGroupForThetaDigi(*theta_segm,1);
	    if( bti_group != -1 ) {
	      out.push_back(processDigis(*phi_segm_1,*theta_segm,bti_group));
	    } else {
	      out.push_back(processDigis(*phi_segm_1,1));
	    }
	  }
	  
	  if( phi_segm_2 ) {
	    const int bti_group = findBTIGroupForThetaDigi(*theta_segm,2);
	    if( bti_group != -1 ) {
	      out.push_back(processDigis(*phi_segm_2,*theta_segm,bti_group));
	    } else {
	      out.push_back(processDigis(*phi_segm_2,2));
	    }
	  }

	  phi_segm_1.release();
	  phi_segm_2.release();
	  theta_segm.release();
	}
      }
    }
  }
}

TriggerPrimitive DTCollector::processDigis(const L1MuDTChambPhDigi& digi,
					   const int segment_number) const {
  DTChamberId detid(digi.whNum(),digi.stNum(), digi.scNum());
  return TriggerPrimitive(detid,digi,segment_number);
}

TriggerPrimitive DTCollector::processDigis(const L1MuDTChambPhDigi& digi_phi,
					   const L1MuDTChambThDigi& digi_theta,
					   const int bti_group) const {
  DTChamberId detid(digi_phi.whNum(),digi_phi.stNum(), digi_phi.scNum());
  return TriggerPrimitive(detid,digi_phi,digi_theta,bti_group);
}

int DTCollector::
findBTIGroupForThetaDigi(const L1MuDTChambThDigi& digi,
			 const int pos) const {
  int result = -1;
  for( int i = 0; i < 7; ++i ) {
    if( digi.position(i) == pos ) result = i;
  }
  return result;
}

#include "L1Trigger/L1IntegratedMuonTrigger/interface/SubsystemCollectorFactory.h"
DEFINE_EDM_PLUGIN( SubsystemCollectorFactory, DTCollector, "DTCollector");
