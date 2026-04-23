// PgConstants.hh
#ifndef PGCONSTANTS_HH
#define PGCONSTANTS_HH

#include "G4Types.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// 用于设置 ParticleGun 的常量定义
constexpr G4double C_PG_Energy      = 70.0*MeV;

// 入射点（全局坐标）
constexpr G4double C_PG_Position_Z  = -10.0*cm;
constexpr G4double C_PG_Center_X    = 0.0*cm;
constexpr G4double C_PG_Center_Y    = -21.5 * cm; // 调整为与 TT 组件中心对齐

// “模仿上文”：方形范围内二维高斯（截断）
constexpr G4double C_PG_HalfSide    = 2.5*cm;        // 方形半边长
constexpr G4double C_PG_FWHM        = 7.33448*mm;    // FWHM

const G4ThreeVector C_PG_Direction(0, 0, 1);

#endif // PGCONSTANTS_HH