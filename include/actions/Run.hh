#ifndef Run_h
#define Run_h 1

#include "G4Run.hh"
#include "G4VProcess.hh"
#include "globals.hh"

#include <map>

class G4ParticleDefinition;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/// Simple data struct used to track per-particle-species statistics inside
/// the sample volumes (equivalent to rdecay02's ParticleData).
struct ParticleData {
    G4int    fCount;
    G4double fEmean; // running sum; divided by fCount at print time
    G4double fEmin;
    G4double fEmax;

    ParticleData(G4int n, G4double e, G4double emin, G4double emax)
        : fCount(n), fEmean(e), fEmin(emin), fEmax(emax) {}
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/// Custom G4Run that accumulates rdecay02-style run statistics across all
/// sample volumes:
///   - per-process call counters
///   - per-particle-species creation statistics (count, Emean, Emin, Emax)
///   - energy-deposit mean and RMS
class Run : public G4Run
{
public:
    Run();

    /// Store primary particle information (called once per event from
    /// PrimaryGeneratorAction::GeneratePrimaries).
    void SetPrimary(G4ParticleDefinition* particle, G4double energy);

    /// Increment the call counter for the process that ended this step.
    /// Call for every step inside a sample volume.
    void CountProcesses(const G4VProcess* process);

    /// Record a newly created secondary particle.
    /// Call from TrackingAction::PreUserTrackingAction for every secondary
    /// whose vertex lies inside a sample volume.
    void ParticleCount(const G4String& name, G4double Ekin);

    /// Accumulate energy deposited in sample volumes (one step at a time).
    void AddEdep(G4double edep);

    /// G4Run merge hook (called automatically in MT mode by the master run).
    void Merge(const G4Run* run) override;

    /// Print the rdecay02-style summary to G4cout.  Called from
    /// RunAction::EndOfRunAction on the master (or single-thread) run.
    void EndOfRun() const;

private:
    G4ParticleDefinition* fParticle = nullptr;
    G4double              fEkin     = 0.;

    std::map<G4String, G4int>         fProcCounter;      // process-name → call count
    std::map<G4String, ParticleData>  fParticleDataMap;  // particle-name → stats

    G4double fEdep  = 0.;  // running sum of edep (MeV)
    G4double fEdep2 = 0.;  // running sum of edep² (MeV²)
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif // Run_h