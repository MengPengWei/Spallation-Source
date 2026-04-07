// #include "Run.hh"

// #include "DetectorConstruction.hh"
// #include "PrimaryGeneratorAction.hh"

// #include "G4ProcessTable.hh"
// #include "G4Radioactivation.hh"
// #include "G4SystemOfUnits.hh"
// #include "G4TwoVector.hh"
// #include "G4UnitsTable.hh"

// #include <fstream>

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Run::Run(DetectorConstruction* det) : fDetector(det) {}

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::SetPrimary(G4ParticleDefinition* particle, G4double energy)
// {
//     fParticle = particle;
//     fEkin = energy;
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::CountProcesses(const G4VProcess* process, G4int iVol)
// {
//     if (process == nullptr) return;
//     G4String procName = process->GetProcessName();
//     if (iVol == 1) {
//     std::map<G4String, G4int>::iterator it1 = fProcCounter1.find(procName);//如果当前步骤发生在逻辑体1中，我们检查当前步骤所涉及的物理过程的名称（procName）是否已经存在于fProcCounter1中，如果不存在，我们将该物理过程的计数器初始化为1；如果已经存在，我们将该物理过程的计数器加1，以便在运行结束时进行统计和分析。
//     if (it1 == fProcCounter1.end()) {
//         fProcCounter1[procName] = 1;//如果当前步骤发生在逻辑体1中，我们检查当前步骤所涉及的物理过程的名称（procName）是否已经存在于fProcCounter1中，如果不存在，我们将该物理过程的计数器初始化为1；如果已经存在，我们将该物理过程的计数器加1，以便在运行结束时进行统计和分析。
//     }
//     else {
//         fProcCounter1[procName]++;//
//     }
//     }

//     if (iVol == 2) {//如果当前步骤发生在逻辑体2中，我们检查当前步骤所涉及的物理过程的名称（procName）是否已经存在于fProcCounter2中，如果不存在，我们将该物理过程的计数器初始化为1；如果已经存在，我们将该物理过程的计数器加1，以便在运行结束时进行统计和分析。
//     std::map<G4String, G4int>::iterator it2 = fProcCounter2.find(procName);
//     if (it2 == fProcCounter2.end()) {
//         fProcCounter2[procName] = 1;
//     }
//     else {
//         fProcCounter2[procName]++;
//     }
//   }
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::ParticleCount(G4String name, G4double Ekin, G4int iVol)//如果当前步骤发生在逻辑体1中，我们检查当前步骤所涉及的粒子名称（name）是否已经存在于fParticleDataMap1中，如果不存在，我们将该粒子的数据初始化为一个ParticleData对象，其中计数器为1，平均能量、最小能量和最大能量都设置为当前粒子的动能Ekin；如果已经存在，我们将该粒子的数据更新：计数器加1，平均能量加上当前粒子的动能Ekin，并且更新最小能量和最大能量，以便在运行结束时进行统计和分析。
// {
//   if (iVol == 1) {
//     std::map<G4String, ParticleData>::iterator it = fParticleDataMap1.find(name);
//     if (it == fParticleDataMap1.end()) {
//       fParticleDataMap1[name] = ParticleData(1, Ekin, Ekin, Ekin);//如果当前步骤发生在逻辑体1中，我们检查当前步骤所涉及的粒子名称（name）是否已经存在于fParticleDataMap1中，如果不存在，我们将该粒子的数据初始化为一个ParticleData对象，其中计数器为1，平均能量、最小能量和最大能量都设置为当前粒子的动能Ekin；如果已经存在，我们将该粒子的数据更新：计数器加1，平均能量加上当前粒子的动能Ekin，并且更新最小能量和最大能量，以便在运行结束时进行统计和分析。
//     }
//     else {
//       ParticleData& data = it->second;//it->second是一个ParticleData对象的引用，表示当前步骤所涉及的粒子在fParticleDataMap1中的数据。通过引用data，我们可以直接修改该粒子的数据，包括计数器、平均能量、最小能量和最大能量，以便在运行结束时进行统计和分析。
//       data.fCount++;
//       data.fEmean += Ekin;
//       // update min max
//       G4double emin = data.fEmin;
//       if (Ekin < emin) data.fEmin = Ekin;
//       G4double emax = data.fEmax;
//       if (Ekin > emax) data.fEmax = Ekin;
//     }
//   }

//   if (iVol == 2) {
//     std::map<G4String, ParticleData>::iterator it = fParticleDataMap2.find(name);
//     if (it == fParticleDataMap2.end()) {
//       fParticleDataMap2[name] = ParticleData(1, Ekin, Ekin, Ekin);
//     }
//     else {
//       ParticleData& data = it->second;//it->second是一个ParticleData对象的引用，表示当前步骤所涉及的粒子在fParticleDataMap2中的数据。通过引用data，我们可以直接修改该粒子的数据，包括计数器、平均能量、最小能量和最大能量，以便在运行结束时进行统计和分析。
//       data.fCount++;
//       data.fEmean += Ekin;
//       // update min max
//       G4double emin = data.fEmin;
//       if (Ekin < emin) data.fEmin = Ekin;
//       G4double emax = data.fEmax;
//       if (Ekin > emax) data.fEmax = Ekin;
//     }
//   }
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::AddEdep(G4double edep1, G4double edep2)
// {
//   fEdepTarget += edep1;
//   fEdepTarget2 += edep1 * edep1;
//   fEdepDetect += edep2;
//   fEdepDetect2 += edep2 * edep2;
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::Merge(const G4Run* run)
// {
//     const Run* localRun = static_cast<const Run*>(run);//将传入的G4Run对象转换为Run对象，以便访问Run类中特有的数据成员和成员函数。通过使用static_cast，我们可以安全地将基类指针转换为派生类指针，从而在Merge函数中访问Run类中的数据成员和成员函数，以便在运行结束时进行统计和分析。

//   // primary particle info
//   //
//     fParticle = localRun->fParticle;
//     fEkin = localRun->fEkin;

//   // accumulate sums
//   //
//   fEdepTarget += localRun->fEdepTarget;
//   fEdepTarget2 += localRun->fEdepTarget2;
//   fEdepDetect += localRun->fEdepDetect;
//   fEdepDetect2 += localRun->fEdepDetect2;

//   // map: processes count in target

//   std::map<G4String, G4int>::const_iterator itp1;//建立一个迭代器itp1，用于遍历localRun->fProcCounter1中的元素，以便将localRun中的物理过程计数器与当前Run对象中的计数器进行合并和累积。通过使用迭代器itp1，我们可以访问localRun->fProcCounter1中的每个物理过程的名称（procName）和计数值（localCount），并将它们合并到当前Run对象的fProcCounter1中，以便在运行结束时进行统计和分析。
//   for (itp1 = localRun->fProcCounter1.begin(); itp1 != localRun->fProcCounter1.end(); ++itp1) {//遍历localRun->fProcCounter1中的每个元素，获取物理过程的名称（procName）和计数值（localCount），并将它们合并到当前Run对象的fProcCounter1中，以便在运行结束时进行统计和分析。
//     G4String procName = itp1->first;//得到key
//     G4int localCount = itp1->second;//得到value
//     if (fProcCounter1.find(procName) == fProcCounter1.end()) {//
//       fProcCounter1[procName] = localCount;//
//     }
//     else {
//       fProcCounter1[procName] += localCount;
//     }
//   }

//   // map: processes count in detector

//   std::map<G4String, G4int>::const_iterator itp2;
//   for (itp2 = localRun->fProcCounter2.begin(); itp2 != localRun->fProcCounter2.end(); ++itp2) {
//     G4String procName = itp2->first;
//     G4int localCount = itp2->second;
//     if (fProcCounter2.find(procName) == fProcCounter2.end()) {
//       fProcCounter2[procName] = localCount;
//     }
//     else {
//       fProcCounter2[procName] += localCount;
//     }
//   }

//   // map: created particles in target
//   std::map<G4String, ParticleData>::const_iterator itc;
//   for (itc = localRun->fParticleDataMap1.begin(); itc != localRun->fParticleDataMap1.end(); ++itc) {
//     G4String name = itc->first;
//     const ParticleData& localData = itc->second;//得到粒子数据
//     if (fParticleDataMap1.find(name) == fParticleDataMap1.end()) {
//       fParticleDataMap1[name] =
//         ParticleData(localData.fCount, localData.fEmean, localData.fEmin, localData.fEmax);//得到粒子的计数器、平均能量、最小能量和最大能量，并将它们合并到当前Run对象的fParticleDataMap1中，以便在运行结束时进行统计和分析。
//     }
//     else {
//       ParticleData& data = fParticleDataMap1[name];
//       data.fCount += localData.fCount;
//       data.fEmean += localData.fEmean;
//       G4double emin = localData.fEmin;
//       if (emin < data.fEmin) data.fEmin = emin;
//       G4double emax = localData.fEmax;
//       if (emax > data.fEmax) data.fEmax = emax;
//     }
//   }

//   // map: created particle in detector
//   std::map<G4String, ParticleData>::const_iterator itn;
//   for (itn = localRun->fParticleDataMap2.begin(); itn != localRun->fParticleDataMap2.end(); ++itn) {
//     G4String name = itn->first;
//     const ParticleData& localData = itn->second;
//     if (fParticleDataMap2.find(name) == fParticleDataMap2.end()) {
//       fParticleDataMap2[name] =
//         ParticleData(localData.fCount, localData.fEmean, localData.fEmin, localData.fEmax);
//     }
//     else {
//       ParticleData& data = fParticleDataMap2[name];
//       data.fCount += localData.fCount;
//       data.fEmean += localData.fEmean;
//       G4double emin = localData.fEmin;
//       if (emin < data.fEmin) data.fEmin = emin;
//       G4double emax = localData.fEmax;
//       if (emax > data.fEmax) data.fEmax = emax;
//     }
//   }

//   G4Run::Merge(run);
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::EndOfRun()
// {
//   G4int prec = 5, wid = prec + 2;
//   G4int dfprec = G4cout.precision(prec);

//   // run condition
//   //
//   G4String Particle = fParticle->GetParticleName();
//   G4cout << "\n The run is " << numberOfEvent << " " << Particle << " of "
//          << G4BestUnit(fEkin, "Energy") << " through : ";

//   G4cout << "\n Target   : Length = " << G4BestUnit(fDetector->GetTargetLength(), "Length")
//          << " Radius    = " << G4BestUnit(fDetector->GetTargetRadius(), "Length")
//          << " Material = " << fDetector->GetTargetMaterial()->GetName();
//   G4cout << "\n Detector : Length = " << G4BestUnit(fDetector->GetDetectorLength(), "Length")
//          << " Thickness = " << G4BestUnit(fDetector->GetDetectorThickness(), "Length")
//          << " Material = " << fDetector->GetDetectorMaterial()->GetName() << G4endl;

//   if (numberOfEvent == 0) {
//     G4cout.precision(dfprec);
//     return;
//   }

//   // compute mean Energy deposited and rms in target
//   //
//   G4int TotNbofEvents = numberOfEvent;
//   fEdepTarget /= TotNbofEvents;
//   fEdepTarget2 /= TotNbofEvents;
//   G4double rmsEdep = fEdepTarget2 - fEdepTarget * fEdepTarget;
//   if (rmsEdep > 0.)
//     rmsEdep = std::sqrt(rmsEdep);
//   else
//     rmsEdep = 0.;

//   G4cout << "\n Mean energy deposit in target,   in time window = "
//          << G4BestUnit(fEdepTarget, "Energy") << ";  rms = " << G4BestUnit(rmsEdep, "Energy")
//          << G4endl;

//   // compute mean Energy deposited and rms in detector
//   //
//   fEdepDetect /= TotNbofEvents;
//   fEdepDetect2 /= TotNbofEvents;
//   rmsEdep = fEdepDetect2 - fEdepDetect * fEdepDetect;
//   if (rmsEdep > 0.)
//     rmsEdep = std::sqrt(rmsEdep);
//   else
//     rmsEdep = 0.;

//   G4cout << " Mean energy deposit in detector, in time window = "
//          << G4BestUnit(fEdepDetect, "Energy") << ";  rms = " << G4BestUnit(rmsEdep, "Energy")
//          << G4endl;

//   // frequency of processes in target
//   //
//   G4cout << "\n Process calls frequency in target :" << G4endl;
//   G4int index = 0;
//   std::map<G4String, G4int>::iterator it1;
//   for (it1 = fProcCounter1.begin(); it1 != fProcCounter1.end(); it1++) {
//     G4String procName = it1->first;
//     G4int count = it1->second;
//     G4String space = " ";
//     if (++index % 3 == 0) space = "\n";//为了在输出过程中每隔三个物理过程名称和计数值后换行，以便更清晰地显示结果。通过使用index变量来跟踪已经输出的物理过程数量，我们可以在每输出三个物理过程后插入一个换行符，从而使输出结果更加整齐和易读。
//     G4cout << " " << std::setw(20) << procName << "=" << std::setw(7) << count << space;
//   }
//   G4cout << G4endl;

//   // frequency of processes in detector
//   //
//   G4cout << "\n Process calls frequency in detector:" << G4endl;
//   index = 0;
//   std::map<G4String, G4int>::iterator it2;
//   for (it2 = fProcCounter2.begin(); it2 != fProcCounter2.end(); it2++) {
//     G4String procName = it2->first;
//     G4int count = it2->second;
//     G4String space = " ";
//     if (++index % 3 == 0) space = "\n";
//     G4cout << " " << std::setw(20) << procName << "=" << std::setw(7) << count << space;
//   }
//   G4cout << G4endl;

//   // particles count in target
//   //
//   G4cout << "\n List of generated particles in target:" << G4endl;

//   std::map<G4String, ParticleData>::iterator itc;
//   for (itc = fParticleDataMap1.begin(); itc != fParticleDataMap1.end(); itc++) {
//     G4String name = itc->first;
//     ParticleData data = itc->second;
//     G4int count = data.fCount;
//     G4double eMean = data.fEmean / count;
//     G4double eMin = data.fEmin;
//     G4double eMax = data.fEmax;

//     G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
//            << "  Emean = " << std::setw(wid) << G4BestUnit(eMean, "Energy") << "\t( "
//            << G4BestUnit(eMin, "Energy") << " --> " << G4BestUnit(eMax, "Energy") << ")" << G4endl;
//   }

//   // particles count in detector
//   //
//   G4cout << "\n List of generated particles in detector:" << G4endl;

//   std::map<G4String, ParticleData>::iterator itn;
//   for (itn = fParticleDataMap2.begin(); itn != fParticleDataMap2.end(); itn++) {
//     G4String name = itn->first;
//     ParticleData data = itn->second;
//     G4int count = data.fCount;
//     G4double eMean = data.fEmean / count;
//     G4double eMin = data.fEmin;
//     G4double eMax = data.fEmax;

//     G4cout << "  " << std::setw(13) << name << ": " << std::setw(7) << count
//            << "  Emean = " << std::setw(wid) << G4BestUnit(eMean, "Energy") << "\t( "
//            << G4BestUnit(eMin, "Energy") << " --> " << G4BestUnit(eMax, "Energy") << ")" << G4endl;
//   }
//   G4cout << G4endl;

//   // activities in VR mode
//   //
//   WriteActivity(numberOfEvent);

//   // remove all contents in fProcCounter, fCount
//   fProcCounter1.clear();
//   fProcCounter2.clear();
//   fParticleDataMap1.clear();
//   fParticleDataMap2.clear();

//   // restore default format
//   G4cout.precision(dfprec);
// }

// //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void Run::WriteActivity(G4int nevent)
// {
//   G4ProcessTable* pTable = G4ProcessTable::GetProcessTable();
//   G4Radioactivation* rDecay =
//     (G4Radioactivation*)pTable->FindProcess("Radioactivation", "GenericIon");

//   // output the induced radioactivities (in VR mode only)
//   //
//   if ((rDecay == 0) || (rDecay->IsAnalogueMonteCarlo())) return;

//   G4String fileName = G4AnalysisManager::Instance()->GetFileName() + ".activity";
//   std::ofstream outfile(fileName, std::ios::out);

//   std::vector<G4RadioactivityTable*> theTables = rDecay->GetTheRadioactivityTables();

//   for (size_t i = 0; i < theTables.size(); i++) {
//     G4double rate, error;
//     outfile << "Radioactivities in decay window no. " << i << G4endl;
//     outfile << "Z \tA \tE \tActivity (decays/window) \tError (decays/window) " << G4endl;

//     map<G4ThreeVector, G4TwoVector>* aMap = theTables[i]->GetTheMap();
//     map<G4ThreeVector, G4TwoVector>::iterator iter;
//     for (iter = aMap->begin(); iter != aMap->end(); iter++) {
//       rate = iter->second.x() / nevent;
//       error = std::sqrt(iter->second.y()) / nevent;
//       if (rate < 0.) rate = 0.;  // statically it can be < 0.
//       outfile << iter->first.x() << "\t" << iter->first.y() << "\t" << iter->first.z() << "\t"
//               << rate << "\t" << error << G4endl;
//     }
//     outfile << G4endl;
//   }
//   outfile.close();
// }
