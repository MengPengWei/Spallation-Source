#ifndef COMPTON_SUPPRESSION_SYSTEM_HH
#define COMPTON_SUPPRESSION_SYSTEM_HH

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"
#include "G4Material.hh"
#include "CssConstants.hh"

class ComptonSuppressionSystem
{
public:
    ComptonSuppressionSystem();
    ~ComptonSuppressionSystem() = default;

    G4LogicalVolume* Build(G4LogicalVolume* motherLV,
                            const G4ThreeVector& pos = G4ThreeVector(0,0,0));

    // 外部获取探测器LV —— 你以后随时可以取！
    G4LogicalVolume* Get_CSS_LV()         const { return CSS_LV; }
    G4LogicalVolume* Get_CSS_Shield_LV()      const { return CSS_Shield_LV; }
    G4LogicalVolume* Get_CSS_Collimator_LV()  const { return CSS_Collimator_LV; }
    G4LogicalVolume* Get_BGO_LV(G4int i) const {
        if (i >= 0 && i < C_CSS_BGO_n) return CSS_BGO_LV[i];
        return nullptr;
    }
    G4LogicalVolume* Get_CSS_Al_LV()          const { return CSS_Al_LV; }
    G4LogicalVolume* Get_CSS_HPGe_LV()        const { return CSS_HPGe_LV; }

private:
    // 逻辑体积（全部放在类内，方便后续访问）
    G4LogicalVolume* CSS_LV         = nullptr;
    G4LogicalVolume* CSS_Shield_LV      = nullptr;
    G4LogicalVolume* CSS_Collimator_LV   = nullptr;
    G4LogicalVolume* CSS_BGO_LV[C_CSS_BGO_n]          = {nullptr};
    G4LogicalVolume* CSS_Al_LV           = nullptr;
    G4LogicalVolume* CSS_HPGe_LV         = nullptr;

    // 固体体积
    G4Tubs* CSS_SV         = nullptr;
    G4Tubs* CSS_Shield_SV      = nullptr;
    G4Tubs* CSS_Collimator_SV  = nullptr;
    G4Tubs* CSS_BGO_SV[C_CSS_BGO_n]         = {nullptr};
    G4Tubs* CSS_Al_SV          = nullptr;
    G4Tubs* CSS_HPGe_SV        = nullptr;

    // 材料
    G4Material* CSS_mat_Air    = nullptr;
    G4Material* CSS_mat_Al     = nullptr;
    G4Material* CSS_mat_Ge     = nullptr;
    G4Material* CSS_mat_BGO    = nullptr;
    G4Material* CSS_mat_Pb     = nullptr;
};

#endif