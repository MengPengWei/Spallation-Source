#include "ShieldingDataCollector.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4TouchableHandle.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4VSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4Threading.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace {
constexpr G4double kTiny = 1.0e-30;

// 输出目录前缀 - 所有文件输出到build目录
const char* OUTPUT_DIR = "./";

// Thread-local data structure for lock-free multi-threading
struct ThreadLocalData {
  std::vector<G4double> gammaDoseWeightedTrackLenCm;
  std::vector<G4double> neutronDoseWeightedTrackLenCm;
  std::vector<G4double> gammaTrackLenCm;
  std::vector<G4double> neutronTrackLenCm;

  // Probe data stored as parallel vectors
  std::vector<G4String> probeNames;
  std::vector<std::array<G4double, 3>> probePositions;
  std::vector<G4double> probeRadii;
  std::vector<G4double> probeGammaDoseWeightedTrackLenCm;
  std::vector<G4double> probeNeutronDoseWeightedTrackLenCm;
  std::vector<G4double> probeC9GammaGeomTrackLenCm;
  std::vector<G4double> probeC9NeutronGeomTrackLenCm;
  std::vector<std::vector<G4double>> probeGammaSpectrumCounts;
  std::vector<std::vector<G4double>> probeNeutronSpectrumCounts;

  std::map<G4String, G4double> energyDepositionByRegionJ;

  // Simple struct for nuclide yield
  struct NuclideYieldData {
    G4double producedPerPrimary {};
    G4double meanLifeS {};
  };
  std::map<G4String, std::map<G4String, NuclideYieldData>> nuclideYieldsByRegion;

  ThreadLocalData() = default;

  explicit ThreadLocalData(const ShieldingDataCollector* parent) {
    const std::size_t meshSize = parent->GetMeshSize();
    gammaDoseWeightedTrackLenCm.assign(meshSize, 0.0);
    neutronDoseWeightedTrackLenCm.assign(meshSize, 0.0);
    gammaTrackLenCm.assign(meshSize, 0.0);
    neutronTrackLenCm.assign(meshSize, 0.0);

    // Copy probe definitions using public getter
    const std::size_t nProbes = parent->GetNumberOfProbes();
    probeNames.reserve(nProbes);
    probePositions.reserve(nProbes);
    probeRadii.reserve(nProbes);
    probeGammaDoseWeightedTrackLenCm.assign(nProbes, 0.0);
    probeNeutronDoseWeightedTrackLenCm.assign(nProbes, 0.0);
    probeC9GammaGeomTrackLenCm.assign(nProbes, 0.0);
    probeC9NeutronGeomTrackLenCm.assign(nProbes, 0.0);
    probeGammaSpectrumCounts.resize(nProbes);
    probeNeutronSpectrumCounts.resize(nProbes);

    for (std::size_t i = 0; i < nProbes; ++i) {
      G4String name;
      std::array<G4double, 3> position;
      G4double radius;
      std::size_t spectrumSize;
      parent->GetProbeInfo(i, name, position, radius, spectrumSize);
      probeNames.push_back(name);
      probePositions.push_back(position);
      probeRadii.push_back(radius);
      probeGammaSpectrumCounts[i].assign(spectrumSize, 0.0);
      probeNeutronSpectrumCounts[i].assign(spectrumSize, 0.0);
    }
  }

  void Reset() {
    std::fill(gammaDoseWeightedTrackLenCm.begin(), gammaDoseWeightedTrackLenCm.end(), 0.0);
    std::fill(neutronDoseWeightedTrackLenCm.begin(), neutronDoseWeightedTrackLenCm.end(), 0.0);
    std::fill(gammaTrackLenCm.begin(), gammaTrackLenCm.end(), 0.0);
    std::fill(neutronTrackLenCm.begin(), neutronTrackLenCm.end(), 0.0);

    const std::size_t nProbes = probeGammaDoseWeightedTrackLenCm.size();
    for (std::size_t i = 0; i < nProbes; ++i) {
      probeGammaDoseWeightedTrackLenCm[i] = 0.0;
      probeNeutronDoseWeightedTrackLenCm[i] = 0.0;
      probeC9GammaGeomTrackLenCm[i] = 0.0;
      probeC9NeutronGeomTrackLenCm[i] = 0.0;
      std::fill(probeGammaSpectrumCounts[i].begin(), probeGammaSpectrumCounts[i].end(), 0.0);
      std::fill(probeNeutronSpectrumCounts[i].begin(), probeNeutronSpectrumCounts[i].end(), 0.0);
    }

    energyDepositionByRegionJ.clear();
    nuclideYieldsByRegion.clear();
  }

  void MergeInto(ShieldingDataCollector* parent) {
    // Merge mesh data
    for (std::size_t i = 0; i < gammaDoseWeightedTrackLenCm.size(); ++i) {
      parent->AccumulateMeshData(i, gammaDoseWeightedTrackLenCm[i],
                                 neutronDoseWeightedTrackLenCm[i],
                                 gammaTrackLenCm[i], neutronTrackLenCm[i]);
    }

    // Merge probe data
    const std::size_t nProbes = probeGammaDoseWeightedTrackLenCm.size();
    for (std::size_t i = 0; i < nProbes; ++i) {
      for (std::size_t j = 0; j < probeGammaSpectrumCounts[i].size(); ++j) {
        parent->AccumulateProbeData(i, probeGammaDoseWeightedTrackLenCm[i],
                                    probeNeutronDoseWeightedTrackLenCm[i], j,
                                    probeGammaSpectrumCounts[i][j],
                                    probeNeutronSpectrumCounts[i][j]);
      }
      parent->AccumulateProbeC9TrackLen(i, probeC9GammaGeomTrackLenCm[i],
                                        probeC9NeutronGeomTrackLenCm[i]);
    }

    // Merge energy deposition
    for (const auto& kv : energyDepositionByRegionJ) {
      parent->AccumulateEnergyDeposition(kv.first, kv.second);
    }

    // Merge nuclide yields
    for (const auto& regionEntry : nuclideYieldsByRegion) {
      for (const auto& nucEntry : regionEntry.second) {
        parent->AccumulateNuclideYield(regionEntry.first, nucEntry.first,
                                       nucEntry.second.producedPerPrimary,
                                       nucEntry.second.meanLifeS);
      }
    }
  }
};

// Thread-local storage for lock-free multi-threading
thread_local ThreadLocalData* tls_data = nullptr;

}  // namespace

// --- c9（RPP12）/ c20（BPE 门包络）/ c22（钢门 RPP34）外包络网格全局索引
namespace {
std::array<std::size_t, 6> g_c9FaceStart {};
std::array<G4int, 6> g_c9Nu {};
std::array<G4int, 6> g_c9Nv {};
std::size_t g_c9ProbeCount = 0;

std::array<std::size_t, 6> g_c20FaceStart {};
std::array<G4int, 6> g_c20Nu {};
std::array<G4int, 6> g_c20Nv {};
std::size_t g_c20ProbeCount = 0;

std::array<std::size_t, 6> g_c22FaceStart {};
std::array<G4int, 6> g_c22Nu {};
std::array<G4int, 6> g_c22Nv {};
std::size_t g_c22ProbeCount = 0;
}  // namespace

static bool IsOuterEnvelopeMeshName(const G4String& name) {
  return name.find("c9_outer_") == 0 || name.find("c20_outer_") == 0 || name.find("c22_outer_") == 0;
}

ShieldingDataCollector::Probe ShieldingDataCollector::MakeAxisAlignedBoxProbe(
    const G4String& name,
    const std::array<G4double, 6>& boxMm,
    const std::array<G4double, 3>& centerMm) {
  Probe p;
  p.name = name;
  p.positionMm = centerMm;
  p.radiusMm = 0.0;
  p.useAxisAlignedBox = true;
  p.boxMinMaxMm = boxMm;
  p.gammaDoseWeightedTrackLenCm = 0.0;
  p.neutronDoseWeightedTrackLenCm = 0.0;
  p.gammaSpectrumCounts.assign(60, 0.0);
  p.neutronSpectrumCounts.assign(60, 0.0);
  return p;
}

void ShieldingDataCollector::AppendC9OuterEnvelopeProbes(std::vector<Probe>& probes) {
  const G4double pitch = 10.0 * cm;
  const G4double sx = -222.7 * cm;
  const G4double ex = 222.7 * cm;
  const G4double sy = -222.7 * cm;
  const G4double ey = 222.7 * cm;
  const G4double sz = 0.0 * cm;
  const G4double ez = 340.5 * cm;
  const G4double dout = 1.0 * cm;
  const G4double halfSlab = 2.0 * mm;

  const G4int nx = static_cast<G4int>(std::ceil((ex - sx) / pitch));
  const G4int ny = static_cast<G4int>(std::ceil((ey - sy) / pitch));
  const G4int nz = static_cast<G4int>(std::ceil((ez - sz) / pitch));

  std::size_t rel = 0;
  int fi = 0;

  // +X
  g_c9Nu[fi] = ny;
  g_c9Nv[fi] = nz;
  g_c9FaceStart[fi] = rel;
  {
    const G4double xPlane = ex + dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_px_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  // -X
  g_c9Nu[fi] = ny;
  g_c9Nv[fi] = nz;
  g_c9FaceStart[fi] = rel;
  {
    const G4double xPlane = sx - dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_mx_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  // +Y
  g_c9Nu[fi] = nx;
  g_c9Nv[fi] = nz;
  g_c9FaceStart[fi] = rel;
  {
    const G4double yPlane = ey + dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_py_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  // -Y
  g_c9Nu[fi] = nx;
  g_c9Nv[fi] = nz;
  g_c9FaceStart[fi] = rel;
  {
    const G4double yPlane = sy - dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_my_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  // +Z
  g_c9Nu[fi] = nx;
  g_c9Nv[fi] = ny;
  g_c9FaceStart[fi] = rel;
  {
    const G4double zPlane = ez + dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_pz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  // -Z
  g_c9Nu[fi] = nx;
  g_c9Nv[fi] = ny;
  g_c9FaceStart[fi] = rel;
  {
    const G4double zPlane = sz - dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c9_outer_mz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }

  g_c9ProbeCount = rel;
}

void ShieldingDataCollector::AppendC20OuterEnvelopeProbes(std::vector<Probe>& probes) {
  // RPP30∪RPP31 轴对齐包络：与 GetRpp(30)、GetRpp(31) 合成的外包络 AABB 一致
  const G4double pitch = 10.0 * cm;
  const G4double sx = -60.0 * cm;
  const G4double ex = 10.0 * cm;
  const G4double sy = -300.0 * cm;
  const G4double ey = -222.7 * cm;
  const G4double sz = 0.0 * cm;
  const G4double ez = 105.0 * cm;
  const G4double dout = 1.0 * cm;
  const G4double halfSlab = 2.0 * mm;

  const G4int nx = static_cast<G4int>(std::ceil((ex - sx) / pitch));
  const G4int ny = static_cast<G4int>(std::ceil((ey - sy) / pitch));
  const G4int nz = static_cast<G4int>(std::ceil((ez - sz) / pitch));

  std::size_t rel = 0;
  int fi = 0;

  g_c20Nu[fi] = ny;
  g_c20Nv[fi] = nz;
  g_c20FaceStart[fi] = rel;
  {
    const G4double xPlane = ex + dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_px_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c20Nu[fi] = ny;
  g_c20Nv[fi] = nz;
  g_c20FaceStart[fi] = rel;
  {
    const G4double xPlane = sx - dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_mx_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c20Nu[fi] = nx;
  g_c20Nv[fi] = nz;
  g_c20FaceStart[fi] = rel;
  {
    const G4double yPlane = ey + dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_py_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c20Nu[fi] = nx;
  g_c20Nv[fi] = nz;
  g_c20FaceStart[fi] = rel;
  {
    const G4double yPlane = sy - dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_my_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c20Nu[fi] = nx;
  g_c20Nv[fi] = ny;
  g_c20FaceStart[fi] = rel;
  {
    const G4double zPlane = ez + dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_pz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c20Nu[fi] = nx;
  g_c20Nv[fi] = ny;
  g_c20FaceStart[fi] = rel;
  {
    const G4double zPlane = sz - dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c20_outer_mz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }

  g_c20ProbeCount = rel;
}

void ShieldingDataCollector::AppendC22OuterEnvelopeProbes(std::vector<Probe>& probes) {
  // 钢门外包络 RPP34（GetRpp(34)），与 c22 构造中外壳一致
  const G4double pitch = 10.0 * cm;
  const G4double sx = -60.0 * cm;
  const G4double ex = 10.0 * cm;
  const G4double sy = -315.0 * cm;
  const G4double ey = -222.7 * cm;
  const G4double sz = 0.0 * cm;
  const G4double ez = 115.0 * cm;
  const G4double dout = 1.0 * cm;
  const G4double halfSlab = 2.0 * mm;

  const G4int nx = static_cast<G4int>(std::ceil((ex - sx) / pitch));
  const G4int ny = static_cast<G4int>(std::ceil((ey - sy) / pitch));
  const G4int nz = static_cast<G4int>(std::ceil((ez - sz) / pitch));

  std::size_t rel = 0;
  int fi = 0;

  g_c22Nu[fi] = ny;
  g_c22Nv[fi] = nz;
  g_c22FaceStart[fi] = rel;
  {
    const G4double xPlane = ex + dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_px_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c22Nu[fi] = ny;
  g_c22Nv[fi] = nz;
  g_c22FaceStart[fi] = rel;
  {
    const G4double xPlane = sx - dout;
    const G4double xmin = xPlane - halfSlab;
    const G4double xmax = xPlane + halfSlab;
    for (G4int iy = 0; iy < ny; ++iy) {
      const G4double y0 = sy + iy * pitch;
      const G4double y1 = std::min(y0 + pitch, ey);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {xmin, xmax, y0, y1, z0, z1};
        const std::array<G4double, 3> ctr = {(xmin + xmax) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_mx_" + std::to_string(iy) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c22Nu[fi] = nx;
  g_c22Nv[fi] = nz;
  g_c22FaceStart[fi] = rel;
  {
    const G4double yPlane = ey + dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_py_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c22Nu[fi] = nx;
  g_c22Nv[fi] = nz;
  g_c22FaceStart[fi] = rel;
  {
    const G4double yPlane = sy - dout;
    const G4double ymin = yPlane - halfSlab;
    const G4double ymax = yPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iz = 0; iz < nz; ++iz) {
        const G4double z0 = sz + iz * pitch;
        const G4double z1 = std::min(z0 + pitch, ez);
        const std::array<G4double, 6> box = {x0, x1, ymin, ymax, z0, z1};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (ymin + ymax) * 0.5, (z0 + z1) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_my_" + std::to_string(ix) + "_" + std::to_string(iz), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c22Nu[fi] = nx;
  g_c22Nv[fi] = ny;
  g_c22FaceStart[fi] = rel;
  {
    const G4double zPlane = ez + dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_pz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }
  ++fi;

  g_c22Nu[fi] = nx;
  g_c22Nv[fi] = ny;
  g_c22FaceStart[fi] = rel;
  {
    const G4double zPlane = sz - dout;
    const G4double zmin = zPlane - halfSlab;
    const G4double zmax = zPlane + halfSlab;
    for (G4int ix = 0; ix < nx; ++ix) {
      const G4double x0 = sx + ix * pitch;
      const G4double x1 = std::min(x0 + pitch, ex);
      for (G4int iy = 0; iy < ny; ++iy) {
        const G4double y0 = sy + iy * pitch;
        const G4double y1 = std::min(y0 + pitch, ey);
        const std::array<G4double, 6> box = {x0, x1, y0, y1, zmin, zmax};
        const std::array<G4double, 3> ctr = {(x0 + x1) * 0.5, (y0 + y1) * 0.5, (zmin + zmax) * 0.5};
        probes.push_back(MakeAxisAlignedBoxProbe(
            "c22_outer_mz_" + std::to_string(ix) + "_" + std::to_string(iy), box, ctr));
        ++rel;
      }
    }
  }

  g_c22ProbeCount = rel;
}

G4double ShieldingDataCollector::ComputeProbeScoringVolumeCm3(const Probe& probe) {
  if (probe.useAxisAlignedBox) {
    const auto& b = probe.boxMinMaxMm;
    const G4double vx = (b[1] - b[0]) / mm;
    const G4double vy = (b[3] - b[2]) / mm;
    const G4double vz = (b[5] - b[4]) / mm;
    return vx * vy * vz / 1000.0;
  }
  return (4.0 / 3.0) * CLHEP::pi * std::pow(probe.radiusMm / mm, 3);
}

ShieldingDataCollector& ShieldingDataCollector::Instance() {
  static ShieldingDataCollector instance;
  return instance;
}

ShieldingDataCollector::ShieldingDataCollector() {
  fMeshOriginMm = {-8000.0, -10000.0, -4500.0};
  fMeshMaxMm = {8000.0, 10000.0, 7500.0};
  fMeshBins = {199, 249, 149};

  const auto nx = static_cast<std::size_t>(fMeshBins[0]);
  const auto ny = static_cast<std::size_t>(fMeshBins[1]);
  const auto nz = static_cast<std::size_t>(fMeshBins[2]);
  const auto meshSize = nx * ny * nz;

  fGammaDoseWeightedTrackLenCm.assign(meshSize, 0.0);
  fNeutronDoseWeightedTrackLenCm.assign(meshSize, 0.0);
  fGammaTrackLenCm.assign(meshSize, 0.0);
  fNeutronTrackLenCm.assign(meshSize, 0.0);

  const G4double dxCm = (fMeshMaxMm[0] - fMeshOriginMm[0]) / mm / fMeshBins[0];
  const G4double dyCm = (fMeshMaxMm[1] - fMeshOriginMm[1]) / mm / fMeshBins[1];
  const G4double dzCm = (fMeshMaxMm[2] - fMeshOriginMm[2]) / mm / fMeshBins[2];
  fVoxelVolumeCm3 = dxCm * dyCm * dzCm;

  fGammaDeMeV = {0.01, 0.03, 0.05, 0.07, 0.1,  0.15, 0.2,  0.25, 0.3,  0.35,
                 0.4,  0.45, 0.5,  0.55, 0.6,  0.65, 0.7,  0.8,  1.0,  1.4,
                 1.8,  2.2,  2.6,  2.8,  3.25, 3.75, 4.25, 4.75, 5.0,  5.25,
                 5.75, 6.25, 6.75, 7.5,  9.0,  11.0, 13.0, 15.0};
  fGammaDfSvPerHourPerFlux = {
      3.96e-08, 5.82e-09, 2.90e-09, 2.58e-09, 2.83e-09, 3.79e-09, 5.01e-09,
      6.31e-09, 7.59e-09, 8.78e-09, 9.85e-09, 1.08e-08, 1.17e-08, 1.27e-08,
      1.36e-08, 1.44e-08, 1.52e-08, 1.68e-08, 1.98e-08, 2.51e-08, 2.99e-08,
      3.42e-08, 3.82e-08, 4.01e-08, 4.41e-08, 4.83e-08, 5.23e-08, 5.60e-08,
      5.80e-08, 6.01e-08, 6.37e-08, 6.74e-08, 7.11e-08, 7.66e-08, 8.77e-08,
      1.03e-07, 1.18e-07, 1.33e-07};

  fNeutronDeMeV = {1.00e-09, 1.00e-08, 2.53e-08, 1.00e-07, 2.00e-07, 5.00e-07,
                   1.00e-06, 2.00e-06, 5.00e-06, 1.00e-05, 2.00e-05, 5.00e-05,
                   1.00e-04, 2.00e-04, 5.00e-04, 1.00e-03, 2.00e-03, 5.00e-03,
                   1.00e-02, 2.00e-02, 3.00e-02, 5.00e-02, 7.00e-02, 1.00e-01,
                   1.50e-01, 2.00e-01, 3.00e-01, 5.00e-01, 7.00e-01, 9.00e-01,
                   1.00e+00, 1.20e+00, 2.00e+00, 3.00e+00, 4.00e+00, 5.00e+00,
                   6.00e+00, 7.00e+00, 8.00e+00, 9.00e+00, 1.00e+01, 1.20e+01,
                   1.40e+01, 1.50e+01, 1.60e+01, 1.80e+01, 2.00e+01};
  fNeutronDfSvPerHourPerFlux = {
      2.376e-08, 3.240e-08, 3.816e-08, 4.644e-08, 4.860e-08, 4.896e-08,
      4.788e-08, 4.644e-08, 4.320e-08, 4.068e-08, 3.816e-08, 3.564e-08,
      3.384e-08, 3.204e-08, 2.988e-08, 2.844e-08, 2.772e-08, 2.880e-08,
      3.780e-08, 5.976e-08, 8.532e-08, 1.480e-07, 2.160e-07, 3.168e-07,
      4.752e-07, 6.120e-07, 8.388e-07, 1.159e-06, 1.350e-06, 1.440e-06,
      1.498e-06, 1.530e-06, 1.512e-06, 1.483e-06, 1.469e-06, 1.458e-06,
      1.440e-06, 1.458e-06, 1.472e-06, 1.512e-06, 1.584e-06, 1.728e-06,
      1.872e-06, 1.944e-06, 1.944e-06, 2.052e-06, 2.160e-06};

  fSpectrumEdgesMeV.resize(61, 0.0);
  const G4double logMin = std::log10(1.0e-9);
  const G4double logMax = std::log10(100.0);
  const G4double dLog = (logMax - logMin) / 60.0;
  for (std::size_t i = 0; i < fSpectrumEdgesMeV.size(); ++i) {
    fSpectrumEdgesMeV[i] = std::pow(10.0, logMin + dLog * static_cast<G4double>(i));
  }

  std::vector<Probe> probes;
  probes.push_back({"shield_outer_30cm", {2527.0, 0.0, 1702.5}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"hotcell_outer_30cm", {2827.0, 0.0, 1702.5}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"channel_22_exit", {0.0, 3200.0, 900.0}, 35.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"channel_23_exit", {2250.0, 2250.0, 900.0}, 35.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"channel_24_exit", {-2250.0, 2250.0, 900.0}, 35.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"door_operation_point", {-350.0, -2500.0, 950.0}, 80.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  
  // 新增：CenterTube 水区探测器
  probes.push_back({"center_tube_water", {0.0, 0.0, 905.6}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"center_tube_edge", {0.0, -185.0, 900.0}, 30.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  
  // 新增：c4 铁壳外表面探测器
  probes.push_back({"c4_fe_outer_side", {677.0, 0.0, 900.0}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"c4_fe_outer_top", {0.0, 0.0, 1855.0}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  
  // 新增：c4 铁壳内表面探测器（靠近水区）
  probes.push_back({"c4_fe_inner_side", {652.0, 0.0, 900.0}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"c4_fe_inner_top", {0.0, 0.0, 1830.0}, 50.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});
  probes.push_back({"c4_fe_inner_corner", {652.0, 652.0, 900.0}, 40.0, 0.0, 0.0,
                    std::vector<G4double>(60, 0.0), std::vector<G4double>(60, 0.0)});

  fFirstC9ProbeIndex = probes.size();
  AppendC9OuterEnvelopeProbes(probes);
  fFirstC20ProbeIndex = probes.size();
  AppendC20OuterEnvelopeProbes(probes);
  fFirstC22ProbeIndex = probes.size();
  AppendC22OuterEnvelopeProbes(probes);

  fProbes = probes;
}

// Getters for thread-local initialization
std::size_t ShieldingDataCollector::GetMeshSize() const {
  const auto nx = static_cast<std::size_t>(fMeshBins[0]);
  const auto ny = static_cast<std::size_t>(fMeshBins[1]);
  const auto nz = static_cast<std::size_t>(fMeshBins[2]);
  return nx * ny * nz;
}

std::size_t ShieldingDataCollector::GetNumberOfProbes() const {
  return fProbes.size();
}

void ShieldingDataCollector::GetProbeInfo(std::size_t index, G4String& name,
                                          std::array<G4double, 3>& position,
                                          G4double& radius,
                                          std::size_t& spectrumSize) const {
  if (index < fProbes.size()) {
    name = fProbes[index].name;
    position = fProbes[index].positionMm;
    radius = fProbes[index].radiusMm;
    spectrumSize = fProbes[index].gammaSpectrumCounts.size();
  }
}

// Accumulation methods for thread-local data merging
void ShieldingDataCollector::AccumulateMeshData(std::size_t index, G4double gammaDoseTrackLen,
                                                G4double neutronDoseTrackLen, G4double gammaTrackLen,
                                                G4double neutronTrackLen) {
  if (index < fGammaDoseWeightedTrackLenCm.size()) {
    std::lock_guard<std::mutex> lock(fMutex);
    fGammaDoseWeightedTrackLenCm[index] += gammaDoseTrackLen;
    fNeutronDoseWeightedTrackLenCm[index] += neutronDoseTrackLen;
    fGammaTrackLenCm[index] += gammaTrackLen;
    fNeutronTrackLenCm[index] += neutronTrackLen;
  }
}

void ShieldingDataCollector::AccumulateProbeData(std::size_t probeIndex, G4double gammaDoseTrackLen,
                                                 G4double neutronDoseTrackLen, std::size_t spectrumBin,
                                                 G4double gammaCount, G4double neutronCount) {
  if (probeIndex < fProbes.size()) {
    std::lock_guard<std::mutex> lock(fMutex);
    // 合并线程数据时按能量仓重复调用；剂量只应累加一次（spectrumBin==0）
    if (spectrumBin == 0) {
      fProbes[probeIndex].gammaDoseWeightedTrackLenCm += gammaDoseTrackLen;
      fProbes[probeIndex].neutronDoseWeightedTrackLenCm += neutronDoseTrackLen;
    }
    if (spectrumBin < fProbes[probeIndex].gammaSpectrumCounts.size()) {
      fProbes[probeIndex].gammaSpectrumCounts[spectrumBin] += gammaCount;
      fProbes[probeIndex].neutronSpectrumCounts[spectrumBin] += neutronCount;
    }
  }
}

void ShieldingDataCollector::AccumulateProbeC9TrackLen(std::size_t probeIndex,
                                                       G4double gammaGeomCm,
                                                       G4double neutronGeomCm) {
  if (probeIndex < fProbes.size()) {
    std::lock_guard<std::mutex> lock(fMutex);
    fProbes[probeIndex].c9GammaGeomTrackLenCm += gammaGeomCm;
    fProbes[probeIndex].c9NeutronGeomTrackLenCm += neutronGeomCm;
  }
}

void ShieldingDataCollector::AccumulateEnergyDeposition(const G4String& region, G4double edep) {
  std::lock_guard<std::mutex> lock(fMutex);
  fEnergyDepositionByRegionJ[region] += edep;
}

void ShieldingDataCollector::AccumulateNuclideYield(const G4String& region, const G4String& nuclide,
                                                  G4double weight, G4double meanLifeS) {
  std::lock_guard<std::mutex> lock(fMutex);
  auto& entry = fNuclideYieldsByRegion[region][nuclide];
  entry.producedPerPrimary += weight;
  if (entry.meanLifeS <= 0.0) {
    entry.meanLifeS = meanLifeS;
  }
}

void ShieldingDataCollector::ConfigureRun(G4double sourceIntensityPerSecond,
                                          G4double irradiationHours,
                                          G4bool doorOpen) {
  std::lock_guard<std::mutex> lock(fMutex);
  fSourceIntensityPerSecond = sourceIntensityPerSecond;
  fIrradiationHours = irradiationHours;
  fDoorOpen = doorOpen;
}

void ShieldingDataCollector::BeginRun() {
  std::lock_guard<std::mutex> lock(fMutex);
  Reset();
}

void ShieldingDataCollector::EndRun(G4int numberOfEvents) {
  std::lock_guard<std::mutex> lock(fMutex);
  fNumberOfEvents = numberOfEvents;
  if (fNumberOfEvents <= 0) {
    return;
  }
  WritePromptDoseOutputs();
  WriteThermalOutputs();
  WriteNuclideOutputs();
  WriteSummaryReport();
  WriteRequirementParameterReport();
}

void ShieldingDataCollector::MergeThreadLocalData() {
  // Merge thread-local data from the current thread into the master collector.
  if (tls_data) {
    tls_data->MergeInto(this);
    // Clear the thread-local data after merging
    tls_data->Reset();
  }
}

void ShieldingDataCollector::ScoreStep(const G4Step* step) {
  // Initialize thread-local storage on first use
  if (!tls_data) {
    tls_data = new ThreadLocalData(this);
  }

  const auto* track = step->GetTrack();
  const auto* particle = track->GetDefinition();
  const auto* prePoint = step->GetPreStepPoint();

  const auto* physical = prePoint->GetTouchableHandle()->GetVolume();
  if (physical == nullptr) {
    return;
  }

  const G4double stepLenCm = step->GetStepLength() / cm;
  const G4double energyMeV = std::max(track->GetKineticEnergy() / MeV, 1.0e-12);
  const G4double edepJ = step->GetTotalEnergyDeposit();
  const G4ThreeVector pointMm = prePoint->GetPosition();

  // Update thread-local energy deposition (no lock needed)
  if (edepJ > 0.0) {
    tls_data->energyDepositionByRegionJ[physical->GetName()] += edepJ;
  }

  // 收集区域质量信息用于计算放射性比活度 (只收集一次)
  const G4String regionName = physical->GetName();
  if (fRegionMassKg.find(regionName) == fRegionMassKg.end()) {
    G4LogicalVolume* logicVol = physical->GetLogicalVolume();
    if (logicVol) {
      G4Material* material = logicVol->GetMaterial();
      G4VSolid* solid = logicVol->GetSolid();
      if (material && solid) {
        // GetDensity() 返回内部单位制的密度，转换为 g/cm³ 需要除以 (g/cm3)
        // g/cm3 = 1.0*g / (10*mm)^3 = 1.0 / 1000.0 = 0.001
        G4double densityGperCm3 = material->GetDensity() / (g/cm3);
        // GetCubicVolume() 返回 mm³，转换为 cm³: 1 cm³ = 1000 mm³
        G4double volumeMm3 = solid->GetCubicVolume();
        G4double volumeCm3 = volumeMm3 / 1000.0;
        // 质量 = 密度(g/cm³) × 体积(cm³) / 1000 = kg
        G4double massKg = densityGperCm3 * volumeCm3 / 1000.0;
        fRegionMassKg[regionName] = massKg;
      }
    }
  }

  // Update thread-local mesh data (no lock needed)
  const std::size_t idx = MeshIndex(pointMm.x(), pointMm.y(), pointMm.z());
  if (idx != static_cast<std::size_t>(-1) && stepLenCm > 0.0) {
    if (particle->GetParticleName() == "gamma") {
      const G4double df = InterpolateDoseFactor(energyMeV, fGammaDeMeV, fGammaDfSvPerHourPerFlux);
      tls_data->gammaTrackLenCm[idx] += stepLenCm;
      tls_data->gammaDoseWeightedTrackLenCm[idx] += stepLenCm * df;
    } else if (particle->GetParticleName() == "neutron") {
      const G4double df =
          InterpolateDoseFactor(energyMeV, fNeutronDeMeV, fNeutronDfSvPerHourPerFlux);
      tls_data->neutronTrackLenCm[idx] += stepLenCm;
      tls_data->neutronDoseWeightedTrackLenCm[idx] += stepLenCm * df;
    }
  }

  // Update thread-local probe data (球形点探测器；c9 外包络网格见 ScoreC9OuterSurfaceProbes)
  const std::size_t nLegacy =
      std::min(fFirstC9ProbeIndex, tls_data->probeGammaDoseWeightedTrackLenCm.size());
  for (std::size_t i = 0; i < nLegacy; ++i) {
    const G4double dx = pointMm.x() - tls_data->probePositions[i][0];
    const G4double dy = pointMm.y() - tls_data->probePositions[i][1];
    const G4double dz = pointMm.z() - tls_data->probePositions[i][2];
    if (dx * dx + dy * dy + dz * dz > tls_data->probeRadii[i] * tls_data->probeRadii[i]) {
      continue;
    }

    if (particle->GetParticleName() == "gamma") {
      const G4double df = InterpolateDoseFactor(energyMeV, fGammaDeMeV, fGammaDfSvPerHourPerFlux);
      tls_data->probeGammaDoseWeightedTrackLenCm[i] += stepLenCm * df;
      const auto binIdx = EnergyBin(energyMeV);
      if (binIdx < tls_data->probeGammaSpectrumCounts[i].size()) {
        tls_data->probeGammaSpectrumCounts[i][binIdx] += stepLenCm;
      }
    } else if (particle->GetParticleName() == "neutron") {
      const G4double df =
          InterpolateDoseFactor(energyMeV, fNeutronDeMeV, fNeutronDfSvPerHourPerFlux);
      tls_data->probeNeutronDoseWeightedTrackLenCm[i] += stepLenCm * df;
      const auto binIdx = EnergyBin(energyMeV);
      if (binIdx < tls_data->probeNeutronSpectrumCounts[i].size()) {
        tls_data->probeNeutronSpectrumCounts[i][binIdx] += stepLenCm;
      }
    }
  }

  ScoreC9OuterSurfaceProbes(step);
}

void ShieldingDataCollector::ScoreC9OuterSurfaceProbes(const G4Step* step) {
  if (!tls_data) {
    return;
  }

  const G4double stepLenCm = step->GetStepLength() / cm;
  if (stepLenCm <= 0.0) {
    return;
  }

  const auto* track = step->GetTrack();
  const auto* particle = track->GetDefinition();
  const bool isGamma = particle->GetParticleName() == "gamma";
  const bool isNeutron = particle->GetParticleName() == "neutron";
  if (!isGamma && !isNeutron) {
    return;
  }

  const auto* prePoint = step->GetPreStepPoint();
  const G4ThreeVector pointMm = prePoint->GetPosition();
  const G4double x = pointMm.x();
  const G4double y = pointMm.y();
  const G4double z = pointMm.z();
  const G4double energyMeV = std::max(track->GetKineticEnergy() / MeV, 1.0e-12);

  const G4double pitch = 10.0 * cm;
  const G4double dout = 1.0 * cm;
  const G4double halfSlab = 2.0 * mm;

  auto scoreIndex = [&](std::size_t globalProbeIdx, G4bool recordSpectrum) {
    if (globalProbeIdx >= tls_data->probeGammaDoseWeightedTrackLenCm.size()) {
      return;
    }
    if (isGamma) {
      const G4double df = InterpolateDoseFactor(energyMeV, fGammaDeMeV, fGammaDfSvPerHourPerFlux);
      tls_data->probeGammaDoseWeightedTrackLenCm[globalProbeIdx] += stepLenCm * df;
      tls_data->probeC9GammaGeomTrackLenCm[globalProbeIdx] += stepLenCm;
      if (recordSpectrum) {
        const auto binIdx = EnergyBin(energyMeV);
        if (binIdx < tls_data->probeGammaSpectrumCounts[globalProbeIdx].size()) {
          tls_data->probeGammaSpectrumCounts[globalProbeIdx][binIdx] += stepLenCm;
        }
      }
    } else if (isNeutron) {
      const G4double df =
          InterpolateDoseFactor(energyMeV, fNeutronDeMeV, fNeutronDfSvPerHourPerFlux);
      tls_data->probeNeutronDoseWeightedTrackLenCm[globalProbeIdx] += stepLenCm * df;
      tls_data->probeC9NeutronGeomTrackLenCm[globalProbeIdx] += stepLenCm;
      if (recordSpectrum) {
        const auto binIdx = EnergyBin(energyMeV);
        if (binIdx < tls_data->probeNeutronSpectrumCounts[globalProbeIdx].size()) {
          tls_data->probeNeutronSpectrumCounts[globalProbeIdx][binIdx] += stepLenCm;
        }
      }
    }
  };

  auto clampIndex = [](G4int& v, G4int vmax) {
    if (v < 0) {
      v = 0;
    }
    if (v >= vmax) {
      v = vmax - 1;
    }
  };

  auto scoreOuterEnvelope = [&](std::size_t firstProbeIdx,
                                std::size_t probeCount,
                                const std::array<std::size_t, 6>& faceStart,
                                G4double sx,
                                G4double ex,
                                G4double sy,
                                G4double ey,
                                G4double sz,
                                G4double ez,
                                G4bool recordSpectrumMinusZ) {
    if (probeCount == 0 || firstProbeIdx >= fProbes.size()) {
      return;
    }
    const G4int nx = static_cast<G4int>(std::ceil((ex - sx) / pitch));
    const G4int ny = static_cast<G4int>(std::ceil((ey - sy) / pitch));
    const G4int nz = static_cast<G4int>(std::ceil((ez - sz) / pitch));

    // Face +X
    {
      const G4double xPlane = ex + dout;
      if (x >= xPlane - halfSlab && x <= xPlane + halfSlab && y >= sy && y <= ey && z >= sz && z <= ez) {
        G4int iy = static_cast<G4int>(std::floor((y - sy) / pitch));
        G4int iz = static_cast<G4int>(std::floor((z - sz) / pitch));
        clampIndex(iy, ny);
        clampIndex(iz, nz);
        const std::size_t loc =
            static_cast<std::size_t>(iy) * static_cast<std::size_t>(nz) + static_cast<std::size_t>(iz);
        scoreIndex(firstProbeIdx + faceStart[0] + loc, true);
      }
    }
    // -X
    {
      const G4double xPlane = sx - dout;
      if (x >= xPlane - halfSlab && x <= xPlane + halfSlab && y >= sy && y <= ey && z >= sz && z <= ez) {
        G4int iy = static_cast<G4int>(std::floor((y - sy) / pitch));
        G4int iz = static_cast<G4int>(std::floor((z - sz) / pitch));
        clampIndex(iy, ny);
        clampIndex(iz, nz);
        const std::size_t loc =
            static_cast<std::size_t>(iy) * static_cast<std::size_t>(nz) + static_cast<std::size_t>(iz);
        scoreIndex(firstProbeIdx + faceStart[1] + loc, true);
      }
    }
    // +Y
    {
      const G4double yPlane = ey + dout;
      if (y >= yPlane - halfSlab && y <= yPlane + halfSlab && x >= sx && x <= ex && z >= sz && z <= ez) {
        G4int ix = static_cast<G4int>(std::floor((x - sx) / pitch));
        G4int iz = static_cast<G4int>(std::floor((z - sz) / pitch));
        clampIndex(ix, nx);
        clampIndex(iz, nz);
        const std::size_t loc =
            static_cast<std::size_t>(ix) * static_cast<std::size_t>(nz) + static_cast<std::size_t>(iz);
        scoreIndex(firstProbeIdx + faceStart[2] + loc, true);
      }
    }
    // -Y
    {
      const G4double yPlane = sy - dout;
      if (y >= yPlane - halfSlab && y <= yPlane + halfSlab && x >= sx && x <= ex && z >= sz && z <= ez) {
        G4int ix = static_cast<G4int>(std::floor((x - sx) / pitch));
        G4int iz = static_cast<G4int>(std::floor((z - sz) / pitch));
        clampIndex(ix, nx);
        clampIndex(iz, nz);
        const std::size_t loc =
            static_cast<std::size_t>(ix) * static_cast<std::size_t>(nz) + static_cast<std::size_t>(iz);
        scoreIndex(firstProbeIdx + faceStart[3] + loc, true);
      }
    }
    // +Z
    {
      const G4double zPlane = ez + dout;
      if (z >= zPlane - halfSlab && z <= zPlane + halfSlab && x >= sx && x <= ex && y >= sy && y <= ey) {
        G4int ix = static_cast<G4int>(std::floor((x - sx) / pitch));
        G4int iy = static_cast<G4int>(std::floor((y - sy) / pitch));
        clampIndex(ix, nx);
        clampIndex(iy, ny);
        const std::size_t loc =
            static_cast<std::size_t>(ix) * static_cast<std::size_t>(ny) + static_cast<std::size_t>(iy);
        scoreIndex(firstProbeIdx + faceStart[4] + loc, true);
      }
    }
    // -Z
    {
      const G4double zPlane = sz - dout;
      if (z >= zPlane - halfSlab && z <= zPlane + halfSlab && x >= sx && x <= ex && y >= sy && y <= ey) {
        G4int ix = static_cast<G4int>(std::floor((x - sx) / pitch));
        G4int iy = static_cast<G4int>(std::floor((y - sy) / pitch));
        clampIndex(ix, nx);
        clampIndex(iy, ny);
        const std::size_t loc =
            static_cast<std::size_t>(ix) * static_cast<std::size_t>(ny) + static_cast<std::size_t>(iy);
        scoreIndex(firstProbeIdx + faceStart[5] + loc, recordSpectrumMinusZ);
      }
    }
  };

  scoreOuterEnvelope(fFirstC9ProbeIndex, g_c9ProbeCount, g_c9FaceStart, -222.7 * cm, 222.7 * cm,
                     -222.7 * cm, 222.7 * cm, 0.0 * cm, 340.5 * cm, false);
  scoreOuterEnvelope(fFirstC20ProbeIndex, g_c20ProbeCount, g_c20FaceStart, -60.0 * cm, 10.0 * cm,
                     -300.0 * cm, -222.7 * cm, 0.0 * cm, 105.0 * cm, true);
  scoreOuterEnvelope(fFirstC22ProbeIndex, g_c22ProbeCount, g_c22FaceStart, -60.0 * cm, 10.0 * cm,
                     -315.0 * cm, -222.7 * cm, 0.0 * cm, 115.0 * cm, true);
}

void ShieldingDataCollector::ScoreTrackBirth(const G4Track* track) {
  if (track->GetTrackID() == 1) {
    return;
  }
  const auto* particle = track->GetDefinition();
  if (!G4IonTable::IsIon(particle)) {
    return;
  }

  const auto* volume = track->GetTouchableHandle()->GetVolume();
  if (volume == nullptr) {
    return;
  }

  // Initialize thread-local storage on first use
  if (!tls_data) {
    tls_data = new ThreadLocalData(this);
  }

  const G4String region = volume->GetName();
  const G4String nuclide = particle->GetParticleName();
  const G4double weight = track->GetWeight();
  const G4double meanLifeS = particle->GetPDGLifeTime() / s;

  // Update thread-local nuclide yield (no lock needed)
  auto& entry = tls_data->nuclideYieldsByRegion[region][nuclide];
  entry.producedPerPrimary += weight;
  if (entry.meanLifeS <= 0.0) {
    entry.meanLifeS = meanLifeS;
  }
}

std::size_t ShieldingDataCollector::MeshIndex(G4double xMm, G4double yMm, G4double zMm) const {
  if (xMm < fMeshOriginMm[0] || xMm >= fMeshMaxMm[0] || yMm < fMeshOriginMm[1] ||
      yMm >= fMeshMaxMm[1] || zMm < fMeshOriginMm[2] || zMm >= fMeshMaxMm[2]) {
    return static_cast<std::size_t>(-1);
  }

  const G4double fx = (xMm - fMeshOriginMm[0]) / (fMeshMaxMm[0] - fMeshOriginMm[0]);
  const G4double fy = (yMm - fMeshOriginMm[1]) / (fMeshMaxMm[1] - fMeshOriginMm[1]);
  const G4double fz = (zMm - fMeshOriginMm[2]) / (fMeshMaxMm[2] - fMeshOriginMm[2]);

  const auto ix = static_cast<std::size_t>(std::min(static_cast<G4int>(fx * fMeshBins[0]),
                                                    fMeshBins[0] - 1));
  const auto iy = static_cast<std::size_t>(std::min(static_cast<G4int>(fy * fMeshBins[1]),
                                                    fMeshBins[1] - 1));
  const auto iz = static_cast<std::size_t>(std::min(static_cast<G4int>(fz * fMeshBins[2]),
                                                    fMeshBins[2] - 1));
  return (iz * static_cast<std::size_t>(fMeshBins[1]) + iy) * static_cast<std::size_t>(fMeshBins[0]) +
         ix;
}

G4double ShieldingDataCollector::InterpolateDoseFactor(
    G4double energyMeV, const std::vector<G4double>& de, const std::vector<G4double>& df) const {
  if (energyMeV <= de.front()) {
    return df.front();
  }
  if (energyMeV >= de.back()) {
    return df.back();
  }

  const auto upper = std::lower_bound(de.begin(), de.end(), energyMeV);
  const auto idx = static_cast<std::size_t>(upper - de.begin());
  const G4double e1 = de[idx - 1];
  const G4double e2 = de[idx];
  const G4double f1 = df[idx - 1];
  const G4double f2 = df[idx];
  const G4double t = (energyMeV - e1) / (e2 - e1 + kTiny);
  return f1 + t * (f2 - f1);
}

void ShieldingDataCollector::FillSpectrum(std::vector<G4double>& bins,
                                          G4double energyMeV,
                                          G4double weight) {
  const auto idx = EnergyBin(energyMeV);
  if (idx < bins.size()) {
    bins[idx] += weight;
  }
}

std::size_t ShieldingDataCollector::EnergyBin(G4double energyMeV) const {
  const auto upper = std::upper_bound(fSpectrumEdgesMeV.begin(), fSpectrumEdgesMeV.end(), energyMeV);
  if (upper == fSpectrumEdgesMeV.begin()) {
    return 0;
  }
  const auto idx = static_cast<std::size_t>(upper - fSpectrumEdgesMeV.begin() - 1);
  return std::min(idx, fSpectrumEdgesMeV.size() - 2);
}

void ShieldingDataCollector::WritePromptDoseOutputs() const {
  const G4double invEvents = 1.0 / static_cast<G4double>(fNumberOfEvents);
  const G4double scaleDose = fSourceIntensityPerSecond * invEvents / (fVoxelVolumeCm3 + kTiny);
  const G4double scaleFlux = invEvents / (fVoxelVolumeCm3 + kTiny);

  // 输出目录
  std::string outputDir = OUTPUT_DIR;

  // 即时剂量网格 CSV
  std::ofstream mesh((outputDir + "即时剂量网格.csv").c_str());
  mesh << "网格X,网格Y,网格Z,伽马剂量率_Sv每小时,中子剂量率_Sv每小时,总剂量率_Sv每小时,"
          "伽马通量_每平方厘米每秒,中子通量_每平方厘米每秒\n";

  for (G4int iz = 0; iz < fMeshBins[2]; ++iz) {
    for (G4int iy = 0; iy < fMeshBins[1]; ++iy) {
      for (G4int ix = 0; ix < fMeshBins[0]; ++ix) {
        const std::size_t idx =
            (static_cast<std::size_t>(iz) * fMeshBins[1] + iy) * fMeshBins[0] + ix;
        const G4double gd = fGammaDoseWeightedTrackLenCm[idx] * scaleDose;
        const G4double nd = fNeutronDoseWeightedTrackLenCm[idx] * scaleDose;
        const G4double gf = fGammaTrackLenCm[idx] * scaleFlux;
        const G4double nf = fNeutronTrackLenCm[idx] * scaleFlux;
        if (gd <= 0.0 && nd <= 0.0 && gf <= 0.0 && nf <= 0.0) {
          continue;
        }
        mesh << ix << "," << iy << "," << iz << "," << gd << "," << nd << "," << (gd + nd) << ","
             << gf << "," << nf << "\n";
      }
    }
  }

  // 探测器点剂量 CSV（不含 c9/c20/c22 外包络面网格；详见对应「外表面探测器通量与剂量」）
  std::ofstream points((outputDir + "探测器点剂量.csv").c_str());
  points << "探测器名称,伽马剂量率_Sv每小时,中子剂量率_Sv每小时,总剂量率_Sv每小时\n";
  for (const auto& probe : fProbes) {
    if (IsOuterEnvelopeMeshName(probe.name)) {
      continue;
    }
    const G4double volumeCm3 = ComputeProbeScoringVolumeCm3(probe);
    const G4double localScale = fSourceIntensityPerSecond * invEvents / (volumeCm3 + kTiny);
    const G4double gd = probe.gammaDoseWeightedTrackLenCm * localScale;
    const G4double nd = probe.neutronDoseWeightedTrackLenCm * localScale;
    points << probe.name << "," << gd << "," << nd << "," << (gd + nd) << "\n";
  }

  // c9 / c20 / c22 外包络外法向 1 cm、约 1 dm² 薄层：通量与剂量（径迹长度估计）
  const std::pair<const char*, const char*> kOuterFluxFiles[] = {
      {"c9_outer_", "c9外表面探测器通量与剂量.csv"},
      {"c20_outer_", "c20外表面探测器通量与剂量.csv"},
      {"c22_outer_", "c22外表面探测器通量与剂量.csv"},
  };
  for (const auto& ent : kOuterFluxFiles) {
    std::ofstream surf((outputDir + std::string(ent.second)).c_str());
    surf << "探测器名称,中心X_mm,中心Y_mm,中心Z_mm,计分体积_cm3,伽马剂量率_Sv每小时,中子剂量率_Sv每小时,"
            "总剂量率_Sv每小时,伽马通量_每平方厘米每秒,中子通量_每平方厘米每秒\n";
    for (const auto& probe : fProbes) {
      if (probe.name.find(ent.first) != 0) {
        continue;
      }
      const G4double volumeCm3 = ComputeProbeScoringVolumeCm3(probe);
      const G4double localScaleDose = fSourceIntensityPerSecond * invEvents / (volumeCm3 + kTiny);
      const G4double localScaleFlux = fSourceIntensityPerSecond * invEvents / (volumeCm3 + kTiny);
      const G4double gd = probe.gammaDoseWeightedTrackLenCm * localScaleDose;
      const G4double nd = probe.neutronDoseWeightedTrackLenCm * localScaleDose;
      const G4double gf = probe.c9GammaGeomTrackLenCm * localScaleFlux;
      const G4double nf = probe.c9NeutronGeomTrackLenCm * localScaleFlux;
      surf << probe.name << "," << probe.positionMm[0] << "," << probe.positionMm[1] << ","
           << probe.positionMm[2] << "," << volumeCm3 << "," << gd << "," << nd << "," << (gd + nd) << ","
           << gf << "," << nf << "\n";
    }
  }

  // 探测器能谱 CSV（不含外包络网格；详见各「外表面探测器能谱」）
  std::ofstream spectra((outputDir + "探测器能谱.csv").c_str());
  spectra << "探测器名称,粒子类型,能量下限_MeV,能量上限_MeV,径迹长度_厘米每初级粒子\n";
  for (const auto& probe : fProbes) {
    if (IsOuterEnvelopeMeshName(probe.name)) {
      continue;
    }
    for (std::size_t i = 0; i < probe.gammaSpectrumCounts.size(); ++i) {
      spectra << probe.name << ",伽马," << fSpectrumEdgesMeV[i] << "," << fSpectrumEdgesMeV[i + 1]
              << "," << probe.gammaSpectrumCounts[i] * invEvents << "\n";
    }
    for (std::size_t i = 0; i < probe.neutronSpectrumCounts.size(); ++i) {
      spectra << probe.name << ",中子," << fSpectrumEdgesMeV[i] << "," << fSpectrumEdgesMeV[i + 1]
              << "," << probe.neutronSpectrumCounts[i] * invEvents << "\n";
    }
  }

  struct OuterSpecFiles {
    const char* prefix;
    const char* gammaCsv;
    const char* neutronCsv;
    G4bool skipMinusZSpectrum;
  };
  const OuterSpecFiles kOuterSpec[] = {
      {"c9_outer_", "c9外表面探测器能谱_伽马.csv", "c9外表面探测器能谱_中子.csv", true},
      {"c20_outer_", "c20外表面探测器能谱_伽马.csv", "c20外表面探测器能谱_中子.csv", false},
      {"c22_outer_", "c22外表面探测器能谱_伽马.csv", "c22外表面探测器能谱_中子.csv", false},
  };
  for (const auto& os : kOuterSpec) {
    std::ofstream specG((outputDir + std::string(os.gammaCsv)).c_str());
    specG << "探测器名称,能量下限_MeV,能量上限_MeV,径迹长度_厘米每初级粒子\n";
    std::ofstream specN((outputDir + std::string(os.neutronCsv)).c_str());
    specN << "探测器名称,能量下限_MeV,能量上限_MeV,径迹长度_厘米每初级粒子\n";
    for (const auto& probe : fProbes) {
      if (probe.name.find(os.prefix) != 0) {
        continue;
      }
      if (os.skipMinusZSpectrum && probe.name.find("c9_outer_mz_") == 0) {
        continue;
      }
      for (std::size_t i = 0; i < probe.gammaSpectrumCounts.size(); ++i) {
        specG << probe.name << "," << fSpectrumEdgesMeV[i] << "," << fSpectrumEdgesMeV[i + 1]
              << "," << probe.gammaSpectrumCounts[i] * invEvents << "\n";
      }
      for (std::size_t i = 0; i < probe.neutronSpectrumCounts.size(); ++i) {
        specN << probe.name << "," << fSpectrumEdgesMeV[i] << "," << fSpectrumEdgesMeV[i + 1]
              << "," << probe.neutronSpectrumCounts[i] * invEvents << "\n";
      }
    }
  }
}

void ShieldingDataCollector::WriteThermalOutputs() const {
  const G4double irradiationSeconds = fIrradiationHours * 3600.0;
  const G4double invEvents = 1.0 / static_cast<G4double>(fNumberOfEvents);
  const G4double eventRate = fSourceIntensityPerSecond;

  std::string outputDir = OUTPUT_DIR;

  std::ofstream thermal((outputDir + "热负载.csv").c_str());
  thermal << "区域名称,每初级粒子能量沉积_焦耳,功率_瓦,1000小时总能量_焦耳\n";
  for (const auto& kv : fEnergyDepositionByRegionJ) {
    const G4double ePerPrimary = kv.second * invEvents;
    const G4double powerW = ePerPrimary * eventRate;
    const G4double energyJ = powerW * irradiationSeconds;
    thermal << kv.first << "," << ePerPrimary << "," << powerW << "," << energyJ << "\n";
  }
}

void ShieldingDataCollector::WriteNuclideOutputs() const {
  const G4double invEvents = 1.0 / static_cast<G4double>(fNumberOfEvents);
  const G4double beamRate = fSourceIntensityPerSecond;
  const G4double tIrr = fIrradiationHours * 3600.0;
  const std::array<G4double, 5> coolingTimes = {0.0, 24.0 * 3600.0, 30.0 * 24.0 * 3600.0,
                                                 365.0 * 24.0 * 3600.0,
                                                 5.0 * 365.0 * 24.0 * 3600.0};

  std::string outputDir = OUTPUT_DIR;

  std::ofstream nuclides((outputDir + "核素库存.csv").c_str());
  nuclides << "区域名称,废物类别,核素名称,每初级粒子产额,区域质量_kg,"
              "照射结束时比活度_Bq_per_kg,冷却1天比活度_Bq_per_kg,"
              "冷却30天比活度_Bq_per_kg,冷却1年比活度_Bq_per_kg,冷却5年比活度_Bq_per_kg,"
              "照射结束时活度_Bq,冷却1天活度_Bq,冷却30天活度_Bq,冷却1年活度_Bq,冷却5年活度_Bq\n";

  for (const auto& regionEntry : fNuclideYieldsByRegion) {
    const G4String regionName = regionEntry.first;
    const G4String category = CategoryFromVolume(regionName);
    std::string categoryCN;
    if (category == "waste_liquid") categoryCN = "液体废物";
    else if (category == "waste_gas") categoryCN = "气体废物";
    else if (category == "waste_solid") categoryCN = "固体废物";
    else categoryCN = "未分类";

    // 获取区域质量，如果未找到则默认为1.0 kg避免除零
    G4double regionMassKg = 1.0;
    auto massIt = fRegionMassKg.find(regionName);
    if (massIt != fRegionMassKg.end()) {
      regionMassKg = massIt->second;
    }
    // 防止除零，最小质量设为1e-6 kg (1 mg)
    if (regionMassKg < 1.0e-6) regionMassKg = 1.0e-6;

    for (const auto& nucEntry : regionEntry.second) {
      const G4double producedPerPrimary = nucEntry.second.producedPerPrimary * invEvents;
      const G4double productionRate = producedPerPrimary * beamRate;
      const G4double meanLife = nucEntry.second.meanLifeS;
      G4double lambda = 0.0;
      if (meanLife > 0.0 && meanLife < 1.0e30) {
        lambda = 1.0 / meanLife;
      }

      // 计算活度 (Bq)
      std::array<G4double, 5> activity {};
      if (lambda > 0.0) {
        const G4double nEob = productionRate * (1.0 - std::exp(-lambda * tIrr)) / lambda;
        for (std::size_t i = 0; i < coolingTimes.size(); ++i) {
          activity[i] = lambda * nEob * std::exp(-lambda * coolingTimes[i]);
        }
      } else {
        activity[0] = 0.0;
        activity[1] = 0.0;
        activity[2] = 0.0;
        activity[3] = 0.0;
        activity[4] = 0.0;
      }

      // 转换为比活度 (Bq/kg)
      std::array<G4double, 5> specificActivity {};
      for (std::size_t i = 0; i < 5; ++i) {
        specificActivity[i] = activity[i] / regionMassKg;
      }

      nuclides << regionName << "," << categoryCN << "," << nucEntry.first << ","
               << producedPerPrimary << "," << regionMassKg << ","
               << specificActivity[0] << "," << specificActivity[1] << ","
               << specificActivity[2] << "," << specificActivity[3] << ","
               << specificActivity[4] << ","
               << activity[0] << "," << activity[1] << ","
               << activity[2] << "," << activity[3] << ","
               << activity[4] << "\n";
    }
  }
}

G4double ShieldingDataCollector::GetRegionMass(const G4String& regionName) const {
  auto it = fRegionMassKg.find(regionName);
  if (it != fRegionMassKg.end()) {
    return it->second;
  }
  return 0.0;
}

void ShieldingDataCollector::WriteSummaryReport() const {
  std::string outputDir = OUTPUT_DIR;

  std::ofstream report((outputDir + "屏蔽计算摘要报告.md").c_str());
  report << "# Geant4 屏蔽计算运行摘要\n\n";
  report << "- 源强 (每秒): " << fSourceIntensityPerSecond << "\n";
  report << "- 照射时间 (小时): " << fIrradiationHours << "\n";
  report << "- 门状态: " << (fDoorOpen ? "开启" : "关闭") << "\n";
  report << "- 模拟事件数: " << fNumberOfEvents << "\n\n";
  report << "## 生成文件列表\n\n";
  report << "- `即时剂量网格.csv`\n";
  report << "- `探测器点剂量.csv`\n";
  report << "- `探测器能谱.csv`\n";
  report << "- `c9外表面探测器通量与剂量.csv`\n";
  report << "- `c20外表面探测器通量与剂量.csv`\n";
  report << "- `c22外表面探测器通量与剂量.csv`\n";
  report << "- `c9外表面探测器能谱_伽马.csv`\n";
  report << "- `c9外表面探测器能谱_中子.csv`\n";
  report << "- `c20外表面探测器能谱_伽马.csv`\n";
  report << "- `c20外表面探测器能谱_中子.csv`\n";
  report << "- `c22外表面探测器能谱_伽马.csv`\n";
  report << "- `c22外表面探测器能谱_中子.csv`\n";
  report << "- `热负载.csv`\n";
  report << "- `核素库存.csv`\n";
  report << "- `需求参数输出.txt`\n";
  report << "\n本运行结果可使用 Python 脚本 `postprocess_results.py` 进行后处理分析。\n";
}

void ShieldingDataCollector::WriteRequirementParameterReport() const {
  struct ProbeDose {
    G4String name;
    G4double gammaSvPerH {};
    G4double neutronSvPerH {};
    G4double totalSvPerH {};
  };

  const G4double invEvents = 1.0 / static_cast<G4double>(fNumberOfEvents);
  std::vector<ProbeDose> probeDoses;
  probeDoses.reserve(fProbes.size());
  for (const auto& probe : fProbes) {
    const G4double volumeCm3 = ComputeProbeScoringVolumeCm3(probe);
    const G4double localScale = fSourceIntensityPerSecond * invEvents / (volumeCm3 + kTiny);
    const G4double gd = probe.gammaDoseWeightedTrackLenCm * localScale;
    const G4double nd = probe.neutronDoseWeightedTrackLenCm * localScale;
    ProbeDose entry;
    entry.name = probe.name;
    entry.gammaSvPerH = gd;
    entry.neutronSvPerH = nd;
    entry.totalSvPerH = gd + nd;
    probeDoses.push_back(entry);
  }

  G4double totalPowerW = 0.0;
  for (const auto& kv : fEnergyDepositionByRegionJ) {
    totalPowerW += kv.second * invEvents * fSourceIntensityPerSecond;
  }

  struct NuclideStat {
    G4String name;
    G4double activity1yBq {};
  };
  std::map<G4String, NuclideStat> topByCategory;
  const G4double beamRate = fSourceIntensityPerSecond;
  const G4double tIrr = fIrradiationHours * 3600.0;
  const G4double cool1y = 365.0 * 24.0 * 3600.0;
  for (const auto& regionEntry : fNuclideYieldsByRegion) {
    const G4String category = CategoryFromVolume(regionEntry.first);
    for (const auto& nucEntry : regionEntry.second) {
      const G4double producedPerPrimary = nucEntry.second.producedPerPrimary * invEvents;
      const G4double productionRate = producedPerPrimary * beamRate;
      const G4double meanLife = nucEntry.second.meanLifeS;
      if (!(meanLife > 0.0 && meanLife < 1.0e30)) {
        continue;
      }
      const G4double lambda = 1.0 / meanLife;
      const G4double nEob = productionRate * (1.0 - std::exp(-lambda * tIrr)) / lambda;
      const G4double activity1y = lambda * nEob * std::exp(-lambda * cool1y);
      auto it = topByCategory.find(category);
      if (it == topByCategory.end() || activity1y > it->second.activity1yBq) {
        NuclideStat stat;
        stat.name = nucEntry.first;
        stat.activity1yBq = activity1y;
        topByCategory[category] = stat;
      }
    }
  }

  auto findProbe = [&](const G4String& name) -> const ProbeDose* {
    for (const auto& p : probeDoses) {
      if (p.name == name) {
        return &p;
      }
    }
    return nullptr;
  };

  const ProbeDose* shieldOuter = findProbe("shield_outer_30cm");
  const ProbeDose* hotcellOuter = findProbe("hotcell_outer_30cm");
  const ProbeDose* doorPoint = findProbe("door_operation_point");

  std::string outputDir = OUTPUT_DIR;

  std::ofstream out((outputDir + "需求参数输出.txt").c_str());
  out << "2026 屏蔽计算需求参数输出\n";
  out << "运行事件数=" << fNumberOfEvents << "\n";
  out << "源强(每秒)=" << fSourceIntensityPerSecond << "\n";
  out << "照射时间(小时)=" << fIrradiationHours << "\n";
  out << "门状态=" << (fDoorOpen ? "开启" : "关闭") << "\n\n";

  out << "[1] 即时外照射剂量率 (Sv/h)\n";
  out << "探测器名称,伽马剂量率,中子剂量率,总剂量率\n";
  for (const auto& p : probeDoses) {
    if (IsOuterEnvelopeMeshName(p.name)) {
      continue;
    }
    out << p.name << "," << p.gammaSvPerH << "," << p.neutronSvPerH << "," << p.totalSvPerH << "\n";
  }
  out << "说明=c9/c20/c22 外包络外1cm约1dm2薄层网格：c9共 " << g_c9ProbeCount << " 个、c20共 "
      << g_c20ProbeCount << " 个、c22共 " << g_c22ProbeCount
      << " 个探测器；通量/剂量见 c9/c20/c22外表面探测器通量与剂量.csv，能谱见对应外表面能谱 CSV。\n";
  if (shieldOuter != nullptr) {
    out << "屏蔽体外100mSv每小时限值通过=" << (shieldOuter->totalSvPerH < 0.1 ? "是" : "否") << "\n";
  }
  if (hotcellOuter != nullptr) {
    out << "热室外2.5uSv每小时限值通过="
        << (hotcellOuter->totalSvPerH < 2.5e-6 ? "是" : "否") << "\n";
  }
  out << "\n";

  out << "[2] 1000小时照射后废靶剂量\n";
  out << "状态=需要独立的废靶残余场景计算（当前几何中废靶未隔离）\n\n";

  out << "[3] 带废靶的屏蔽外剂量\n";
  out << "状态=需要残余场景+屏蔽活化耦合计算\n\n";

  out << "[4] 开门状态下带废靶的剂量\n";
  if (doorPoint != nullptr) {
    out << "门操作点总剂量率_Sv每小时=" << doorPoint->totalSvPerH << "\n";
  }
  out << "状态=使用 /shield/doorOpen true 运行以获取开门条件\n\n";

  out << "[5] 各层不同冷却时间的活化比活度\n";
  out << "状态=核素比活度已导出至 核素库存.csv（单位: Bq/kg, EOB=照射结束,1d=1天,30d=30天,1y=1年,5y=5年）\n\n";

  out << "[6] 主冷却水源项及剂量\n";
  auto waterIt = topByCategory.find("waste_liquid");
  if (waterIt != topByCategory.end()) {
    out << "1年后主要水核素=" << waterIt->second.name << ",活度(Bq)=" << waterIt->second.activity1yBq
        << "\n";
  } else {
    out << "1年后主要水核素=无\n";
  }
  out << "\n";

  out << "[6.1] CenterTube 水区即时剂量\n";
  const ProbeDose* centerTubeWater = findProbe("center_tube_water");
  const ProbeDose* centerTubeEdge = findProbe("center_tube_edge");
  if (centerTubeWater != nullptr) {
    out << "CenterTube水区中心总剂量率_Sv每小时=" << centerTubeWater->totalSvPerH << "\n";
    out << "CenterTube水区中心伽马剂量率_Sv每小时=" << centerTubeWater->gammaSvPerH << "\n";
    out << "CenterTube水区中心中子剂量率_Sv每小时=" << centerTubeWater->neutronSvPerH << "\n";
  }
  if (centerTubeEdge != nullptr) {
    out << "CenterTube边缘(近源侧)总剂量率_Sv每小时=" << centerTubeEdge->totalSvPerH << "\n";
  }
  out << "\n";

  out << "[6.2] c4 铁壳外表面即时剂量\n";
  const ProbeDose* c4Side = findProbe("c4_fe_outer_side");
  const ProbeDose* c4Top = findProbe("c4_fe_outer_top");
  if (c4Side != nullptr) {
    out << "c4铁壳外侧面总剂量率_Sv每小时=" << c4Side->totalSvPerH << "\n";
    out << "c4铁壳外侧面伽马剂量率_Sv每小时=" << c4Side->gammaSvPerH << "\n";
    out << "c4铁壳外侧面中子剂量率_Sv每小时=" << c4Side->neutronSvPerH << "\n";
  }
  if (c4Top != nullptr) {
    out << "c4铁壳顶部总剂量率_Sv每小时=" << c4Top->totalSvPerH << "\n";
  }
  out << "\n";

  out << "[6.3] c4 铁壳内表面即时剂量（靠近水区）\n";
  const ProbeDose* c4InnerSide = findProbe("c4_fe_inner_side");
  const ProbeDose* c4InnerTop = findProbe("c4_fe_inner_top");
  const ProbeDose* c4InnerCorner = findProbe("c4_fe_inner_corner");
  if (c4InnerSide != nullptr) {
    out << "c4铁壳内侧面(X=65.2cm)总剂量率_Sv每小时=" << c4InnerSide->totalSvPerH << "\n";
    out << "c4铁壳内侧面(X=65.2cm)伽马剂量率_Sv每小时=" << c4InnerSide->gammaSvPerH << "\n";
    out << "c4铁壳内侧面(X=65.2cm)中子剂量率_Sv每小时=" << c4InnerSide->neutronSvPerH << "\n";
  }
  if (c4InnerTop != nullptr) {
    out << "c4铁壳内顶部(Z=183cm)总剂量率_Sv每小时=" << c4InnerTop->totalSvPerH << "\n";
    out << "c4铁壳内顶部(Z=183cm)伽马剂量率_Sv每小时=" << c4InnerTop->gammaSvPerH << "\n";
    out << "c4铁壳内顶部(Z=183cm)中子剂量率_Sv每小时=" << c4InnerTop->neutronSvPerH << "\n";
  }
  if (c4InnerCorner != nullptr) {
    out << "c4铁壳内角总剂量率_Sv每小时=" << c4InnerCorner->totalSvPerH << "\n";
  }
  out << "\n";

  out << "[7] 屏蔽体内辐射热功率\n";
  out << "总功率(瓦)=" << totalPowerW << "\n\n";

  out << "[8] 三种废物核素源项（1年后最高活度）\n";
  auto solidIt = topByCategory.find("waste_solid");
  auto gasIt = topByCategory.find("waste_gas");
  out << "1年后主要固体废物核素="
      << (solidIt != topByCategory.end() ? solidIt->second.name : G4String("无")) << ",活度(Bq)="
      << (solidIt != topByCategory.end() ? solidIt->second.activity1yBq : 0.0) << "\n";
  out << "1年后主要气体废物核素=" << (gasIt != topByCategory.end() ? gasIt->second.name : G4String("无"))
      << ",活度(Bq)=" << (gasIt != topByCategory.end() ? gasIt->second.activity1yBq : 0.0) << "\n";
  out << "1年后主要液体废物核素=" << (waterIt != topByCategory.end() ? waterIt->second.name : G4String("无"))
      << ",活度(Bq)=" << (waterIt != topByCategory.end() ? waterIt->second.activity1yBq : 0.0) << "\n\n";

  out << "[9] 热室源项（用于墙体设计）\n";
  if (hotcellOuter != nullptr) {
    out << "热室外30cm总剂量率_Sv每小时=" << hotcellOuter->totalSvPerH << "\n";
  }
  out << "状态=结合 探测器能谱.csv 中的能谱进行民用屏蔽设计\n\n";

  out << "[10] 热室墙体厚度建议\n";
  if (hotcellOuter != nullptr) {
    if (hotcellOuter->totalSvPerH <= 2.5e-6) {
      out << "建议=当前墙体厚度满足2.5uSv/h目标\n";
    } else {
      out << "建议=当前墙体厚度不满足目标; 请运行墙体扫描计算\n";
    }
  } else {
    out << "建议=需要热室外30cm探测器结果\n";
  }
}

G4String ShieldingDataCollector::CategoryFromVolume(const G4String& volumeName) const {
  G4String lower = volumeName;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

  if (lower.find("water") != std::string::npos || lower.find("coolant") != std::string::npos) {
    return "waste_liquid";
  }
  if (lower.find("air") != std::string::npos || lower.find("channel") != std::string::npos) {
    return "waste_gas";
  }
  if (lower.find("target") != std::string::npos || lower.find("filter") != std::string::npos ||
      lower.find("shield") != std::string::npos || lower.find("door") != std::string::npos) {
    return "waste_solid";
  }
  return "uncategorized";
}

void ShieldingDataCollector::Reset() {
  std::fill(fGammaDoseWeightedTrackLenCm.begin(), fGammaDoseWeightedTrackLenCm.end(), 0.0);
  std::fill(fNeutronDoseWeightedTrackLenCm.begin(), fNeutronDoseWeightedTrackLenCm.end(), 0.0);
  std::fill(fGammaTrackLenCm.begin(), fGammaTrackLenCm.end(), 0.0);
  std::fill(fNeutronTrackLenCm.begin(), fNeutronTrackLenCm.end(), 0.0);

  for (auto& probe : fProbes) {
    probe.gammaDoseWeightedTrackLenCm = 0.0;
    probe.neutronDoseWeightedTrackLenCm = 0.0;
    probe.c9GammaGeomTrackLenCm = 0.0;
    probe.c9NeutronGeomTrackLenCm = 0.0;
    std::fill(probe.gammaSpectrumCounts.begin(), probe.gammaSpectrumCounts.end(), 0.0);
    std::fill(probe.neutronSpectrumCounts.begin(), probe.neutronSpectrumCounts.end(), 0.0);
  }

  fEnergyDepositionByRegionJ.clear();
  fNuclideYieldsByRegion.clear();
  fRegionMassKg.clear();
}
