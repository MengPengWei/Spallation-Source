// #include "FissionBuilder.hh"
// #include "Constants.hh"

// #include "G4NistManager.hh"
// #include "G4Material.hh"
// #include "G4Isotope.hh"
// #include "G4Element.hh"
// #include "G4Tubs.hh"
// #include "G4LogicalVolume.hh"
// #include "G4PVPlacement.hh"
// #include "G4SystemOfUnits.hh"
// #include "G4VisAttributes.hh"
// #include "G4Colour.hh"

// FissionBuilder::FissionBuilder()
// {}

// G4LogicalVolume* FissionBuilder::Build(G4LogicalVolume* motherLV, const G4ThreeVector& pos)
// {
//     G4NistManager* nist = G4NistManager::Instance();

//     // 定义铀同位素
//     G4Isotope* U235 = new G4Isotope("U235", 92, 235, 235.0439*g/mole);
//     G4Isotope* U238 = new G4Isotope("U238", 92, 238, 238.0508*g/mole);

//     // 富集铀元素（90% U235 + 10% U238）
//     G4Element* elU = new G4Element("FissileU", "U", 2);
//     elU->AddIsotope(U235, 90.0*perCent);
//     elU->AddIsotope(U238, 10.0*perCent);

//     // 裂变靶材料：高浓铀
//     fTargetMaterial = new G4Material("HighU235", 19.1*g/cm3, 1);
//     fTargetMaterial->AddElement(elU, 1.0);

//     // 裂变靶几何体（圆柱）
//     // G4Tubs: rmin, rmax, 半高, startPhi, deltaPhi
//     G4Tubs* solidTarget = new G4Tubs("Target",
//                                         0.,
//                                         C_TargetRadius,
//                                         C_TargetLength,  // Geant4 用半长
//                                         0.,
//                                         CLHEP::twopi);

//     // 逻辑体积
//     fLogicTarget = new G4LogicalVolume(solidTarget, fTargetMaterial, "Target");

//     // 可视化：红色半透明，便于识别裂变靶
//     G4VisAttributes* visTarget = new G4VisAttributes(G4Colour(1.0, 0.1, 0.1, 0.7));
//     visTarget->SetForceSolid(true);
//     fLogicTarget->SetVisAttributes(visTarget);

//     // 放置到世界中
//     new G4PVPlacement(0,
//                         pos,
//                         fLogicTarget,
//                         "Target",
//                         motherLV,
//                         false,
//                         0,
//                         true);

//     return fLogicTarget;
// }