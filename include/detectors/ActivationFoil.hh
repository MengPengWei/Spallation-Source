#ifndef ACTIVATION_FOIL_HH
#define ACTIVATION_FOIL_HH

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4Material.hh"

class ActivationFoil
{
public:
    ActivationFoil();
    ~ActivationFoil() = default;

    G4LogicalVolume* Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos);

    G4LogicalVolume* Get_LV() const {  return AF_CFC_LV; }

private:
    // -------------------- 所有变量都在头文件里 --------------------
    G4Material* AF_CFC_mat = nullptr;
    G4Material* AF_Air_mat = nullptr;
    G4Material* AF_In_mat  = nullptr;
    G4Material* AF_Si_mat  = nullptr;
    G4Material* AF_Cu_mat  = nullptr;

    G4Tubs* AF_CFC_SV = nullptr;
    G4Tubs* AF_Air_SV = nullptr;
    G4Tubs* AF_In_SV  = nullptr;
    G4Tubs* AF_Si_SV  = nullptr;
    G4Tubs* AF_Cu_SV  = nullptr;

    G4LogicalVolume* AF_CFC_LV = nullptr;
    G4LogicalVolume* AF_Air_LV = nullptr;
    G4LogicalVolume* AF_In_LV  = nullptr;
    G4LogicalVolume* AF_Si_LV  = nullptr;
    G4LogicalVolume* AF_Cu_LV  = nullptr;

};

#endif