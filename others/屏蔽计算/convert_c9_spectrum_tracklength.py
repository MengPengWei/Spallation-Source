#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将 c9 外表面能谱 CSV 中的「径迹长度_厘米每初级粒子」转化为：
  - 径迹长度累计_cm（全模拟在本能道的总 cm）
  - 能谱份额（按径迹长度归一，等价于“计数型”能谱的相对计数）
  - 分能道通量估算（与 ShieldingDataCollector 中 track-length 通量一致：源强×径迹/体积）

用法:
  python3 convert_c9_spectrum_tracklength.py [--build-dir path]
"""

from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict
from pathlib import Path

REQ_EVENTS_RE = re.compile(r"运行事件数\s*=\s*(\d+)")
REQ_SOURCE_RE = re.compile(r"源强\(每秒\)\s*=\s*([\d.eE+-]+)")


def parse_requirements(req_path: Path) -> tuple[int, float]:
    text = req_path.read_text(encoding="utf-8", errors="replace")
    em = REQ_EVENTS_RE.search(text)
    sm = REQ_SOURCE_RE.search(text)
    if not em or not sm:
        raise SystemExit(f"无法在 {req_path} 中解析 运行事件数 或 源强(每秒)")
    return int(em.group(1)), float(sm.group(1))


def load_probe_volume_cm3(flux_csv: Path) -> dict[str, float]:
    out: dict[str, float] = {}
    with flux_csv.open(encoding="utf-8-sig", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            name = row["探测器名称"].strip()
            out[name] = float(row["计分体积_cm3"])
    return out


def sum_track_per_probe(spec_csv: Path, col_track: str) -> dict[str, float]:
    sums: dict[str, float] = defaultdict(float)
    with spec_csv.open(encoding="utf-8-sig", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            sums[row["探测器名称"].strip()] += float(row[col_track])
    return dict(sums)


def convert_file(
    spec_csv: Path,
    out_csv: Path,
    n_events: int,
    source_per_s: float,
    vol_map: dict[str, float],
    col_track: str = "径迹长度_厘米每初级粒子",
) -> None:
    sums = sum_track_per_probe(spec_csv, col_track)
    extra_fields = [
        "径迹长度累计_cm",
        "能谱份额_径迹长度归一",
        "分能道通量估算_每平方厘米每秒",
    ]
    with spec_csv.open(encoding="utf-8-sig", newline="") as fin, out_csv.open(
        "w", encoding="utf-8", newline=""
    ) as fout:
        r = csv.DictReader(fin)
        fieldnames = list(r.fieldnames or []) + extra_fields
        w = csv.DictWriter(fout, fieldnames=fieldnames)
        w.writeheader()
        for row in r:
            name = row["探测器名称"].strip()
            t = float(row[col_track])
            v = vol_map.get(name)
            if v is None:
                raise KeyError(f"计分体积表中缺少探测器: {name}")
            st = sums.get(name, 0.0)
            cum = t * n_events
            frac = (t / st) if st > 0.0 else 0.0
            flux_bin = source_per_s * t / v
            row["径迹长度累计_cm"] = f"{cum:.12g}"
            row["能谱份额_径迹长度归一"] = f"{frac:.12g}"
            row["分能道通量估算_每平方厘米每秒"] = f"{flux_bin:.12g}"
            w.writerow(row)


def main() -> None:
    ap = argparse.ArgumentParser(description="c9 能谱径迹长度列转化为份额与分能道通量")
    ap.add_argument(
        "--build-dir",
        type=Path,
        default=Path(__file__).resolve().parent / "build",
        help="含 CSV 的 build 目录",
    )
    args = ap.parse_args()
    build = args.build_dir.resolve()
    req = build / "需求参数输出.txt"
    flux_csv = build / "c9外表面探测器通量与剂量.csv"
    g_in = build / "c9外表面探测器能谱_伽马.csv"
    n_in = build / "c9外表面探测器能谱_中子.csv"
    g_out = build / "c9外表面探测器能谱_伽马_转化.csv"
    n_out = build / "c9外表面探测器能谱_中子_转化.csv"

    n_events, i_src = parse_requirements(req)
    vol_map = load_probe_volume_cm3(flux_csv)

    convert_file(g_in, g_out, n_events, i_src, vol_map)
    convert_file(n_in, n_out, n_events, i_src, vol_map)
    print(f"事件数={n_events}, 源强(1/s)={i_src}")
    print(f"已写入: {g_out}")
    print(f"已写入: {n_out}")


if __name__ == "__main__":
    main()
