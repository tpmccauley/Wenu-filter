import FWCore.ParameterSet.Config as cms

WenuFilter = cms.EDFilter('WenuFilter',
                           electronInputTag = cms.InputTag("gsfElectrons"),
                           pfMETInputTag = cms.InputTag("pfMet"),
                           csvFileName = cms.string("Wenu.csv"),
                           minElectronPt = cms.double(20.0)
                          )
