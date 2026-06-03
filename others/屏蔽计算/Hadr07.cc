//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file Hadr07.cc
/// \brief Main program of the hadronic/Hadr07 example with multi-threading support
///
/// Added explicit multi-threading control with -t option and automatic
/// thread count detection for optimal performance.
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

#include "G4Types.hh"
#include "G4UImanager.hh"
#include "Randomize.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4UIcommand.hh"
#include "G4Threading.hh"

// Multi-threading support
#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "SteppingVerbose.hh"

namespace {
  void PrintUsage() {
    G4cout << " Usage: " << G4endl;
    G4cout << " Hadr07 [-m macro ] [-u UIsession] [-t nThreads]" << G4endl;
    G4cout << "   -m macro      : Execute macro file in batch mode" << G4endl;
    G4cout << "   -u UIsession  : UI session type (qt, xm, etc.)" << G4endl;
#ifdef G4MULTITHREADED
    G4cout << "   -t nThreads   : Number of threads (0=auto, use all cores)" << G4endl;
    G4cout << "   note: -t option is available only for multi-threaded mode." << G4endl;
#else
    G4cout << "   note: Multi-threaded mode not available (rebuild with -DGEANT4_BUILD_MULTITHREADED=ON)" << G4endl;
#endif
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

int main(int argc, char** argv) {

  // Parse command line arguments
  if (argc > 7) {
    PrintUsage();
    return 1;
  }

  G4String macro;
  G4String session;
#ifdef G4MULTITHREADED
  G4int nThreads = 0;  // 0 = auto-detect (use all available cores)
#endif

  for (G4int i = 1; i < argc; i = i + 2) {
    if (G4String(argv[i]) == "-m") {
      macro = argv[i + 1];
    }
    else if (G4String(argv[i]) == "-u") {
      session = argv[i + 1];
    }
#ifdef G4MULTITHREADED
    else if (G4String(argv[i]) == "-t") {
      nThreads = G4UIcommand::ConvertToInt(argv[i + 1]);
    }
#endif
    else {
      PrintUsage();
      return 1;
    }
  }

  // Detect interactive mode (if no macro provided) and define UI session
  G4UIExecutive* ui = nullptr;
  if (!macro.size()) {
    ui = new G4UIExecutive(argc, argv, session);
  }

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  // Multi-threading random seed management
#ifdef G4MULTITHREADED
  if (nThreads != 1) {
    G4cout << "=== Geant4 Multi-Threaded Mode ===" << G4endl;
    if (nThreads > 0) {
      G4cout << "Using " << nThreads << " threads as specified" << G4endl;
    } else {
      G4cout << "Auto-detecting optimal thread count (use all available cores)" << G4endl;
    }
    G4cout << "Automatic random seed management enabled" << G4endl;
  }
#endif

  // Construct the run manager with explicit multi-threading control
#ifdef G4MULTITHREADED
  auto* runManager = new G4MTRunManager;
  if (nThreads > 0) {
    runManager->SetNumberOfThreads(nThreads);
  }
  // else: nThreads=0, G4MTRunManager will auto-detect
#else
  auto* runManager = new G4RunManager;
  G4cout << "=== Geant4 Single-Threaded Mode ===" << G4endl;
#endif

  // Set mandatory initialization classes
  DetectorConstruction* det = new DetectorConstruction;
  runManager->SetUserInitialization(det);

  G4PhysListFactory physFactory;
  G4VModularPhysicsList* phys = physFactory.GetReferencePhysList("FTFP_BERT_HP");
  runManager->SetUserInitialization(phys);
  runManager->SetUserInitialization(new ActionInitialization(det));

  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  if (macro.size()) {
    // Batch mode
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + macro);
  }
  else {
    // Interactive mode
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    // Auto-open visualization if not already opened by init_vis.mac
    // Ensure viewer is ready before starting session
    G4cout << "Starting interactive session..." << G4endl;
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  delete visManager;
  delete runManager;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....
