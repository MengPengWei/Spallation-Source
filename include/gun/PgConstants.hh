// PgConstants.hh
#ifndef PGCONSTANTS_HH
#define PGCONSTANTS_HH

#include "G4Types.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

// 用于设置 各种类型的ParticleGun 的开关，以及一些相关的常量（如束斑半径等）

// 开关唯一
constexpr G4bool PG_Is_default=false;//使用默认发射方式 
constexpr G4bool PG_Is_ProtonGun = true; // 是否使用 ParticleGun（默认不使用，使用内置的 PrimaryGeneratorAction）


#endif // PGCONSTANTS_HH