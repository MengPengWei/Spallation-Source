/*
 * ShieldConfig.hh - 屏蔽体参数化配置定义
 * 
 * 说明：
 *   - 集中定义所有屏蔽层的参数（厚度、材料、几何）
 *   - 使用struct管理每层的配置，便于后期调整
 *   - 支持通过宏开关启用/禁用各屏蔽层
 */

#ifndef SHIELD_CONFIG_HH
#define SHIELD_CONFIG_HH

#include "G4SystemOfUnits.hh"
#include <array>
#include <string>
#include <map>

// ========== 屏蔽层启用开关 ==========
#define ENABLE_SHIELD_LAYER_C4_FE         1   // c4: 内层铁壳
#define ENABLE_SHIELD_LAYER_C5_PB         1   // c5: 内层铅壳
#define ENABLE_SHIELD_LAYER_C6_GRAPHITE   1   // c6: 石墨壳
#define ENABLE_SHIELD_LAYER_C7_BPE        1   // c7: 含硼聚乙烯
#define ENABLE_SHIELD_LAYER_C8_PB         1   // c8: 外层铅壳
#define ENABLE_SHIELD_LAYER_C9_SS         1   // c9: 外层不锈钢壳
#define ENABLE_SHIELD_DOOR                1   // 门组件(c20-c22)
#define ENABLE_SHIELD_FOUNDATION          1   // 地基和冷却系统

// ========== 屏蔽层参数化定义 ==========
namespace ShieldConfig {

/**
 * RPP (长方体) 定义
 * 格式: {xmin, xmax, ymin, ymax, zmin, zmax} (单位: cm)
 */
struct RppDef {
    std::string name;
    std::array<G4double, 6> bounds;  // {xmin, xmax, ymin, ymax, zmin, zmax}
};

/**
 * 屏蔽层定义
 */
struct ShieldLayer {
    int id;                    // 层级ID
    std::string name;          // 层名称 (如 "c4_M6_FE")
    std::string material;      // 材料名称 (G4材料名或自定义)
    RppDef outer;              // 外边界RPP
    RppDef inner;              // 内边界RPP (如有)
    bool hasInner;             // 是否为壳体结构
    bool hasOpenings;          // 是否包含开口
    bool hasChamfer;           // 是否应用切边平面
};

// ========== RPP定义集合 ==========
extern const std::map<int, RppDef> RPP_DEFINITIONS;

// ========== 屏蔽层定义集合 ==========
extern const std::map<int, ShieldLayer> SHIELD_LAYER_DEFINITIONS;

/**
 * 获取RPP定义
 * @param id RPP标识符
 * @return RPP定义数据
 */
inline std::array<G4double, 6> GetRppBounds(int id) {
    auto it = RPP_DEFINITIONS.find(id);
    if (it != RPP_DEFINITIONS.end()) {
        return it->second.bounds;
    }
    return {0., 0., 0., 0., 0., 0.};
}

/**
 * 获取屏蔽层定义
 * @param id 屏蔽层ID
 * @return 屏蔽层定义
 */
inline ShieldLayer GetShieldLayer(int id) {
    auto it = SHIELD_LAYER_DEFINITIONS.find(id);
    if (it != SHIELD_LAYER_DEFINITIONS.end()) {
        return it->second;
    }
    return ShieldLayer{};
}

/**
 * 检查屏蔽层是否启用
 * @param id 屏蔽层ID
 * @return 是否启用
 */
bool IsShieldLayerEnabled(int id);

/**
 * 打印配置信息
 */
void PrintShieldConfig();

}  // namespace ShieldConfig

#endif  // SHIELD_CONFIG_HH
