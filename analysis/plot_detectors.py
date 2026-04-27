#!/usr/bin/env python3
"""
plot_detectors.py
=================
Python + PyROOT 分析脚本：从 activation_output.root 读取 source 探测器数据，
生成并保存三类图像：
  1. proton_3D_heatmap.png  – 质子探测器内质子 3D 热力分布（三视图 xy/xz/yz 投影）
  2. neutron_yield_2D.png   – 中子探测器内中子产额热力图（x-y）
  3. neutron_spectrum.png   – 中子探测器内中子动能谱

用法：
  python3 analysis/plot_detectors.py [root_file]
  python3 analysis/plot_detectors.py activation_output.root

依赖：ROOT（通过 ROOTSYS 或 conda/pip install root）
"""

import sys
import os

# ------ 引入 ROOT ------
try:
    import ROOT
except ImportError:
    print("[plot_detectors] ERROR: PyROOT not found. "
          "Source ROOT setup (source $ROOTSYS/bin/thisroot.sh) and retry.")
    sys.exit(1)

ROOT.gROOT.SetBatch(True)   # 无图形界面批量模式
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette(ROOT.kBird)
ROOT.gStyle.SetNumberContours(64)


# =============================================================
def open_root_file(fname: str) -> ROOT.TFile:
    f = ROOT.TFile.Open(fname)
    if not f or f.IsZombie():
        print(f"[plot_detectors] ERROR: cannot open '{fname}'")
        sys.exit(1)
    print(f"[plot_detectors] Opened: {fname}")
    return f


# =============================================================
def get_object(f: ROOT.TFile, name: str):
    """从文件中安全获取对象，不存在时返回 None。"""
    obj = f.Get(name)
    if not obj:
        return None
    obj.SetDirectory(0)   # 0 = 脱离文件所有权（等同于 nullptr），文件关闭后仍可用
    return obj


# =============================================================
def plot_proton_3d(f: ROOT.TFile, outdir: str) -> None:
    """1. 质子探测器内质子 3D 热力分布（三视图）"""

    # 优先使用模拟运行时已经填充好的 TH3D
    h3 = get_object(f, "hProton3D")

    if h3 is None:
        tree = f.Get("ProtonDetStep")
        if not tree:
            print("[plot_detectors] WARNING: no hProton3D or ProtonDetStep found. "
                  "Skipping proton plot.")
            return
        h3 = ROOT.TH3D("hProton3D_dyn",
                        "Proton 3D distribution;x (mm);y (mm);z (mm)",
                        25, -60., 60.,
                        25, -270., -160.,
                        20, -97., -83.)
        h3.SetDirectory(0)
        tree.Draw("z_mm:y_mm:x_mm>>hProton3D_dyn", "", "goff")
        print(f"[plot_detectors] Built hProton3D from ntuple "
              f"({h3.GetEntries():.0f} entries).")
    else:
        print(f"[plot_detectors] Read hProton3D from file "
              f"({h3.GetEntries():.0f} entries).")

    if h3.GetEntries() == 0:
        print("[plot_detectors] hProton3D is empty – no proton plot produced.")
        return

    c = ROOT.TCanvas("c_proton3d", "Proton 3D heatmap", 1500, 500)
    c.Divide(3, 1)

    # XY 投影 (束流俯视图)
    c.cd(1)
    hxy = h3.Project3D("yx")
    hxy.SetTitle("Proton detector: x-y (beam view);x (mm);y (mm)")
    hxy.Draw("COLZ")

    # XZ 投影 (侧视图)
    c.cd(2)
    hxz = h3.Project3D("zx")
    hxz.SetTitle("Proton detector: x-z (side view);x (mm);z (mm)")
    hxz.Draw("COLZ")

    # YZ 投影 (正视图)
    c.cd(3)
    hyz = h3.Project3D("zy")
    hyz.SetTitle("Proton detector: y-z (front view);y (mm);z (mm)")
    hyz.Draw("COLZ")

    out = os.path.join(outdir, "proton_3D_heatmap.png")
    c.SaveAs(out)
    print(f"[plot_detectors] Saved: {out}")


# =============================================================
def plot_neutron_yield_2d(f: ROOT.TFile, outdir: str) -> None:
    """2. 中子探测器内中子产额热力图（x-y）"""

    h2 = get_object(f, "hNeutronYield2D")

    if h2 is None:
        tree = f.Get("NeutronDetStep")
        if not tree:
            print("[plot_detectors] WARNING: no hNeutronYield2D or NeutronDetStep "
                  "found. Skipping neutron yield plot.")
            return
        h2 = ROOT.TH2D("hNeutronYield2D_dyn",
                        "Neutron yield map;x (mm);y (mm)",
                        50, -60., 60.,
                        50, -270., -160.)
        h2.SetDirectory(0)
        tree.Draw("y_mm:x_mm>>hNeutronYield2D_dyn", "", "goff")
        print(f"[plot_detectors] Built hNeutronYield2D from ntuple "
              f"({h2.GetEntries():.0f} entries).")
    else:
        print(f"[plot_detectors] Read hNeutronYield2D from file "
              f"({h2.GetEntries():.0f} entries).")

    if h2.GetEntries() == 0:
        print("[plot_detectors] hNeutronYield2D is empty – no neutron yield plot.")
        return

    c = ROOT.TCanvas("c_n2d", "Neutron yield 2D", 700, 600)
    h2.Draw("COLZ")
    out = os.path.join(outdir, "neutron_yield_2D.png")
    c.SaveAs(out)
    print(f"[plot_detectors] Saved: {out}")


# =============================================================
def plot_neutron_spectrum(f: ROOT.TFile, outdir: str) -> None:
    """3. 中子探测器内中子动能谱（1D）"""

    h1 = get_object(f, "hNeutronSpectrum")

    if h1 is None:
        tree = f.Get("NeutronDetStep")
        if not tree:
            print("[plot_detectors] WARNING: no hNeutronSpectrum or NeutronDetStep "
                  "found. Skipping spectrum plot.")
            return
        h1 = ROOT.TH1D("hNeutronSpectrum_dyn",
                        "Neutron energy spectrum;"
                        "E_{kin} (MeV);Counts",
                        200, 0., 100.)
        h1.SetDirectory(0)
        tree.Draw("Ekin_MeV>>hNeutronSpectrum_dyn", "", "goff")
        print(f"[plot_detectors] Built hNeutronSpectrum from ntuple "
              f"({h1.GetEntries():.0f} entries).")
    else:
        print(f"[plot_detectors] Read hNeutronSpectrum from file "
              f"({h1.GetEntries():.0f} entries).")

    if h1.GetEntries() == 0:
        print("[plot_detectors] hNeutronSpectrum is empty – no spectrum plot.")
        return

    c = ROOT.TCanvas("c_n1d", "Neutron spectrum", 800, 600)
    c.SetLogy()
    h1.SetLineColor(ROOT.kBlue + 1)
    h1.SetLineWidth(2)
    h1.Draw("HIST")
    out = os.path.join(outdir, "neutron_spectrum.png")
    c.SaveAs(out)
    print(f"[plot_detectors] Saved: {out}")


# =============================================================
def main() -> None:
    fname  = sys.argv[1] if len(sys.argv) > 1 else "activation_output.root"
    outdir = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(fname) or "."

    os.makedirs(outdir, exist_ok=True)

    f = open_root_file(fname)

    plot_proton_3d(f, outdir)
    plot_neutron_yield_2d(f, outdir)
    plot_neutron_spectrum(f, outdir)

    f.Close()
    print("[plot_detectors] Done.")


if __name__ == "__main__":
    main()
