#!/usr/bin/env python3
"""Post-process Geant4 shielding CSV outputs into requirement summaries."""

from __future__ import annotations

import csv
import math
from pathlib import Path


def read_probe_points(path: Path) -> dict[str, dict[str, float]]:
    out: dict[str, dict[str, float]] = {}
    if not path.exists():
        return out
    with path.open("r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            name = row["probe"]
            out[name] = {
                "gamma": float(row["gamma_dose_Sv_per_h"]),
                "neutron": float(row["neutron_dose_Sv_per_h"]),
                "total": float(row["total_dose_Sv_per_h"]),
            }
    return out


def read_thermal(path: Path) -> dict[str, float]:
    power: dict[str, float] = {}
    if not path.exists():
        return power
    with path.open("r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            power[row["region"]] = float(row["power_W"])
    return power


def read_nuclides(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open("r", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def top_nuclides(records: list[dict[str, str]], category: str, n: int = 10) -> list[tuple[str, float]]:
    summed: dict[str, float] = {}
    for r in records:
        if r.get("category") != category:
            continue
        nuclide = r["nuclide"]
        a1y = float(r["activity_1y_Bq"])
        summed[nuclide] = summed.get(nuclide, 0.0) + a1y
    return sorted(summed.items(), key=lambda x: x[1], reverse=True)[:n]


def fmt(v: float) -> str:
    if v == 0.0:
        return "0"
    p = int(math.floor(math.log10(abs(v))))
    if -2 <= p <= 3:
        return f"{v:.6f}"
    return f"{v:.6e}"


def main() -> None:
    root = Path(".")
    probes = read_probe_points(root / "probe_points.csv")
    thermal = read_thermal(root / "thermal_load.csv")
    nuclides = read_nuclides(root / "nuclide_inventory.csv")

    md = []
    md.append("# 2026屏蔽计算需求对照摘要")
    md.append("")
    md.append("本报告由 `postprocess_results.py` 基于本次运行输出自动生成。")
    md.append("")
    md.append("## 1) 打靶过程外部剂量率（点位）")
    md.append("")
    if probes:
        md.append("| 点位 | Gamma (Sv/h) | Neutron (Sv/h) | Total (Sv/h) |")
        md.append("|---|---:|---:|---:|")
        for name, v in probes.items():
            md.append(f"| {name} | {fmt(v['gamma'])} | {fmt(v['neutron'])} | {fmt(v['total'])} |")
    else:
        md.append("- 未找到 `probe_points.csv`。")
    md.append("")

    md.append("## 7) 屏蔽体内辐射热")
    md.append("")
    if thermal:
        total_power = sum(thermal.values())
        md.append(f"- 全系统热沉积功率合计: **{fmt(total_power)} W**")
        hottest = sorted(thermal.items(), key=lambda x: x[1], reverse=True)[:10]
        md.append("")
        md.append("| 区域 | 功率 (W) |")
        md.append("|---|---:|")
        for region, power in hottest:
            md.append(f"| {region} | {fmt(power)} |")
    else:
        md.append("- 未找到 `thermal_load.csv`。")
    md.append("")

    md.append("## 2/3/4/5/6/8) 活化与三废核素（按1年冷却活度排序）")
    md.append("")
    if nuclides:
        for cat, title in [
            ("waste_solid", "固废/屏蔽构件"),
            ("waste_liquid", "废液/冷却水"),
            ("waste_gas", "废气/空气通道"),
        ]:
            md.append(f"### {title}")
            top = top_nuclides(nuclides, cat, n=10)
            if not top:
                md.append("- 无记录。")
                md.append("")
                continue
            md.append("| 核素 | 1年冷却活度 (Bq) |")
            md.append("|---|---:|")
            for nuc, act in top:
                md.append(f"| {nuc} | {fmt(act)} |")
            md.append("")
    else:
        md.append("- 未找到 `nuclide_inventory.csv`。")
        md.append("")

    md.append("## 9/10) 热室源项与墙厚建议")
    md.append("")
    md.append("- 可基于 `probe_points.csv` 的 `hotcell_outer_30cm` 点位进行墙厚参数扫描决策。")
    md.append("- 建议配合多次运行（不同墙厚）并汇总成 `wall_scan_results.csv` 后再确定推荐厚度。")
    md.append("")

    out_path = root / "requirement_summary.md"
    out_path.write_text("\n".join(md) + "\n", encoding="utf-8")
    print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
