#ifndef CSSCONSTANTS_HH
#define CSSCONSTANTS_HH

#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

//=============================================================
// 反康普顿系统 CSS 几何参数
// 由内向外：HPGe → 铝壳 → BGO阵列 → 铅屏蔽体 → 铅准直器
// 布局：准直器在最前端（+Z），后面依次是HPGe/Al/BGO，最后是后端屏蔽
// 零重叠、零错误、不崩溃
//=============================================================

// ========================
// 1) HPGe 晶体（中心）
// ========================
constexpr G4double C_HPGe_OutD = 63.1*mm;
constexpr G4double C_HPGe_L   = 63.0*mm;

// ========================
// 2) 铝层 (包裹 HPGe)
// ========================
constexpr G4double C_Al_T     = 3.0*mm;    // 铝层厚度（合理厚度）
constexpr G4double C_Al_InD   = C_HPGe_OutD;
constexpr G4double C_Al_OutD  = C_Al_InD + 2*C_Al_T;
constexpr G4double C_Al_L     = 100.0*mm;   // 长度包裹HPGe

// ========================
// 3) BGO 8瓣扇形探测器
// ========================
constexpr G4int    C_BGONumber  = 8;
constexpr G4double C_BGO_T      = 40.0*mm;   // BGO径向厚度
constexpr G4double C_BGO_InD    = C_Al_OutD;
constexpr G4double C_BGO_OutD   = C_BGO_InD + 2*C_BGO_T;
constexpr G4double C_BGO_L      = 100.0*mm;
constexpr G4double C_BGOAngle   = 360.0*deg / C_BGONumber;

// ========================
// 4) 准直器 (最前端 +Z)
// ========================
constexpr G4double C_Collimator_InD  = 31.0*mm;  // 小孔
constexpr G4double C_Collimator_OutD = C_BGO_OutD;
constexpr G4double C_Collimator_L    = 127.0*mm;

// ========================
// 5) 铅屏蔽体 (后端)
// ========================
constexpr G4double C_Shield_InD   = C_BGO_OutD;
constexpr G4double C_Shield_OutD   = C_Shield_InD + 100*mm;
constexpr G4double C_Shield_L      = C_Collimator_L+C_BGO_L+20*mm; // 足够包裹BGO和准直器

// ========================
// 6) CSS 总外壳
// ========================
constexpr G4double C_CSS_D = C_Shield_OutD;
constexpr G4double C_CSS_L = C_Shield_L;

// ========================
// 🎯 Z 轴位置（完全对齐，零重叠）
// ========================
const G4ThreeVector pos_CSS_Collimator (0, 0,  C_CSS_L/2 - C_Collimator_L/2);
const G4ThreeVector pos_CSS_BGO       (0, 0,  pos_CSS_Collimator.z() - C_Collimator_L/2 - C_BGO_L/2);
const G4ThreeVector pos_CSS_Al         (0, 0,  pos_CSS_BGO.z());
const G4ThreeVector pos_CSS_HPGe       (0, 0,  pos_CSS_BGO.z());
const G4ThreeVector pos_CSS_Shield     (0, 0, 0);

#endif