#include "DetectorConstruction.hh"
#include "DetectorConstructionManager.hh"
#include "ActivationFoilBuilder.hh"
#include "Constants.hh"

// 构造函数
DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction()
{
    // 从常量读取世界尺寸
    fWorldSizeX = C_WordSizeX;
    fWorldSizeY = C_WordSizeY;
    fWorldSizeZ = C_WordSizeZ;

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
    WorldMat = nist->FindOrBuildMaterial("G4_AIR");

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
    solidWorld = new G4Box("World",
                        fWorldSizeX / 2,
                        fWorldSizeY / 2,
                        fWorldSizeZ / 2);

    // 逻辑体
    logicWorld = new G4LogicalVolume(solidWorld,
                                    WorldMat,
                                    "World");

    // 物理体放置
    physWorld = new G4PVPlacement(
        0,                      // 旋转
        G4ThreeVector(0,0,0),   // 位置
        logicWorld,             // 逻辑体
        "World",            // 名称
        0,                      // 母体积
        false,                  // 无布尔操作
        0,                      // 复制编号
        true                    // 检查重叠
    );

    return physWorld;
}

// Geant4 主构造接口
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto worldPV = ConstructWorld();

    // Build capsule + inner cavity + 3 sample cylinders at origin
    ActivationFoilBuilder builder;//创建一个ActivationFoilBuilder对象，用于构建胶囊组件。
    fSampleLV = builder.Build(logicWorld, G4ThreeVector(0, 0, 0));//将胶囊组件构建并放置在世界逻辑体的原点位置，并将返回的样品逻辑体指针保存到成员变量fSampleLV中，以便后续使用。



    return worldPV;
}