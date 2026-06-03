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
/// \file PrimaryGeneratorMessenger.cc
/// \brief Implementation of the PrimaryGeneratorMessenger class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PrimaryGeneratorMessenger.hh"

#include "PrimaryGeneratorAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(
                                                   PrimaryGeneratorAction* Gun)
:G4UImessenger(),
 fAction(Gun),
 fGunDir(0),
 fDefaultCmd(0),
 fBeamRadiusCmd(0),
 fSourceIntensityCmd(0),
 fIrradiationHoursCmd(0)
{ 
  fGunDir = new G4UIdirectory("/shield/gun/");
  fGunDir->SetGuidance("gun control");

  fDefaultCmd = new G4UIcmdWithoutParameter("/shield/gun/setDefault",this);
  fDefaultCmd->SetGuidance("set/reset kinematic defined in PrimaryGenerator");
  fDefaultCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  fBeamRadiusCmd = new G4UIcmdWithADoubleAndUnit("/shield/gun/beamRadius",this);
  fBeamRadiusCmd->SetGuidance("Set beam radius (default 25 mm).");
  fBeamRadiusCmd->SetParameterName("beamRadius",false);
  fBeamRadiusCmd->SetRange("beamRadius>=0.");
  fBeamRadiusCmd->SetUnitCategory("Length");
  fBeamRadiusCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  fSourceIntensityCmd = new G4UIcmdWithADouble("/shield/gun/sourceIntensity",this);
  fSourceIntensityCmd->SetGuidance("Set source intensity [proton/s].");
  fSourceIntensityCmd->SetParameterName("sourceIntensity",false);
  fSourceIntensityCmd->SetRange("sourceIntensity>0.");
  fSourceIntensityCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  fIrradiationHoursCmd = new G4UIcmdWithADouble("/shield/gun/irradiationHours",this);
  fIrradiationHoursCmd->SetGuidance("Set irradiation duration [h].");
  fIrradiationHoursCmd->SetParameterName("irradiationHours",false);
  fIrradiationHoursCmd->SetRange("irradiationHours>0.");
  fIrradiationHoursCmd->AvailableForStates(G4State_PreInit,G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger()
{
  delete fDefaultCmd;
  delete fBeamRadiusCmd;
  delete fSourceIntensityCmd;
  delete fIrradiationHoursCmd;
  delete fGunDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command,
                                               G4String newValue)
{ 
  if (command == fDefaultCmd)
   {fAction->SetDefaultKinematic();}
  
  if (command == fBeamRadiusCmd)
   {fAction->SetBeamRadius(fBeamRadiusCmd->GetNewDoubleValue(newValue));}

  if (command == fSourceIntensityCmd)
   {fAction->SetSourceIntensity(fSourceIntensityCmd->GetNewDoubleValue(newValue));}

  if (command == fIrradiationHoursCmd)
   {fAction->SetIrradiationHours(fIrradiationHoursCmd->GetNewDoubleValue(newValue));}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

