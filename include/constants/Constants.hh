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
constexpr G4bool is_AF = true; // 是否构建活化片胶囊组件
constexpr G4bool is_FC = false; // 是否构建裂变室组件
constexpr G4bool is_CSS = true; // 是否构建CSS组件
// 钽靶+质子束流模式：
//   true  → 构建钽靶（Ta芯+SS316L包壳+冷却水），束流切换为70MeV均匀质子束
//   false → 保持默认中子束流，不构建钽靶（原有行为不变）
// 修改后需重新编译：mkdir build && cd build && cmake .. && make -j$(nproc)
constexpr G4bool is_TT = false; // 是否构建钽靶+质子束流组件

//各组件位置

//CSS,//存在旋转，沿着x轴分布，即旋转为90度
const G4ThreeVector C_CSS_Pos(1.*m, 0, 10*cm);
constexpr G4double C_CSS_degree=90*deg;

//AF
const G4ThreeVector C_AF_Pos(0, 0, 10*cm);
constexpr G4double C_AF_degree=90*deg;

//TT（钽靶）：放置于坐标原点，束流沿 +Z 方向入射
// 靶体尺寸见 TtConstants.hh；此处仅定义位置
const G4ThreeVector C_TT_Pos(0, 0, 0);

#endif // CONSTANTS_HH