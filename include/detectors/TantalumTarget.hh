#ifndef TANTALUM_TARGET_HH
#define TANTALUM_TARGET_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4Material.hh"

// -----------------------------------------------------------------------
// TantalumTarget – 钽靶几何构建器
//
// 几何层次（外 → 内）：
//   TT_Water  (G4_WATER)           冷却水通道
//   TT_Clad   (G4_STAINLESS-STEEL) 结构包壳（SS316L）
//   TT_Target (G4_Ta)              钽靶芯
//
// 用法：
//   TantalumTarget tt;
//   tt.Build(worldLV, G4ThreeVector(0,0,0));
// -----------------------------------------------------------------------

class TantalumTarget
{
public:
    TantalumTarget();
    ~TantalumTarget() = default;

    /// 在母体积 motherLV 的 pos 位置构建钽靶组件，返回冷却水外筒逻辑体。
    G4LogicalVolume* Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos);

    // 访问各层逻辑体（Build 之后有效）
    G4LogicalVolume* Get_LV_Water()  const { return TT_Water_LV;  }
    G4LogicalVolume* Get_LV_Clad()   const { return TT_Clad_LV;   }
    G4LogicalVolume* Get_LV_Target() const { return TT_Target_LV; }

private:
    // 材料
    G4Material* TT_Water_mat  = nullptr;
    G4Material* TT_Clad_mat   = nullptr;
    G4Material* TT_Target_mat = nullptr;

    // 固体
    G4Tubs* TT_Water_SV  = nullptr;
    G4Tubs* TT_Clad_SV   = nullptr;
    G4Tubs* TT_Target_SV = nullptr;

    // 逻辑体
    G4LogicalVolume* TT_Water_LV  = nullptr;
    G4LogicalVolume* TT_Clad_LV   = nullptr;
    G4LogicalVolume* TT_Target_LV = nullptr;
};

#endif // TANTALUM_TARGET_HH
