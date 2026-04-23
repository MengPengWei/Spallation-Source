#include "SourceDetector.hh"


SourceDetector::SourceDetector()
{}
SourceDetector::~SourceDetector()
{}

G4LogicalVolume* SourceDetector::Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos,G4String name)
{
    G4NistManager* nist = G4NistManager::Instance();
    SD_Air_mat = nist->FindOrBuildMaterial("G4_AIR");

    // 构建一个简单的探测器：位于质子束流上方，直径包裹质子束流，高度为5mm，材料为空气,尽可能薄，以减少对粒子传播的影响
    SD_SV = new G4Tubs("SD_SV_"+name, 0, 5*cm, 5*mm, 0, 360*deg); //
    SD_LV = new G4LogicalVolume(SD_SV, SD_Air_mat, "SD_LV");
    SD_PV = new G4PVPlacement(nullptr, pos, SD_LV, "SD_PV", motherLV, false, 0, true);




    return SD_LV;
}