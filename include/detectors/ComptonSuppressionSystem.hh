#ifndef COMPTON_SUPPRESSION_SYSTEM_HH
#define COMPTON_SUPPRESSION_SYSTEM_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"

/// Builds a simplified capsule (CFC graphite shell) containing an air cavity
/// with 3 cylindrical sample volumes (Indium) placed along Z.
/// No boolean solids are used.
class ComptonSuppressionSystem
{
public:
    ComptonSuppressionSystem();
    ~ComptonSuppressionSystem() = default;

    /// Build and place the full capsule assembly into motherLV at pos.
    /// Returns the shared sample logical volume (all 3 cylinders use same LV).
    G4LogicalVolume* Build(G4LogicalVolume* motherLV,
                            const G4ThreeVector& pos = G4ThreeVector(0,0,0));

    G4LogicalVolume* GetSampleLV() const { return fSampleLV;}

private:
    G4LogicalVolume* fSampleLV = nullptr;
};

#endif // COMPTON_SUPPRESSION_SYSTEM_HH
