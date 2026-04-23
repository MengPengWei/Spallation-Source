// include/gun/ProtonGun.hh
#ifndef PROTONGUN_HH
#define PROTONGUN_HH

#include "G4ParticleGun.hh"
#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"

class ProtonGun
{
public:
    explicit ProtonGun(G4ParticleGun* particleGun);
    virtual ~ProtonGun() = default;

    // 1) 一次性配置：粒子类型、能量、方向、初始Z、束斑中心等
    void Configure();

    // 2) 每事件：采样束斑横向位置 + 生成 primary vertex
    void GeneratePrimaries(G4Event* event);
    G4double GetCenterX() const { return C_PG_Center_X; }
    G4double GetCenterY() const { return C_PG_Center_Y; }
    G4double GetCenterZ() const { return C_PG_Position_Z; }
    G4double GetMaxRel() const { return C_PG_HalfSide; }
    G4double GetEnergy() const { return C_PG_Energy; }

private:
    // 采样二维高斯（FWHM -> sigma），并在方形 halfSide 内截断
    void SampleTransverseOffset(G4double& x_rel, G4double& y_rel) const;

private:
    G4ParticleGun* fParticleGun;

    // ---- 常量（放在类内，建议 static constexpr）----
    static constexpr G4double C_PG_Energy     = 70.0 * MeV;

    static constexpr G4double C_PG_Position_Z = -10.0 * cm;
    static constexpr G4double C_PG_Center_X   = 0.0 * cm;
    static constexpr G4double C_PG_Center_Y   = -21.5 * cm;

    static constexpr G4double C_PG_HalfSide   = 2.5 * cm;       // 方形半边长
    static constexpr G4double C_PG_FWHM       = 7.33448 * mm;   // FWHM

    // 注意：G4ThreeVector 不能作为 static constexpr（非字面量），用函数返回即可
    static G4ThreeVector Direction() { return G4ThreeVector(0, 0, 1); }
};

#endif // PROTONGUN_HH