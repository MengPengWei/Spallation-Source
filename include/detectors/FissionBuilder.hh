#ifndef FISSIONBUILDER_HH
#define FISSIONBUILDER_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"

class FissionBuilder
{
public:
    FissionBuilder();
    ~FissionBuilder() = default;

    // 构建裂变靶并放置到母逻辑体中
    G4LogicalVolume* Build(G4LogicalVolume* motherLV,
                            const G4ThreeVector& pos = G4ThreeVector(0,0,0));

    // 获取裂变靶逻辑体
    G4LogicalVolume* GetLogicTarget() const { return fLogicTarget; }

private:
    // 材料
    G4Material* fTargetMaterial = nullptr;

    // 几何
    G4LogicalVolume* fLogicTarget = nullptr;
};

#endif