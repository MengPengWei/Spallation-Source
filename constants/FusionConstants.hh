#ifndef FUSIONCONSTANTS_HH
#define FUSIONCONSTANTS_HH

#include "G4Types.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// Fusion scenario switch
constexpr G4bool C_Is_FUSION = false;

// DT/DD source switch
constexpr G4bool C_FUSION_USE_DT = true; // false -> DD

// Source configuration
constexpr G4double C_FUSION_DT_Energy = 14.1 * MeV;
constexpr G4double C_FUSION_DD_Energy = 2.45 * MeV;
constexpr G4double C_FUSION_SourceR   = 20.0 * cm;

// Cylindrical fusion structure
const G4ThreeVector C_FUSION_Center(0., 0., 0.);
constexpr G4double C_FUSION_HalfZ            = 80.0 * cm;
constexpr G4double C_FUSION_PlasmaRadius     = 40.0 * cm;
constexpr G4double C_FUSION_FirstWallThick   = 2.0 * cm;
constexpr G4double C_FUSION_BlanketThick     = 40.0 * cm;

// Tritium detector geometry
constexpr G4double C_FUSION_DetectorGap      = 3.0 * cm;
constexpr G4double C_FUSION_DetectorRadius   = 5.0 * cm;
constexpr G4double C_FUSION_DetectorHalfZ    = 2.0 * mm;

#endif // FUSIONCONSTANTS_HH
