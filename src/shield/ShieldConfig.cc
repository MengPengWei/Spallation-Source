/*
 * ShieldConfig.cc - 屏蔽体参数化配置实现
 * 
 * 包含所有屏蔽层的RPP定义、材料配置和几何参数
 * 支持通过宏开关启用/禁用各层
 */

#include "shield/ShieldConfig.hh"
#include "G4SystemOfUnits.hh"

namespace ShieldConfig {

// ========== RPP定义集合 ==========
const std::map<int, RppDef> RPP_DEFINITIONS = {
    // 主要屏蔽层RPP
    {2,  {"c2_air",            {-65.2, 65.2, -65.2, 65.2, 0.0, 183.0}}},
    {3,  {"c3_notch",          {42.8, 65.2, -65.2, 65.2, 113.0, 183.0}}},
    {4,  {"c4_outer",          {-67.7, 67.7, -67.7, 67.7, 0.0, 185.5}}},
    {5,  {"c5_outer",          {-87.7, 87.7, -87.7, 87.7, 0.0, 205.5}}},
    {6,  {"c6_outer",          {-107.7, 107.7, -107.7, 107.7, 0.0, 225.5}}},
    {7,  {"c7_outer",          {-207.7, 207.7, -207.7, 207.7, 0.0, 325.5}}},
    {8,  {"c8_outer",          {-217.7, 217.7, -217.7, 217.7, 0.0, 335.5}}},
    {9,  {"c9_outer",          {-222.7, 222.7, -222.7, 222.7, 0.0, 340.5}}},
    
    // 开口和管道
    {21, {"opening_21",        {-50.0, 0.0, -300.0, 0.0, 35.0, 85.0}}},
    
    // 门组件RPP
    {30, {"door_30",           {-60.0, 10.0, -300.0, -250.0, 0.0, 85.0}}},
    {31, {"door_31",           {-60.0, 10.0, -300.0, -222.7, 85.0, 105.0}}},
    {32, {"door_32",           {-60.0, 10.0, -310.0, -222.7, 0.0, 110.0}}},
    {33, {"door_33",           {-60.0, 10.0, -250.0, -222.7, 0.0, 85.0}}},
    {34, {"door_34",           {-60.0, 10.0, -315.0, -222.7, 0.0, 115.0}}},
    
    // 地基和冷却
    {35, {"foundation_35",     {-790.0, 790.0, -735.0, 735.0, -150.0, 0.0}}},
    {36, {"cooling_water_36",  {47.7, 247.7, -20.0, 20.0, -15.0, 0.0}}},
    {37, {"foundation_37",     {-341.2, -70.0, -62.5, 62.5, -5.0, 0.0}}}
};

// ========== 屏蔽层定义集合 ==========
const std::map<int, ShieldLayer> SHIELD_LAYER_DEFINITIONS = {
    // c4: 内层铁壳 (M6_FE)
    {4, ShieldLayer{
        4,
        "c4_M6_FE",
        "G4_Fe",
        {"c4_outer", {-67.7, 67.7, -67.7, 67.7, 0.0, 185.5}},
        {"c4_inner", {-65.2, 65.2, -65.2, 65.2, 0.0, 183.0}},
        true,      // hasInner
        true,      // hasOpenings
        false      // hasChamfer
    }},
    
    // c5: 内层铅壳 (M4_PB)
    {5, ShieldLayer{
        5,
        "c5_M4_PB",
        "G4_Pb",
        {"c5_outer", {-87.7, 87.7, -87.7, 87.7, 0.0, 205.5}},
        {"c5_inner", {-67.7, 67.7, -67.7, 67.7, 0.0, 185.5}},
        true,      // hasInner
        true,      // hasOpenings
        false      // hasChamfer
    }},
    
    // c6: 石墨壳 (M8_GRAPHITE)
    {6, ShieldLayer{
        6,
        "c6_M8_GRAPHITE",
        "G4_GRAPHITE",
        {"c6_outer", {-107.7, 107.7, -107.7, 107.7, 0.0, 225.5}},
        {"c6_inner", {-87.7, 87.7, -87.7, 87.7, 0.0, 205.5}},
        true,      // hasInner
        true,      // hasOpenings
        false      // hasChamfer
    }},
    
    // c7: 含硼聚乙烯 (M7_BPE) - 最复杂，包含切边平面
    {7, ShieldLayer{
        7,
        "c7_M7_BPE",
        "G4_POLYETHYLENE",
        {"c7_outer", {-207.7, 207.7, -207.7, 207.7, 0.0, 325.5}},
        {"c7_inner", {-107.7, 107.7, -107.7, 107.7, 0.0, 225.5}},
        true,      // hasInner
        true,      // hasOpenings
        true       // hasChamfer (1201-1224平面)
    }},
    
    // c8: 外层铅壳 (M4_PB)
    {8, ShieldLayer{
        8,
        "c8_M4_PB",
        "G4_Pb",
        {"c8_outer", {-217.7, 217.7, -217.7, 217.7, 0.0, 335.5}},
        {"c8_inner", {-207.7, 207.7, -207.7, 207.7, 0.0, 325.5}},
        true,      // hasInner
        true,      // hasOpenings
        true       // hasChamfer
    }},
    
    // c9: 外层不锈钢壳 (M10_SS)
    {9, ShieldLayer{
        9,
        "c9_M10_SS",
        "M10_STAINLESS-STEEL",
        {"c9_outer", {-222.7, 222.7, -222.7, 222.7, 0.0, 340.5}},
        {"c9_inner", {-217.7, 217.7, -217.7, 217.7, 0.0, 335.5}},
        true,      // hasInner
        true,      // hasOpenings
        true       // hasChamfer
    }}
};

// ========== 屏蔽层启用状态检查 ==========
bool IsShieldLayerEnabled(int id) {
    switch(id) {
        case 4: return ENABLE_SHIELD_LAYER_C4_FE;
        case 5: return ENABLE_SHIELD_LAYER_C5_PB;
        case 6: return ENABLE_SHIELD_LAYER_C6_GRAPHITE;
        case 7: return ENABLE_SHIELD_LAYER_C7_BPE;
        case 8: return ENABLE_SHIELD_LAYER_C8_PB;
        case 9: return ENABLE_SHIELD_LAYER_C9_SS;
        default: return false;
    }
}

// ========== 配置打印函数 ==========
void PrintShieldConfig() {
    G4cout << "\n========== 屏蔽体配置状态 ==========" << G4endl;
    G4cout << " c4 (Fe):        " << (IsShieldLayerEnabled(4) ? "启用" : "禁用") << G4endl;
    G4cout << " c5 (Pb):        " << (IsShieldLayerEnabled(5) ? "启用" : "禁用") << G4endl;
    G4cout << " c6 (Graphite):  " << (IsShieldLayerEnabled(6) ? "启用" : "禁用") << G4endl;
    G4cout << " c7 (BPE):       " << (IsShieldLayerEnabled(7) ? "启用" : "禁用") << G4endl;
    G4cout << " c8 (Pb):        " << (IsShieldLayerEnabled(8) ? "启用" : "禁用") << G4endl;
    G4cout << " c9 (SS):        " << (IsShieldLayerEnabled(9) ? "启用" : "禁用") << G4endl;
    G4cout << " Door:           " << (ENABLE_SHIELD_DOOR ? "启用" : "禁用") << G4endl;
    G4cout << " Foundation:     " << (ENABLE_SHIELD_FOUNDATION ? "启用" : "禁用") << G4endl;
    G4cout << "===================================\n" << G4endl;
}

}  // namespace ShieldConfig
