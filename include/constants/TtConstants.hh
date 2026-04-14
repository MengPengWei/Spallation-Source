#ifndef TT_CONSTANTS_HH
#define TT_CONSTANTS_HH

#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

// -----------------------------------------------------------------------
// 钽靶组件（TT – Tantalum Target）几何与束流参数
// -----------------------------------------------------------------------
// 层次结构（由外到内）：
//   冷却水筒  TT_Water  – G4_WATER
//   包壳筒    TT_Clad   – G4_STAINLESS-STEEL（SS316L）
//   钽靶芯    TT_Target – G4_Ta
// -----------------------------------------------------------------------

// --------------------------
// 钽靶芯尺寸（可调）
// --------------------------
constexpr G4double C_TT_Target_R      = 20.0 * mm;  // 靶芯半径
constexpr G4double C_TT_Target_HalfL  = 50.0 * mm;  // 靶芯半长（沿束流方向）

// --------------------------
// 结构包壳（SS316L 不锈钢）
// --------------------------
constexpr G4double C_TT_Clad_Thickness = 2.0 * mm;  // 包壳厚度

// --------------------------
// 冷却水层
// --------------------------
constexpr G4double C_TT_Water_Thickness = 3.0 * mm;  // 冷却水层厚度

// --------------------------
// 衍生尺寸（计算量）
// --------------------------
// 包壳外半径与半长
constexpr G4double C_TT_Clad_R     = C_TT_Target_R     + C_TT_Clad_Thickness;
constexpr G4double C_TT_Clad_HalfL = C_TT_Target_HalfL + C_TT_Clad_Thickness;

// 冷却水外半径与半长
constexpr G4double C_TT_Water_R     = C_TT_Clad_R     + C_TT_Water_Thickness;
constexpr G4double C_TT_Water_HalfL = C_TT_Clad_HalfL + C_TT_Water_Thickness;

// --------------------------
// 质子束流参数
// --------------------------
// 束流粒子：质子（proton）
// 束流动能：70 MeV（典型中低能散裂源）
// 束流强度单位：particles/s；实际强度在宏/运行参数中指定
//   默认示例：1e13 p/s（中强度散裂源）
constexpr G4double C_TT_Beam_Energy = 70.0 * MeV;   // 质子束动能

// 束流均匀圆形截面半径（应 ≤ 靶面半径，以保证全束打靶）
// 单位：mm；0 表示点源（退化为 G4ParticleGun 默认行为）
constexpr G4double C_TT_Beam_Radius = 18.0 * mm;    // 均匀分布半径

// --------------------------
// 体积名称
// --------------------------
constexpr const char* C_TT_Water_Name  = "TT_Water";
constexpr const char* C_TT_Clad_Name   = "TT_Clad";
constexpr const char* C_TT_Target_Name = "TT_Target";

// --------------------------
// 可视化属性
// --------------------------
namespace TT_Vis
{
    // 冷却水：淡蓝色半透明
    inline static G4VisAttributes TT_Water_Vis(G4Colour(0.2, 0.6, 1.0, 0.3));
    // 包壳：银灰色半透明
    inline static G4VisAttributes TT_Clad_Vis(G4Colour(0.75, 0.75, 0.75, 0.6));
    // 钽靶：蓝紫色半透明（钽为蓝灰色金属）
    inline static G4VisAttributes TT_Target_Vis(G4Colour(0.3, 0.3, 0.8, 0.8));
}

#endif // TT_CONSTANTS_HH
