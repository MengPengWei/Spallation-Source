#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cmath>
#include "G4Types.hh"
#include "G4SystemOfUnits.hh"


// -----------------------------------------------------------------------
// 几何尺寸常量
// -----------------------------------------------------------------------
constexpr G4double C_WordSizeX= 10.0*m;
constexpr G4double C_WordSizeY= 10.0*m;
constexpr G4double C_WordSizeZ= 10.0*m;

// 有关活化片的物理量：胶囊、内腔、样品
constexpr G4double C_CapsuleOuterRadius = 13.0*mm; // 胶囊外半径
constexpr G4double C_CapsuleHalfZ       = 23.0*mm; // 胶囊半高
constexpr G4double C_CavityOuterRadius  =  6.5*mm; // 内腔外半径
constexpr G4double C_CavityHalfZ        = 22.0*mm; // 内腔半高
constexpr G4double C_SampleOuterRadius  =  6.0*mm; // 样品外半径
constexpr G4double C_SampleHalfZ        =  6.0*mm; // 样品半高

// 样品数量（一个胶囊内）
constexpr G4int    C_NSamples           = 3;

// -----------------------------------------------------------------------
// 几何体名称常量（solid / logical / physical volume 统一使用）
// -----------------------------------------------------------------------
constexpr const char* kWorldName        = "World";        // 世界体
constexpr const char* kCapsuleShellName = "CapsuleShell";  // 胶囊外壳
constexpr const char* kCavityName       = "CapsuleCavity"; // 胶囊内腔（空气）
constexpr const char* kSampleName       = "Sample";        // 样品圆柱体


#endif // CONSTANTS_HH