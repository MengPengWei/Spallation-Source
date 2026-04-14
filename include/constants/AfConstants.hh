#ifndef AF_CONSTANTS_HH
#define AF_CONSTANTS_HH

#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

// -----------------------------------------------------------------------
// 活化片组件（AF）几何参数
// -----------------------------------------------------------------------
// CFC 胶囊：φ26mm × 34mm
// 内部样品：
//   - In115：φ12mm × 24mm
//   - Si/Cu：φ10mm × 15mm
// -----------------------------------------------------------------------

// --------------------------
// CFC 碳/碳复合材料胶囊外壳
// --------------------------
constexpr G4double C_AF_CFC_OutD = 26 * mm;
constexpr G4double C_AF_CFC_L    = 34 * mm;

// --------------------------


//空气间隙
constexpr G4double C_AF_Air_OutD_Thickness= 0.1 * mm;
constexpr G4double C_AF_Air_L_Thickness    = 0.1 * mm;

// 样品尺寸
// --------------------------
// In115 样品
constexpr G4double C_AF_In_OutD = 12 * mm;
constexpr G4double C_AF_In_L    = 24 * mm;

// Si / Cu 样品
constexpr G4double C_AF_SiCu_OutD = 10 * mm;
constexpr G4double C_AF_SiCu_L    = 15 * mm;

// --------------------------
// 体积名称
// --------------------------

constexpr const char* C_AF_CFC_Name = "AF_CFC";
constexpr const char* C_AF_Air_Name = "AF_Air";
constexpr const char* C_AF_In_Name  = "AF_In";
constexpr const char* C_AF_Si_Name  = "AF_Si";
constexpr const char* C_AF_Cu_Name  = "AF_Cu";

// --------------------------
// 样品开关（三选一）
// --------------------------
constexpr G4bool is_AF_In = false;
constexpr G4bool is_AF_Si = true;
constexpr G4bool is_AF_Cu = false;

// --------------------------
// 可视化属性（修复 static 冲突）
// --------------------------





namespace AF_Vis
{
    //颜色四个参数，分别为红绿蓝（RGB）和透明度（alpha），范围都是0-1
    inline static G4VisAttributes AF_Air_Vis(G4Colour(0, 0, 0, 0));
    // CFC 胶囊：灰色半透明
    inline static G4VisAttributes AF_CFC_Vis(G4Colour(0.4, 0.4, 0.4, 0.5));
    // In 样品：淡紫色半透明
    inline static G4VisAttributes AF_In_Vis(G4Colour(1, 0.9, 1.0, 0.8));
    // Si 样品：银白色
    inline static G4VisAttributes AF_Si_Vis(G4Colour(0.92, 0.92, 0.92, 0.8));
    // Cu 样品：铜橙色
    inline static G4VisAttributes AF_Cu_Vis(G4Colour(1.0, 0.65, 0.0, 0.8));
}

#endif // AF_CONSTANTS_HH