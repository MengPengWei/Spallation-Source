#ifndef EventAction_h
#define EventAction_h

#include "G4UserEventAction.hh"
#include "G4Types.hh"
#include "Constants.hh"
#include "AfConstants.hh"
#include "CssConstants.hh"

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

    // ---- Source detector accumulators (called by SteppingAction) ----
    /// 质子探测器：每记录一步质子时调用（同时累加能量沉积）
    void AddProtonStep(G4double edep_MeV);
    /// 中子探测器：中子首次进入（边界步）时调用
    void AddNeutronEntry();

private:
    // 质子探测器每事件统计
    G4int    fNProtonSteps   = 0;   // 质子步数
    G4double fProtonEdep_MeV = 0.;  // 质子能量沉积 (MeV)
    // 中子探测器每事件统计
    G4int    fNNeutronEntries = 0;  // 中子进入次数
};

#endif
