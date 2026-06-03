#ifndef ShieldingDataCollector_h
#define ShieldingDataCollector_h 1

#include "globals.hh"

#include <array>
#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <vector>

class G4Step;
class G4Track;

class ShieldingDataCollector {
 public:
  static ShieldingDataCollector& Instance();

  void ConfigureRun(G4double sourceIntensityPerSecond,
                    G4double irradiationHours,
                    G4bool doorOpen);
  void BeginRun();
  void EndRun(G4int numberOfEvents);

  void ScoreStep(const G4Step* step);
  void ScoreTrackBirth(const G4Track* track);

  // Public method for multi-threading data collection
  void MergeThreadLocalData();

  // Thread-local data accumulation methods (for multi-threading performance)
  // These are called by ThreadLocalData::MergeInto
  void AccumulateMeshData(std::size_t index, G4double gammaDoseTrackLen,
                          G4double neutronDoseTrackLen, G4double gammaTrackLen,
                          G4double neutronTrackLen);
  void AccumulateProbeData(std::size_t probeIndex, G4double gammaDoseTrackLen,
                           G4double neutronDoseTrackLen, std::size_t spectrumBin,
                           G4double gammaCount, G4double neutronCount);
  /// c9 外包络薄层：几何径迹长度(cm)，用于通量；-Z 面不写能谱时仍累计此项
  void AccumulateProbeC9TrackLen(std::size_t probeIndex, G4double gammaGeomCm,
                                 G4double neutronGeomCm);
  void AccumulateEnergyDeposition(const G4String& region, G4double edep);
  void AccumulateNuclideYield(const G4String& region, const G4String& nuclide,
                              G4double weight, G4double meanLifeS);

  // Getters for thread-local initialization
  std::size_t GetMeshSize() const;
  std::size_t GetNumberOfProbes() const;
  void GetProbeInfo(std::size_t index, G4String& name, std::array<G4double, 3>& position,
                    G4double& radius, std::size_t& spectrumSize) const;

 private:
  ShieldingDataCollector();
  ~ShieldingDataCollector() = default;

  ShieldingDataCollector(const ShieldingDataCollector&) = delete;
  ShieldingDataCollector& operator=(const ShieldingDataCollector&) = delete;

  struct Probe {
    G4String name;
    std::array<G4double, 3> positionMm {};
    G4double radiusMm {};
    /// 若为 true，则在 boxMinMaxMm（mm）轴对齐盒内计分；否则为球形探测区（radiusMm）
    G4bool useAxisAlignedBox {};
    std::array<G4double, 6> boxMinMaxMm {};  // xmin,xmax, ymin,ymax, zmin,zmax
    G4double gammaDoseWeightedTrackLenCm {};
    G4double neutronDoseWeightedTrackLenCm {};
    /// 仅 c9 外包络：ScoreC9OuterSurfaceProbes 累计的 γ/中子几何径迹(cm)，供通量（与能谱分箱独立）
    G4double c9GammaGeomTrackLenCm {};
    G4double c9NeutronGeomTrackLenCm {};
    std::vector<G4double> gammaSpectrumCounts;
    std::vector<G4double> neutronSpectrumCounts;

    Probe() = default;
    Probe(const G4String& inName,
          const std::array<G4double, 3>& inPositionMm,
          G4double inRadiusMm,
          G4double inGammaDoseWeightedTrackLenCm,
          G4double inNeutronDoseWeightedTrackLenCm,
          const std::vector<G4double>& inGammaSpectrumCounts,
          const std::vector<G4double>& inNeutronSpectrumCounts)
        : name(inName),
          positionMm(inPositionMm),
          radiusMm(inRadiusMm),
          gammaDoseWeightedTrackLenCm(inGammaDoseWeightedTrackLenCm),
          neutronDoseWeightedTrackLenCm(inNeutronDoseWeightedTrackLenCm),
          gammaSpectrumCounts(inGammaSpectrumCounts),
          neutronSpectrumCounts(inNeutronSpectrumCounts) {}
  };

  struct NuclideYield {
    G4double producedPerPrimary {};
    G4double meanLifeS {};
  };

  std::size_t MeshIndex(G4double xMm, G4double yMm, G4double zMm) const;
  G4double InterpolateDoseFactor(G4double energyMeV,
                                 const std::vector<G4double>& de,
                                 const std::vector<G4double>& df) const;
  void FillSpectrum(std::vector<G4double>& bins, G4double energyMeV, G4double weight);
  std::size_t EnergyBin(G4double energyMeV) const;
  /// c9/c20/c22 外包络外 1 cm、约 1 dm² 网格薄层探测器快速计分（见实现文件内网格常量）
  void ScoreC9OuterSurfaceProbes(const G4Step* step);
  void WritePromptDoseOutputs() const;
  void WriteThermalOutputs() const;
  void WriteNuclideOutputs() const;
  void WriteSummaryReport() const;
  void WriteRequirementParameterReport() const;
  G4String CategoryFromVolume(const G4String& volumeName) const;
  void Reset();

  static Probe MakeAxisAlignedBoxProbe(const G4String& name,
                                       const std::array<G4double, 6>& boxMm,
                                       const std::array<G4double, 3>& centerMm);
  static void AppendC9OuterEnvelopeProbes(std::vector<Probe>& probes);
  /// c20 BPE 门：RPP30∪RPP31 轴对齐外包络外 1 cm 薄层网格（与 ConstructVolumes 一致）
  static void AppendC20OuterEnvelopeProbes(std::vector<Probe>& probes);
  /// c22 钢门：RPP34 外包络外 1 cm 薄层网格
  static void AppendC22OuterEnvelopeProbes(std::vector<Probe>& probes);
  static G4double ComputeProbeScoringVolumeCm3(const Probe& probe);

  // Global mutex only used for configuration and final output
  mutable std::mutex fMutex;

  G4double fSourceIntensityPerSecond {};
  G4double fIrradiationHours {};
  G4bool fDoorOpen {};
  G4int fNumberOfEvents {};

  std::array<G4double, 3> fMeshOriginMm {};
  std::array<G4double, 3> fMeshMaxMm {};
  std::array<G4int, 3> fMeshBins {};
  G4double fVoxelVolumeCm3 {};

  // Master data (aggregated from all threads)
  std::vector<G4double> fGammaDoseWeightedTrackLenCm;
  std::vector<G4double> fNeutronDoseWeightedTrackLenCm;
  std::vector<G4double> fGammaTrackLenCm;
  std::vector<G4double> fNeutronTrackLenCm;

  std::vector<G4double> fGammaDeMeV;
  std::vector<G4double> fGammaDfSvPerHourPerFlux;
  std::vector<G4double> fNeutronDeMeV;
  std::vector<G4double> fNeutronDfSvPerHourPerFlux;

  std::vector<Probe> fProbes;
  /// 第一个 c9 / c20 / c22 外包络网格探测器在 fProbes 中的下标
  std::size_t fFirstC9ProbeIndex {0};
  std::size_t fFirstC20ProbeIndex {0};
  std::size_t fFirstC22ProbeIndex {0};
  std::vector<G4double> fSpectrumEdgesMeV;

  std::map<G4String, G4double> fEnergyDepositionByRegionJ;
  std::map<G4String, std::map<G4String, NuclideYield>> fNuclideYieldsByRegion;
  
  // 存储每个物理体积的质量(kg)用于计算放射性比活度
  mutable std::map<G4String, G4double> fRegionMassKg;
  G4double GetRegionMass(const G4String& regionName) const;
};

#endif
