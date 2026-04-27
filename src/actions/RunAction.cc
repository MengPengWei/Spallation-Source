#include "RunAction.hh"
#include "Run.hh"

#include "G4Run.hh"
#include "G4RootAnalysisManager.hh"

RunAction::RunAction() = default;
RunAction::~RunAction() = default;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* RunAction::GenerateRun()
{
    return new Run();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
    auto man = G4RootAnalysisManager::Instance();
    man->SetFileName("activation_output");
    man->OpenFile();

    // ------------------------------------------------------------------
    // Ntuple 0 "NeutronEntry": 中子穿入样品边界时的物理量
    //   eventId, sampleCopyNo, Ekin(MeV), time(ns),
    //   x(mm), y(mm), z(mm), px(MeV/c), py(MeV/c), pz(MeV/c)
    // ------------------------------------------------------------------
    man->CreateNtuple("NeutronEntry", "Neutron crossing into sample volume");
    man->CreateNtupleIColumn("eventId");    // 0
    man->CreateNtupleIColumn("copyNo");     // 1
    man->CreateNtupleDColumn("Ekin_MeV");  // 2
    man->CreateNtupleDColumn("time_ns");   // 3
    man->CreateNtupleDColumn("x_mm");      // 4
    man->CreateNtupleDColumn("y_mm");      // 5
    man->CreateNtupleDColumn("z_mm");      // 6
    man->CreateNtupleDColumn("px_MeV");    // 7
    man->CreateNtupleDColumn("py_MeV");    // 8
    man->CreateNtupleDColumn("pz_MeV");    // 9
    man->FinishNtuple(0);

    // ------------------------------------------------------------------
    // Ntuple 1 "NeutronInteraction": 样品内中子发生非输运过程的步骤
    //   eventId, sampleCopyNo, processName,
    //   edep(MeV), stepLen(mm), preKE(MeV), postKE(MeV),
    //   x(mm), y(mm), z(mm)
    // ------------------------------------------------------------------
    man->CreateNtuple("NeutronInteraction", "Neutron interaction steps in sample");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleSColumn("process");      // 2
    man->CreateNtupleDColumn("edep_MeV");     // 3
    man->CreateNtupleDColumn("stepLen_mm");   // 4
    man->CreateNtupleDColumn("preKE_MeV");    // 5
    man->CreateNtupleDColumn("postKE_MeV");   // 6
    man->CreateNtupleDColumn("x_mm");         // 7
    man->CreateNtupleDColumn("y_mm");         // 8
    man->CreateNtupleDColumn("z_mm");         // 9
    man->FinishNtuple(1);

    // ------------------------------------------------------------------
    // Ntuple 2 "Secondary": 样品内中子反应产生的次级粒子
    //   eventId, sampleCopyNo, particleName, Ekin(MeV),
    //   x(mm), y(mm), z(mm)
    // ------------------------------------------------------------------
    man->CreateNtuple("Secondary", "Secondary particles from neutron interactions");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleSColumn("particle");     // 2
    man->CreateNtupleDColumn("Ekin_MeV");    // 3
    man->CreateNtupleDColumn("x_mm");        // 4
    man->CreateNtupleDColumn("y_mm");        // 5
    man->CreateNtupleDColumn("z_mm");        // 6
    man->FinishNtuple(2);

    // ------------------------------------------------------------------
    // Ntuple 3 "EventSummary": 每个事件每个样品的汇总统计
    //   eventId, sampleCopyNo, nEnter, nInteract, nCapture, totalEdep(MeV)
    // ------------------------------------------------------------------
    man->CreateNtuple("EventSummary", "Per-event per-sample summary");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleIColumn("nEnter");       // 2
    man->CreateNtupleIColumn("nInteract");    // 3
    man->CreateNtupleIColumn("nCapture");     // 4
    man->CreateNtupleDColumn("edep_MeV");     // 5
    man->FinishNtuple(3);

    // ==================================================================
    // Source Detector Ntuples & Histograms
    // ==================================================================

    // ------------------------------------------------------------------
    // Ntuple 4 "ProtonDetStep": 质子探测器内质子的逐步信息
    //   step 级别：位置、动能、能量沉积、动量方向
    //   用于后续 3D 热力分布图
    // ------------------------------------------------------------------
    man->CreateNtuple("ProtonDetStep", "Proton steps in proton source detector");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleDColumn("x_mm");         // 1  全局 x 坐标 (mm)
    man->CreateNtupleDColumn("y_mm");         // 2  全局 y 坐标 (mm)
    man->CreateNtupleDColumn("z_mm");         // 3  全局 z 坐标 (mm)
    man->CreateNtupleDColumn("Ekin_MeV");     // 4  步前动能 (MeV)
    man->CreateNtupleDColumn("edep_MeV");     // 5  本步能量沉积 (MeV)
    man->CreateNtupleDColumn("px_MeV");       // 6  动量 x 分量 (MeV/c)
    man->CreateNtupleDColumn("py_MeV");       // 7  动量 y 分量 (MeV/c)
    man->CreateNtupleDColumn("pz_MeV");       // 8  动量 z 分量 (MeV/c)
    man->FinishNtuple(4);

    // ------------------------------------------------------------------
    // Ntuple 5 "NeutronDetStep": 中子探测器（靶后）内中子的逐步信息
    //   step 级别：位置、动能、动量
    //   用于后续产额热力图与能谱
    // ------------------------------------------------------------------
    man->CreateNtuple("NeutronDetStep", "Neutron steps in neutron source detector");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleDColumn("x_mm");         // 1
    man->CreateNtupleDColumn("y_mm");         // 2
    man->CreateNtupleDColumn("z_mm");         // 3
    man->CreateNtupleDColumn("Ekin_MeV");     // 4
    man->CreateNtupleDColumn("px_MeV");       // 5
    man->CreateNtupleDColumn("py_MeV");       // 6
    man->CreateNtupleDColumn("pz_MeV");       // 7
    man->FinishNtuple(5);

    // ------------------------------------------------------------------
    // Ntuple 6 "SourceDetEvent": 每事件的 source 探测器汇总
    //   event 级别：事件内质子步数/中子进入次数及能量沉积
    // ------------------------------------------------------------------
    man->CreateNtuple("SourceDetEvent", "Per-event summary for source detectors");
    man->CreateNtupleIColumn("eventId");          // 0
    man->CreateNtupleIColumn("nProtonSteps");      // 1  质子探测器内质子步数
    man->CreateNtupleIColumn("nNeutronEntries");   // 2  中子探测器内中子进入次数
    man->CreateNtupleDColumn("protonEdep_MeV");    // 3  质子探测器质子能量沉积 (MeV)
    man->FinishNtuple(6);

    // ------------------------------------------------------------------
    // 1D 直方图: 中子探测器内中子动能谱
    //   200 bins, [0, 100] MeV
    // ------------------------------------------------------------------
    man->CreateH1("hNeutronSpectrum",
                  "Neutron energy spectrum in neutron detector;E_{kin} (MeV);Counts",
                  200, 0., 100.);

    // ------------------------------------------------------------------
    // 2D 直方图: 中子探测器内中子产额热力图 (x vs y)
    //   探测器中心约 (0, -215 mm, +56 mm)，半径 50 mm
    //   50×50 bins
    // ------------------------------------------------------------------
    man->CreateH2("hNeutronYield2D",
                  "Neutron yield map in neutron detector;x (mm);y (mm)",
                  50, -60., 60.,
                  50, -270., -160.);

    // ------------------------------------------------------------------
    // 3D 直方图: 质子探测器内质子位置分布（热力图）
    //   探测器中心约 (0, -215 mm, -90 mm)，半径 50 mm，半高 5 mm
    //   25×25×20 bins
    // ------------------------------------------------------------------
    man->CreateH3("hProton3D",
                  "Proton 3D distribution in proton detector;x (mm);y (mm);z (mm)",
                  25, -60., 60.,
                  25, -270., -160.,
                  20, -97., -83.);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
    // Write ROOT file
    auto man = G4RootAnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    // Print rdecay02-style run summary to console
    const Run* theRun = static_cast<const Run*>(run);
    theRun->EndOfRun();
}
