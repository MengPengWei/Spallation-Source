#ifndef CONSTANTS_HH
#define CONSTANTS_HH

#include <cmath>
#include "G4Types.hh"
#include "G4SystemOfUnits.hh"

#include "G4ThreeVector.hh"
// -----------------------------------------------------------------------
// 几何尺寸常量
// -----------------------------------------------------------------------
constexpr G4double C_World_X = 10.0*m;
constexpr G4double C_World_Y = 10.0*m;
constexpr G4double C_World_Z = 10.0*m;


//bool 开关
constexpr G4bool C_Is_AF = false; // 是否构建活化片胶囊组件
constexpr G4bool C_Is_FC = false; // 是否构建裂变室组件
constexpr G4bool C_Is_CSS = false; // 是否构建CSS组件
constexpr G4bool C_Is_TT = true; // 是否构建钽靶组件


//各组件位置

//CSS,//存在旋转，沿着x轴分布，即旋转为90度
const G4ThreeVector C_CSS_Pos(1.*m, 0, 10*cm);
constexpr G4double C_CSS_degree=90*deg;

//AF
const G4ThreeVector C_AF_Pos(0, 0, 10*cm);
constexpr G4double C_AF_degree=90*deg;

//TT（钽靶）：放置于坐标原点，束流沿 +Z 方向入射
// 靶体尺寸见 TtConstants.hh；此处仅定义位置
const G4ThreeVector C_TT_Pos(0, 0, 0);//TT组件放置于坐标原点，束流沿 +Z 方向入射，照射点为圆盘上的点，具体位置根据需要调整

#endif // CONSTANTS_HH