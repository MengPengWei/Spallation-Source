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
//各圆环内外径（可调，单位mm）：
constexpr G4double C_TT_InD  = 370 * mm;   // 靶芯内径（空心时有效）
constexpr G4double C_TT_OutD = 500 * mm;
// --------------------------
// 第一层钽靶芯尺寸
// --------------------------

constexpr G4double C_TT_Target1_L    = 3.9 * mm; // 靶芯长度

// 第二层钽靶芯尺寸

constexpr G4double C_TT_Target2_L    = 1.3 * mm;

// 第二层内设置两个水道，成多扇形分布，单个扇形内弧度为0.4mm,厚度0.5mm，上下水道相交叉
constexpr G4double C_TT_Water_Channel_Arc = 0.4 * mm; // 水道弧长
constexpr G4double C_TT_Water_Channel_Thickness = 0.5 * mm; // 水道厚度


//第二层坦靶和第三层坦靶中间为冷却水

constexpr G4double C_TT_Water_L = 4.5 * mm; // 冷却水筒长度 

//第三层坦靶尺寸（可调）
constexpr G4double C_TT_Target3_L    = 1.5 * mm;



//各层在TT_LV中的位置（相对于母体积中心，单位mm）：从下到上依次排布

const G4ThreeVector C_TT_Target1_Pos(0, 0, - (C_TT_Target1_L + C_TT_Target2_L + C_TT_Water_L + C_TT_Target3_L)/2 + C_TT_Target1_L/2);// 第一层钽靶芯位置,位于最下方
const G4ThreeVector C_TT_Target2_Pos(0, 0, C_TT_Target1_Pos.z() + C_TT_Target1_L/2 + C_TT_Target2_L/2); // 第二层钽靶芯位置，位于第一层上方
const G4ThreeVector C_TT_Water_Pos(0, 0, C_TT_Target2_Pos.z() + C_TT_Target2_L/2 + C_TT_Water_L/2); // 冷却水筒位置，位于第二层上方
const G4ThreeVector C_TT_Target3_Pos(0, 0, C_TT_Water_Pos.z() + C_TT_Water_L/2 + C_TT_Target3_L/2); // 第三层钽靶芯位置，位于冷却水筒上方


// --------------------------
//水道位置：
//水道母体为第二层钽靶芯，水道相对于第二层钽靶芯中心的位置（相对于第二层钽靶芯中心，单位mm）：

//根据弧长计算弧度值，放置两层水道，第一层水道在下部，第二层水道在上部，错位分布。
const G4double C_TT_Water_Channel_Arc_Rad = C_TT_Water_Channel_Arc / (C_TT_OutD/2); // 水道弧长对应的弧度
const G4double C_TT_Water_Channel_Degree = C_TT_Water_Channel_Arc_Rad * 180.0 / CLHEP::pi; // 水道弧长对应的角度

const G4double C_TT_Water_Channel1_Z = -C_TT_Target2_L/2+C_TT_Water_Channel_Thickness/2; // 水道1  Z位置
const G4double C_TT_Water_Channel2_Z = C_TT_Target2_L/2-C_TT_Water_Channel_Thickness/2; // 水道2  Z位置

const G4double C_TT_Water_Channel_Eps  = 1e-6; // 或者 1e-4，按需要调整，确保水道之间没有重叠







// --------------------------
// 体积名称
// --------------------------
// constexpr const char* C_TT_Name = "TantalumTarget";
// constexpr const char* C_TT_Water1_Name = "TT_Water_Channel1";
// constexpr const char* C_TT_Water2_Name = "TT_Water_Channel2";
// constexpr const char* C_TT_Water3_Name  = "TT_Water";
// constexpr const char* C_TT_Cu_Name   = "TT_Cu";
// constexpr const char* C_TT_Target1_Name = "TT_Target1";
// constexpr const char* C_TT_Target2_Name = "TT_Target2";
// constexpr const char* C_TT_Target3_Name = "TT_Target3";

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
