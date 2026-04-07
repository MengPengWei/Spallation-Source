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
    G4LogicalVolume* GetSampleLV() const { return fSampleLV; }
    G4LogicalVolume* GetWorldLV() const { return logicWorld; }

    G4double fWorldSizeX;
    G4double fWorldSizeY;
    G4double fWorldSizeZ;

    G4Material* WorldMat;
    G4VPhysicalVolume* physWorld;
    G4LogicalVolume* logicWorld;
    G4Box* solidWorld;

    G4LogicalVolume* fSampleLV = nullptr;

private:
    void DefineMaterials();
    G4VPhysicalVolume* ConstructWorld();




};

#endif // DETECTORCONSTRUCTION_HH