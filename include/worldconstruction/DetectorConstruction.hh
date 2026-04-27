#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;//override关键字表示该方法重写了基类中的虚函数Construct，确保编译器检查函数签名是否正确匹配基类中的虚函数

    /// Returns the shared logical volume of the 3 sample cylinders.
    /// Valid only after Construct() has been called.
    G4LogicalVolume* Get_LV_Detector() const { return LV_Detector; }
    G4LogicalVolume* Get_LV_World() const { return LV_World; }
    /// Source-detector logical volumes (valid after Construct()).
    G4LogicalVolume* Get_LV_ProtonDet()  const { return LV_ProtonDet; }
    G4LogicalVolume* Get_LV_NeutronDet() const { return LV_NeutronDet; }

    G4Material* Mat_World;
    G4VPhysicalVolume* PV_World;
    G4LogicalVolume* LV_World;
    G4Box* SV_World;

    G4LogicalVolume* LV_Detector = nullptr;
    /// 两个 source 探测器的逻辑体指针（Construct() 后有效）
    G4LogicalVolume* LV_ProtonDet  = nullptr;
    G4LogicalVolume* LV_NeutronDet = nullptr;

private:
    void DefineMaterials();
    G4VPhysicalVolume* ConstructWorld();




};

#endif // DETECTORCONSTRUCTION_HH