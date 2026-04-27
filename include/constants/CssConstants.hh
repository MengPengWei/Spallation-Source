#ifndef CSSCONSTANTS_HH
#define CSSCONSTANTS_HH

#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"  

#include "G4VisAttributes.hh"
//=============================================================
// 反康普顿系统 CSS 几何参数
// 由内向外：HPGe → 铝壳 → BGO阵列 → 铅屏蔽体 → 铅准直器
// 布局：准直器在最前端（+Z），后面依次是HPGe/Al/BGO，最后是后端屏蔽
// 零重叠、零错误、不崩溃
//=============================================================

// ========================
// 1) HPGe 晶体（中心）
// ========================
constexpr G4double C_CSS_HPGe_OutD = 63.1*mm;
constexpr G4double C_CSS_HPGe_L   = 63.0*mm;

// ========================
// 2) 铝层 (包裹 HPGe)
// ========================
constexpr G4double C_CSS_Al_T     = 3.0*mm;    // 铝层厚度（合理厚度）
constexpr G4double C_CSS_Al_InD   = C_CSS_HPGe_OutD;
constexpr G4double C_CSS_Al_OutD  = C_CSS_Al_InD + 2*C_CSS_Al_T;
constexpr G4double C_CSS_Al_L     = 100.0*mm;   // 长度包裹HPGe

// ========================
// 3) BGO 8瓣扇形探测器
// ========================
constexpr G4int    C_CSS_BGO_n = 8;
constexpr G4double C_CSS_BGO_T      = 40.0*mm;   // BGO径向厚度
constexpr G4double C_CSS_BGO_InD    = C_CSS_Al_OutD;
constexpr G4double C_CSS_BGO_OutD   = C_CSS_BGO_InD + 2*C_CSS_BGO_T;
constexpr G4double C_CSS_BGO_L      = 100.0*mm;
constexpr G4double C_CSS_BGO_Angle   = 360.0*deg / C_CSS_BGO_n;

// ========================
// 4) 准直器 (最前端 +Z)
// ========================
constexpr G4double C_CSS_Collimator_InD  = 31.0*mm;  // 小孔
constexpr G4double C_CSS_Collimator_OutD = C_CSS_BGO_OutD;
constexpr G4double C_CSS_Collimator_L    = 127.0*mm;

// ========================
// 5) 铅屏蔽体 (后端)
// ========================
constexpr G4double C_CSS_Shield_InD   = C_CSS_BGO_OutD;
constexpr G4double C_CSS_Shield_OutD   = C_CSS_Shield_InD + 100*mm;
constexpr G4double C_CSS_Shield_L      = C_CSS_Collimator_L+C_CSS_BGO_L+20*mm; // 足够包裹BGO和准直器

// ========================
// 6) CSS 总外壳
// ========================
constexpr G4double C_CSS_D = C_CSS_Shield_OutD;
constexpr G4double C_CSS_L = C_CSS_Shield_L;

// ========================
// ========================
const G4ThreeVector C_CSS_Collimator_Pos (0, 0,  C_CSS_L/2 - C_CSS_Collimator_L/2);
const G4ThreeVector C_CSS_BGO_Pos       (0, 0,  C_CSS_Collimator_Pos.z() - C_CSS_Collimator_L/2 - C_CSS_BGO_L/2);
const G4ThreeVector C_CSS_Al_Pos         (0, 0,  C_CSS_BGO_Pos.z());
const G4ThreeVector C_CSS_HPGe_Pos       (0, 0,  C_CSS_BGO_Pos.z());
const G4ThreeVector C_CSS_Shield_Pos     (0, 0, 0);

//整体系统坐标,位于x轴方向，距离世界坐标原点1m处，方



//可视化
namespace CSS_Vis
{
    inline static G4VisAttributes C_CSS_Vis(G4Colour(0.0, 0.0, 0.0, 1.0)); // CSS系统设置为黑色
    inline static G4VisAttributes C_CSS_BGO_Vis(G4Colour(0.0, 1.0, 0.0, 1.0)); // BGO探测器设置为绿色
    inline static G4VisAttributes C_CSS_Al_Vis(G4Colour(1.0, 0.0, 0.0, 1.0)); // 铝层设置为红色
    inline static G4VisAttributes C_CSS_HPGe_Vis(G4Colour(0.0, 0.0, 1.0, 1.0)); // HPGe探测器设置为蓝色
    inline static G4VisAttributes C_CSS_Shield_Vis(G4Colour(0.5, 0.5, 0.5, 1.0)); // 屏蔽体设置为灰色
    inline static G4VisAttributes C_CSS_Collimator_Vis(G4Colour(1.0, 1.0, 0.0, 1.0)); // 准直器设置为黄色
}
    

#endif