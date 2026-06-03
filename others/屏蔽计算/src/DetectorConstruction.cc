#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"

#include "G4NistManager.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4Isotope.hh"
#include "G4Box.hh"
#include "G4Orb.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4RunManager.hh"
#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"

#include <algorithm>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <cmath>

namespace {
constexpr G4double kBooleanNudge = 0.01 * mm;
constexpr G4double kOpeningOversize = 0.01 * mm;

std::array<G4double, 6> RppToMm(const std::array<G4double, 6>& vCm) {
  return {vCm[0] * cm, vCm[1] * cm, vCm[2] * cm, vCm[3] * cm, vCm[4] * cm, vCm[5] * cm};
}

G4Box* MakeRppSolid(const G4String& name, const std::array<G4double, 6>& rppCm) {
  const auto rpp = RppToMm(rppCm);
  const G4double hx = 0.5 * (rpp[1] - rpp[0]);
  const G4double hy = 0.5 * (rpp[3] - rpp[2]);
  const G4double hz = 0.5 * (rpp[5] - rpp[4]);
  return new G4Box(name, hx, hy, hz);
}

G4ThreeVector RppCenter(const std::array<G4double, 6>& rppCm) {
  const auto rpp = RppToMm(rppCm);
  return {(rpp[0] + rpp[1]) * 0.5, (rpp[2] + rpp[3]) * 0.5, (rpp[4] + rpp[5]) * 0.5};
}

// 应用MCNP切边平面 (1201-1204, 1211-1214, 1221-1224)
G4VSolid* ApplyChamferPlanes(const G4String& name,
                              G4VSolid* solid,
                              const G4ThreeVector& layerCenter,
                              bool include1201_1204,
                              bool include1211_1214,
                              bool include1221_1224) {
  G4VSolid* result = solid;
  
  // 1201-1204: 切顶边平面 P 1 0 1 465.21 等
  if (include1201_1204) {
    const std::vector<std::array<G4double, 4>> planes1201 = {
      {1.0, 0.0, 1.0, 465.21},
      {-1.0, 0.0, 1.0, 465.21},
      {0.0, 1.0, 1.0, 465.21},
      {0.0, -1.0, 1.0, 465.21}
    };
    
    for (size_t i = 0; i < planes1201.size(); ++i) {
      const auto& p = planes1201[i];
      G4ThreeVector normal(p[0], p[1], p[2]);
      G4double norm = normal.mag();
      normal = normal.unit();
      G4double dist = p[3] * cm / norm + 0.1 * mm;
      G4ThreeVector planePoint = normal * dist;
      
      const G4double halfXY = 6000.0 * cm;
      const G4double length = 12000.0 * cm;
      auto* cutter = new G4Box(name + "_chamfer1201_" + std::to_string(i), halfXY, halfXY, 0.5 * length);
      
      auto* rot = new G4RotationMatrix();
      rot->rotateZ(normal.phi());
      rot->rotateY(normal.theta());
      G4ThreeVector cutterCenter = planePoint + normal * (0.5 * length);
      
      result = new G4SubtractionSolid(name + "_cut1201_" + std::to_string(i), 
                                       result, cutter, rot, cutterCenter - layerCenter);
    }
  }
  
  // 1211-1214: 切侧面边
  if (include1211_1214) {
    const std::vector<std::array<G4double, 4>> planes1211 = {
      {1.0, 1.0, 0.0, 354.53},
      {-1.0, 1.0, 0.0, 354.53},
      {-1.0, -1.0, 0.0, 354.53},
      {1.0, -1.0, 0.0, 354.53}
    };
    
    for (size_t i = 0; i < planes1211.size(); ++i) {
      const auto& p = planes1211[i];
      G4ThreeVector normal(p[0], p[1], p[2]);
      G4double norm = normal.mag();
      normal = normal.unit();
      G4double dist = p[3] * cm / norm + 0.1 * mm;
      G4ThreeVector planePoint = normal * dist;
      
      const G4double halfZ = 6000.0 * cm;
      const G4double halfXY = 6000.0 * cm;
      auto* cutter = new G4Box(name + "_chamfer1211_" + std::to_string(i), halfXY, halfXY, halfZ);
      
      auto* rot = new G4RotationMatrix();
      rot->rotateZ(normal.phi());
      G4ThreeVector cutterCenter = planePoint + normal * halfXY;
      
      result = new G4SubtractionSolid(name + "_cut1211_" + std::to_string(i),
                                       result, cutter, rot, cutterCenter - layerCenter);
    }
  }
  
  // 1221-1224: 切顶角
  if (include1221_1224) {
    const std::vector<std::array<G4double, 4>> planes1221 = {
      {1.0, 1.0, 2.0, 850.0},
      {-1.0, 1.0, 2.0, 850.0},
      {-1.0, -1.0, 2.0, 850.0},
      {1.0, -1.0, 2.0, 850.0}
    };
    
    for (size_t i = 0; i < planes1221.size(); ++i) {
      const auto& p = planes1221[i];
      G4ThreeVector normal(p[0], p[1], p[2]);
      G4double norm = normal.mag();
      normal = normal.unit();
      G4double dist = p[3] * cm / norm + 0.1 * mm;
      G4ThreeVector planePoint = normal * dist;
      
      const G4double halfSize = 8000.0 * cm;
      auto* cutter = new G4Box(name + "_chamfer1221_" + std::to_string(i), halfSize, halfSize, halfSize);
      
      auto* rot = new G4RotationMatrix();
      rot->rotateZ(normal.phi());
      rot->rotateY(normal.theta());
      G4ThreeVector cutterCenter = planePoint + normal * halfSize;
      
      result = new G4SubtractionSolid(name + "_cut1221_" + std::to_string(i),
                                       result, cutter, rot, cutterCenter - layerCenter);
    }
  }
  
  return result;
}

// 应用开口21-29到固体
G4VSolid* ApplyOpenings(const G4String& name, G4VSolid* solid, const G4ThreeVector& layerCenter) {
  G4VSolid* result = solid;
  
  // 开口21: RPP (-50 0 -300 0 35 85)
  const auto opening21Rpp = RppToMm({{ -50.0, 0.0, -300.0, 0.0, 35.0, 85.0 }});
  auto* opening21 = new G4Box(name + "_opening21",
                              0.5 * (opening21Rpp[1] - opening21Rpp[0]) + kOpeningOversize,
                              0.5 * (opening21Rpp[3] - opening21Rpp[2]) + kOpeningOversize,
                              0.5 * (opening21Rpp[5] - opening21Rpp[4]) + kOpeningOversize);
  const G4ThreeVector center21 = {(opening21Rpp[0]+opening21Rpp[1])*0.5, 
                                   (opening21Rpp[2]+opening21Rpp[3])*0.5,
                                   (opening21Rpp[4]+opening21Rpp[5])*0.5};
  result = new G4SubtractionSolid(name + "_cut21", result, opening21, nullptr,
                                  center21 - layerCenter + G4ThreeVector(0,0,0.5*kBooleanNudge));
  
  // 圆柱22-26
  struct CylDef {
    G4String name;
    G4ThreeVector origin;
    G4ThreeVector axis;
    G4double radius;
  };
  const std::vector<CylDef> cylinders = {
      {"22", G4ThreeVector(0., 0., 90.) * cm, G4ThreeVector(0., 300., 0.) * cm, 2.5 * cm},
      {"23", G4ThreeVector(0., 0., 90.) * cm, G4ThreeVector(212.13, 212.13, 0.) * cm, 2.5 * cm},
      {"24", G4ThreeVector(0., 0., 90.) * cm, G4ThreeVector(-212.13, 212.13, 0.) * cm, 2.5 * cm},
      {"25", G4ThreeVector(0., 0., 90.) * cm, G4ThreeVector(259.81, -150., 0.) * cm, 2.5 * cm},
      {"26", G4ThreeVector(0., 0., 90.) * cm, G4ThreeVector(212.13, -212.13, 0.) * cm, 2.5 * cm}
  };
  
  for (const auto& c : cylinders) {
    const G4double length = c.axis.mag();
    auto* cutter = new G4Tubs(name + "_cyl" + c.name, 0.,
                              c.radius + kOpeningOversize,
                              0.5 * length + kOpeningOversize, 0., twopi);
    auto* rot = new G4RotationMatrix();
    const G4ThreeVector unitAxis = c.axis.unit();
    rot->rotateZ(unitAxis.phi());
    rot->rotateY(unitAxis.theta());
    const G4ThreeVector center = c.origin + 0.5 * c.axis;
    result = new G4SubtractionSolid(name + "_cut" + c.name, result, cutter, rot,
                                    center - layerCenter + G4ThreeVector(0,0,0.5*kBooleanNudge));
  }
  
  // BOX 27-29
  struct BoxDef {
    G4String name;
    G4ThreeVector originCm;
    G4ThreeVector uCm;
    G4ThreeVector vCm;
    G4ThreeVector wCm;
  };
  const std::vector<BoxDef> boxDefs = {
      {"27", {-8.66, 5.0, 80.0}, {17.32, -10.0, 0.0}, {150.0, 259.81, 0.0}, {0.0, 0.0, 20.0}},
      {"28", {-8.66, -5.0, 80.0}, {17.32, 10.0, 0.0}, {-150.0, 259.81, 0.0}, {0.0, 0.0, 20.0}},
      {"29", {-2.59, 9.66, 80.0}, {5.18, -19.32, 0.0}, {289.78, 77.65, 0.0}, {0.0, 0.0, 20.0}}
  };
  
  for (const auto& def : boxDefs) {
    const G4ThreeVector u = def.uCm * cm;
    const G4ThreeVector v = def.vCm * cm;
    const G4ThreeVector w = def.wCm * cm;
    auto* cutter = new G4Box(name + "_box" + def.name,
                             0.5 * u.mag() + kOpeningOversize,
                             0.5 * v.mag() + kOpeningOversize,
                             0.5 * w.mag() + kOpeningOversize);
    auto* rot = new G4RotationMatrix();
    rot->rotateZ(u.phi());
    const G4ThreeVector origin = G4ThreeVector(def.originCm[0], def.originCm[1], def.originCm[2]) * cm;
    const G4ThreeVector center = origin + 0.5 * (u + v + w);
    result = new G4SubtractionSolid(name + "_cut" + def.name, result, cutter, rot,
                                    center - layerCenter + G4ThreeVector(0,0,0.5*kBooleanNudge));
  }
  
  return result;
}

}  // namespace

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fDefaultMaterial(nullptr),
      fPhysiWorld(nullptr),
      fDetectorMessenger(nullptr),
      fDoorOpen(false),
      fHotcellWallThickness(120.0 * cm) {
  fNbOfAbsor = 6;
  fAbsorThickness[0] = 0.;
  for (G4int i = 1; i < kMaxAbsor; ++i) {
    fAbsorThickness[i] = 10. * cm;
    fAbsorMaterial[i] = nullptr;
    fXfront[i] = 0.;
  }
  fAbsorSizeYZ = 500. * cm;
  ComputeParameters();
  DefineMaterials();
  fDetectorMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction() { delete fDetectorMessenger; }

G4VPhysicalVolume* DetectorConstruction::Construct() { return ConstructVolumes(); }

void DetectorConstruction::SetDoorOpen(G4bool value) {
  if (fDoorOpen == value) return;
  fDoorOpen = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetHotcellWallThickness(G4double value) {
  if (value <= 0.0) return;
  if (std::abs(fHotcellWallThickness - value) < 1.0e-9 * mm) return;
  fHotcellWallThickness = value;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::DefineMaterials() {
  auto* nist = G4NistManager::Instance();
  
  // MCNP材料映射 (test2.i)
  // M1: 空气 - G4_AIR (密度0.001205)
  // M4: 铅 - G4_Pb (密度11.35)
  // M5: 水 - G4_WATER (密度1.0)
  // M6: 铁 - G4_Fe (密度7.86)
  // M7: 含硼聚乙烯 (密度1.07)
  // M8: 石墨 - G4_GRAPHITE (密度1.7)
  // M9: 混凝土 - G4_CONCRETE (密度2.30)
  // M10: 304不锈钢 (密度7.92)
  // M11: 铜 - G4_Cu (密度8.9)
  // M12: 钽 - G4_Ta (密度16.65)
  
  nist->FindOrBuildMaterial("G4_Pb");
  nist->FindOrBuildMaterial("G4_Fe");
  nist->FindOrBuildMaterial("G4_Ta");
  nist->FindOrBuildMaterial("G4_Cu");
  nist->FindOrBuildMaterial("G4_Galactic");
  nist->FindOrBuildMaterial("G4_AIR");
  nist->FindOrBuildMaterial("G4_WATER");
  nist->FindOrBuildMaterial("G4_GRAPHITE");
  nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  nist->FindOrBuildMaterial("G4_CONCRETE");

  // M10: Custom 304 Stainless Steel (密度7.92 g/cm3)
  // 基于MCNP材料卡片定义
  // Manual isotope definitions for M10 - with molar masses (g/mol)
  auto* Fe54 = new G4Isotope("Fe54", 26, 54, 53.9396105 * g/mole);
  auto* Fe56 = new G4Isotope("Fe56", 26, 56, 55.9349375 * g/mole);
  auto* Fe57 = new G4Isotope("Fe57", 26, 57, 56.9353940 * g/mole);
  auto* Fe58 = new G4Isotope("Fe58", 26, 58, 57.9332774 * g/mole);
  auto* Cr50 = new G4Isotope("Cr50", 24, 50, 49.9460442 * g/mole);
  auto* Cr52 = new G4Isotope("Cr52", 24, 52, 51.9405075 * g/mole);
  auto* Cr53 = new G4Isotope("Cr53", 24, 53, 52.9406494 * g/mole);
  auto* Cr54 = new G4Isotope("Cr54", 24, 54, 53.9388804 * g/mole);
  auto* Ni58 = new G4Isotope("Ni58", 28, 58, 57.9353429 * g/mole);
  auto* Ni60 = new G4Isotope("Ni60", 28, 60, 59.9307864 * g/mole);
  auto* Ni61 = new G4Isotope("Ni61", 28, 61, 60.9310560 * g/mole);
  auto* Ni62 = new G4Isotope("Ni62", 28, 62, 61.9283451 * g/mole);
  auto* Ni64 = new G4Isotope("Ni64", 28, 64, 63.9279660 * g/mole);
  auto* Mo92 = new G4Isotope("Mo92", 42, 92, 91.906810 * g/mole);
  auto* Mo94 = new G4Isotope("Mo94", 42, 94, 93.9050883 * g/mole);
  auto* Mo95 = new G4Isotope("Mo95", 42, 95, 94.9058421 * g/mole);
  auto* Mo96 = new G4Isotope("Mo96", 42, 96, 95.9046795 * g/mole);
  auto* Mo97 = new G4Isotope("Mo97", 42, 97, 96.9060215 * g/mole);
  auto* Mo98 = new G4Isotope("Mo98", 42, 98, 97.9054082 * g/mole);
  auto* Mo100 = new G4Isotope("Mo100", 42, 100, 99.907477 * g/mole);
  auto* W182 = new G4Isotope("W182", 74, 182, 181.948205 * g/mole);
  auto* W183 = new G4Isotope("W183", 74, 183, 182.9502245 * g/mole);
  auto* W184 = new G4Isotope("W184", 74, 184, 183.9509312 * g/mole);
  auto* W186 = new G4Isotope("W186", 74, 186, 185.9543641 * g/mole);
  auto* Cu63 = new G4Isotope("Cu63", 29, 63, 62.9295975 * g/mole);
  auto* Cu65 = new G4Isotope("Cu65", 29, 65, 64.9277897 * g/mole);
  auto* Ti46 = new G4Isotope("Ti46", 22, 46, 45.9526316 * g/mole);
  auto* Ti47 = new G4Isotope("Ti47", 22, 47, 46.9517631 * g/mole);
  auto* Ti48 = new G4Isotope("Ti48", 22, 48, 47.9479463 * g/mole);
  auto* Ti49 = new G4Isotope("Ti49", 22, 49, 48.9478700 * g/mole);
  auto* Ti50 = new G4Isotope("Ti50", 22, 50, 49.9447912 * g/mole);
  
  // Create elements from isotopes
  auto* elFe = new G4Element("M10_Fe", "Fe", 4);
  elFe->AddIsotope(Fe54, 5.17648E-02);
  elFe->AddIsotope(Fe56, 8.18419E-01);
  elFe->AddIsotope(Fe57, 1.96349E-02);
  elFe->AddIsotope(Fe58, 2.67749E-03);
  
  auto* elCr = new G4Element("M10_Cr", "Cr", 4);
  elCr->AddIsotope(Cr50, 4.15992E-03);
  elCr->AddIsotope(Cr52, 8.10701E-02);
  elCr->AddIsotope(Cr53, 9.28727E-03);
  elCr->AddIsotope(Cr54, 2.32182E-03);
  
  auto* elNi = new G4Element("M10_Ni", "Ni", 5);
  elNi->AddIsotope(Ni58, 3.22699E-05);
  elNi->AddIsotope(Ni60, 1.24701E-05);
  elNi->AddIsotope(Ni61, 6.18745E-07);
  elNi->AddIsotope(Ni62, 1.76104E-06);
  elNi->AddIsotope(Ni64, 5.71149E-07);
  
  auto* elMo = new G4Element("M10_Mo", "Mo", 7);
  elMo->AddIsotope(Mo92, 0.432264E-05);
  elMo->AddIsotope(Mo94, 0.269437E-05);
  elMo->AddIsotope(Mo95, 0.463723E-05);
  elMo->AddIsotope(Mo96, 0.485860E-05);
  elMo->AddIsotope(Mo97, 0.278175E-05);
  elMo->AddIsotope(Mo98, 0.702866E-05);
  elMo->AddIsotope(Mo100, 0.280506E-05);
  
  auto* elW = new G4Element("M10_W", "W", 4);
  elW->AddIsotope(W182, 8.82837E-04);
  elW->AddIsotope(W183, 4.81548E-04);
  elW->AddIsotope(W184, 1.02329E-03);
  elW->AddIsotope(W186, 9.49719E-04);
  
  auto* elCu = new G4Element("M10_Cu", "Cu", 2);
  elCu->AddIsotope(Cu63, 3.04220E-05);
  elCu->AddIsotope(Cu65, 1.36041E-05);
  
  auto* elTi = new G4Element("M10_Ti", "Ti", 5);
  elTi->AddIsotope(Ti46, 0.962635E-05);
  elTi->AddIsotope(Ti47, 0.868122E-05);
  elTi->AddIsotope(Ti48, 0.860187E-04);
  elTi->AddIsotope(Ti49, 0.631255E-05);
  elTi->AddIsotope(Ti50, 0.604418E-05);
  
  // Recreate M10 material with all elements by mass fraction
  // Convert atom fractions to mass fractions (multiply by atomic mass, normalize)
  // Using atomic masses: Fe~55.8, Cr~52, Ni~58.7, Mo~96, Mn~55, Si~28, C~12, etc.
  // Sum for normalization
  G4double total = 0.0;
  // Fe contributions
  total += 5.17648E-02 * 53.94 + 8.18419E-01 * 55.93 + 1.96349E-02 * 56.94 + 2.67749E-03 * 57.93;
  // O16
  total += 3.49334E-04 * 16.0;
  // Cr contributions
  total += 4.15992E-03 * 49.95 + 8.10701E-02 * 51.94 + 9.28727E-03 * 52.94 + 2.32182E-03 * 53.94;
  // Mo contributions
  total += (0.432264E-05 + 0.269437E-05 + 0.463723E-05 + 0.485860E-05 + 0.278175E-05 + 0.702866E-05 + 0.280506E-05) * 96.0;
  // Mn55
  total += 4.06940E-03 * 54.94;
  // Ni contributions
  total += 3.22699E-05 * 57.94 + 1.24701E-05 * 59.93 + 6.18745E-07 * 60.93 + 1.76104E-06 * 61.93 + 5.71149E-07 * 63.93;
  // W contributions
  total += 8.82837E-04 * 183.84 + 4.81548E-04 * 183.93 + 1.02329E-03 * 184.0 + 9.49719E-04 * 186.0;
  // V (nat)
  total += 2.19433E-03 * 50.94;
  // Cu contributions  
  total += 3.04220E-05 * 62.93 + 1.36041E-05 * 64.93;
  // Nb93
  total += 6.01587E-06 * 92.91;
  // Co59
  total += 4.73709E-05 * 58.93;
  // Al27
  total += 2.07146E-04 * 26.98;
  // Ta181
  total += 2.16216E-04 * 180.95;
  // Ti contributions
  total += 0.962635E-05 * 45.95 + 0.868122E-05 * 46.95 + 0.860187E-04 * 47.95 + 0.631255E-05 * 48.95 + 0.604418E-05 * 49.95;
  
  // Mass fractions (normalized)
  G4double feMass = (5.17648E-02 * 53.94 + 8.18419E-01 * 55.93 + 1.96349E-02 * 56.94 + 2.67749E-03 * 57.93) / total;
  G4double oMass = 3.49334E-04 * 16.0 / total;
  G4double crMass = (4.15992E-03 * 49.95 + 8.10701E-02 * 51.94 + 9.28727E-03 * 52.94 + 2.32182E-03 * 53.94) / total;
  G4double moMass = (0.432264E-05 + 0.269437E-05 + 0.463723E-05 + 0.485860E-05 + 0.278175E-05 + 0.702866E-05 + 0.280506E-05) * 96.0 / total;
  G4double mnMass = 4.06940E-03 * 54.94 / total;
  G4double niMass = (3.22699E-05 * 57.94 + 1.24701E-05 * 59.93 + 6.18745E-07 * 60.93 + 1.76104E-06 * 61.93 + 5.71149E-07 * 63.93) / total;
  G4double wMass = (8.82837E-04 * 183.84 + 4.81548E-04 * 183.93 + 1.02329E-03 * 184.0 + 9.49719E-04 * 186.0) / total;
  G4double vMass = 2.19433E-03 * 50.94 / total;
  G4double cuMass = (3.04220E-05 * 62.93 + 1.36041E-05 * 64.93) / total;
  G4double nbMass = 6.01587E-06 * 92.91 / total;
  G4double coMass = 4.73709E-05 * 58.93 / total;
  G4double alMass = 2.07146E-04 * 26.98 / total;
  G4double taMass = 2.16216E-04 * 180.95 / total;
  G4double tiMass = (0.962635E-05 * 45.95 + 0.868122E-05 * 46.95 + 0.860187E-04 * 47.95 + 0.631255E-05 * 48.95 + 0.604418E-05 * 49.95) / total;
  
  // Create M10 material with proper definition (14 components)
  auto* M10_SS = new G4Material("M10_STAINLESS-STEEL", 7.92 * g/cm3, 14);
  M10_SS->AddElement(elFe, feMass);
  M10_SS->AddElement(nist->FindOrBuildElement("O"), oMass);
  M10_SS->AddElement(elCr, crMass);
  M10_SS->AddElement(elMo, moMass);
  M10_SS->AddElement(nist->FindOrBuildElement("Mn"), mnMass);
  M10_SS->AddElement(elNi, niMass);
  M10_SS->AddElement(elW, wMass);
  M10_SS->AddElement(nist->FindOrBuildElement("V"), vMass);
  M10_SS->AddElement(elCu, cuMass);
  M10_SS->AddElement(nist->FindOrBuildElement("Nb"), nbMass);
  M10_SS->AddElement(nist->FindOrBuildElement("Co"), coMass);
  M10_SS->AddElement(nist->FindOrBuildElement("Al"), alMass);
  M10_SS->AddElement(nist->FindOrBuildElement("Ta"), taMass);
  M10_SS->AddElement(elTi, tiMass);

  fDefaultMaterial = nist->FindOrBuildMaterial("G4_AIR");
  fAbsorMaterial[1] = nist->FindOrBuildMaterial("G4_Fe");
  fAbsorMaterial[2] = nist->FindOrBuildMaterial("G4_Pb");
  fAbsorMaterial[3] = nist->FindOrBuildMaterial("G4_GRAPHITE");
  fAbsorMaterial[4] = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  fAbsorMaterial[5] = nist->FindOrBuildMaterial("G4_Pb");
  fAbsorMaterial[6] = M10_SS;  // Use custom M10 stainless steel
  fAbsorMaterial[7] = nist->FindOrBuildMaterial("G4_CONCRETE");
  fAbsorMaterial[8] = nist->FindOrBuildMaterial("G4_WATER");
  fAbsorMaterial[9] = nist->FindOrBuildMaterial("G4_CONCRETE");
}

G4Material* DetectorConstruction::MaterialWithSingleIsotope(G4String name,
                                                             G4String symbol,
                                                             G4double density,
                                                             G4int Z,
                                                             G4int A) {
  G4int ncomponents = 1;
  G4double abundance = 100. * perCent;
  G4double fractionmass = 100. * perCent;

  auto* isotope = new G4Isotope(symbol, Z, A);
  auto* element = new G4Element(name, symbol, ncomponents);
  element->AddIsotope(isotope, abundance);

  auto* material = new G4Material(name, density, ncomponents);
  material->AddElement(element, fractionmass);
  return material;
}

void DetectorConstruction::ComputeParameters() {
  fAbsorSizeX = 445.4 * cm;
  fAbsorSizeYZ = 445.4 * cm;
  fWorldSizeX = 10000. * cm;
  fWorldSizeYZ = 10000. * cm;
}

std::array<G4double, 6> DetectorConstruction::GetRpp(G4int surfaceId) const {
  // 完全匹配 test2.i 的RPP定义
  static const std::map<G4int, std::array<G4double, 6>> kRpp = {
      // 主要屏蔽层RPP
      {2, {-65.2, 65.2, -65.2, 65.2, 0.0, 183.0}},
      {3, {42.8, 65.2, -65.2, 65.2, 113.0, 183.0}},
      {4, {-67.7, 67.7, -67.7, 67.7, 0.0, 185.5}},
      {5, {45.3, 67.7, -67.7, 67.7, 115.5, 185.5}},
      {6, {-87.7, 87.7, -87.7, 87.7, 0.0, 205.5}},
      {7, {65.3, 87.7, -87.7, 87.7, 135.5, 205.5}},
      {8, {-107.7, 107.7, -107.7, 107.7, 0.0, 225.5}},
      {9, {85.3, 107.7, -107.7, 107.7, 155.5, 225.5}},
      {10, {-207.7, 207.7, -207.7, 207.7, 0.0, 325.5}},
      {11, {-217.7, 217.7, -217.7, 217.7, 0.0, 335.5}},
      {12, {-222.7, 222.7, -222.7, 222.7, 0.0, 340.5}},
      // 开口和管道
      {21, {-50.0, 0.0, -300.0, 0.0, 35.0, 85.0}},
      // 门组件RPP
      {30, {-60.0, 10.0, -300.0, -250.0, 0.0, 85.0}},
      {31, {-60.0, 10.0, -300.0, -222.7, 85.0, 105.0}},
      {32, {-60.0, 10.0, -310.0, -222.7, 0.0, 110.0}},
      {33, {-60.0, 10.0, -250.0, -222.7, 0.0, 85.0}},
      {34, {-60.0, 10.0, -315.0, -222.7, 0.0, 115.0}},
      // 地基和冷却
      {35, {-790.0, 790.0, -735.0, 735.0, -150.0, 0.0}},
      {36, {47.7, 247.7, -20.0, 20.0, -15.0, 0.0}},
      {37, {-341.2, -70.0, -62.5, 62.5, -5.0, 0.0}}
  };
  
  auto it = kRpp.find(surfaceId);
  if (it == kRpp.end()) return {0., 0., 0., 0., 0., 0.};
  return it->second;
}

G4VPhysicalVolume* DetectorConstruction::ConstructVolumes() {
  ComputeParameters();
  
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
  
  // ==================== WORLD (Cell 1) ====================
  // MCNP Surface 1: SO 5000
  auto* worldSolid = new G4Orb("World", 5000.0 * cm);
  auto* worldLogic = new G4LogicalVolume(worldSolid, fDefaultMaterial, "World");
  fPhysiWorld = new G4PVPlacement(nullptr, G4ThreeVector(), worldLogic, "World", 
                                   nullptr, false, 0, true);
  
  // ==================== c2_M1_AIR: 主空气体积 (Cell 2) ====================
  // MCNP Cell 2: -1 12 #20 #21 #22 #23
  auto* c2Solid = MakeRppSolid("c2_air", GetRpp(2));
  auto* c2Logic = new G4LogicalVolume(c2Solid, fDefaultMaterial, "c2_M1_AIR");
  new G4PVPlacement(nullptr, RppCenter(GetRpp(2)), c2Logic, "c2_M1_AIR", 
                    worldLogic, false, 102, true);
  
  // ==================== c3_M1_AIR: 凹槽 (Cell 3, c2的女儿) ====================
  // MCNP Cell 3: -2 3
  auto* c3Solid = MakeRppSolid("c3_notch", GetRpp(3));
  auto* c3Logic = new G4LogicalVolume(c3Solid, fDefaultMaterial, "c3_M1_AIR");
  new G4PVPlacement(nullptr, RppCenter(GetRpp(3)) - RppCenter(GetRpp(2)), 
                    c3Logic, "c3_M1_AIR", c2Logic, false, 103, true);
  
  // ==================== 屏蔽层 (Cell 4-9) ====================
  
  // --- c4_M6_FE: 内层铁壳 (Cell 4) ---
  // MCNP Cell 4: -4 5 21-29 #3
  {
    auto* outer = MakeRppSolid("c4_outer", GetRpp(4));
    auto* inner = MakeRppSolid("c4_inner", GetRpp(2));
    const G4ThreeVector center4 = RppCenter(GetRpp(4));
    const G4ThreeVector center2 = RppCenter(GetRpp(2));
    
    G4VSolid* shell = new G4SubtractionSolid("c4_shell", outer, inner, nullptr, center2 - center4);
    shell = ApplyOpenings("c4", shell, center4);
    
    auto* c3Cut = MakeRppSolid("c4_c3", GetRpp(3));
    shell = new G4SubtractionSolid("c4_minus_c3", shell, c3Cut, nullptr,
                                    RppCenter(GetRpp(3)) - center4);
    
    auto* c4Logic = new G4LogicalVolume(shell, fAbsorMaterial[1], "c4_M6_FE");
    new G4PVPlacement(nullptr, center4, c4Logic, "c4_M6_FE", worldLogic, false, 4, true);
  }
  
  // --- c5_M4_PB: 内层铅壳 (Cell 5) ---
  // MCNP Cell 5: -6 7 21-29 #3#4
  {
    auto* outer = MakeRppSolid("c5_outer", GetRpp(6));
    auto* inner = MakeRppSolid("c5_inner", GetRpp(4));
    const G4ThreeVector center6 = RppCenter(GetRpp(6));
    const G4ThreeVector center4 = RppCenter(GetRpp(4));
    
    G4VSolid* shell = new G4SubtractionSolid("c5_shell", outer, inner, nullptr, center4 - center6);
    shell = ApplyOpenings("c5", shell, center6);
    
    auto* c3Cut = MakeRppSolid("c5_c3", GetRpp(3));
    shell = new G4SubtractionSolid("c5_minus_c3", shell, c3Cut, nullptr, RppCenter(GetRpp(3)) - center6);
    auto* c4Cut = MakeRppSolid("c5_c4", GetRpp(4));
    shell = new G4SubtractionSolid("c5_minus_c4", shell, c4Cut, nullptr, center4 - center6);
    
    auto* c5Logic = new G4LogicalVolume(shell, fAbsorMaterial[2], "c5_M4_PB");
    new G4PVPlacement(nullptr, center6, c5Logic, "c5_M4_PB", worldLogic, false, 5, true);
  }
  
  // --- c6_M8_GRAPHITE: 石墨壳 (Cell 6) ---
  // MCNP Cell 6: -8 9 21-29 #3#4#5
  {
    auto* outer = MakeRppSolid("c6_outer", GetRpp(8));
    auto* inner = MakeRppSolid("c6_inner", GetRpp(6));
    const G4ThreeVector center8 = RppCenter(GetRpp(8));
    const G4ThreeVector center6 = RppCenter(GetRpp(6));
    
    G4VSolid* shell = new G4SubtractionSolid("c6_shell", outer, inner, nullptr, center6 - center8);
    shell = ApplyOpenings("c6", shell, center8);
    
    auto* c3Cut = MakeRppSolid("c6_c3", GetRpp(3));
    shell = new G4SubtractionSolid("c6_minus_c3", shell, c3Cut, nullptr, RppCenter(GetRpp(3)) - center8);
    auto* c4Cut = MakeRppSolid("c6_c4", GetRpp(4));
    shell = new G4SubtractionSolid("c6_minus_c4", shell, c4Cut, nullptr, RppCenter(GetRpp(4)) - center8);
    auto* c6InnerCut = MakeRppSolid("c6_c5", GetRpp(6));
    shell = new G4SubtractionSolid("c6_minus_c5", shell, c6InnerCut, nullptr, center6 - center8);
    
    auto* c6Logic = new G4LogicalVolume(shell, fAbsorMaterial[3], "c6_M8_GRAPHITE");
    new G4PVPlacement(nullptr, center8, c6Logic, "c6_M8_GRAPHITE", worldLogic, false, 6, true);
  }
  
  // --- c7_M7_BPE: 含硼聚乙烯 (Cell 7, 最复杂) ---
  // MCNP Cell 7: -10 (21 8 -1201...-1224 #3#4#5#6 #901...#914):(-9)
  {
    auto* rpp10Box = MakeRppSolid("c7_rpp10", GetRpp(10));
    const G4ThreeVector center10 = RppCenter(GetRpp(10));
    
    G4VSolid* c7Solid = rpp10Box;
    c7Solid = ApplyChamferPlanes("c7", c7Solid, center10, true, true, true);
    c7Solid = ApplyOpenings("c7", c7Solid, center10);
    
    auto* c3Cut = MakeRppSolid("c7_c3", GetRpp(3));
    c7Solid = new G4SubtractionSolid("c7_minus_c3", c7Solid, c3Cut, nullptr, RppCenter(GetRpp(3)) - center10);
    auto* c4Cut = MakeRppSolid("c7_c4", GetRpp(4));
    c7Solid = new G4SubtractionSolid("c7_minus_c4", c7Solid, c4Cut, nullptr, RppCenter(GetRpp(4)) - center10);
    auto* c6Cut = MakeRppSolid("c7_c6", GetRpp(6));
    c7Solid = new G4SubtractionSolid("c7_minus_c6", c7Solid, c6Cut, nullptr, RppCenter(GetRpp(6)) - center10);
    auto* c8Cut = MakeRppSolid("c7_c8_inner", GetRpp(8));
    c7Solid = new G4SubtractionSolid("c7_minus_inner", c7Solid, c8Cut, nullptr, RppCenter(GetRpp(8)) - center10);
    
    auto* rpp9Box = MakeRppSolid("c7_minus_rpp9", GetRpp(9));
    c7Solid = new G4SubtractionSolid("c7_final", c7Solid, rpp9Box, nullptr, RppCenter(GetRpp(9)) - center10);
    
    auto* c7Logic = new G4LogicalVolume(c7Solid, fAbsorMaterial[4], "c7_M7_BPE");
    new G4PVPlacement(nullptr, center10, c7Logic, "c7_M7_BPE", worldLogic, false, 7, true);
  }
  
  // --- c8_M4_PB: 外层铅壳 (Cell 8) ---
  // MCNP Cell 8: -11 21 8 -1201...-1224 #3#4#5#6#7#901-914
  {
    auto* rpp11Box = MakeRppSolid("c8_rpp11", GetRpp(11));
    const G4ThreeVector center11 = RppCenter(GetRpp(11));
    
    G4VSolid* c8Solid = rpp11Box;
    c8Solid = ApplyChamferPlanes("c8", c8Solid, center11, true, true, true);
    c8Solid = ApplyOpenings("c8", c8Solid, center11);
    
    c8Solid = new G4SubtractionSolid("c8_minus_c3", c8Solid, MakeRppSolid("", GetRpp(3)), nullptr, 
                                     RppCenter(GetRpp(3)) - center11);
    c8Solid = new G4SubtractionSolid("c8_minus_c4", c8Solid, MakeRppSolid("", GetRpp(4)), nullptr, 
                                     RppCenter(GetRpp(4)) - center11);
    c8Solid = new G4SubtractionSolid("c8_minus_c6", c8Solid, MakeRppSolid("", GetRpp(6)), nullptr, 
                                     RppCenter(GetRpp(6)) - center11);
    c8Solid = new G4SubtractionSolid("c8_minus_c10", c8Solid, MakeRppSolid("", GetRpp(10)), nullptr,
                                     RppCenter(GetRpp(10)) - center11);
    
    auto* rpp8Box = MakeRppSolid("c8_minus_rpp8", GetRpp(8));
    c8Solid = new G4SubtractionSolid("c8_shell", c8Solid, rpp8Box, nullptr, RppCenter(GetRpp(8)) - center11);
    
    auto* c8Logic = new G4LogicalVolume(c8Solid, fAbsorMaterial[2], "c8_M4_PB");
    new G4PVPlacement(nullptr, center11, c8Logic, "c8_M4_PB", worldLogic, false, 8, true);
  }
  
  // --- c9_M10_SS: 外层不锈钢壳 (Cell 9) ---
  // MCNP Cell 9: -12 11 21 -1201...-1224 #901-914
  {
    auto* outer = MakeRppSolid("c9_outer", GetRpp(12));
    auto* inner = MakeRppSolid("c9_inner", GetRpp(11));
    const G4ThreeVector center12 = RppCenter(GetRpp(12));
    const G4ThreeVector center11 = RppCenter(GetRpp(11));
    
    G4VSolid* shell = new G4SubtractionSolid("c9_shell", outer, inner, nullptr, center11 - center12);
    shell = ApplyChamferPlanes("c9", shell, center12, true, true, true);
    shell = ApplyOpenings("c9", shell, center12);
    
    auto* c9Logic = new G4LogicalVolume(shell, fAbsorMaterial[6], "c9_M10_SS");
    new G4PVPlacement(nullptr, center12, c9Logic, "c9_M10_SS", worldLogic, false, 9, true);
    
    // --- c901-904_M10_SS: 支撑结构 (Cell 901-904, c9的女儿) ---
    for (int i = 0; i < 4; ++i) {
      auto* support = new G4Box("c90" + std::to_string(i+1), 10*cm, 10*cm, 20*cm);
      auto* supportLogic = new G4LogicalVolume(support, fAbsorMaterial[6], 
                                                "c90" + std::to_string(i+1) + "_M10_SS");
      G4ThreeVector pos((i%2==0?1:-1)*200*cm, (i<2?1:-1)*200*cm, 150*cm);
      new G4PVPlacement(nullptr, pos, supportLogic, "c90" + std::to_string(i+1) + "_M10_SS",
                        c9Logic, false, 900+i+1, true);
    }
  }
  
  // ==================== 门组件 (Cell 20-22) ====================
  if (!fDoorOpen) {
    // --- c20_M7_BPE: BPE门 ---
    // MCNP Cell 20: (-30):(-31)
    {
      auto* rpp30 = MakeRppSolid("c20_rpp30", GetRpp(30));
      auto* rpp31 = MakeRppSolid("c20_rpp31", GetRpp(31));
      const G4ThreeVector center30 = RppCenter(GetRpp(30));
      const G4ThreeVector center31 = RppCenter(GetRpp(31));

      auto* c20Union = new G4UnionSolid("c20_union", rpp30, rpp31, nullptr, center31 - center30);
      auto* c20Logic = new G4LogicalVolume(c20Union, fAbsorMaterial[4], "c20_M7_BPE");
      new G4PVPlacement(nullptr, center30, c20Logic, "c20_M7_BPE", worldLogic, false, 20, true);
    }

    // --- c21_M4_PB: 铅门 ---
    // MCNP Cell 21: -32 33 #20
    {
      auto* rpp32 = MakeRppSolid("c21_rpp32", GetRpp(32));
      auto* rpp33 = MakeRppSolid("c21_rpp33", GetRpp(33));
      const G4ThreeVector center32 = RppCenter(GetRpp(32));
      const G4ThreeVector center33 = RppCenter(GetRpp(33));

      G4VSolid* c21Solid = new G4SubtractionSolid("c21_minus_33", rpp32, rpp33, nullptr, center33 - center32);
      auto* c20Box = MakeRppSolid("c20_cut", GetRpp(30));
      const G4ThreeVector center30 = RppCenter(GetRpp(30));
      c21Solid = new G4SubtractionSolid("c21_minus_c20", c21Solid, c20Box, nullptr, center30 - center32);

      auto* c21Logic = new G4LogicalVolume(c21Solid, fAbsorMaterial[2], "c21_M4_PB");
      new G4PVPlacement(nullptr, center32, c21Logic, "c21_M4_PB", worldLogic, false, 21, true);
    }

    // --- c22_M10_SS: 钢门 ---
    // MCNP Cell 22: -34 33 #21 #20
    {
      auto* rpp34 = MakeRppSolid("c22_rpp34", GetRpp(34));
      auto* rpp33 = MakeRppSolid("c22_rpp33", GetRpp(33));
      const G4ThreeVector center34 = RppCenter(GetRpp(34));
      const G4ThreeVector center33 = RppCenter(GetRpp(33));

      G4VSolid* c22Solid = new G4SubtractionSolid("c22_minus_33", rpp34, rpp33, nullptr, center33 - center34);
      auto* c21Box = MakeRppSolid("c21_cut", GetRpp(32));
      const G4ThreeVector center32 = RppCenter(GetRpp(32));
      c22Solid = new G4SubtractionSolid("c22_minus_c21", c22Solid, c21Box, nullptr, center32 - center34);

      auto* c22Logic = new G4LogicalVolume(c22Solid, fAbsorMaterial[6], "c22_M10_SS");
      new G4PVPlacement(nullptr, center34, c22Logic, "c22_M10_SS", worldLogic, false, 22, true);
    }
  }
  
  // ==================== c23_M9_CONCRETE: 地基 (Cell 23) ====================
  // MCNP Cell 23: -35 36 38
  {
    auto* rpp35 = MakeRppSolid("c23_rpp35", GetRpp(35));
    auto* rpp36 = MakeRppSolid("c23_rpp36", GetRpp(36));
    const G4ThreeVector center35 = RppCenter(GetRpp(35));
    const G4ThreeVector center36 = RppCenter(GetRpp(36));
    
    G4VSolid* c23Solid = new G4SubtractionSolid("c23_minus_36", rpp35, rpp36, nullptr, center36 - center35);
    
    // BOX 38
    G4ThreeVector orig38(-16.05, 51.41, -15.0);
    G4ThreeVector u38(-141.42, 141.42, 0);
    G4ThreeVector v38(-35.36, -35.36, 0);
    G4ThreeVector w38(0, 0, 15.0);
    auto* box38 = new G4Box("c23_box38", 0.5*u38.mag()*cm, 0.5*v38.mag()*cm, 0.5*w38.mag()*cm);
    auto* rot38 = new G4RotationMatrix();
    rot38->rotateZ(u38.phi());
    G4ThreeVector center38 = (orig38 + 0.5*(u38+v38+w38)) * cm;
    c23Solid = new G4SubtractionSolid("c23_final", c23Solid, box38, rot38, center38 - center35);
    
    auto* c23Logic = new G4LogicalVolume(c23Solid, fAbsorMaterial[7], "c23_M9_CONCRETE");
    new G4PVPlacement(nullptr, center35, c23Logic, "c23_M9_CONCRETE", worldLogic, false, 23, true);
    
    // --- c36_M5_WATER: 冷却水 ---
    auto* waterSolid = MakeRppSolid("c36_water", GetRpp(36));
    auto* c36Logic = new G4LogicalVolume(waterSolid, fAbsorMaterial[8], "c36_M5_WATER");
    new G4PVPlacement(nullptr, center36, c36Logic, "c36_M5_WATER", worldLogic, false, 36, true);
  }
  
  // ==================== 热室墙 (扩展功能) ====================
  const auto shieldOuter = GetRpp(12);
  const G4double gap = 30.0 * cm;
  const G4double xFront = shieldOuter[1] * cm + gap;
  const G4double wallHalfX = 0.5 * fHotcellWallThickness;
  const G4double wallHalfY = 1400.0 * cm;
  const G4double wallHalfZ = 2000.0 * cm;
  auto* hotcellWallSolid = new G4Box("hotcell_wall", wallHalfX, wallHalfY, wallHalfZ);
  auto* hotcellWallLogic = new G4LogicalVolume(hotcellWallSolid, fAbsorMaterial[7], "hotcell_wall");
  const G4ThreeVector wallCenter(xFront + wallHalfX, 0.0, wallHalfZ);
  new G4PVPlacement(nullptr, wallCenter, hotcellWallLogic, "hotcell_wall",
                    worldLogic, false, 501, true);

  // ==================== 钽靶 (来自P_Ta_Channel_Angular) ====================
  // 在(0, 0, 90cm)位置放置钽靶
  ConstructTantalumTarget(worldLogic);

  PrintParameters();
  return fPhysiWorld;
}

void DetectorConstruction::PrintParameters() {
  G4cout << "\n-------------------------------------------------------------";
  G4cout << "\n Shield model matched to test2.i";
  G4cout << "\n doorOpen = " << (fDoorOpen ? "true" : "false");
  G4cout << "\n hotcellWallThickness = " << fHotcellWallThickness / cm << " cm";
  for (G4int i = 1; i <= fNbOfAbsor; ++i) {
    if (fAbsorMaterial[i] == nullptr) continue;
    G4cout << "\n layer " << std::setw(2) << i << " material: " << fAbsorMaterial[i]->GetName();
  }
  G4cout << "\n-------------------------------------------------------------\n" << G4endl;
}

void DetectorConstruction::SetNbOfAbsor(G4int val) {
  fNbOfAbsor = std::max(1, std::min(val, kMaxAbsor - 1));
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void DetectorConstruction::SetAbsorMaterial(G4int iabs, const G4String& material) {
  if (iabs <= 0 || iabs >= kMaxAbsor) return;
  auto* mat = G4NistManager::Instance()->FindOrBuildMaterial(material);
  if (mat != nullptr) {
    fAbsorMaterial[iabs] = mat;
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  }
}

void DetectorConstruction::SetAbsorThickness(G4int iabs, G4double val) {
  if (iabs <= 0 || iabs >= kMaxAbsor || val <= 0.) return;
  fAbsorThickness[iabs] = val;
}

void DetectorConstruction::SetAbsorSizeYZ(G4double val) {
  if (val <= 0.) return;
  fAbsorSizeYZ = val;
}

void DetectorConstruction::ConstructSDandField() {
  if (fFieldMessenger.Get() == nullptr) {
    auto* msg = new G4GlobalMagFieldMessenger(G4ThreeVector());
    G4AutoDelete::Register(msg);
    fFieldMessenger.Put(msg);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
// 钽靶建模 - 从P_Ta_Channel_Angular集成，放置在(0, 0, 90cm)
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructTantalumTarget(G4LogicalVolume* worldLogic) {
  G4NistManager* nist = G4NistManager::Instance();

  // 材料定义
  G4Material* Ta_mat = nist->FindOrBuildMaterial("G4_Ta");
  G4Material* water_mat = nist->FindOrBuildMaterial("G4_WATER");
  G4Material* Cu_mat = nist->FindOrBuildMaterial("G4_Cu");

  // 可视化属性
  static const G4Colour yellow = G4Colour(1, 1, 0, 0.9);
  static const G4Colour blue = G4Colour(0.0, 0.0, 1.0, 0.6);
  static const G4Colour brown = G4Colour(0.9, 0.5, 0.0, 0.6);
  G4VisAttributes* Ta_vis = new G4VisAttributes(yellow);
  G4VisAttributes* Water_vis = new G4VisAttributes(blue);
  G4VisAttributes* Cu_vis = new G4VisAttributes(brown);
  Ta_vis->SetForceSolid(true);
  Water_vis->SetForceSolid(true);
  Cu_vis->SetForceSolid(true);
  G4bool checkOverlaps = true;

  // 目标位置 - (0, 0, 90cm)
  G4ThreeVector targetPosition(0, 0, 90 * cm);

  // 创建TargetRegion用于追踪
  auto* TargetRegion = new G4Region("TargetRegion");

  // Shape1: 5.5mm Ta靶 (内半径185mm, 外半径250mm, 厚度3.9mm)
  G4double Shape1_rmina = 185 * mm, Shape1_rmaxa = 250 * mm;
  G4double Shape1_hz = 3.9 * mm;
  G4double Shape1_phimin = 0. * deg, Shape1_phimax = 360. * deg;

  // 相对于目标中心的偏移
  G4ThreeVector pos1 = targetPosition + G4ThreeVector(0, 0, 0.5 * Shape1_hz);

  auto solidShape1 = new G4Tubs("Shape1", Shape1_rmina, Shape1_rmaxa,
      0.5 * Shape1_hz, Shape1_phimin, Shape1_phimax);

  auto logicShape1 = new G4LogicalVolume(solidShape1, Ta_mat, "Shape1");
  logicShape1->SetVisAttributes(Ta_vis);
  new G4PVPlacement(nullptr, pos1, logicShape1, "Shape1",
      worldLogic, false, 0, checkOverlaps);

  logicShape1->SetRegion(TargetRegion);
  TargetRegion->AddRootLogicalVolume(logicShape1);

  // Shape2: 1.3mm Ta层 (内半径185mm, 外半径250mm, 厚度1.3mm)
  G4double Shape2_rmin = 185 * mm, Shape2_rmax = 250 * mm;
  G4double Shape2_hz = 1.3 * mm;
  G4double Shape2_phimin = 0. * deg, Shape2_phimax = 360. * deg;

  G4ThreeVector pos2 = targetPosition + G4ThreeVector(0, 0, (Shape1_hz + 0.5 * Shape2_hz));

  auto solidShape2 = new G4Tubs("Shape2", Shape2_rmin, Shape2_rmax,
      0.5 * Shape2_hz, Shape2_phimin, Shape2_phimax);

  auto logicShape2 = new G4LogicalVolume(solidShape2, Ta_mat, "Shape2");
  logicShape2->SetVisAttributes(Ta_vis);
  new G4PVPlacement(nullptr, pos2, logicShape2, "Shape2",
      worldLogic, false, 0, checkOverlaps);

  logicShape2->SetRegion(TargetRegion);
  TargetRegion->AddRootLogicalVolume(logicShape2);

  // 在Shape2中添加水扇区(简化版本 - 仅添加环形水层)
  G4double sector_Layer1_hz = 0.5 * mm;
  G4int numSectors = 36;  // 简化: 36个扇区
  G4double sectorAngleSpan = 360.0 * deg / numSectors * 0.5;  // 占50%角度

  for (G4int i = 0; i < numSectors; i++) {
    G4double startAngle = i * (360.0 * deg / numSectors);
    G4String sectorName = "Shape2_Sector_" + std::to_string(i);

    auto solidSector = new G4Tubs(sectorName + "_Solid",
        Shape2_rmin + 5 * mm,  // 内半径+5mm安全距离
        Shape2_rmax - 5 * mm,  // 外半径-5mm安全距离
        0.5 * sector_Layer1_hz,
        startAngle,
        sectorAngleSpan);

    auto logicSector = new G4LogicalVolume(solidSector, water_mat, sectorName + "_Logic");
    logicSector->SetVisAttributes(Water_vis);

    // 在Shape2的局部坐标系中放置扇区(Shape2的中心即为局部坐标系原点)
    // 水扇区在Shape2内垂直居中放置，Z=0表示在Shape2的中心位置
    G4ThreeVector sectorPos(0, 0, 0);

    new G4PVPlacement(nullptr, sectorPos, logicSector, sectorName + "_Phys",
        logicShape2, false, i, checkOverlaps);
  }

  // Shape3: 5mm 水层
  G4double Shape3_rmin = 185 * mm, Shape3_rmax = 250 * mm;
  G4double Shape3_hz = 5 * mm;
  G4double Shape3_phimin = 0. * deg, Shape3_phimax = 360. * deg;

  G4ThreeVector pos3 = targetPosition + G4ThreeVector(0, 0, (Shape1_hz + Shape2_hz + 0.5 * Shape3_hz));

  auto solidShape3 = new G4Tubs("Shape3", Shape3_rmin, Shape3_rmax,
      0.5 * Shape3_hz, Shape3_phimin, Shape3_phimax);

  auto logicShape3 = new G4LogicalVolume(solidShape3, water_mat, "Shape3");
  logicShape3->SetVisAttributes(Water_vis);
  new G4PVPlacement(nullptr, pos3, logicShape3, "Shape3",
      worldLogic, false, 0, checkOverlaps);

  logicShape3->SetRegion(TargetRegion);
  TargetRegion->AddRootLogicalVolume(logicShape3);

  // Shape4: 1mm 不锈钢包壳层 (原为Cu层)
  G4double Shape4_rmin = 185 * mm, Shape4_rmax = 250 * mm;
  G4double Shape4_hz = 1.0 * mm;
  G4double Shape4_phimin = 0. * deg, Shape4_phimax = 360. * deg;

  G4ThreeVector pos4 = targetPosition + G4ThreeVector(0, 0,
      (Shape1_hz + Shape2_hz + Shape3_hz + 0.5 * Shape4_hz));

  auto solidShape4 = new G4Tubs("Shape4", Shape4_rmin, Shape4_rmax,
      0.5 * Shape4_hz, Shape4_phimin, Shape4_phimax);

  // 使用304不锈钢替代Cu材料
  G4Material* SS_mat = G4NistManager::Instance()->FindOrBuildMaterial("M10_STAINLESS-STEEL");
  auto logicShape4 = new G4LogicalVolume(solidShape4, SS_mat, "Shape4");
  static const G4Colour steelColor = G4Colour(0.7, 0.7, 0.75, 0.6);
  G4VisAttributes* SS_vis = new G4VisAttributes(steelColor);
  SS_vis->SetForceSolid(true);
  logicShape4->SetVisAttributes(SS_vis);
  new G4PVPlacement(nullptr, pos4, logicShape4, "Shape4",
      worldLogic, false, 0, checkOverlaps);

  logicShape4->SetRegion(TargetRegion);
  TargetRegion->AddRootLogicalVolume(logicShape4);

  // 中心圆筒: 内靶区 (半径18.5cm的圆柱)
  G4double CenterTube_rmin = 0 * mm, CenterTube_rmax = 185 * mm;
  G4double CenterTube_hz = Shape1_hz + Shape2_hz + Shape3_hz + Shape4_hz;
  G4double CenterTube_phimin = 0. * deg, CenterTube_phimax = 360. * deg;

  G4ThreeVector posCenter = targetPosition + G4ThreeVector(0, 0, 0.5 * CenterTube_hz);

  auto solidCenterTube = new G4Tubs("CenterTube", CenterTube_rmin, CenterTube_rmax,
      0.5 * CenterTube_hz, CenterTube_phimin, CenterTube_phimax);

  // 中心填充水
  auto logicCenterTube = new G4LogicalVolume(solidCenterTube, water_mat, "CenterTube");
  logicCenterTube->SetVisAttributes(Water_vis);
  new G4PVPlacement(nullptr, posCenter, logicCenterTube, "CenterTube",
      worldLogic, false, 0, checkOverlaps);

  logicCenterTube->SetRegion(TargetRegion);
  TargetRegion->AddRootLogicalVolume(logicCenterTube);

  G4cout << "\n=== 钽靶已放置在 (0, 0, 90cm) 位置 ===" << G4endl;
  G4cout << "  Shape1 (Ta): " << Shape1_hz/mm << " mm" << G4endl;
  G4cout << "  Shape2 (Ta+水扇区): " << Shape2_hz/mm << " mm" << G4endl;
  G4cout << "  Shape3 (水): " << Shape3_hz/mm << " mm" << G4endl;
  G4cout << "  Shape4 (Cu): " << Shape4_hz/mm << " mm" << G4endl;
  G4cout << "  中心管 (水): 半径 " << CenterTube_rmax/mm << " mm, 长度 " << CenterTube_hz/mm << " mm" << G4endl;
  G4cout << "=====================================\n" << G4endl;
}
