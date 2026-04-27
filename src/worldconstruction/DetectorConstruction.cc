#include "DetectorConstruction.hh"
#include "DetectorConstructionManager.hh"
#include "ActivationFoil.hh"
#include "Constants.hh"
//#include "FissionBuilder.hh"

#include "CssConstants.hh"
#include "ComptonSuppressionSystem.hh"
#include "TantalumTarget.hh"
#include "SourceDetector.hh"
#include "PgConstants.hh"
#include "ProtonGun.hh"

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
    if(C_Is_AF)
    {
        ActivationFoil AF_BD;//创建一个ActivationFoilBuilder对象，用于构建胶囊组件。
        AF_BD.Build(LV_World, C_AF_Pos);

    }
    // if(C_Is_FC)
    // {
    //     FissionChamber ;
    //     fSampleLV = builder.Build(logicWorld, G4ThreeVector(0, 0, 0));
    // }
    if(C_Is_CSS)
    {
        ComptonSuppressionSystem CSS_BD;
        CSS_BD.Build(LV_World,C_CSS_Pos);
    }

    if(C_Is_TT)
    {
        TantalumTarget TT_BD;
        TT_BD.Build(LV_World, C_TT_Pos);
        G4LogicalVolume* TT_LV=TT_BD.Get_TT_LV();//获取钽靶组件的逻辑体，后续可以用于放置探测器等操作
        if(PG_Is_ProtonGun)//如果质子束流打开，进行束流探测器和靶表面中子探测器构建
        {
            //获取质子枪位置信息，根据粒子枪位置，确定探测器位置
            ProtonGun tempPG(nullptr);//创建一个临时的ProtonGun对象（nullptr 安全），用于获取质子枪的位置常量
            G4double SD_Z_N=C_TT_Pos.z()+TT_BD.Get_TT_LV()->GetSolid()->GetZHalfLength()+5*cm;//探测器放置在靶表面上方5cm处
            G4double SD_Z_P=tempPG.GetCenterZ()+1*cm;//质子枪位置的Z坐标上1cm处
            G4double SD_X=tempPG.GetCenterX();
            G4double SD_Y=tempPG.GetCenterY();
            G4double SD_MaxRel=tempPG.GetMaxRel();
            SourceDetector SD_BD_Proton;//创建两个SourceDetector对象，分别用于构建质子束流探测器和靶表面中子探测器
            SourceDetector SD_BD_Neutron;
            LV_ProtonDet  = SD_BD_Proton.Build(LV_World, G4ThreeVector(SD_X,SD_Y,SD_Z_P), "Proton");//位于质子枪前
            LV_NeutronDet = SD_BD_Neutron.Build(LV_World, G4ThreeVector(SD_X,SD_Y,SD_Z_N), "Neutron");//在世界逻辑体中构建探测器，位置根据质子枪位置和靶表面位置确定
        }
    }// closes if(C_Is_TT)

    return PV_World;
}// closes Construct()