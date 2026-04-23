#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "Constants.hh"
#include "TtConstants.hh"
#include "PgConstants.hh"

#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ios.hh"

#include "ProtonGun.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : fBeamRadius(0.0)
    , fParticleGun(nullptr)
    , fProtonGun(nullptr)
{
    fParticleGun = new G4ParticleGun(1);
    if (PG_Is_default)
    {
        //默认设置粒子抢

    }
    if (C_Is_TT&&PG_Is_ProtonGun)
    {
        // 只创建一次 ProtonGun，并只配置一次
        fProtonGun = new ProtonGun(fParticleGun);
        fProtonGun->Configure();
        fBeamRadius = 0.0;
    }

}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    // 建议先删“策略对象”，再删 gun
    delete fProtonGun;
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    if(PG_Is_default)
    {
        fParticleGun->GeneratePrimaryVertex(event);// 默认发射方式：直接调用 ParticleGun 生成 primary vertex
    }
    if (C_Is_TT&&PG_Is_ProtonGun)
    {
        // 每事件：仅采样束斑位置并生成 vertex（不要 new / Configure）
        fProtonGun->GeneratePrimaries(event);
    }
    
    // 记录 primary 信息（保持你原逻辑）
    Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    if (run)
    {
        run->SetPrimary(fParticleGun->GetParticleDefinition(),
                        fParticleGun->GetParticleEnergy());
    }
}