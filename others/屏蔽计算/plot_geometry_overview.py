#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
根据 DetectorConstruction.cc 中 GetRpp / ConstructVolumes 与 ShieldingDataCollector.cc 中探测器定义，
绘制屏蔽体主要 RPP 包络示意及探测器位置（示意，非 Geant4 可视化替换）。

输出（默认 build/geometry_overview/）:
  - geometry_XZ_shells.png   : X–Z 平面嵌套包络（cm 定义×10 → mm）
  - geometry_XY_z900.png   : X–Y 平面，Z≈900 mm（靶区高度）
  - geometry_overview_combined.png : 2×2 总览（XZ + 两幅 XY + 文字）

用法:
  python3 plot_geometry_overview.py [--out dir]
"""

from __future__ import annotations

import argparse
from pathlib import Path

# 与 DetectorConstruction.cc::GetRpp 一致，单位 cm（半宽由 xmin/xmax 给出）
RPP_CM: dict[int, tuple[float, float, float, float, float, float]] = {
    2: (-65.2, 65.2, -65.2, 65.2, 0.0, 183.0),
    3: (42.8, 65.2, -65.2, 65.2, 113.0, 183.0),
    4: (-67.7, 67.7, -67.7, 67.7, 0.0, 185.5),
    6: (-87.7, 87.7, -87.7, 87.7, 0.0, 205.5),
    8: (-107.7, 107.7, -107.7, 107.7, 0.0, 225.5),
    10: (-207.7, 207.7, -207.7, 207.7, 0.0, 325.5),
    11: (-217.7, 217.7, -217.7, 217.7, 0.0, 335.5),
    12: (-222.7, 222.7, -222.7, 222.7, 0.0, 340.5),
    21: (-50.0, 0.0, -300.0, 0.0, 35.0, 85.0),
    30: (-60.0, 10.0, -300.0, -250.0, 0.0, 85.0),
    35: (-790.0, 790.0, -735.0, 735.0, -150.0, 0.0),
}

# ShieldingDataCollector 构造函数中球形/点位探测器（中心 mm，半径 mm）
# c9 外包络为薄层网格，数量多，此处仅文字说明；可另用 c9外表面探测器通量与剂量.csv 绘图
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

TARGET_CENTER_MM = (0.0, 0.0, 900.0)  # DetectorConstruction: targetPosition (0,0,90*cm)
TARGET_R_MM = (185.0, 250.0)  # Shape1 rmin, rmax mm


def cm_to_mm_bounds(rpp: tuple[float, float, float, float, float, float]) -> tuple[float, float, float, float, float, float]:
    """RPP 各边界由 cm 数给出 → 显示用 mm (=×10)。"""
    return tuple(v * 10.0 for v in rpp)


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


def draw_xz_shells(out: Path) -> None:
    import matplotlib.pyplot as plt
    from matplotlib.patches import Rectangle

    setup_cjk()
    fig, ax = plt.subplots(figsize=(12, 8))
    order = [(12, "c9 外包络 RPP12", "#c0392b"), (11, "c8 RPP11", "#e74c3c"), (10, "c7 RPP10", "#e67e22"),
             (8, "c6 RPP8", "#f39c12"), (6, "c5 RPP6", "#27ae60"), (4, "c4 RPP4", "#2980b9"), (2, "c2 空气 RPP2", "#3498db")]
    for sid, label, color in order:
        if sid not in RPP_CM:
            continue
        x0, x1, y0, y1, z0, z1 = cm_to_mm_bounds(RPP_CM[sid])
        ax.add_patch(
            Rectangle(
                (x0, z0),
                x1 - x0,
                z1 - z0,
                fill=False,
                linewidth=1.6 if sid == 12 else 1.0,
                edgecolor=color,
                label=label,
            )
        )
    ax.axhline(0, color="gray", linewidth=0.5, linestyle="--")
    ax.axvline(0, color="gray", linewidth=0.5, linestyle="--")
    ax.set_aspect("equal")
    ax.set_xlabel("X (mm)")
    ax.set_ylabel("Z (mm)")
    ax.set_title("屏蔽主壳嵌套 RPP 包络（X–Z 投影，与 DetectorConstruction::GetRpp 一致）")
    ax.legend(loc="upper left", fontsize=8)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out / "geometry_XZ_shells.png", dpi=160)
    plt.close(fig)


def draw_xy_plane(out: Path, z_title: str, z_ref: float, z_tol: float) -> None:
    import matplotlib.pyplot as plt
    from matplotlib.patches import Circle, Rectangle

    setup_cjk()
    fig, ax = plt.subplots(figsize=(11, 10))
    x0, x1, y0, y1, z0, z1 = cm_to_mm_bounds(RPP_CM[12])
    ax.add_patch(Rectangle((x0, y0), x1 - x0, y1 - y0, fill=False, edgecolor="#c0392b", linewidth=2, label="RPP12 外包络底面投影"))
    x0, x1, y0, y1, _, _ = cm_to_mm_bounds(RPP_CM[2])
    ax.add_patch(Rectangle((x0, y0), x1 - x0, y1 - y0, fill=False, edgecolor="#3498db", linewidth=1, linestyle="--", label="RPP2 空气投影"))

    # 靶区环形（XY）
    ax.add_patch(
        Circle(
            (TARGET_CENTER_MM[0], TARGET_CENTER_MM[1]),
            TARGET_R_MM[1],
            fill=False,
            edgecolor="#8e44ad",
            linewidth=1.5,
            label="靶区外径≈250 mm (Shape1)",
        )
    )
    ax.add_patch(
        Circle(
            (TARGET_CENTER_MM[0], TARGET_CENTER_MM[1]),
            TARGET_R_MM[0],
            fill=False,
            edgecolor="#8e44ad",
            linewidth=1.0,
            linestyle=":",
            label="靶区内径≈185 mm",
        )
    )

    for name, x, y, z, r in POINT_PROBES:
        if abs(z - z_ref) > z_tol:
            continue
        ax.scatter([x], [y], s=max(20, r * 0.8), marker="o", edgecolors="k", facecolors="yellow", linewidths=0.8, zorder=5)
        ax.annotate(
            name.replace("_", "\n"),
            (x, y),
            textcoords="offset points",
            xytext=(4, 4),
            fontsize=6,
            ha="left",
        )

    ax.set_aspect("equal")
    ax.set_xlabel("X (mm)")
    ax.set_ylabel("Y (mm)")
    ax.set_title(f"点位探测器与包络（XY，|Z−{z_ref:.0f}|≤{z_tol:.0f} mm）\n{z_title}")
    ax.legend(loc="lower right", fontsize=7)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fname = "geometry_XY_z900.png" if z_ref < 1200 else "geometry_XY_z1702.png"
    fig.savefig(out / fname, dpi=160)
    plt.close(fig)


def write_text_report(out: Path) -> None:
    lines = []
    lines.append("=== 坐标系（与 Geant4 默认一致）===")
    lines.append("- 右手直角坐标系；本工程 Construct 中 RPP 与 G4PVPlacement 使用内部长度单位，导出/探测器表常用 mm。")
    lines.append("- GetRpp 表中数值单位为 cm；乘 10 得 mm 边界。")
    lines.append("")
    lines.append("=== 主要 RPP 边界 (mm) ===")
    for sid in (2, 4, 6, 8, 10, 11, 12):
        x0, x1, y0, y1, z0, z1 = cm_to_mm_bounds(RPP_CM[sid])
        cx = (x0 + x1) / 2
        cy = (y0 + y1) / 2
        cz = (z0 + z1) / 2
        lines.append(
            f"  RPP{sid}: X[{x0:.1f},{x1:.1f}] Y[{y0:.1f},{y1:.1f}] Z[{z0:.1f},{z1:.1f}] mm  中心({cx:.1f},{cy:.1f},{cz:.1f})"
        )
    lines.append("")
    lines.append("=== World ===")
    lines.append("  G4Orb 半径 5000 cm = 50000 mm（球世界，DetectorConstruction::ConstructVolumes）")
    lines.append("")
    lines.append("=== 靶体参考（DetectorConstruction，近似）===")
    lines.append(f"  targetPosition = (0, 0, 90 cm) → ({TARGET_CENTER_MM[0]:.1f}, {TARGET_CENTER_MM[1]:.1f}, {TARGET_CENTER_MM[2]:.1f}) mm")
    lines.append("")
    lines.append("=== 球形/点位探测器（ShieldingDataCollector，中心 mm / 半径 mm）===")
    for name, x, y, z, r in POINT_PROBES:
        lines.append(f"  {name}: ({x:.1f}, {y:.1f}, {z:.1f}), r={r:.1f}")
    lines.append("")
    lines.append("=== c9 外表面薄层网格探测器 ===")
    lines.append("  由 ScoreC9OuterSurfaceProbes 在 RPP12 外包络外法向 1 cm 六面薄层上计分；")
    lines.append("  名称形如 c9_outer_px_iy_iz 等，中心坐标见 build/c9外表面探测器通量与剂量.csv。")
    lines.append("  可用 plot_c9_flux_dose_heatmaps.py 按面绘制。")
    lines.append("")
    (out / "geometry_coordinates.txt").write_text("\n".join(lines), encoding="utf-8")


def draw_combined_overview(out: Path) -> None:
    """单图总览：XZ + XY(z≈900) + XY(z≈1702) + 坐标说明。"""
    import matplotlib.pyplot as plt
    from matplotlib.patches import Circle, Rectangle

    setup_cjk()
    fig = plt.figure(figsize=(16, 10))

    # --- XZ ---
    ax1 = fig.add_subplot(2, 2, 1)
    order = [(12, "#c0392b"), (11, "#e74c3c"), (10, "#e67e22"), (8, "#f39c12"), (6, "#27ae60"), (4, "#2980b9"), (2, "#3498db")]
    for sid, color in order:
        if sid not in RPP_CM:
            continue
        x0, x1, y0, y1, z0, z1 = cm_to_mm_bounds(RPP_CM[sid])
        ax1.add_patch(Rectangle((x0, z0), x1 - x0, z1 - z0, fill=False, linewidth=1.4 if sid == 12 else 0.9, edgecolor=color))
    ax1.axhline(0, color="gray", linewidth=0.4, linestyle="--")
    ax1.axvline(0, color="gray", linewidth=0.4, linestyle="--")
    ax1.set_aspect("equal")
    ax1.set_xlabel("X (mm)")
    ax1.set_ylabel("Z (mm)")
    ax1.set_title("主壳 RPP 嵌套 (X–Z)")
    ax1.grid(True, alpha=0.25)

    def fill_xy(ax, z_ref: float, z_tol: float, title: str) -> None:
        x0, x1, y0, y1, _, _ = cm_to_mm_bounds(RPP_CM[12])
        ax.add_patch(Rectangle((x0, y0), x1 - x0, y1 - y0, fill=False, edgecolor="#c0392b", linewidth=1.8))
        x0, x1, y0, y1, _, _ = cm_to_mm_bounds(RPP_CM[2])
        ax.add_patch(Rectangle((x0, y0), x1 - x0, y1 - y0, fill=False, edgecolor="#3498db", linewidth=0.8, linestyle="--"))
        ax.add_patch(Circle((0, 0), TARGET_R_MM[1], fill=False, edgecolor="#8e44ad", linewidth=1.0))
        for name, x, y, z, r in POINT_PROBES:
            if abs(z - z_ref) > z_tol:
                continue
            ax.scatter([x], [y], s=max(25, r), c="gold", edgecolors="k", linewidths=0.6, zorder=5)
            ax.annotate(name, (x, y), fontsize=5, xytext=(3, 3), textcoords="offset points")
        ax.set_aspect("equal")
        ax.set_xlabel("X (mm)")
        ax.set_ylabel("Y (mm)")
        ax.set_title(title)
        ax.grid(True, alpha=0.25)

    ax2 = fig.add_subplot(2, 2, 2)
    fill_xy(ax2, 900.0, 80.0, "XY  @ Z≈900 mm（靶区/通道）")
    ax3 = fig.add_subplot(2, 2, 3)
    fill_xy(ax3, 1702.5, 120.0, "XY  @ Z≈1702 mm（外侧监测点）")

    ax4 = fig.add_subplot(2, 2, 4)
    ax4.axis("off")
    coord_txt = (
        "坐标系（Geant4 默认）\n"
        "· 右手系，单位 mm（RPP 表为 cm×10）\n"
        "· +Z 向上；原点 O 在 World 球心\n\n"
        "几何来源\n"
        "· DetectorConstruction.cc\n"
        "  GetRpp / MakeRppSolid / G4PVPlacement\n\n"
        "探测器来源\n"
        "· ShieldingDataCollector.cc 中 probes\n"
        "· c9 六面薄层：见 c9外表面探测器通量与剂量.csv\n\n"
        "RPP12 外包络 (mm)\n"
        f"X[{cm_to_mm_bounds(RPP_CM[12])[0]:.0f},{cm_to_mm_bounds(RPP_CM[12])[1]:.0f}] "
        f"Y[{cm_to_mm_bounds(RPP_CM[12])[2]:.0f},{cm_to_mm_bounds(RPP_CM[12])[3]:.0f}] "
        f"Z[{cm_to_mm_bounds(RPP_CM[12])[4]:.0f},{cm_to_mm_bounds(RPP_CM[12])[5]:.0f}]"
    )
    ax4.text(0.02, 0.98, coord_txt, transform=ax4.transAxes, va="top", ha="left", fontsize=10)

    fig.suptitle("屏蔽几何与探测器位置总览（示意）", fontsize=14)
    fig.tight_layout()
    fig.savefig(out / "geometry_overview_combined.png", dpi=150)
    plt.close(fig)


def main() -> None:
    here = Path(__file__).resolve().parent
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", type=Path, default=here / "build" / "geometry_overview", help="输出目录")
    args = ap.parse_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)

    try:
        import matplotlib  # noqa: F401
    except ImportError:
        raise SystemExit("需要安装 matplotlib: pip install matplotlib")

    draw_xz_shells(out)
    draw_xy_plane(out, "靶道高度附近", 900.0, 80.0)
    draw_xy_plane(out, "壳体竖向中部附近", 1702.5, 120.0)
    draw_combined_overview(out)
    write_text_report(out)
    print(f"已写入: {out}")


if __name__ == "__main__":
    main()
