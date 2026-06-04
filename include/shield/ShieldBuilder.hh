/*
 * ShieldBuilder.hh - 屏蔽体构建器
 * 
 * 职责：
 *   - 根据ShieldConfig参数化定义构建屏蔽体几何
 *   - 支持通过参数开关动态启用/禁用各屏蔽层
 *   - 集成开口、切边平面等几何特征
 */

#ifndef SHIELD_BUILDER_HH
#define SHIELD_BUILDER_HH

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"
#include "shield/ShieldConfig.hh"

/**
 * 屏蔽体构建器类
 * 
 * 使用方式：
 *   ShieldBuilder builder;
 *   builder.ConstructShield(worldLogic);
 */
class ShieldBuilder {
public:
    ShieldBuilder();
    ~ShieldBuilder();
    
    /**
     * 构建所有屏蔽层
     * @param worldLogic 世界逻辑体
     */
    void ConstructShield(G4LogicalVolume* worldLogic);
    
    /**
     * 构建单个屏蔽层
     * @param layerId 屏蔽层ID (4-9)
     * @param worldLogic 世界逻辑体
     */
    void ConstructShieldLayer(int layerId, G4LogicalVolume* worldLogic);
    
    /**
     * 设置屏蔽层启用状态（动态调整）
     * @param layerId 屏蔽层ID
     * @param enabled 是否启用
     */
    void SetShieldLayerEnabled(int layerId, bool enabled);
    
private:
    // 几何构建辅助函数
    G4Box* MakeRppSolid(const G4String& name, const std::array<G4double, 6>& rppCm);
    G4ThreeVector RppCenter(const std::array<G4double, 6>& rppCm);
    
    // 应用几何特征
    G4VSolid* ApplyChamferPlanes(const G4String& name,
                                 G4VSolid* solid,
                                 const G4ThreeVector& layerCenter,
                                 bool include1201_1204,
                                 bool include1211_1214,
                                 bool include1221_1224);
    
    G4VSolid* ApplyOpenings(const G4String& name,
                           G4VSolid* solid,
                           const G4ThreeVector& layerCenter);
    
    // 材料管理
    G4Material* GetMaterial(const G4String& materialName);
    
    // 状态管理
    std::map<int, bool> fLayerEnabledFlags;
};

#endif  // SHIELD_BUILDER_HH
