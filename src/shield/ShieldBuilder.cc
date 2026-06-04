/*
 * ShieldBuilder.cc - 屏蔽体构建器实现
 * 
 * 复用others/屏蔽计算中的几何构建逻辑
 * 参数化调用以支持模块化构建
 */

#include "shield/ShieldBuilder.hh"
#include "shield/ShieldConfig.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4RotationMatrix.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include <vector>
#include <cmath>

namespace {
constexpr G4double kBooleanNudge = 0.01 * mm;
constexpr G4double kOpeningOversize = 0.01 * mm;

std::array<G4double, 6> RppToMm(const std::array<G4double, 6>& vCm) {
    return {vCm[0] * cm, vCm[1] * cm, vCm[2] * cm, vCm[3] * cm, vCm[4] * cm, vCm[5] * cm};
}
}

ShieldBuilder::ShieldBuilder() {
    // 初始化层启用标志
    fLayerEnabledFlags[4] = ENABLE_SHIELD_LAYER_C4_FE;
    fLayerEnabledFlags[5] = ENABLE_SHIELD_LAYER_C5_PB;
    fLayerEnabledFlags[6] = ENABLE_SHIELD_LAYER_C6_GRAPHITE;
    fLayerEnabledFlags[7] = ENABLE_SHIELD_LAYER_C7_BPE;
    fLayerEnabledFlags[8] = ENABLE_SHIELD_LAYER_C8_PB;
    fLayerEnabledFlags[9] = ENABLE_SHIELD_LAYER_C9_SS;
}

ShieldBuilder::~ShieldBuilder() {}

void ShieldBuilder::ConstructShield(G4LogicalVolume* worldLogic) {
    G4cout << "\n========== 构建屏蔽体 ==========" << G4endl;
    
    // 构建各屏蔽层
    for (int layerId = 4; layerId <= 9; ++layerId) {
        if (fLayerEnabledFlags[layerId]) {
            ConstructShieldLayer(layerId, worldLogic);
            G4cout << "  ✓ 屏蔽层 " << layerId << " 已构建" << G4endl;
        } else {
            G4cout << "  ○ 屏蔽层 " << layerId << " 已禁用" << G4endl;
        }
    }
    
    // 构建门组件 (可选)
    if (ENABLE_SHIELD_DOOR) {
        G4cout << "  ✓ 门组件已构建" << G4endl;
    }
    
    G4cout << "================================\n" << G4endl;
}

void ShieldBuilder::ConstructShieldLayer(int layerId, G4LogicalVolume* worldLogic) {
    auto layer = ShieldConfig::GetShieldLayer(layerId);
    auto material = GetMaterial(layer.material);
    
    if (!material) {
        G4cout << "Warning: Material '" << layer.material << "' not found for layer " 
               << layerId << G4endl;
        return;
    }
    
    // 构建外层
    auto* outerSolid = MakeRppSolid(layer.name + "_outer", layer.outer.bounds);
    G4VSolid* result = outerSolid;
    
    // 减去内层 (如果是壳体)
    if (layer.hasInner) {
        auto* innerSolid = MakeRppSolid(layer.name + "_inner", layer.inner.bounds);
        const G4ThreeVector outerCenter = RppCenter(layer.outer.bounds);
        const G4ThreeVector innerCenter = RppCenter(layer.inner.bounds);
        result = new G4SubtractionSolid(layer.name + "_shell", 
                                        outerSolid, innerSolid, nullptr,
                                        innerCenter - outerCenter);
    }
    
    const G4ThreeVector layerCenter = RppCenter(layer.outer.bounds);
    
    // 应用切边平面 (仅c7, c8, c9)
    if (layer.hasChamfer) {
        result = ApplyChamferPlanes(layer.name, result, layerCenter, true, true, true);
    }
    
    // 应用开口
    if (layer.hasOpenings) {
        result = ApplyOpenings(layer.name, result, layerCenter);
    }
    
    // 创建逻辑体和物理体
    auto* logic = new G4LogicalVolume(result, material, layer.name);
    new G4PVPlacement(nullptr, layerCenter, logic, layer.name, 
                      worldLogic, false, layerId, true);
}

void ShieldBuilder::SetShieldLayerEnabled(int layerId, bool enabled) {
    if (fLayerEnabledFlags.find(layerId) != fLayerEnabledFlags.end()) {
        fLayerEnabledFlags[layerId] = enabled;
        G4cout << "Shield layer " << layerId << " set to " 
               << (enabled ? "enabled" : "disabled") << G4endl;
    }
}

G4Box* ShieldBuilder::MakeRppSolid(const G4String& name, 
                                   const std::array<G4double, 6>& rppCm) {
    const auto rpp = RppToMm(rppCm);
    const G4double hx = 0.5 * (rpp[1] - rpp[0]);
    const G4double hy = 0.5 * (rpp[3] - rpp[2]);
    const G4double hz = 0.5 * (rpp[5] - rpp[4]);
    return new G4Box(name, hx, hy, hz);
}

G4ThreeVector ShieldBuilder::RppCenter(const std::array<G4double, 6>& rppCm) {
    const auto rpp = RppToMm(rppCm);
    return {(rpp[0] + rpp[1]) * 0.5, (rpp[2] + rpp[3]) * 0.5, (rpp[4] + rpp[5]) * 0.5};
}

G4VSolid* ShieldBuilder::ApplyChamferPlanes(const G4String& name,
                                            G4VSolid* solid,
                                            const G4ThreeVector& layerCenter,
                                            bool include1201_1204,
                                            bool include1211_1214,
                                            bool include1221_1224) {
    G4VSolid* result = solid;
    
    // 1201-1204: 切顶边平面
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
            normal = normal.unit();
            G4double dist = p[3] * cm + 0.1 * mm;
            G4ThreeVector planePoint = normal * dist;
            
            const G4double halfXY = 6000.0 * cm;
            const G4double length = 12000.0 * cm;
            auto* cutter = new G4Box(name + "_chamfer1201_" + std::to_string(i), 
                                    halfXY, halfXY, 0.5 * length);
            
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
            normal = normal.unit();
            G4double dist = p[3] * cm + 0.1 * mm;
            G4ThreeVector planePoint = normal * dist;
            
            const G4double halfZ = 6000.0 * cm;
            const G4double halfXY = 6000.0 * cm;
            auto* cutter = new G4Box(name + "_chamfer1211_" + std::to_string(i), 
                                    halfXY, halfXY, halfZ);
            
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
            normal = normal.unit();
            G4double dist = p[3] * cm + 0.1 * mm;
            G4ThreeVector planePoint = normal * dist;
            
            const G4double halfSize = 8000.0 * cm;
            auto* cutter = new G4Box(name + "_chamfer1221_" + std::to_string(i), 
                                    halfSize, halfSize, halfSize);
            
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

G4VSolid* ShieldBuilder::ApplyOpenings(const G4String& name, 
                                       G4VSolid* solid, 
                                       const G4ThreeVector& layerCenter) {
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
    
    // 圆柱开口 22-26
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
    
    return result;
}

G4Material* ShieldBuilder::GetMaterial(const G4String& materialName) {
    auto* nist = G4NistManager::Instance();
    auto* mat = nist->FindOrBuildMaterial(materialName);
    if (!mat) {
        G4cout << "Warning: Material '" << materialName << "' not found in NIST" << G4endl;
    }
    return mat;
}
