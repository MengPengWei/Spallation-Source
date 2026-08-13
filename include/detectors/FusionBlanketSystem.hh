#ifndef FUSIONBLANKETSYSTEM_HH
#define FUSIONBLANKETSYSTEM_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"

class FusionBlanketSystem
{
public:
    FusionBlanketSystem() = default;
    ~FusionBlanketSystem() = default;

    void Build(G4LogicalVolume* worldLV, const G4ThreeVector& center);

    G4LogicalVolume* GetFirstWallLV() const { return fFirstWallLV; }
    G4LogicalVolume* GetBlanketLV() const { return fBlanketLV; }
    G4LogicalVolume* GetLiGlassDetLV() const { return fLiGlassDetLV; }
    G4LogicalVolume* GetLiDiamondDetLV() const { return fLiDiamondDetLV; }

private:
    G4LogicalVolume* fFirstWallLV = nullptr;
    G4LogicalVolume* fBlanketLV = nullptr;
    G4LogicalVolume* fLiGlassDetLV = nullptr;
    G4LogicalVolume* fLiDiamondDetLV = nullptr;
};

#endif // FUSIONBLANKETSYSTEM_HH
