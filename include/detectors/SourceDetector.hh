#ifndef SOURCEDETECTOR_HH
#define SOURCEDETECTOR_HH

//专用于获取质子枪与坦靶反应的相关信息，位于质子枪与坦靶组件的交界处，以及从坦靶出来位置的正上方
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"
#include "ProtonGun.hh"
class SourceDetector
{
public:
    SourceDetector();
    ~SourceDetector() = default;

    /// 在母体积 motherLV 的 pos 位置构建探测器，返回逻辑体。
    G4LogicalVolume* Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos,G4String name="");

private:
    // 材料
    G4Material* SD_Air_mat = nullptr;


    G4LogicalVolume* SD_LV = nullptr;
    G4PVPlacement* SD_PV = nullptr;
    G4Tubs* SD_SV = nullptr;

};

#endif // SOURCEDETECTOR_HH