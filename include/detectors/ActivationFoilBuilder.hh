#ifndef ACTIVATIONFOILBUILDER_HH
#define ACTIVATIONFOILBUILDER_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"

/// Builds a simplified capsule (CFC graphite shell) containing an air cavity
/// with 3 cylindrical sample volumes (Indium) placed along Z.
/// No boolean solids are used.
class ActivationFoilBuilder
{
public:
    ActivationFoilBuilder();
    ~ActivationFoilBuilder() = default;

    /// Build and place the full capsule assembly into motherLV at pos.
    /// Returns the shared sample logical volume (all 3 cylinders use same LV).
    G4LogicalVolume* Build(G4LogicalVolume* motherLV,
                            const G4ThreeVector& pos = G4ThreeVector(0,0,0));//建立并放置完整的胶囊组件到母体积中，位置为pos。返回共享的样品逻辑体（所有3个圆柱体使用相同的LV）。

    G4LogicalVolume* GetSampleLV() const { return fSampleLV; }

private:
    G4LogicalVolume* fSampleLV = nullptr;
};

#endif // ACTIVATIONFOILBUILDER_HH
