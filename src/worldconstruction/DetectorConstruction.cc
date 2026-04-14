#include "DetectorConstruction.hh"
#include "DetectorConstructionManager.hh"
#include "ActivationFoil.hh"
#include "Constants.hh"
//#include "FissionBuilder.hh"

#include "CssConstants.hh"
#include "ComptonSuppressionSystem.hh"

// 构造函数
DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction()
{
    // 定义材料
    DefineMaterials();

}

// 析构函数
DetectorConstruction::~DetectorConstruction()
{
}

// 定义材料（真空 / 空气）
void DetectorConstruction::DefineMaterials()
{
    G4NistManager* nist = G4NistManager::Instance();

    // 世界材料：真空 或 G4_AIR
    //WorldMat = nist->FindOrBuildMaterial("G4_Galactic");
    Mat_World = nist->FindOrBuildMaterial("G4_AIR");

    // 输出材料信息
    G4cout << "\n=====================================" << G4endl;
    G4cout << "Defined Materials: " << G4Material::GetNumberOfMaterials() << G4endl;
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
    G4cout << "=====================================\n" << G4endl;
}

// 构建世界
G4VPhysicalVolume* DetectorConstruction::ConstructWorld()
{
    // 世界立方体
    SV_World = new G4Box("World",
                        C_World_X / 2,
                        C_World_Y / 2,
                        C_World_Z / 2);

    // 逻辑体
    LV_World = new G4LogicalVolume(SV_World,
                                    Mat_World,
                                    "World");

    // 物理体放置
    PV_World = new G4PVPlacement(
        0,                      // 旋转
        G4ThreeVector(0,0,0),   // 位置
        LV_World,             // 逻辑体
        "World",             // 名称
        0,                      // 母体积
        false,                  // 无布尔操作
        0,                      // 复制编号
        true                    // 检查重叠
    );

    return PV_World;
}

// Geant4 主构造接口
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto worldPV = ConstructWorld();

    // Build capsule + inner cavity + 3 sample cylinders at origin
    if(is_AF)
    {
        ActivationFoil AF_BD;//创建一个ActivationFoilBuilder对象，用于构建胶囊组件。
        AF_BD.Build(LV_World, C_AF_Pos);

    }
    // if(is_FC)
    // {
    //     FissionChamber ;
    //     fSampleLV = builder.Build(logicWorld, G4ThreeVector(0, 0, 0));
    // }
    if(is_CSS)
    {
        ComptonSuppressionSystem CSS_BD;
        CSS_BD.Build(LV_World,C_CSS_Pos);
    }

    return PV_World;
}