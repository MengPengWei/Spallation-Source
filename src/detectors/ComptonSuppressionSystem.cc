#include "ComptonSuppressionSystem.hh"
#include "Constants.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4RotationMatrix.hh"

#include "CssConstants.hh"

ComptonSuppressionSystem::ComptonSuppressionSystem()
{}

G4LogicalVolume* ComptonSuppressionSystem::Build(G4LogicalVolume* motherLV,
                                                                        const G4ThreeVector& pos)
{
    auto nist = G4NistManager::Instance();
    CSS_mat_Air      = nist->FindOrBuildMaterial("G4_AIR");
    CSS_mat_Al = nist->FindOrBuildMaterial("G4_Al");
    CSS_mat_Ge = nist->FindOrBuildMaterial("G4_Ge");
    CSS_mat_BGO = nist->FindOrBuildMaterial("G4_BGO");
    CSS_mat_Pb = nist->FindOrBuildMaterial("G4_Pb");


    // 构建CSS圆柱体
    CSS_SV = new G4Tubs("CSS_SV", 0, C_CSS_D/2, C_CSS_L/2, 0, 360*deg);
    CSS_LV = new G4LogicalVolume(CSS_SV, CSS_mat_Air, "CSS_LV");
    //沿X轴，
    G4RotationMatrix* CSS_Rotation=new G4RotationMatrix();
    CSS_Rotation->rotateY(C_CSS_degree);
    new G4PVPlacement(CSS_Rotation,pos, CSS_LV, "CSS_PV", motherLV, false, 0, true);



    // 构建屏蔽体，位于CSS系统的最外层，负责gamma射线的屏蔽工作（活化片测试），覆盖BGO与HPGe前端，中间留有小孔
    CSS_Shield_SV = new G4Tubs("CSS_Shield_SV", C_CSS_Shield_InD/2, C_CSS_Shield_OutD/2, C_CSS_Shield_L/2, 0, 360*deg);
    CSS_Shield_LV = new G4LogicalVolume(CSS_Shield_SV, CSS_mat_Pb, "CSS_Shield_LV");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), CSS_Shield_LV, "CSS_Shield_PV", CSS_LV, false, 0, true);


    // 构建准直器，位于前端，靠近样品位置
    CSS_Collimator_SV = new G4Tubs("CSS_Collimator_SV", C_CSS_Collimator_InD/2, C_CSS_Collimator_OutD/2, C_CSS_Collimator_L/2, 0, 360*deg);
    CSS_Collimator_LV = new G4LogicalVolume(CSS_Collimator_SV, CSS_mat_Pb, "CSS_Collimator_LV");
    new G4PVPlacement(nullptr, C_CSS_Collimator_Pos, CSS_Collimator_LV, "CSS_Collimator_PV", CSS_LV, false, 0, true);
    // 设置屏蔽体和准直器颜色




    // 构建BGO探测器,位于准直器以后，环绕HPGe探测器
    for (G4int i = 0; i < C_CSS_BGO_n; ++i) {
        G4double angle = i * C_CSS_BGO_Angle;

        CSS_BGO_SV[i] = new G4Tubs("CSS_BGO_SV",C_CSS_BGO_InD/2, C_CSS_BGO_OutD/2,C_CSS_BGO_L/2, angle,C_CSS_BGO_Angle); 
        CSS_BGO_LV[i] = new G4LogicalVolume(CSS_BGO_SV[i], CSS_mat_BGO, "CSS_BGO_LV");
        new G4PVPlacement(nullptr, C_CSS_BGO_Pos, CSS_BGO_LV[i],"CSS_BGO_PV",CSS_LV, false,i,true);
        CSS_BGO_LV[i]->SetVisAttributes(CSS_Vis::C_CSS_BGO_Vis);
    
    }
    
    // 构建铝层（夹在BGO和HPGe之间）
    CSS_Al_SV = new G4Tubs("CSS_Al_SV", C_CSS_Al_InD/2, C_CSS_Al_OutD/2, C_CSS_Al_L/2, 0, 360*deg);
    CSS_Al_LV = new G4LogicalVolume(CSS_Al_SV, CSS_mat_Al, "CSS_Al_LV");
    new G4PVPlacement(nullptr, C_CSS_Al_Pos, CSS_Al_LV, "CSS_Al_PV", CSS_LV, false, 0, true);



    // 构建HPGe探测器
    CSS_HPGe_SV = new G4Tubs("CSS_HPGe_SV", 0, C_CSS_HPGe_OutD/2, C_CSS_HPGe_L/2, 0, 360*deg);
    CSS_HPGe_LV = new G4LogicalVolume(CSS_HPGe_SV, CSS_mat_Ge, "CSS_HPGe_LV");
    new G4PVPlacement(nullptr, C_CSS_HPGe_Pos, CSS_HPGe_LV, "CSS_HPGe_PV", CSS_LV, false, 0, true);

    CSS_Al_LV->SetVisAttributes(CSS_Vis::C_CSS_Al_Vis);
    CSS_HPGe_LV->SetVisAttributes(CSS_Vis::C_CSS_HPGe_Vis);
    CSS_Shield_LV->SetVisAttributes(CSS_Vis::C_CSS_Shield_Vis);
    CSS_Collimator_LV->SetVisAttributes(CSS_Vis::C_CSS_Collimator_Vis);
    CSS_LV->SetVisAttributes(CSS_Vis::C_CSS_Vis);
    


    return CSS_LV;
}
