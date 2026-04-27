#include "SourceDetector.hh"


SourceDetector::SourceDetector()
{}


G4LogicalVolume* SourceDetector::Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos,G4String name)
{
    G4NistManager* nist = G4NistManager::Instance();
    SD_Air_mat = nist->FindOrBuildMaterial("G4_AIR");

    // 构建一个简单的探测器：位于质子束流上方，直径包裹质子束流，高度为5mm，材料为空气,尽可能薄，以减少对粒子传播的影响
    //获取质子枪的半斤位置，根据质子枪位置，确定探测器位置
    ProtonGun tempPG;//创建一个临时的ProtonGun对象，用于获取质子枪的位置信息
    G4double SD_MaxRel=tempPG.GetMaxRel();//获取质子枪束斑的半边长，探测器半径设置为束斑半边长的1.5倍，以确保完全覆盖束斑
    SD_SV = new G4Tubs("SD_SV_"+name, 0, SD_MaxRel*1.5, 5*mm, 0, 360*deg); //
    SD_LV = new G4LogicalVolume(SD_SV, SD_Air_mat, "SD_LV_"+name);
    SD_PV = new G4PVPlacement(nullptr, pos, SD_LV, "SD_PV_"+name, motherLV, false, 0, true);




    return SD_LV;
}