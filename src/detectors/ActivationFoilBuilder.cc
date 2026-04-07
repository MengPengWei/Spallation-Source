#include "ActivationFoilBuilder.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

ActivationFoilBuilder::ActivationFoilBuilder()
{}

G4LogicalVolume* ActivationFoilBuilder::Build(G4LogicalVolume* motherLV,
                                                const G4ThreeVector& pos)
{
    auto nist = G4NistManager::Instance();

    // --- Materials ---
    G4Material* matAir      = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* matGraphite = nist->FindOrBuildMaterial("G4_GRAPHITE"); // CFC approx
    G4Material* matIn       = nist->FindOrBuildMaterial("G4_In");       // Indium samples

    // --- Capsule outer shell (CFC graphite approximation) --- 
    // Outer cylinder: r = 13 mm, halfZ = 23 mm
    auto solidCapsule = new G4Tubs("Capsule",
                                    0.*mm, 13.*mm,  // rMin, rMax
                                    23.*mm,          // halfZ
                                    0., 360.*deg);

    auto logicCapsule = new G4LogicalVolume(solidCapsule, matGraphite, "Capsule");
    static G4VisAttributes capsuleVis(G4Colour(0.4, 0.4, 0.4, 0.5));
    logicCapsule->SetVisAttributes(&capsuleVis);

    new G4PVPlacement(nullptr, pos, logicCapsule, "Capsule", motherLV,
                        false, 0, true);

    // --- Inner air cavity (placed inside capsule) ---
    // r = 6.5 mm, halfZ = 22 mm (leaves 1 mm shell at ends)
    auto solidCavity = new G4Tubs("Cavity",
                                    0.*mm, 6.5*mm,
                                    22.*mm,
                                    0., 360.*deg);

    auto logicCavity = new G4LogicalVolume(solidCavity, matAir, "Cavity");
    static G4VisAttributes cavityVis(G4Colour(0.8, 0.9, 1.0, 0.2));//设置内腔的可视属性为淡蓝色，半透明，以便在可视化时区分不同组件。
    logicCavity->SetVisAttributes(&cavityVis);

    //建立
    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicCavity, "Cavity",
                        logicCapsule, false, 0, true);

    // --- 3 Indium sample cylinders along Z inside the cavity ---
    // Each: r = 6 mm, halfZ = 6 mm (12 mm tall)
    // Placed at z = -14, 0, +14 mm (2 mm gap between each)
    auto solidSample = new G4Tubs("Sample",
                                    0.*mm, 6.*mm,
                                    6.*mm,
                                    0., 360.*deg);

    fSampleLV = new G4LogicalVolume(solidSample, matIn, "Sample");
    static G4VisAttributes sampleVis(G4Colour(0.2, 0.8, 0.2, 0.9));
    fSampleLV->SetVisAttributes(&sampleVis);

    const G4double zPositions[3] = { -14.*mm, 0.*mm, 14.*mm };
    for (G4int i = 0; i < 3; ++i) {
        new G4PVPlacement(nullptr,
                            G4ThreeVector(0., 0., zPositions[i]),
                            fSampleLV,
                            "Sample",
                            logicCavity,
                            false,
                            i,    // copy number = sample index
                            true);
    }

    return fSampleLV;
}
