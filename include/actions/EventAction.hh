#ifndef EventAction_h
#define EventAction_h

#include "G4UserEventAction.hh"
#include "G4Types.hh"
#include "Constants.hh"

class RunAction;

/// Per-event accumulator for one sample volume.
struct SampleAccum {
    G4int    nEnter    = 0; // 中子进入次数
    G4int    nInteract = 0; // 中子发生非输运过程次数
    G4int    nCapture  = 0; // 中子俘获次数
    G4double edep      = 0.; // 能量沉积 (MeV)
};

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction*);
    ~EventAction() override;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    // Called by SteppingAction to accumulate per-event per-sample data.
    void AddEntry(G4int copyNo);
    void AddInteraction(G4int copyNo);
    void AddCapture(G4int copyNo);
    void AddEdep(G4int copyNo, G4double de);

private:
    SampleAccum fSamples[C_NSamples]; // indexed by sample copy number
};

#endif
