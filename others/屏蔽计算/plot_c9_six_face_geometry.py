#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
六面汇总形式的几何示意（与 plot_c9_flux_dose_heatmaps 中 c9_六面汇总_*.png 相同的 2×3 面顺序与轴标签）。

- RPP12 / RPP11 来自 DetectorConstruction.cc::GetRpp（cm→mm×10）
- 球形/点位探测器来自 ShieldingDataCollector.cc（中心 mm），投影到各面局部坐标
- 横纵轴默认 -2500～2500 mm（与热图一致）

输出默认: build/c9热图/c9_六面汇总_geometry.png

用法:
  python3 plot_c9_six_face_geometry.py [--out path]
"""

from __future__ import annotations

import argparse
from pathlib import Path

# 与 DetectorConstruction::GetRpp 一致 (cm)
RPP12_CM = (-222.7, 222.7, -222.7, 222.7, 0.0, 340.5)
RPP11_CM = (-217.7, 217.7, -217.7, 217.7, 0.0, 335.5)


def to_mm(rpp_cm: tuple[float, float, float, float, float, float]) -> tuple[float, float, float, float, float, float]:
    return tuple(v * 10.0 for v in rpp_cm)


FACE_ORDER = ["px", "mx", "py", "my", "pz", "mz"]
FACE_INFO = {
    "px": {"title": "+X 面 (法向 +x)", "axes": ("Y (mm)", "Z (mm)")},
    "mx": {"title": "-X 面 (法向 -x)", "axes": ("Y (mm)", "Z (mm)")},
    "py": {"title": "+Y 面 (法向 +y)", "axes": ("X (mm)", "Z (mm)")},
    "my": {"title": "-Y 面 (法向 -y)", "axes": ("X (mm)", "Z (mm)")},
    "pz": {"title": "+Z 面 (法向 +z)", "axes": ("X (mm)", "Y (mm)")},
    "mz": {"title": "-Z 面 (法向 -z)", "axes": ("X (mm)", "Y (mm)")},
}

POINT_PROBES: list[tuple[str, float, float, float, float]] = [
    ("shield_outer_30cm", 2527.0, 0.0, 1702.5, 50.0),
    ("hotcell_outer_30cm", 2827.0, 0.0, 1702.5, 50.0),
    ("channel_22_exit", 0.0, 3200.0, 900.0, 35.0),
    ("channel_23_exit", 2250.0, 2250.0, 900.0, 35.0),
    ("channel_24_exit", -2250.0, 2250.0, 900.0, 35.0),
    ("door_operation_point", -350.0, -2500.0, 950.0, 80.0),
    ("center_tube_water", 0.0, 0.0, 905.6, 50.0),
    ("center_tube_edge", 0.0, -185.0, 900.0, 30.0),
    ("c4_fe_outer_side", 677.0, 0.0, 900.0, 50.0),
    ("c4_fe_outer_top", 0.0, 0.0, 1855.0, 50.0),
    ("c4_fe_inner_side", 652.0, 0.0, 900.0, 50.0),
    ("c4_fe_inner_top", 0.0, 0.0, 1830.0, 50.0),
    ("c4_fe_inner_corner", 652.0, 652.0, 900.0, 40.0),
]

AXIS_LO = -2500.0
AXIS_HI = 2500.0


def setup_cjk() -> None:
    try:
        import matplotlib.pyplot as plt
        from matplotlib import font_manager

        for fp in (
            Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"),
            Path("/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"),
        ):
            if fp.is_file():
                font_manager.fontManager.addfont(str(fp))
                prop = font_manager.FontProperties(fname=str(fp))
                plt.rcParams["font.sans-serif"] = [prop.get_name()] + plt.rcParams.get("font.sans-serif", [])
                break
        plt.rcParams["axes.unicode_minus"] = False
    except Exception:
        pass


def outer_inner_horiz_vert(
    face: str, r12: tuple[float, ...], r11: tuple[float, ...]
) -> tuple[tuple[float, float, float, float], tuple[float, float, float, float]]:
    """与 plot_c9_flux_dose_heatmaps 的 imshow 一致：水平轴=第 2 个网格索引对应坐标，竖直=第 1 个。
    返回 ((h0,h1,v0,v1)_外, (…)_内)，单位 mm。"""
    x0, x1, y0, y1, z0, z1 = r12
    xi0, xi1, yi0, yi1, zi0, zi1 = r11
    if face in ("px", "mx"):
        return (z0, z1, y0, y1), (zi0, zi1, yi0, yi1)
    if face in ("py", "my"):
        return (z0, z1, x0, x1), (zi0, zi1, xi0, xi1)
    if face in ("pz", "mz"):
        return (y0, y1, x0, x1), (yi0, yi1, xi0, xi1)
    raise ValueError(face)


def plane_dist_mm(face: str, x: float, y: float, z: float, r12: tuple[float, ...]) -> float:
    x0, x1, y0, y1, z0, z1 = r12
    if face == "px":
        return abs(x - x1)
    if face == "mx":
        return abs(x - x0)
    if face == "py":
        return abs(y - y1)
    if face == "my":
        return abs(y - y0)
    if face == "pz":
        return abs(z - z1)
    if face == "mz":
        return abs(z - z0)
    return 1e9


def project_probe(face: str, x: float, y: float, z: float) -> tuple[float, float]:
    """返回 (水平坐标, 竖直坐标)，与热图 imshow extent 一致。"""
    if face in ("px", "mx"):
        return z, y
    if face in ("py", "my"):
        return z, x
    return y, x


def draw_six_face_geometry(out_png: Path, dpi: int) -> None:
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle

    setup_cjk()
    r12 = to_mm(RPP12_CM)
    r11 = to_mm(RPP11_CM)

    fig, axes = plt.subplots(2, 3, figsize=(16, 10))
    axes = axes.ravel()

    for idx, face in enumerate(FACE_ORDER):
        ax = axes[idx]
        info = FACE_INFO[face]
        (h0, h1, v0, v1), (ih0, ih1, iv0, iv1) = outer_inner_horiz_vert(face, r12, r11)

        ax.add_patch(
            Rectangle(
                (h0, v0),
                h1 - h0,
                v1 - v0,
                fill=True,
                facecolor="#d5e8f7",
                edgecolor="#1a5276",
                linewidth=2.0,
                label="RPP12 外包络投影",
            )
        )
        ax.add_patch(
            Rectangle(
                (ih0, iv0),
                ih1 - ih0,
                iv1 - iv0,
                fill=False,
                edgecolor="#c0392b",
                linewidth=1.5,
                linestyle="--",
                label="RPP11 内腔投影",
            )
        )

        near_mm = 350.0
        for name, px, py, pz, pr in POINT_PROBES:
            du, dv = project_probe(face, px, py, pz)
            dplane = plane_dist_mm(face, px, py, pz, r12)
            alpha = 0.95 if dplane < near_mm else 0.35
            sz = max(28.0, pr * 0.9)
            ax.scatter([du], [dv], s=sz, c="gold" if dplane < near_mm else "#bbbbbb", edgecolors="k", linewidths=0.6, zorder=5, alpha=alpha)
            if dplane < near_mm or name.startswith("center_tube"):
                ax.annotate(
                    name.replace("_", " "),
                    (du, dv),
                    fontsize=5,
                    xytext=(3, 3),
                    textcoords="offset points",
                    alpha=0.9,
                )

        ax.set_xlim(AXIS_LO, AXIS_HI)
        ax.set_ylim(AXIS_LO, AXIS_HI)
        ax.set_aspect("equal")
        ax.set_xlabel(info["axes"][1])
        ax.set_ylabel(info["axes"][0])
        ax.set_title(info["title"])
        ax.grid(True, alpha=0.35)
        if idx == 0:
            ax.legend(loc="upper right", fontsize=7)

    fig.suptitle(
        "c9 不锈钢外包络几何六面视图（RPP12 外表面 / RPP11 内腔投影）\n"
        "坐标系：Geant4 右手系，单位 mm；计分薄层在 RPP12 外法向 1 cm（示意未画出偏移）",
        fontsize=12,
    )
    fig.tight_layout()
    fig.savefig(out_png, dpi=dpi)
    plt.close(fig)


def main() -> None:
    here = Path(__file__).resolve().parent
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--out",
        type=Path,
        default=here / "build" / "c9热图" / "c9_六面汇总_geometry.png",
        help="输出 PNG 路径",
    )
    ap.add_argument("--dpi", type=int, default=150)
    args = ap.parse_args()
    out = args.out.resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    try:
        import matplotlib  # noqa: F401
    except ImportError:
        raise SystemExit("需要 matplotlib")
    draw_six_face_geometry(out, args.dpi)
    print(f"已保存: {out}")


if __name__ == "__main__":
    main()
