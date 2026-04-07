#include "TrackingAction.hh"

TrackingAction::TrackingAction(DetectorConstruction* detector)
    : G4UserTrackingAction(),
        fDetector(detector)
{}

TrackingAction::~TrackingAction()
{}

void TrackingAction::PreUserTrackingAction(const G4Track*track)
{
    
    G4LogicalVolume* lVolume = track->GetVolume()->GetLogicalVolume();


    // secondary particles only
    if (track->GetTrackID() == 1) return;//如果当前跟踪的粒子是初级粒子（TrackID为1），我们直接返回，不进行后续的统计和分析，因为我们只对次级粒子感兴趣。

    const G4ParticleDefinition* particle = track->GetParticleDefinition();
    G4String name = particle->GetParticleName();
    G4int pid = particle->GetPDGEncoding();
    G4int Z = particle->GetAtomicNumber();
    G4int A = particle->GetAtomicMass();
    G4double charge = particle->GetPDGCharge();
    G4double energy = track->GetKineticEnergy();//获取当前跟踪的粒子的动能，以便在事件结束时进行统计和分析。该动能为粒子在当前步骤中的动能，通常用于分析粒子的能量分布和衰变产物的能谱。
    G4double time = track->GetGlobalTime();
    G4double weight = track->GetWeight();//获取当前跟踪的粒子的权重值，以便在事件结束时进行统计和分析。该权重为蒙特卡洛模拟中用于调整事件权重的参数，通常用于处理重要性采样或其他加权事件生成技术。相比step中的权重，track的权重可能会随着粒子传播和相互作用而发生变化，因此在跟踪过程中获取粒子的权重值是必要的，以便在事件结束时进行准确的统计和分析。
    

}
void TrackingAction::PostUserTrackingAction(const G4Track*track)
{}