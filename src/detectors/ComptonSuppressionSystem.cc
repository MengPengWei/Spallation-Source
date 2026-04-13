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
    G4Material* matAir      = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* matAl = nist->FindOrBuildMaterial("G4_Al");
    G4Material* matGe = nist->FindOrBuildMaterial("G4_Ge");
    G4Material* matBGO = nist->FindOrBuildMaterial("G4_BGO");
    G4Material* matPb = nist->FindOrBuildMaterial("G4_Pb");


    // 构建CSS圆柱体
    G4Tubs* CSS_Solid = new G4Tubs("CSS_Solid", 0, C_CSS_D/2, C_CSS_L/2, 0, 360*deg);
    G4LogicalVolume* CSS_LV = new G4LogicalVolume(CSS_Solid, matAir, "CSS_LV");
    new G4PVPlacement(nullptr, pos, CSS_LV, "CSS_PV", motherLV, false, 0, true);

    // 设置CSS的可视化属性
    G4VisAttributes* cssVisAttr = new G4VisAttributes(G4Colour(0.0, 0, 0.0, 0)); // CSS系统设置为完全透明
    cssVisAttr->SetForceSolid(true);
    CSS_LV->SetVisAttributes(cssVisAttr);

    // 构建屏蔽体，位于CSS系统的最外层，负责gamma射线的屏蔽工作（活化片测试），覆盖BGO与HPGe前端，中间留有小孔
    G4Tubs*CSS_Shield_Solid = new G4Tubs("CSS_Shield_Solid", C_Shield_InD/2, C_Shield_OutD/2, C_Shield_L/2, 0, 360*deg);
    G4LogicalVolume* CSS_Shield_LV = new G4LogicalVolume(CSS_Shield_Solid, matPb, "CSS_Shield_LV");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), CSS_Shield_LV, "CSS_Shield_PV", CSS_LV, false, 0, true);

    // 构建准直器，位于前端，靠近样品位置
    G4Tubs* CSS_Collimator_Solid = new G4Tubs("CSS_Collimator_Solid", C_Collimator_InD/2, C_Collimator_OutD/2, C_Collimator_L/2, 0, 360*deg);
    G4LogicalVolume* CSS_Collimator_LV = new G4LogicalVolume(CSS_Collimator_Solid, matPb, "CSS_Collimator_LV");
    new G4PVPlacement(nullptr, pos_CSS_Collimator, CSS_Collimator_LV, "CSS_Collimator_PV", CSS_LV, false, 0, true);
    // 设置屏蔽体和准直器颜色
    G4VisAttributes* shieldVisAttr = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 1.0)); // 屏蔽体设置为灰色
    shieldVisAttr->SetForceWireframe(true);  // 关键：线框模式，看穿内部
    shieldVisAttr->SetForceSolid(false); // 关键：不强制实心，允许线框显示
    CSS_Shield_LV->SetVisAttributes(shieldVisAttr);

    G4VisAttributes* collimatorVisAttr = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 1.0)); // 准直器设置为灰色
    collimatorVisAttr->SetForceWireframe(true);  // 线框模式，看穿内部
    collimatorVisAttr->SetForceSolid(false); // 不强制
    CSS_Collimator_LV->SetVisAttributes(collimatorVisAttr);



    // 构建BGO探测器,位于准直器以后，环绕HPGe探测器
    for (G4int i = 0; i < C_BGONumber; ++i) {
        G4double angle = i * C_BGOAngle;

        G4Tubs* CSS_BGO_Solid = new G4Tubs("CSS_BGO_Solid",C_BGO_InD/2, C_BGO_OutD/2,C_BGO_L/2, angle,C_BGOAngle); 
        G4LogicalVolume* BGO_LV = new G4LogicalVolume(CSS_BGO_Solid, matBGO, "CSS_BGO_LV");
        new G4PVPlacement(nullptr, pos_CSS_BGO, BGO_LV,"CSS_BGO_PV",CSS_LV, false,i,true);
        G4VisAttributes* bgoVisAttr = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0)); // BGO探测器设置为绿色
        bgoVisAttr->SetForceWireframe(true);  // 线框模式，看穿内部
        bgoVisAttr->SetForceSolid(false); // 不强制实心，允许
        BGO_LV->SetVisAttributes(bgoVisAttr);
    
    }
    
    // 构建铝层（夹在BGO和HPGe之间）
    G4Tubs* CSS_Al_Solid = new G4Tubs("CSS_Al_Solid", C_Al_InD/2, C_Al_OutD/2, C_Al_L/2, 0, 360*deg);
    G4LogicalVolume* CSS_Al_LV = new G4LogicalVolume(CSS_Al_Solid, matAl, "CSS_Al_LV");
    new G4PVPlacement(nullptr, pos_CSS_Al, CSS_Al_LV, "CSS_Al_PV", CSS_LV, false, 0, true);
    G4VisAttributes* alVisAttr = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1.0)); // 铝层设置为红色
    alVisAttr->SetForceWireframe(true);  // 线框模式，看穿
    alVisAttr->SetForceSolid(false); // 不强制实心，允许线框显示
    CSS_Al_LV->SetVisAttributes(alVisAttr);

    // 构建HPGe探测器
    G4Tubs* CSS_HPGe_Solid = new G4Tubs("CSS_HPGe_Solid", 0, C_HPGe_OutD/2, C_HPGe_L/2, 0, 360*deg);
    G4LogicalVolume* CSS_HPGe_LV = new G4LogicalVolume(CSS_HPGe_Solid, matGe, "CSS_HPGe_LV");
    new G4PVPlacement(nullptr, pos_CSS_HPGe, CSS_HPGe_LV, "CSS_HPGe_PV", CSS_LV, false, 0, true);
    G4VisAttributes* hpgeVisAttr = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 1.0)); // HPGe探测器设置为蓝色
    hpgeVisAttr->SetForceWireframe(true);  // 线框模式，看穿内部
    hpgeVisAttr->SetForceSolid(false); // 不强制实心
    CSS_HPGe_LV->SetVisAttributes(hpgeVisAttr);


    return CSS_LV;
}
