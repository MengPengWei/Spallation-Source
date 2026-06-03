#include "TantalumTarget.hh"
#include "TtConstants.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

TantalumTarget::TantalumTarget()
{}

G4LogicalVolume* TantalumTarget::Build(G4LogicalVolume* motherLV,
                                          const G4ThreeVector& pos)
{
       auto nist = G4NistManager::Instance();

    // ----------------------------------------------------------------
    // 材料
    // ----------------------------------------------------------------
       TT_Air_mat      = nist->FindOrBuildMaterial("G4_AIR");
       TT_Water_mat  = nist->FindOrBuildMaterial("G4_WATER");
       TT_Cu_mat   = nist->FindOrBuildMaterial("G4_Cu"); //
       TT_Ta_mat = nist->FindOrBuildMaterial("G4_Ta");              // 钽
       TT_Steel_mat = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL"); // SS316L

    // ----------------------------------------------------------------
       //靶的整体结构为层状，内置微流孔道，用于冷却水的运行。
       //从下到上依次为 坦靶1 3.9mm 坦靶2 1.3mm（内有微流空，上下错位隔开） 水层 4.5mm 坦靶3 1.5mm
       //质子束流从下部照射，依次穿过坦靶1、坦靶2、水层、坦靶3，最后到达束流窗口（铜板，厚度0.5mm）。


    // ----------------------------------------------------------------
    //整体放置在母体积 motherLV 的 pos 位置，沿Z轴正向（从下往上）构建钽靶组件。首先构建TT整体
       TT_SV=new G4Tubs("TT_SV",
                                   C_TT_InD/2, C_TT_OutD/2,
                                   (C_TT_Target1_L+C_TT_Target2_L+C_TT_Water_L+C_TT_Target3_L)/2,
                                   0., 360.*deg);
       TT_LV=new G4LogicalVolume(TT_SV, TT_Air_mat, "TT_LV");
       new G4PVPlacement(nullptr,
                            pos,
                            TT_LV,
                            "TT_PV",
                            motherLV,
                            false, 0, true);



    // ----------------------------------------------------------------
    //第一层坦靶
       TT_Target1_SV = new G4Tubs("TT_Target1_SV",
                                   C_TT_InD/2, C_TT_OutD/2,
                                   C_TT_Target1_L/2,
                                   0., 360.*deg);

       TT_Target1_LV = new G4LogicalVolume(TT_Target1_SV, TT_Ta_mat, "TT_Target1_LV");


       new G4PVPlacement(nullptr,
                            C_TT_Target1_Pos,
                            TT_Target1_LV,
                            "TT_Target1_PV",
                            TT_LV,
                            false, 0, true);

     // ----------------------------------------------------------------
     //第二层坦靶
       TT_Target2_SV = new G4Tubs("TT_Target2_SV",
                                   C_TT_InD/2, C_TT_OutD/2,
                                   C_TT_Target2_L/2,
                                   0., 360.*deg);
       TT_Target2_LV = new G4LogicalVolume(TT_Target2_SV, TT_Ta_mat, "TT_Target2_LV");
       new G4PVPlacement(nullptr,
                            C_TT_Target2_Pos,
                            TT_Target2_LV,
                            "TT_Target2_PV",
                            TT_LV,
                            false, 0, true);
              // ----------------------------------------------------------------
              //第二次坦靶内嵌上下微流道，做角度循环，每个微流道弧长0.4mm，厚度0.5mm，错位分布。

              G4int nChannels = static_cast<G4int>(360.0 / (2*C_TT_Water_Channel_Degree));
              for(G4int i=0;i<nChannels;i++){
                     G4String Name="TT_Target2_Water_Channel1_"+std::to_string(i);
                     G4double Start_angle=2*i*C_TT_Water_Channel_Degree;
                    // G4double Eps=C_TT_Water_Channel_Degree-C_TT_Water_Channel_Eps; // 确保水道之间没有重叠
                     //第一层水道
                     auto Water_Channel1_SV = new G4Tubs(Name+"_SV",
                                                               C_TT_InD/2, C_TT_OutD/2,
                                                               C_TT_Water_Channel_Thickness/2,
                                                               Start_angle*deg, C_TT_Water_Channel_Degree*deg);
                     auto Water_Channel1_LV = new G4LogicalVolume(Water_Channel1_SV, TT_Water_mat, Name+"_LV");
                     new G4PVPlacement(nullptr,
                                          G4ThreeVector(0, 0, C_TT_Water_Channel1_Z),
                                          Water_Channel1_LV,
                                          Name+"_PV",
                                          TT_Target2_LV,
                                          false, i, true);

                     //第二层水道
                     Name="TT_Target2_Water_Channel2_"+std::to_string(i);
                            Start_angle=(i-0.5)*2*C_TT_Water_Channel_Degree; // 错位分布
                     auto Water_Channel2_SV = new G4Tubs(Name+"_SV",
                                                               C_TT_InD/2, C_TT_OutD/2,
                                                               C_TT_Water_Channel_Thickness/2,
                                                               Start_angle*deg, C_TT_Water_Channel_Degree*deg);
                     auto Water_Channel2_LV = new G4LogicalVolume(Water_Channel2_SV, TT_Water_mat, Name+"_LV");
                     new G4PVPlacement(nullptr,
                                          G4ThreeVector(0, 0, C_TT_Water_Channel2_Z),
                                          Water_Channel2_LV,
                                          Name+"_PV",
                                          TT_Target2_LV,
                                          false, i, true);
                     }
       // ----------------------------------------------------------------
       //水层，位于第二层和第三层坦靶之间
       TT_Water_SV = new G4Tubs("TT_Water_SV",
                                   C_TT_InD/2, C_TT_OutD/2,
                                   C_TT_Water_L/2,
                                   0., 360.*deg);
       TT_Water_LV = new G4LogicalVolume(TT_Water_SV, TT_Water_mat, "TT_Water_LV");
       new G4PVPlacement(nullptr,
                            C_TT_Water_Pos,
                            TT_Water_LV,
                            "TT_Water_PV",
                            TT_LV,
                            false, 0, true);

       // ----------------------------------------------------------------
       //第三层坦靶
       TT_Target3_SV = new G4Tubs("TT_Target3_SV",
                                   C_TT_InD/2, C_TT_OutD/2,
                                   C_TT_Target3_L/2,
                                   0., 360.*deg);
       TT_Target3_LV = new G4LogicalVolume(TT_Target3_SV, TT_Ta_mat, "TT_Target3_LV");
       new G4PVPlacement(nullptr,
                            C_TT_Target3_Pos,
                            TT_Target3_LV,
                            "TT_Target3_PV",
                            TT_LV,
                            false, 0, true);






       return TT_LV;

}
