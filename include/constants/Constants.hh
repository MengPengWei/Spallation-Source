#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cmath>
#include "G4SystemOfUnits.hh"


constexpr G4double C_WordSizeX= 10.0*m;
constexpr G4double C_WordSizeY= 10.0*m;
constexpr G4double C_WordSizeZ= 10.0*m;


//有关活化片的物理量,包括胶囊、样品

constexpr G4double C_CapsuleOuterRadius = 13.0*mm;//胶囊外半径
constexpr G4double C_CapsuleHalfZ = 23.0*mm;//胶囊半高
constexpr G4double C_CavityOuterRadius = 6.5*mm;//内腔外半径
constexpr G4double C_CavityHalfZ = 22.0*mm;//内腔半高
constexpr G4double C_SampleOuterRadius = 6.0*mm;//样品外半径
constexpr G4double C_SampleHalfZ = 6.0*mm;//样品半高



#endif // CONSTANTS_HH