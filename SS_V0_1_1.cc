#include <iostream>

// Geant4 核心
#include "G4RunManager.hh"
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#endif

// 可视化 + UI
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4UIcommand.hh"

// 物理列表
#include "FTFP_BERT_HP.hh"
#include "G4EmExtraPhysics.hh"

// 你的模块
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"


// 随机数 + 单位
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace {
  void PrintUsage() {
    G4cerr << " Usage: " << G4endl;
    G4cerr << " sim [-m macro] [-u session] [-t nThreads]" << G4endl;
    G4cerr << " -t : 多线程数量（仅MT模式生效）" << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
  // 解析命令行参数
  G4String macro;
  G4String session;
#ifdef G4MULTITHREADED
  G4int nThreads = 0;
#endif

  for (G4int i=1; i<argc; i+=2) {
    if (G4String(argv[i]) == "-m") macro = argv[i+1];
    else if (G4String(argv[i]) == "-u") session = argv[i+1];
#ifdef G4MULTITHREADED
    else if (G4String(argv[i]) == "-t") nThreads = G4UIcommand::ConvertToInt(argv[i+1]);
#endif
    else { PrintUsage(); return 1; }
  }

  // 交互式模式
  G4UIExecutive* ui = nullptr;
  if (!macro.size()) ui = new G4UIExecutive(argc, argv, session);

  // 随机数引擎
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  // ==============================================
  // 🔹 运行管理器（多线程/单线程）
  // ==============================================
#ifdef G4MULTITHREADED
  auto runManager = new G4MTRunManager();
  if (nThreads > 0) runManager->SetNumberOfThreads(nThreads);
#else
  auto runManager = new G4RunManager();
#endif

  // ==============================================
  // 🔹 物理列表 + 探测器 + 动作初始化
  // ==============================================
    auto physicsList = new FTFP_BERT_HP();
    auto det = new DetectorConstruction();
    runManager->SetUserInitialization(physicsList);
    runManager->SetUserInitialization(det);
    runManager->SetUserInitialization(new ActionInitialization(det));

  // ==============================================
  // 🔹 可视化
  // ==============================================
  auto visManager = new G4VisExecutive();
  visManager->Initialize();

  // ==============================================
  // 🔹 UI 管理器
  // ==============================================
  auto UImanager = G4UImanager::GetUIpointer();

  if (!macro.empty()) {
    // 批量模式
    UImanager->ApplyCommand("/control/execute " + macro);
  } else {
    // 交互模式
    UImanager->ApplyCommand("/control/execute macros/init_vis.mac");
    if (ui->IsGUI()) UImanager->ApplyCommand("/control/execute macros/gui.mac");
    ui->SessionStart();
    delete ui;
  }

  // ==============================================
  // 🔹 结束释放
  // ==============================================
  delete visManager;
  delete runManager;

  return 0;
}