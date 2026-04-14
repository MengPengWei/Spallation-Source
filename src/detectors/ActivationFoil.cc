#include "ActivationFoil.hh"
#include "Constants.hh"
#include "AfConstants.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

ActivationFoil::ActivationFoil()
{}

G4LogicalVolume* ActivationFoil::Build(G4LogicalVolume* motherLV,
                                                const G4ThreeVector& pos
        )
{
    auto nist = G4NistManager::Instance();

    // --- Materials --- （变量已在.hh声明，这里只赋值）
    AF_CFC_mat = nist->FindOrBuildMaterial("G4_GRAPHITE");
    AF_Air_mat = nist->FindOrBuildMaterial("G4_AIR");
    AF_In_mat  = nist->FindOrBuildMaterial("G4_In");
    AF_Si_mat  = nist->FindOrBuildMaterial("G4_Si");
    AF_Cu_mat  = nist->FindOrBuildMaterial("G4_Cu");


    // CFC 
    AF_CFC_SV = new G4Tubs(C_AF_CFC_Name,
                                    0.*mm, C_AF_CFC_OutD/2,
                                    C_AF_CFC_L/2,
                                    0., 360.*deg);

    AF_CFC_LV = new G4LogicalVolume(AF_CFC_SV, AF_CFC_mat, C_AF_CFC_Name);
    AF_CFC_LV->SetVisAttributes(&AF_Vis::AF_CFC_Vis);
    G4RotationMatrix AF_rot;
    AF_rot.rotateY(C_AF_degree);
    new G4PVPlacement(G4Transform3D(AF_rot, pos), AF_CFC_LV, C_AF_CFC_Name, motherLV,
                        false, 0, true);



    //In115
    if(is_AF_In)
    {
        

        //AIR 胶囊内填充空气
        AF_Air_SV=new G4Tubs(C_AF_Air_Name,
                                        0.*mm, C_AF_In_OutD/2+C_AF_Air_OutD_Thickness,
                                        C_AF_In_L/2+C_AF_Air_L_Thickness,
                                        0., 360.*deg);
        AF_Air_LV = new G4LogicalVolume(AF_Air_SV, AF_Air_mat, C_AF_Air_Name);
        AF_Air_LV->SetVisAttributes(&AF_Vis::AF_Air_Vis);
        new G4PVPlacement(0,G4ThreeVector(0,0,0) , AF_Air_LV, C_AF_Air_Name, AF_CFC_LV,
                        false, 0, true);
        
        AF_In_SV = new G4Tubs(C_AF_In_Name,
                                    0.*mm, C_AF_In_OutD/2,
                                    C_AF_In_L/2,
                                    0., 360.*deg);

        AF_In_LV = new G4LogicalVolume(AF_In_SV, AF_In_mat, C_AF_In_Name);
        AF_In_LV->SetVisAttributes(&AF_Vis::AF_In_Vis);

        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), AF_In_LV, C_AF_In_Name,
                        AF_Air_LV, false, 0, true);
    }


    if(is_AF_Si)
    {
             //AIR 胶囊内填充空气
        AF_Air_SV=new G4Tubs(C_AF_Air_Name,
                                        0.*mm, C_AF_SiCu_OutD/2+C_AF_Air_OutD_Thickness,
                                        C_AF_SiCu_L/2+C_AF_Air_L_Thickness,
                                        0., 360.*deg);
        AF_Air_LV = new G4LogicalVolume(AF_Air_SV, AF_Air_mat, C_AF_Air_Name);
        AF_Air_LV->SetVisAttributes(&AF_Vis::AF_Air_Vis);
        new G4PVPlacement(0,G4ThreeVector(0,0,0) , AF_Air_LV, C_AF_Air_Name, AF_CFC_LV,
                        false, 0, true);

        AF_Si_SV = new G4Tubs(C_AF_Si_Name,
                                        0.*mm, C_AF_SiCu_OutD/2,
                                        C_AF_SiCu_L/2,
                                        0., 360.*deg);

        AF_Si_LV = new G4LogicalVolume(AF_Si_SV, AF_Si_mat, C_AF_Si_Name);
        AF_Si_LV->SetVisAttributes(&AF_Vis::AF_Si_Vis);

        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), AF_Si_LV, C_AF_Si_Name,
                            AF_Air_LV, false, 0, true);
    }

    if(is_AF_Cu)
    {        
        AF_Air_SV=new G4Tubs(C_AF_Air_Name,
                                        0.*mm, C_AF_SiCu_OutD/2+C_AF_Air_OutD_Thickness,
                                        C_AF_SiCu_L/2+C_AF_Air_L_Thickness,
                                        0., 360.*deg);
        AF_Air_LV = new G4LogicalVolume(AF_Air_SV, AF_Air_mat, C_AF_Air_Name);
        AF_Air_LV->SetVisAttributes(&AF_Vis::AF_Air_Vis);
        new G4PVPlacement(0,G4ThreeVector(0,0,0) , AF_Air_LV, C_AF_Air_Name, AF_CFC_LV,
                        false, 0, true);

        AF_Cu_SV = new G4Tubs(C_AF_Cu_Name,
                                        0.*mm, C_AF_SiCu_OutD/2,
                                        C_AF_SiCu_L/2,
                                        0., 360.*deg);

        AF_Cu_LV = new G4LogicalVolume(AF_Cu_SV, AF_Cu_mat, C_AF_Cu_Name);
        AF_Cu_LV->SetVisAttributes(&AF_Vis::AF_Cu_Vis);
        new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), AF_Cu_LV, C_AF_Cu_Name,
                            AF_Air_LV, false, 0, true);
    }
    return AF_CFC_LV;
}