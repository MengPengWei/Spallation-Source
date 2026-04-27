/// plot_detectors.C
/// ================
/// C++ ROOT macro：从 activation_output.root 读取 source 探测器数据，
/// 生成并保存三类图像：
///   1. proton_3D_heatmap.png  – 质子探测器内质子 3D 热力分布（三视图投影）
///   2. neutron_yield_2D.png   – 中子探测器内中子产额热力图（x-y）
///   3. neutron_spectrum.png   – 中子探测器内中子动能谱
///
/// 用法（ROOT 命令行）：
///   root -l -q 'analysis/plot_detectors.C("activation_output.root")'
/// 或直接：
///   root -l -q analysis/plot_detectors.C

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCanvas.h"
#include "TStyle.h"

#include <iostream>
#include <string>

void plot_detectors(const char* fname = "activation_output.root")
{
    // ---------- 打开 ROOT 文件 ----------
    TFile* f = TFile::Open(fname);
    if (!f || f->IsZombie()) {
        std::cerr << "[plot_detectors] ERROR: cannot open " << fname << "\n";
        return;
    }
    std::cout << "[plot_detectors] Opened: " << fname << "\n";

    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);
    gStyle->SetNumberContours(64);

    // =========================================================
    // 1. 质子 3D 热力分布
    // =========================================================
    TH3D* h3p = nullptr;
    f->GetObject("hProton3D", h3p);   // 先尝试预填充直方图

    if (!h3p) {
        // 从 ProtonDetStep ntuple 动态生成
        TTree* tProton = nullptr;
        f->GetObject("ProtonDetStep", tProton);
        if (!tProton) {
            std::cerr << "[plot_detectors] WARNING: no hProton3D or ProtonDetStep. "
                         "Skipping proton plot.\n";
        } else {
            h3p = new TH3D("hProton3D_dyn",
                           "Proton 3D distribution;"
                           "x (mm);y (mm);z (mm)",
                           25, -60., 60.,
                           25, -270., -160.,
                           20, -97., -83.);
            h3p->SetDirectory(nullptr);
            tProton->Draw("z_mm:y_mm:x_mm>>hProton3D_dyn", "", "goff");
            std::cout << "[plot_detectors] Built hProton3D from ntuple ("
                      << h3p->GetEntries() << " entries).\n";
        }
    } else {
        std::cout << "[plot_detectors] Read hProton3D from file ("
                  << h3p->GetEntries() << " entries).\n";
    }

    if (h3p && h3p->GetEntries() > 0) {
        TCanvas* c1 = new TCanvas("c1", "Proton 3D heatmap", 1500, 500);
        c1->Divide(3, 1);

        c1->cd(1);
        TH2D* hXY = (TH2D*)h3p->Project3D("yx");
        hXY->SetTitle("Proton detector: x-y (beam view);x (mm);y (mm)");
        hXY->Draw("COLZ");

        c1->cd(2);
        TH2D* hXZ = (TH2D*)h3p->Project3D("zx");
        hXZ->SetTitle("Proton detector: x-z (side view);x (mm);z (mm)");
        hXZ->Draw("COLZ");

        c1->cd(3);
        TH2D* hYZ = (TH2D*)h3p->Project3D("zy");
        hYZ->SetTitle("Proton detector: y-z (front view);y (mm);z (mm)");
        hYZ->Draw("COLZ");

        c1->SaveAs("proton_3D_heatmap.png");
        std::cout << "[plot_detectors] Saved: proton_3D_heatmap.png\n";
        delete c1;
    } else if (h3p) {
        std::cout << "[plot_detectors] hProton3D is empty – no proton plot.\n";
    }

    // =========================================================
    // 2. 中子产额热力图（x-y）
    // =========================================================
    TH2D* h2n = nullptr;
    f->GetObject("hNeutronYield2D", h2n);

    if (!h2n) {
        TTree* tNeutron = nullptr;
        f->GetObject("NeutronDetStep", tNeutron);
        if (!tNeutron) {
            std::cerr << "[plot_detectors] WARNING: no hNeutronYield2D or "
                         "NeutronDetStep. Skipping neutron yield plot.\n";
        } else {
            h2n = new TH2D("hNeutronYield2D_dyn",
                           "Neutron yield map;x (mm);y (mm)",
                           50, -60., 60.,
                           50, -270., -160.);
            h2n->SetDirectory(nullptr);
            tNeutron->Draw("y_mm:x_mm>>hNeutronYield2D_dyn", "", "goff");
            std::cout << "[plot_detectors] Built hNeutronYield2D from ntuple ("
                      << h2n->GetEntries() << " entries).\n";
        }
    } else {
        std::cout << "[plot_detectors] Read hNeutronYield2D from file ("
                  << h2n->GetEntries() << " entries).\n";
    }

    if (h2n && h2n->GetEntries() > 0) {
        TCanvas* c2 = new TCanvas("c2", "Neutron yield heatmap", 700, 600);
        h2n->Draw("COLZ");
        c2->SaveAs("neutron_yield_2D.png");
        std::cout << "[plot_detectors] Saved: neutron_yield_2D.png\n";
        delete c2;
    } else if (h2n) {
        std::cout << "[plot_detectors] hNeutronYield2D is empty – no neutron yield plot.\n";
    }

    // =========================================================
    // 3. 中子能谱
    // =========================================================
    TH1D* h1n = nullptr;
    f->GetObject("hNeutronSpectrum", h1n);

    if (!h1n) {
        TTree* tNeutron2 = nullptr;
        f->GetObject("NeutronDetStep", tNeutron2);
        if (!tNeutron2) {
            std::cerr << "[plot_detectors] WARNING: no hNeutronSpectrum or "
                         "NeutronDetStep. Skipping spectrum plot.\n";
        } else {
            h1n = new TH1D("hNeutronSpectrum_dyn",
                           "Neutron energy spectrum;"
                           "E_{kin} (MeV);Counts",
                           200, 0., 100.);
            h1n->SetDirectory(nullptr);
            tNeutron2->Draw("Ekin_MeV>>hNeutronSpectrum_dyn", "", "goff");
            std::cout << "[plot_detectors] Built hNeutronSpectrum from ntuple ("
                      << h1n->GetEntries() << " entries).\n";
        }
    } else {
        std::cout << "[plot_detectors] Read hNeutronSpectrum from file ("
                  << h1n->GetEntries() << " entries).\n";
    }

    if (h1n && h1n->GetEntries() > 0) {
        TCanvas* c3 = new TCanvas("c3", "Neutron spectrum", 800, 600);
        c3->SetLogy();
        h1n->SetLineColor(kBlue + 1);
        h1n->SetLineWidth(2);
        h1n->Draw("HIST");
        c3->SaveAs("neutron_spectrum.png");
        std::cout << "[plot_detectors] Saved: neutron_spectrum.png\n";
        delete c3;
    } else if (h1n) {
        std::cout << "[plot_detectors] hNeutronSpectrum is empty – no spectrum plot.\n";
    }

    f->Close();
    std::cout << "[plot_detectors] Done.\n";
}
