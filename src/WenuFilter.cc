//
// Original Author: thomas.mccauley@cern.ch
//

#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"

#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/PFMETCollection.h"

#include <iostream>
#include <string>
#include <fstream>

class WenuFilter : public edm::EDFilter {
public:
  explicit WenuFilter(const edm::ParameterSet&);
  ~WenuFilter();

private:
  virtual void beginJob();
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  edm::InputTag electronInputTag_;
  edm::InputTag pfMETInputTag_;

  std::ofstream csvOut_;
  std::string csvFileName_;

  double minElectronPt_;
};

WenuFilter::WenuFilter(const edm::ParameterSet& iConfig)
  : electronInputTag_(iConfig.getParameter<edm::InputTag>("electronInputTag")),
    pfMETInputTag_(iConfig.getParameter<edm::InputTag>("pfMETInputTag")),
    csvFileName_(iConfig.getParameter<std::string>("csvFileName")),
    minElectronPt_(iConfig.getParameter<double>("minElectronPt"))
{
  csvOut_.open(csvFileName_.c_str());
}

WenuFilter::~WenuFilter()
{}

bool
WenuFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<reco::GsfElectronCollection> electrons;
  iEvent.getByLabel(electronInputTag_, electrons);

  if ( ! electrons.isValid() )
  {  
    std::cerr<<"WenuFilter: invalid collection"<<std::endl;
    return false;
  }

  edm::Handle<reco::PFMETCollection> pfMets;
  iEvent.getByLabel(pfMETInputTag_, pfMets);

  if ( ! pfMets.isValid() ) 
  {    
    std::cerr<<"WenuFilter: invalid PFMET collection"<<std::endl;
    return false;
  }
  
  if ( pfMets->size() != 1 )
    return false;

  reco::PFMETCollection::const_iterator it = pfMets->begin();
  double phiMET = (*it).phi();
  double MET = (*it).pt();
  
  double pt, eta, phi, charge;
  unsigned int nElectrons = 0;
  float sigmaEtaEta, hcalOverEcal, isoTrack, isoECAL, isoHCAL;
  double delEta = 0.0;
  double delPhi = 0.0;
  std::string type;

  for ( reco::GsfElectronCollection::const_iterator ei = electrons->begin(), eie = electrons->end();
        ei != eie; ++ei )
  {     
    pt = ei->pt();
    
    if ( pt < minElectronPt_ )
      continue;

    if ( pt > 20 && nElectrons != 0 )
      return false;
    
    if ( pt > minElectronPt_ ) 
    {
      sigmaEtaEta = ei->sigmaEtaEta();
      hcalOverEcal = ei->hcalOverEcal();
      
      isoTrack = ei->dr04TkSumPt();
      isoECAL = ei->dr04EcalRecHitSumEt(); 
      isoHCAL = ei->dr04HcalTowerSumEt();

      if ( ei->isEB() ) 
      {
        type = "EB";
        
        //if ( sigmaEtaEta > 0.01 && hcalOverEcal > 0.15 && isoTrack > 0.15 && isoECAL > 2.0 && isoHCAL > 0.12 )
        //  return false;
      }
      
      if ( ei->isEE() ) 
      {  
        type = "EE";

        //if ( sigmaEtaEta > 0.03 && hcalOverEcal > 0.07 && isoTrack > 0.08 && isoECAL > 0.06 && isoHCAL > 0.05 )
        //  return false;
      }

      nElectrons++;
      eta = ei->eta();
      phi = ei->phi();
      charge = ei->charge();
    }    
  }
  
  if ( nElectrons == 1 ) 
  {
    csvOut_<< iEvent.id().run()<<","<< iEvent.id().event() <<","<<  pt <<","<< eta <<","<< phi <<","<< charge <<","<<
      type <<","<< delEta <<","<< delPhi <<","<< sigmaEtaEta <<","<< hcalOverEcal <<","<< 
      isoTrack <<","<< isoECAL <<","<< isoHCAL <<","<<  MET <<","<< phiMET <<std::endl;
    return true;
  }
  
  return false;
}

void 
WenuFilter::beginJob()
{
  csvOut_<<"Run,Event,pt,eta,phi,Q,type,delEta,delPhi,sigmaEtaEta,HoverE,isoTrack,isoEcal,isoHcal,MET,phiMET"<<std::endl;
}

void 
WenuFilter::endJob() 
{
  csvOut_.close();
}

DEFINE_FWK_MODULE(WenuFilter);
