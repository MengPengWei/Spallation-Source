#include "ActivationFoilBuilder.hh"
#include "Constants.hh"

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
    auto solidCapsule = new G4Tubs(kCapsuleShellName,
                                    0.*mm, C_CapsuleOuterRadius,
                                    C_CapsuleHalfZ,
                                    0., 360.*deg);

    auto logicCapsule = new G4LogicalVolume(solidCapsule, matGraphite, kCapsuleShellName);
    static G4VisAttributes capsuleVis(G4Colour(0.4, 0.4, 0.4, 0.5));
    logicCapsule->SetVisAttributes(&capsuleVis);

    new G4PVPlacement(nullptr, pos, logicCapsule, kCapsuleShellName, motherLV,
                        false, 0, true);

    // --- Inner air cavity (placed inside capsule shell) ---
    auto solidCavity = new G4Tubs(kCavityName,
                                    0.*mm, C_CavityOuterRadius,
                                    C_CavityHalfZ,
                                    0., 360.*deg);

    auto logicCavity = new G4LogicalVolume(solidCavity, matAir, kCavityName);
    static G4VisAttributes cavityVis(G4Colour(0.8, 0.9, 1.0, 0.2));
    logicCavity->SetVisAttributes(&cavityVis);

    new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), logicCavity, kCavityName,
                        logicCapsule, false, 0, true);

    // --- C_NSamples Indium sample cylinders along Z inside the cavity ---
    // Each: r = C_SampleOuterRadius, halfZ = C_SampleHalfZ
    // Spacing = 2*C_SampleHalfZ + 2 mm gap; centred in the cavity.
    auto solidSample = new G4Tubs(kSampleName,
                                    0.*mm, C_SampleOuterRadius,
                                    C_SampleHalfZ,
                                    0., 360.*deg);

    fSampleLV = new G4LogicalVolume(solidSample, matIn, kSampleName);
    static G4VisAttributes sampleVis(G4Colour(0.2, 0.8, 0.2, 0.9));
    fSampleLV->SetVisAttributes(&sampleVis);

    // z_i = (i - (N-1)/2) * (2*halfZ + gap)
    const G4double gap    = 2.*mm;
    const G4double pitch  = 2.*C_SampleHalfZ + gap; // 14 mm
    const G4double center = (C_NSamples - 1) / 2.0; // offset so the stack is centred
    for (G4int i = 0; i < C_NSamples; ++i) {
        G4double zi = (i - center) * pitch;
        new G4PVPlacement(nullptr,
                            G4ThreeVector(0., 0., zi),
                            fSampleLV,
                            kSampleName,
                            logicCavity,
                            false,
                            i,   // copy number = sample index
                            true);
    }

    return fSampleLV;
}
