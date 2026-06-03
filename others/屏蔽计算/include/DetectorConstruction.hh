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
/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4Cache.hh"
#include <array>

class G4Box;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class DetectorMessenger;
class G4VSolid;

const G4int kMaxAbsor = 10;                        // 0 + 9

class G4GlobalMagFieldMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  
  DetectorConstruction();
 ~DetectorConstruction();

public:

  G4Material* 
  MaterialWithSingleIsotope(G4String, G4String, G4double, G4int, G4int);
  
  void SetNbOfAbsor     (G4int);      
  void SetAbsorMaterial (G4int,const G4String&);     
  void SetAbsorThickness(G4int,G4double);
                
  void SetAbsorSizeYZ   (G4double);          
     
  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();
  void SetDoorOpen(G4bool value);
  G4bool IsDoorOpen() const { return fDoorOpen; }
  void SetHotcellWallThickness(G4double value);
  G4double GetHotcellWallThickness() const { return fHotcellWallThickness; }
     
public:

  G4int       GetNbOfAbsor()             {return fNbOfAbsor;}     
  G4Material* GetAbsorMaterial (G4int i) {return fAbsorMaterial[i];};
  G4double    GetAbsorThickness(G4int i) {return fAbsorThickness[i];};      
  G4double    GetXfront        (G4int i) {return fXfront[i];};
            
  G4double GetAbsorSizeX()               {return fAbsorSizeX;}; 
  G4double GetAbsorSizeYZ()              {return fAbsorSizeYZ;};
  
  G4double GetWorldSizeX()               {return fWorldSizeX;}; 
  G4double GetWorldSizeYZ()              {return fWorldSizeYZ;}; 
  
  void PrintParameters();
   
private:

  G4int              fNbOfAbsor;
  G4Material*        fAbsorMaterial [kMaxAbsor];
  G4double           fAbsorThickness[kMaxAbsor];
  G4double           fXfront[kMaxAbsor];  

  G4double           fAbsorSizeX;
  G4double           fAbsorSizeYZ;
  
  G4double           fWorldSizeX;
  G4double           fWorldSizeYZ;  
  G4Material*        fDefaultMaterial;  
  
  G4VPhysicalVolume* fPhysiWorld;

  DetectorMessenger* fDetectorMessenger;
  G4Cache<G4GlobalMagFieldMessenger*> fFieldMessenger;
  G4bool             fDoorOpen;
  G4double           fHotcellWallThickness;

private:

  void DefineMaterials();
  void ComputeParameters();
  G4VPhysicalVolume* ConstructVolumes();
  void PlaceLayerWithOpenings(const G4String& name,
                              G4Material* material,
                              const std::array<G4double, 6>& rpp,
                              G4LogicalVolume* mother,
                              G4int copyNo);
  std::array<G4double, 6> GetRpp(G4int surfaceId) const;

  // 钽靶构造方法 - 从P_Ta_Channel_Angular集成
  void ConstructTantalumTarget(G4LogicalVolume* worldLogic);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

