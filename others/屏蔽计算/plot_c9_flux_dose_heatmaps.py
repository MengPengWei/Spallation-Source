#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
根据 c9外表面探测器通量与剂量.csv 绘制各外包络面 γ/中子 剂量率与通量热图。

- 默认使用标准库生成 SVG（无需 numpy/matplotlib）。
- 若已安装 matplotlib + numpy，同时生成 PNG（色彩标度更柔和）。
- 默认将「伽马/中子通量」列乘以 需求参数输出.txt 中的源强(每秒)，使热图与物理通量 (1/cm²/s) 一致；
  若 CSV 已由新版程序写出已含源强的通量，请加 --no-flux-scale-by-source。

用法:
  python3 plot_c9_flux_dose_heatmaps.py [--csv path] [--out dir] [--no-png]
  PNG 默认横纵轴均为 -2500～2500 mm，可用 --axis-mm MIN MAX 修改。
"""

from __future__ import annotations

import argparse
import csv
import math
import re
from pathlib import Path

NAME_RE = re.compile(r"^c9_outer_(px|mx|py|my|pz|mz)_(\d+)_(\d+)$")

FACE_INFO = {
    "px": {"title": "+X 面 (法向 +x)", "axes": ("Y (mm)", "Z (mm)"), "coord": ("中心Y_mm", "中心Z_mm")},
    "mx": {"title": "-X 面 (法向 -x)", "axes": ("Y (mm)", "Z (mm)"), "coord": ("中心Y_mm", "中心Z_mm")},
    "py": {"title": "+Y 面 (法向 +y)", "axes": ("X (mm)", "Z (mm)"), "coord": ("中心X_mm", "中心Z_mm")},
    "my": {"title": "-Y 面 (法向 -y)", "axes": ("X (mm)", "Z (mm)"), "coord": ("中心X_mm", "中心Z_mm")},
    "pz": {"title": "+Z 面 (法向 +z)", "axes": ("X (mm)", "Y (mm)"), "coord": ("中心X_mm", "中心Y_mm")},
    "mz": {"title": "-Z 面 (法向 -z)", "axes": ("X (mm)", "Y (mm)"), "coord": ("中心X_mm", "中心Y_mm")},
}
# 热图 mat[i][j]：行 i = 探测器名中第 1 个索引 → coord[0] → extent 的竖直方向；
# 列 j = 第 2 个索引 → coord[1] → extent 的水平方向。故 xlabel=axes[1], ylabel=axes[0]。

METRICS = [
    ("伽马剂量率_Sv每小时", "gamma_dose_Sv_per_h", "γ 剂量率 (Sv/h)", True),
    ("中子剂量率_Sv每小时", "neutron_dose_Sv_per_h", "中子 剂量率 (Sv/h)", True),
    ("伽马通量_每平方厘米每秒", "gamma_flux_per_cm2_s", "γ 通量 (1/cm²/s)", True),
    ("中子通量_每平方厘米每秒", "neutron_flux_per_cm2_s", "中子 通量 (1/cm²/s)", True),
]

FLUX_CSV_COLUMNS = frozenset({"伽马通量_每平方厘米每秒", "中子通量_每平方厘米每秒"})
REQ_SOURCE_RE = re.compile(r"源强\(每秒\)\s*=\s*([\d.eE+-]+)")


def parse_source_intensity_req(req_path: Path) -> float:
    text = req_path.read_text(encoding="utf-8-sig", errors="replace")
    m = REQ_SOURCE_RE.search(text)
    if not m:
        raise SystemExit(f"无法在 {req_path} 中解析 源强(每秒)")
    return float(m.group(1))


def apply_flux_scale_to_rows(rows: list[dict], i_per_s: float) -> None:
    for r in rows:
        for k in FLUX_CSV_COLUMNS:
            v = r.get(k)
            if v is None or str(v).strip() == "":
                continue
            try:
                r[k] = str(float(v) * i_per_s)
            except (TypeError, ValueError):
                pass

# PNG（matplotlib）坐标轴统一范围 (mm)；侧面上 Z>2500 mm 等会落在显示区外被裁切
DEFAULT_AXIS_MM_MIN = -2500.0
DEFAULT_AXIS_MM_MAX = 2500.0


def apply_fixed_axis_mm(ax, lo: float, hi: float) -> None:
    ax.set_xlim(lo, hi)
    ax.set_ylim(lo, hi)

# SVG 文本字体栈（浏览器/system sans 回退）
SVG_FONT_FAMILY = "Noto Sans CJK SC, WenQuanYi Zen Hei, Droid Sans Fallback, sans-serif"

_MPL_CJK_CONFIGURED = False


def setup_matplotlib_chinese_font() -> None:
    """注册系统中文字体并配置 matplotlib，避免 PNG 中中文显示为方块。"""
    global _MPL_CJK_CONFIGURED
    if _MPL_CJK_CONFIGURED:
        return
    try:
        import matplotlib.pyplot as plt
        from matplotlib import font_manager
    except ImportError:
        return

    font_paths = [
        Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"),
        Path("/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc"),
        Path("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc"),
        Path("/usr/share/fonts/truetype/wqy/wqy-microhei.ttc"),
        Path("/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc"),
        Path("/usr/share/fonts/truetype/arphic/ukai.ttc"),
        Path("/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf"),
    ]
    chosen_family = None
    for fp in font_paths:
        if fp.is_file():
            try:
                font_manager.fontManager.addfont(str(fp))
                prop = font_manager.FontProperties(fname=str(fp))
                # TTC 在 matplotlib 中常注册为 Noto Sans CJK JP，该字体仍含汉字字形，勿强行写 SC 以免名称不匹配回落到 DejaVu
                chosen_family = prop.get_name()
                break
            except (OSError, ValueError, RuntimeError):
                continue

    if chosen_family is None:
        names = [f.name for f in font_manager.fontManager.ttflist]
        for prefer in (
            "Noto Sans CJK SC",
            "Noto Sans CJK TC",
            "WenQuanYi Zen Hei",
            "WenQuanYi Micro Hei",
            "AR PL UKai CN",
            "Droid Sans Fallback",
        ):
            if prefer in names:
                chosen_family = prefer
                break

    if chosen_family:
        sans = [chosen_family] + [
            x for x in plt.rcParams.get("font.sans-serif", []) if x != chosen_family
        ]
        plt.rcParams["font.sans-serif"] = sans
        plt.rcParams["font.family"] = "sans-serif"
    plt.rcParams["axes.unicode_minus"] = False
    _MPL_CJK_CONFIGURED = True


def parse_name(name: str):
    m = NAME_RE.match(str(name).strip())
    if not m:
        return None
    return m.group(1), int(m.group(2)), int(m.group(3))


def load_rows(csv_path: Path) -> list[dict]:
    with csv_path.open(encoding="utf-8-sig", newline="") as f:
        return list(csv.DictReader(f))


def group_face(rows: list[dict]) -> dict[str, list[dict]]:
    out: dict[str, list[dict]] = {k: [] for k in FACE_INFO}
    for r in rows:
        p = parse_name(r.get("探测器名称", ""))
        if p:
            out[p[0]].append(r)
    return out


def build_matrix(face_rows: list[dict], face: str, col: str):
    """返回 imax, jmax, mat[i][j], row_centers[i], col_centers[j]。
    行 i = 探测器名中第 1 个索引（如 px 的 iy）→ coord[0]（竖直轴物理坐标）；
    列 j = 第 2 个索引（如 iz）→ coord[1]（水平轴）。与 matplotlib imshow(..., extent=(列范围, 行范围)) 一致。"""
    if not face_rows:
        return None, None, None, None, None
    pts = []
    for r in face_rows:
        p = parse_name(r["探测器名称"])
        if not p:
            continue
        _, i, j = p
        pts.append((i, j, r))
    if not pts:
        return None, None, None, None, None
    imax = max(t[0] for t in pts) + 1
    jmax = max(t[1] for t in pts) + 1
    cr, cc = FACE_INFO[face]["coord"]
    mat = [[float("nan")] * jmax for _ in range(imax)]
    grid_r = [[float("nan")] * jmax for _ in range(imax)]
    grid_c = [[float("nan")] * jmax for _ in range(imax)]
    for i, j, r in pts:
        try:
            v = float(r[col])
        except (TypeError, ValueError):
            v = float("nan")
        mat[i][j] = v
        try:
            grid_r[i][j] = float(r[cr])
            grid_c[i][j] = float(r[cc])
        except (TypeError, ValueError):
            pass
    # 行=第 1 个网格索引 i 的物理轴，列=第 2 个索引 j
    row_centers = []
    for ii in range(imax):
        rs = [grid_r[ii][jj] for jj in range(jmax) if math.isfinite(grid_r[ii][jj])]
        row_centers.append(sum(rs) / len(rs) if rs else float("nan"))
    col_centers = []
    for jj in range(jmax):
        cs = [grid_c[ii][jj] for ii in range(imax) if math.isfinite(grid_c[ii][jj])]
        col_centers.append(sum(cs) / len(cs) if cs else float("nan"))
    return imax, jmax, mat, row_centers, col_centers


def turbo_rgb(t: float) -> tuple[int, int, int]:
    """matplotlib turbo 近似（t∈[0,1]）"""
    t = max(0.0, min(1.0, t))
    # 分段线性近似 turbo 主要色相
    stops = [
        (0.0, (48, 18, 59)),
        (0.25, (40, 120, 181)),
        (0.5, (144, 215, 134)),
        (0.75, (246, 209, 57)),
        (1.0, (122, 4, 3)),
    ]
    for k in range(len(stops) - 1):
        t0, c0 = stops[k]
        t1, c1 = stops[k + 1]
        if t <= t1 or k == len(stops) - 2:
            if t1 > t0:
                u = (t - t0) / (t1 - t0) if t >= t0 else 0.0
            else:
                u = 0.0
            u = max(0.0, min(1.0, u))
            return (
                int(c0[0] + u * (c1[0] - c0[0])),
                int(c0[1] + u * (c1[1] - c0[1])),
                int(c0[2] + u * (c1[2] - c0[2])),
            )
    return stops[-1][1]


def color_for_value(v: float, vmin: float, vmax: float, use_log: bool) -> str:
    if not math.isfinite(v) or v <= 0:
        return "#e8e8e8"
    if use_log and vmin > 0 and vmax > vmin:
        lv = math.log(max(v, vmin))
        l0 = math.log(vmin)
        l1 = math.log(vmax)
        t = (lv - l0) / (l1 - l0) if l1 > l0 else 0.5
    elif vmax > vmin:
        t = (v - vmin) / (vmax - vmin)
    else:
        t = 0.5
    t = max(0.0, min(1.0, t))
    r, g, b = turbo_rgb(t)
    return f"#{r:02x}{g:02x}{b:02x}"


def value_range(mat: list[list[float]], use_log: bool) -> tuple[float, float]:
    pos = [x for row in mat for x in row if math.isfinite(x) and x > 0]
    if not pos:
        return 1e-30, 1.0
    lo, hi = min(pos), max(pos)
    if use_log:
        return max(lo, 1e-30), max(hi, lo * 1.0001)
    return lo, hi


def write_svg(
    path: Path,
    title: str,
    zlabel: str,
    mat: list[list[float]],
    row_centers: list[float],
    col_centers: list[float],
    xlabel_str: str,
    ylabel_str: str,
    use_log: bool,
) -> None:
    imax = len(mat)
    jmax = len(mat[0]) if imax else 0
    w_pix, h_pix = 900, 700
    margin_l, margin_b, margin_t = 80, 60, 70
    cw = (w_pix - margin_l - 40) / max(jmax, 1)
    ch = (h_pix - margin_b - margin_t) / max(imax, 1)
    vmin, vmax = value_range(mat, use_log)

    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w_pix}" height="{h_pix}" '
        f'viewBox="0 0 {w_pix} {h_pix}">',
        f'<title>{title}</title>',
        f'<text x="{w_pix / 2}" y="28" text-anchor="middle" font-size="16" font-family="{SVG_FONT_FAMILY}">{title}</text>',
        f'<text x="{w_pix / 2}" y="48" text-anchor="middle" font-size="12" fill="#444" font-family="{SVG_FONT_FAMILY}">{zlabel}</text>',
    ]
    for i in range(imax):
        for j in range(jmax):
            v = mat[i][j]
            fill = color_for_value(v, vmin, vmax, use_log)
            x = margin_l + j * cw
            y = margin_t + (imax - 1 - i) * ch
            lines.append(
                f'<rect x="{x:.2f}" y="{y:.2f}" width="{cw - 1:.2f}" height="{ch - 1:.2f}" '
                f'fill="{fill}" stroke="#ffffff" stroke-width="0.5"/>'
            )
    # 轴标注（两端）
    if jmax and math.isfinite(col_centers[0]) and math.isfinite(col_centers[-1]):
        lines.append(
            f'<text x="{margin_l}" y="{h_pix - 20}" font-size="11" font-family="{SVG_FONT_FAMILY}">{xlabel_str}≈{col_centers[0]:.0f}</text>'
        )
        lines.append(
            f'<text x="{margin_l + jmax * cw - 80}" y="{h_pix - 20}" font-size="11" font-family="{SVG_FONT_FAMILY}">{col_centers[-1]:.0f}</text>'
        )
    if imax and math.isfinite(row_centers[0]) and math.isfinite(row_centers[-1]):
        lines.append(
            f'<text x="10" y="{margin_t + imax * ch / 2}" font-size="11" font-family="{SVG_FONT_FAMILY}" '
            f'transform="rotate(-90 10 {margin_t + imax * ch / 2})">{ylabel_str}</text>'
        )
    # 色标条
    bar_w, bar_h = 200, 12
    bx = w_pix - bar_w - 30
    by = margin_t
    for k in range(100):
        t = k / 99.0
        r, g, b = turbo_rgb(t)
        lines.append(
            f'<rect x="{bx + k * bar_w / 100:.2f}" y="{by}" width="{bar_w / 100 + 0.5:.2f}" '
            f'height="{bar_h}" fill="#{r:02x}{g:02x}{b:02x}"/>'
        )
    scale_txt = f"log [{vmin:.3g}, {vmax:.3g}]" if use_log else f"[{vmin:.3g}, {vmax:.3g}]"
    lines.append(
        f'<text x="{bx}" y="{by + bar_h + 18}" font-size="10" font-family="{SVG_FONT_FAMILY}">{scale_txt}</text>'
    )
    lines.append("</svg>")
    path.write_text("\n".join(lines), encoding="utf-8")


def try_matplotlib_png(
    face: str,
    fname_part: str,
    zlabel: str,
    imax: int,
    jmax: int,
    mat: list[list[float]],
    row_c: list[float],
    col_c: list[float],
    xlabel_str: str,
    ylabel_str: str,
    title_face: str,
    use_log: bool,
    out_dir: Path,
    dpi: int,
    axis_mm_min: float,
    axis_mm_max: float,
) -> bool:
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
        from matplotlib.colors import LogNorm
    except ImportError:
        return False

    setup_matplotlib_chinese_font()

    arr = np.array(mat, dtype=float)
    plot_data = np.ma.masked_invalid(arr)
    col_c_arr = np.asarray(col_c, dtype=float)
    row_c_arr = np.asarray(row_c, dtype=float)
    uc = np.sort(np.unique(col_c_arr[np.isfinite(col_c_arr)]))
    ur = np.sort(np.unique(row_c_arr[np.isfinite(row_c_arr)]))
    dc = float(np.median(np.diff(uc))) if len(uc) > 1 else 100.0
    dr = float(np.median(np.diff(ur))) if len(ur) > 1 else 100.0
    pad = 50.0
    c0, c1 = float(np.min(col_c_arr)), float(np.max(col_c_arr))
    r0, r1 = float(np.min(row_c_arr)), float(np.max(row_c_arr))
    # extent: 水平轴 = 列 j = col_c（coord[1]）；竖直轴 = 行 i = row_c（coord[0]）
    extent = (c0 - dc / 2.0 - pad, c1 + dc / 2.0 + pad, r0 - dr / 2.0 - pad, r1 + dr / 2.0 + pad)

    fig, ax = plt.subplots(figsize=(10, 8))
    pos = None
    if use_log:
        pm = (plot_data > 0) & np.isfinite(plot_data)
        if pm.any():
            vmin = float(np.min(plot_data[pm]))
            vmax = float(np.max(plot_data[pm]))
            if vmin < vmax:
                pos = ax.imshow(
                    plot_data,
                    origin="lower",
                    aspect="auto",
                    cmap="turbo",
                    norm=LogNorm(vmin=max(vmin, 1e-30), vmax=vmax),
                    extent=extent,
                )
            else:
                pos = ax.imshow(plot_data, origin="lower", aspect="auto", cmap="turbo", extent=extent)
        else:
            pos = ax.imshow(plot_data, origin="lower", aspect="auto", cmap="turbo", vmin=0, vmax=1, extent=extent)
    else:
        pos = ax.imshow(plot_data, origin="lower", aspect="auto", cmap="turbo", extent=extent)

    if pos is not None:
        fig.colorbar(pos, ax=ax, fraction=0.046, pad=0.04).set_label(zlabel)
    ax.set_xlabel(xlabel_str)
    ax.set_ylabel(ylabel_str)
    ax.set_title(f"{title_face}\n{zlabel}")
    apply_fixed_axis_mm(ax, axis_mm_min, axis_mm_max)
    fig.tight_layout()
    fig.savefig(out_dir / f"c9_{face}_{fname_part}.png", dpi=dpi)
    plt.close(fig)
    return True


def main() -> None:
    here = Path(__file__).resolve().parent
    default_csv = here / "build" / "c9外表面探测器通量与剂量.csv"
    default_out = here / "build" / "c9热图"

    ap = argparse.ArgumentParser(description="c9 外表面剂量/通量热图")
    ap.add_argument("--csv", type=Path, default=default_csv, help="输入 CSV 路径")
    ap.add_argument("--out", type=Path, default=default_out, help="输出目录")
    ap.add_argument("--dpi", type=int, default=150)
    ap.add_argument("--no-png", action="store_true", help="仅生成 SVG")
    ap.add_argument(
        "--axis-mm",
        type=float,
        nargs=2,
        metavar=("MIN", "MAX"),
        default=(DEFAULT_AXIS_MM_MIN, DEFAULT_AXIS_MM_MAX),
        help="PNG 热图横纵坐标范围 (mm)，默认 -2500 2500",
    )
    ap.add_argument(
        "--requirements",
        type=Path,
        default=None,
        help="需求参数输出.txt（解析源强）；默认同 --csv 所在目录",
    )
    ap.add_argument(
        "--no-flux-scale-by-source",
        action="store_true",
        help="不对通量列乘源强（CSV 已含 I 时使用）",
    )
    args = ap.parse_args()
    axis_lo, axis_hi = float(args.axis_mm[0]), float(args.axis_mm[1])
    if axis_hi <= axis_lo:
        raise SystemExit("--axis-mm 要求 MAX > MIN")

    if not args.csv.is_file():
        raise SystemExit(f"找不到文件: {args.csv}")

    rows = load_rows(args.csv)
    flux_factor = 1.0
    if not args.no_flux_scale_by_source:
        req_path = args.requirements if args.requirements is not None else (args.csv.parent / "需求参数输出.txt")
        if not req_path.is_file():
            raise SystemExit(f"通量乘源强需要文件: {req_path}（或加 --no-flux-scale-by-source）")
        flux_factor = parse_source_intensity_req(req_path)
        apply_flux_scale_to_rows(rows, flux_factor)
        print(f"通量列已乘以源强 I = {flux_factor:.6g} /s（来自 {req_path.name}）")

    by_face = group_face(rows)
    args.out.mkdir(parents=True, exist_ok=True)

    png_ok = 0
    svg_n = 0
    for face in ["px", "mx", "py", "my", "pz", "mz"]:
        fr = by_face[face]
        info = FACE_INFO[face]
        for col, fname_part, zlabel_base, use_log in METRICS:
            zlabel = zlabel_base
            if flux_factor != 1.0 and col in FLUX_CSV_COLUMNS:
                zlabel = f"{zlabel_base}（×源强）"
            got = build_matrix(fr, face, col)
            if got[0] is None:
                continue
            imax, jmax, mat, row_c, col_c = got
            title = f'{info["title"]} — {zlabel}'
            svg_path = args.out / f"c9_{face}_{fname_part}.svg"
            write_svg(
                svg_path,
                title,
                zlabel,
                mat,
                row_c,
                col_c,
                info["axes"][1],
                info["axes"][0],
                use_log,
            )
            svg_n += 1
            if not args.no_png:
                if try_matplotlib_png(
                    face,
                    fname_part,
                    zlabel,
                    imax,
                    jmax,
                    mat,
                    row_c,
                    col_c,
                    info["axes"][1],
                    info["axes"][0],
                    info["title"],
                    use_log,
                    args.out,
                    args.dpi,
                    axis_lo,
                    axis_hi,
                ):
                    png_ok += 1

    # 六面汇总 PNG（需 matplotlib）
    if not args.no_png:
        try:
            import matplotlib

            matplotlib.use("Agg")
            import matplotlib.pyplot as plt
            import numpy as np
            from matplotlib.colors import LogNorm
        except ImportError:
            print(f"已生成 {svg_n} 个 SVG。" + ("未检测到 matplotlib，跳过 PNG。" if png_ok == 0 else ""))
            print("完成。")
            return

        setup_matplotlib_chinese_font()

        for col, fname_part, zlabel_base, use_log in METRICS:
            zlabel = zlabel_base
            if flux_factor != 1.0 and col in FLUX_CSV_COLUMNS:
                zlabel = f"{zlabel_base}（×源强）"
            fig, axes = plt.subplots(2, 3, figsize=(16, 10))
            axes = axes.ravel()
            for idx, face in enumerate(["px", "mx", "py", "my", "pz", "mz"]):
                ax = axes[idx]
                got = build_matrix(by_face[face], face, col)
                if got[0] is None:
                    ax.set_visible(False)
                    continue
                _, _, mat, row_c, col_c = got
                arr = np.array(mat, dtype=float)
                plot_data = np.ma.masked_invalid(arr)
                col_c_arr = np.asarray(col_c, dtype=float)
                row_c_arr = np.asarray(row_c, dtype=float)
                uc = np.sort(np.unique(col_c_arr[np.isfinite(col_c_arr)]))
                ur = np.sort(np.unique(row_c_arr[np.isfinite(row_c_arr)]))
                dc = float(np.median(np.diff(uc))) if len(uc) > 1 else 100.0
                dr = float(np.median(np.diff(ur))) if len(ur) > 1 else 100.0
                pad = 50.0
                c0, c1 = float(np.min(col_c_arr)), float(np.max(col_c_arr))
                r0, r1 = float(np.min(row_c_arr)), float(np.max(row_c_arr))
                ext = (c0 - dc / 2.0 - pad, c1 + dc / 2.0 + pad, r0 - dr / 2.0 - pad, r1 + dr / 2.0 + pad)
                pm = (plot_data > 0) & np.isfinite(plot_data)
                if use_log and pm.any():
                    vmin = float(np.min(plot_data[pm]))
                    vmax = float(np.max(plot_data[pm]))
                    if vmin < vmax:
                        im = ax.imshow(
                            plot_data,
                            origin="lower",
                            aspect="auto",
                            cmap="turbo",
                            norm=LogNorm(vmin=max(vmin, 1e-30), vmax=vmax),
                            extent=ext,
                        )
                    else:
                        im = ax.imshow(plot_data, origin="lower", aspect="auto", cmap="turbo", extent=ext)
                else:
                    im = ax.imshow(plot_data, origin="lower", aspect="auto", cmap="turbo", extent=ext)
                ax.set_title(FACE_INFO[face]["title"])
                ax.set_xlabel(FACE_INFO[face]["axes"][1])
                ax.set_ylabel(FACE_INFO[face]["axes"][0])
                apply_fixed_axis_mm(ax, axis_lo, axis_hi)
                fig.colorbar(im, ax=ax, fraction=0.046)
            fig.suptitle(zlabel, fontsize=14)
            fig.tight_layout()
            fig.savefig(args.out / f"c9_六面汇总_{fname_part}.png", dpi=args.dpi)
            plt.close(fig)
            print(f"  已保存 c9_六面汇总_{fname_part}.png")

    print(f"已生成 {svg_n} 个 SVG -> {args.out}")
    if png_ok:
        print(f"另生成单面 PNG {png_ok} 张（若已安装 numpy/matplotlib）。")
    print("完成。")


if __name__ == "__main__":
    main()
