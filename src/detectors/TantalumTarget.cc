#include "TantalumTarget.hh"
#include "TtConstants.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

TantalumTarget::TantalumTarget()
{}

G4LogicalVolume* TantalumTarget::Build(G4LogicalVolume* motherLV,
                                       const G4ThreeVector& pos)
{
    auto nist = G4NistManager::Instance();

    // ----------------------------------------------------------------
    // 材料
    // ----------------------------------------------------------------
    TT_Water_mat  = nist->FindOrBuildMaterial("G4_WATER");
    TT_Clad_mat   = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL"); // SS316L
    TT_Target_mat = nist->FindOrBuildMaterial("G4_Ta");              // 钽

    // ----------------------------------------------------------------
    // 冷却水外筒（最外层）
    //   半径 = 靶半径 + 包壳厚度 + 冷却水厚度 = C_TT_Water_R
    //   半长 = 靶半长 + 包壳厚度 + 冷却水厚度 = C_TT_Water_HalfL
    // ----------------------------------------------------------------
    TT_Water_SV = new G4Tubs(C_TT_Water_Name,
                              0.*mm, C_TT_Water_R,
                              C_TT_Water_HalfL,
                              0., 360.*deg);

    TT_Water_LV = new G4LogicalVolume(TT_Water_SV, TT_Water_mat, C_TT_Water_Name);
    TT_Water_LV->SetVisAttributes(&TT_Vis::TT_Water_Vis);

    new G4PVPlacement(nullptr,
                      pos,
                      TT_Water_LV,
                      C_TT_Water_Name,
                      motherLV,
                      false, 0, true);

    G4cout << "\n[TantalumTarget] 冷却水外筒: R=" << C_TT_Water_R/mm
           << " mm, HalfL=" << C_TT_Water_HalfL/mm << " mm" << G4endl;

    // ----------------------------------------------------------------
    // 结构包壳（SS316L，嵌入冷却水筒内）
    //   半径 = 靶半径 + 包壳厚度 = C_TT_Clad_R
    //   半长 = 靶半长 + 包壳厚度 = C_TT_Clad_HalfL
    // ----------------------------------------------------------------
    TT_Clad_SV = new G4Tubs(C_TT_Clad_Name,
                             0.*mm, C_TT_Clad_R,
                             C_TT_Clad_HalfL,
                             0., 360.*deg);

    TT_Clad_LV = new G4LogicalVolume(TT_Clad_SV, TT_Clad_mat, C_TT_Clad_Name);
    TT_Clad_LV->SetVisAttributes(&TT_Vis::TT_Clad_Vis);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, 0),
                      TT_Clad_LV,
                      C_TT_Clad_Name,
                      TT_Water_LV,
                      false, 0, true);

    G4cout << "[TantalumTarget] 结构包壳(SS316L): R=" << C_TT_Clad_R/mm
           << " mm, HalfL=" << C_TT_Clad_HalfL/mm
           << " mm, 壁厚=" << C_TT_Clad_Thickness/mm << " mm" << G4endl;

    // ----------------------------------------------------------------
    // 钽靶芯（嵌入包壳内）
    //   半径 = C_TT_Target_R
    //   半长 = C_TT_Target_HalfL
    // ----------------------------------------------------------------
    TT_Target_SV = new G4Tubs(C_TT_Target_Name,
                               0.*mm, C_TT_Target_R,
                               C_TT_Target_HalfL,
                               0., 360.*deg);

    TT_Target_LV = new G4LogicalVolume(TT_Target_SV, TT_Target_mat, C_TT_Target_Name);
    TT_Target_LV->SetVisAttributes(&TT_Vis::TT_Target_Vis);

    new G4PVPlacement(nullptr,
                      G4ThreeVector(0, 0, 0),
                      TT_Target_LV,
                      C_TT_Target_Name,
                      TT_Clad_LV,
                      false, 0, true);

    G4cout << "[TantalumTarget] 钽靶芯(G4_Ta): R=" << C_TT_Target_R/mm
           << " mm, HalfL=" << C_TT_Target_HalfL/mm << " mm" << G4endl;

    G4cout << "[TantalumTarget] 构建完成，位于 pos=("
           << pos.x()/mm << "," << pos.y()/mm << "," << pos.z()/mm
           << ") mm\n" << G4endl;

    return TT_Water_LV;
}
