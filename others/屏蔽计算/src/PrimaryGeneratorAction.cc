//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file PrimaryGeneratorAction.cc
/// \brief Implementation of the PrimaryGeneratorAction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4SingleParticleSource.hh"
#include "G4SPSPosDistribution.hh"
#include "G4SPSAngDistribution.hh"
#include "G4SPSEneDistribution.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* det)
:G4VUserPrimaryGeneratorAction(),
 fDetector(det),
 fBeamRadius(0. * mm),  // test1.i uses point source
 fSourceIntensityPerSecond(1.05238e15),  // neutrons/s
 fIrradiationHours(1000.0),
 fGunMessenger(0)
{
  fParticleSource = new G4GeneralParticleSource();
  SetDefaultKinematic();
    
  //create a messenger for this class
  fGunMessenger = new PrimaryGeneratorMessenger(this);  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleSource;
  delete fGunMessenger;  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::SetDefaultKinematic()
{
  // test1.i neutron source configuration
  // SDEF POS 0 -0.1 0 ERG=D2 PAR=1 WGT=2.0304E+13
  G4ParticleDefinition* particle =
      G4ParticleTable::GetParticleTable()->FindParticle("neutron");
  auto* source = fParticleSource->GetCurrentSource();
  
  // Position: Point source at (0, -21.5, 90) cm - located on the tantalum target ring
  // Y = -21.5 cm places the source within the target ring (r = 185-250 mm)
  source->GetPosDist()->SetPosDisType("Point");
  source->GetPosDist()->SetCentreCoords(G4ThreeVector(0., -21.5*cm, 90*cm));
  
  // Direction: Isotropic (MCNP default when no VEC/DIR specified)
  source->GetAngDist()->SetAngDistType("iso");
  
  // Energy distribution: Histogram from test1.i SI2/SP2
  // SI2 H 9.8189E-6 3.3841 ... 67.682 (21 points)
  // SP2 0 8.4910E-01 ... 8.5482E-06 (probabilities)
  std::vector<G4double> energies = {
    9.8189E-6 * MeV, 3.3841 * MeV, 6.7682 * MeV, 10.152 * MeV, 13.536 * MeV,
    16.921 * MeV, 20.305 * MeV, 23.689 * MeV, 27.073 * MeV, 30.457 * MeV,
    33.841 * MeV, 37.225 * MeV, 40.609 * MeV, 43.993 * MeV, 47.378 * MeV,
    50.762 * MeV, 54.146 * MeV, 57.53 * MeV, 60.914 * MeV, 64.298 * MeV,
    67.682 * MeV
  };
  std::vector<G4double> probs = {
    0.0, 8.4910E-01, 1.0513E-01, 1.5701E-02, 6.2567E-03,
    4.5777E-03, 3.8071E-03, 3.1974E-03, 2.6814E-03, 2.2017E-03,
    1.7869E-03, 1.4490E-03, 1.1559E-03, 9.0826E-04, 7.1639E-04,
    5.3054E-04, 3.6585E-04, 2.4003E-04, 1.3536E-04, 5.1412E-05,
    8.5482E-06
  };
  source->GetEneDist()->SetEnergyDisType("Arb");
  for (size_t i = 0; i < energies.size(); ++i) {
    source->GetEneDist()->ArbEnergyHisto(G4ThreeVector(energies[i], probs[i], 0.));
  }
  source->GetEneDist()->ArbInterpolate("Lin");
  
  source->SetParticleDefinition(particle);
  
  // Update source intensity
  fSourceIntensityPerSecond = 1.05238e15;  // neutrons/s
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  fParticleSource->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

