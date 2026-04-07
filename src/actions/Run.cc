#include "Run.hh"
#include "Constants.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include <iomanip>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Run::Run() : G4Run() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::SetPrimary(G4ParticleDefinition* particle, G4double energy)
{
    fParticle = particle;
    fEkin     = energy;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::CountProcesses(const G4VProcess* process)
{
    if (!process) return;
    const G4String& procName = process->GetProcessName();
    auto it = fProcCounter.find(procName);
    if (it == fProcCounter.end())
        fProcCounter[procName] = 1;
    else
        ++(it->second);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::ParticleCount(const G4String& name, G4double Ekin)
{
    auto it = fParticleDataMap.find(name);
    if (it == fParticleDataMap.end()) {
        fParticleDataMap.emplace(name, ParticleData(1, Ekin, Ekin, Ekin));
    } else {
        ParticleData& data = it->second;
        ++data.fCount;
        data.fEmean += Ekin;
        if (Ekin < data.fEmin) data.fEmin = Ekin;
        if (Ekin > data.fEmax) data.fEmax = Ekin;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::AddEdep(G4double edep)
{
    fEdep  += edep;
    fEdep2 += edep * edep;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::Merge(const G4Run* run)
{
    const Run* local = static_cast<const Run*>(run);

    // primary (same for all threads – just overwrite)
    fParticle = local->fParticle;
    fEkin     = local->fEkin;

    // energy deposit
    fEdep  += local->fEdep;
    fEdep2 += local->fEdep2;

    // process counters
    for (const auto& kv : local->fProcCounter) {
        auto it = fProcCounter.find(kv.first);
        if (it == fProcCounter.end())
            fProcCounter[kv.first] = kv.second;
        else
            it->second += kv.second;
    }

    // particle data
    for (const auto& kv : local->fParticleDataMap) {
        const ParticleData& ld = kv.second;
        auto it = fParticleDataMap.find(kv.first);
        if (it == fParticleDataMap.end()) {
            fParticleDataMap.emplace(kv.first,
                ParticleData(ld.fCount, ld.fEmean, ld.fEmin, ld.fEmax));
        } else {
            ParticleData& d = it->second;
            d.fCount += ld.fCount;
            d.fEmean += ld.fEmean;
            if (ld.fEmin < d.fEmin) d.fEmin = ld.fEmin;
            if (ld.fEmax > d.fEmax) d.fEmax = ld.fEmax;
        }
    }

    G4Run::Merge(run);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Run::EndOfRun() const
{
    G4int nEvents = numberOfEvent;
    if (nEvents == 0) return;

    constexpr G4int prec = 5;
    constexpr G4int wid  = prec + 2;
    G4int dfprec = G4cout.precision(prec);

    // ---------------------------------------------------------------
    // Run header
    // ---------------------------------------------------------------
    G4String particleName = fParticle ? fParticle->GetParticleName() : "unknown";
    G4cout << "\n ============================================================"
           << "\n The run: " << nEvents << "  " << particleName
           << "  of  " << G4BestUnit(fEkin, "Energy")
           << "  irradiating the activation-foil capsule"
           << "\n ============================================================\n";

    // ---------------------------------------------------------------
    // Mean energy deposit ± RMS in all sample volumes (aggregate)
    // ---------------------------------------------------------------
    G4double meanEdep = fEdep  / nEvents;
    G4double meanEd2  = fEdep2 / nEvents;
    G4double rms = meanEd2 - meanEdep * meanEdep;
    rms = (rms > 0.) ? std::sqrt(rms) : 0.;

    G4cout << "\n Mean energy deposit in sample volumes (all " << C_NSamples << ", per event)"
           << " = " << G4BestUnit(meanEdep, "Energy")
           << ";  rms = " << G4BestUnit(rms, "Energy") << "\n";

    // ---------------------------------------------------------------
    // Process call frequency
    // ---------------------------------------------------------------
    G4cout << "\n Process calls frequency in sample volumes :\n";
    G4int index = 0;
    for (const auto& kv : fProcCounter) {
        G4String space = " ";
        if (++index % 3 == 0) space = "\n";
        G4cout << "  " << std::setw(20) << kv.first
               << " = " << std::setw(7) << kv.second << space;
    }
    G4cout << "\n";

    // ---------------------------------------------------------------
    // Generated-particle list
    // ---------------------------------------------------------------
    G4cout << "\n List of secondary particles generated in sample volumes :\n";
    for (const auto& kv : fParticleDataMap) {
        const ParticleData& d = kv.second;
        G4double eMean = d.fEmean / d.fCount;
        G4cout << "  " << std::setw(15) << kv.first
               << " : " << std::setw(7) << d.fCount
               << "  Emean = " << std::setw(wid) << G4BestUnit(eMean,    "Energy")
               << "\t( "       << G4BestUnit(d.fEmin, "Energy")
               << " --> "      << G4BestUnit(d.fEmax, "Energy") << " )"
               << "\n";
    }
    G4cout << "\n ============================================================\n"
           << G4endl;

    // restore default format
    G4cout.precision(dfprec);
}
