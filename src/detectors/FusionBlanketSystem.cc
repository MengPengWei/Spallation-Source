#include "FusionBlanketSystem.hh"

#include "FusionConstants.hh"

#include "G4NistManager.hh"
#include "G4Tubs.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

namespace {
G4Material* GetOrBuildLiPbMaterial()
{
    const G4String name = "FUSION_LiPb";
    if (auto* existing = G4Material::GetMaterial(name, false)) return existing;

    auto* nist = G4NistManager::Instance();
    auto* li = nist->FindOrBuildElement("Li");
    auto* pb = nist->FindOrBuildElement("Pb");

    auto* mat = new G4Material(name, 9.4 * g / cm3, 2);
    mat->AddElement(li, 0.17);
    mat->AddElement(pb, 0.83);
    return mat;
}

G4Material* GetOrBuildLiGlassMaterial()
{
    const G4String name = "FUSION_LiGlass";
    if (auto* existing = G4Material::GetMaterial(name, false)) return existing;

    auto* nist = G4NistManager::Instance();
    auto* li = nist->FindOrBuildElement("Li");
    auto* o = nist->FindOrBuildElement("O");
    auto* si = nist->FindOrBuildElement("Si");

    auto* mat = new G4Material(name, 2.5 * g / cm3, 3);
    mat->AddElement(li, 0.22);
    mat->AddElement(o, 0.48);
    mat->AddElement(si, 0.30);
    return mat;
}

G4Material* GetOrBuildLiDiamondMaterial()
{
    const G4String name = "FUSION_LiDiamond";
    if (auto* existing = G4Material::GetMaterial(name, false)) return existing;

    auto* nist = G4NistManager::Instance();
    auto* li = nist->FindOrBuildElement("Li");
    auto* c = nist->FindOrBuildElement("C");

    auto* mat = new G4Material(name, 3.2 * g / cm3, 2);
    mat->AddElement(li, 0.05);
    mat->AddElement(c, 0.95);
    return mat;
}
} // namespace

void FusionBlanketSystem::Build(G4LogicalVolume* worldLV, const G4ThreeVector& center)
{
    if (!worldLV) return;

    auto* nist = G4NistManager::Instance();
    auto* steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    auto* blanketMat = GetOrBuildLiPbMaterial();

    const G4double firstWallInner = C_FUSION_PlasmaRadius;
    const G4double firstWallOuter = firstWallInner + C_FUSION_FirstWallThick;
    const G4double blanketInner = firstWallOuter;
    const G4double blanketOuter = blanketInner + C_FUSION_BlanketThick;

    auto* fwSolid = new G4Tubs("FUS_SV_FirstWall", firstWallInner, firstWallOuter,
                               C_FUSION_HalfZ, 0., CLHEP::twopi);
    fFirstWallLV = new G4LogicalVolume(fwSolid, steel, "FUS_LV_FirstWall");
    new G4PVPlacement(nullptr, center, fFirstWallLV, "FUS_PV_FirstWall",
                      worldLV, false, 0, true);

    auto* blanketSolid = new G4Tubs("FUS_SV_Blanket", blanketInner, blanketOuter,
                                    C_FUSION_HalfZ, 0., CLHEP::twopi);
    fBlanketLV = new G4LogicalVolume(blanketSolid, blanketMat, "FUS_LV_Blanket");
    new G4PVPlacement(nullptr, center, fBlanketLV, "FUS_PV_Blanket",
                      worldLV, false, 0, true);

    auto* liGlassMat = GetOrBuildLiGlassMaterial();
    auto* liDiamondMat = GetOrBuildLiDiamondMaterial();

    auto* detSolid = new G4Tubs("TRIT_SV_Det", 0., C_FUSION_DetectorRadius,
                                C_FUSION_DetectorHalfZ, 0., CLHEP::twopi);

    fLiGlassDetLV = new G4LogicalVolume(detSolid, liGlassMat, "TRIT_LV_LiGlass");
    fLiDiamondDetLV = new G4LogicalVolume(detSolid, liDiamondMat, "TRIT_LV_LiDiamond");

    const G4double detR = blanketOuter + C_FUSION_DetectorGap;
    const G4ThreeVector glassPos = center + G4ThreeVector(detR, 0., 0.);
    const G4ThreeVector diamondPos = center + G4ThreeVector(-detR, 0., 0.);

    new G4PVPlacement(nullptr, glassPos, fLiGlassDetLV, "TRIT_PV_LiGlass",
                      worldLV, false, 0, true);
    new G4PVPlacement(nullptr, diamondPos, fLiDiamondDetLV, "TRIT_PV_LiDiamond",
                      worldLV, false, 0, true);

    auto* fwVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7));
    auto* blanketVis = new G4VisAttributes(G4Colour(0.3, 0.8, 0.3));
    auto* glassVis = new G4VisAttributes(G4Colour(0.2, 0.5, 1.0));
    auto* diamondVis = new G4VisAttributes(G4Colour(0.9, 0.8, 0.2));
    fwVis->SetForceSolid(true);
    blanketVis->SetForceSolid(true);
    glassVis->SetForceSolid(true);
    diamondVis->SetForceSolid(true);

    fFirstWallLV->SetVisAttributes(fwVis);
    fBlanketLV->SetVisAttributes(blanketVis);
    fLiGlassDetLV->SetVisAttributes(glassVis);
    fLiDiamondDetLV->SetVisAttributes(diamondVis);

    G4cout << "\n[FusionBlanketSystem] Constructed fusion structures\n"
           << "  First wall: R=[" << firstWallInner / cm << ", "
           << firstWallOuter / cm << "] cm\n"
           << "  Blanket:    R=[" << blanketInner / cm << ", "
           << blanketOuter / cm << "] cm\n"
           << "  Tritium detectors at R=" << detR / cm << " cm" << G4endl;
}
