#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从 build 目录读取 c9 外表面 CSV，生成带「_校正」后缀的新表：

1) c9外表面探测器通量与剂量_校正.csv
   - 通量两列保持原表数值（ShieldingDataCollector 写表时 gf/nf 已含 源强×径迹/(N·V)）。
   - 追加「伽马通量_再乘源强_数值」「中子通量_再乘源强_数值」= 表内通量 × 源强(每秒)，
     满足「乘源强」数值需求；若表已含 I，该两列为「通量×I」对照量，勿当物理通量。

2) c9外表面探测器能谱_伽马_校正.csv、c9外表面探测器能谱_中子_校正.csv
   - 能量下限/上限由 MeV 改为 keV；按相同 (能量下限_keV, 能量上限_keV) 合并全部 c9 外包络探测器，
     径迹与能谱计数对各探测器求和，每能道仅一行（探测器名称列为合计标识）。

用法:
  python3 postprocess_c9_corrected_csv.py [--build-dir path]
"""

from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict
from pathlib import Path

REQ_EVENTS_RE = re.compile(r"运行事件数\s*=\s*(\d+)")
REQ_SOURCE_RE = re.compile(r"源强\(每秒\)\s*=\s*([\d.eE+-]+)")
FLUX_KEYS = ("伽马通量_每平方厘米每秒", "中子通量_每平方厘米每秒")
EXTRA_FLUX_GAMMA = "伽马通量_再乘源强_数值"
EXTRA_FLUX_NEUTRON = "中子通量_再乘源强_数值"


def parse_requirements(req_path: Path) -> tuple[int, float]:
    text = req_path.read_text(encoding="utf-8", errors="replace")
    em = REQ_EVENTS_RE.search(text)
    sm = REQ_SOURCE_RE.search(text)
    if not em or not sm:
        raise SystemExit(f"无法在 {req_path} 中解析 运行事件数 或 源强(每秒)")
    return int(em.group(1)), float(sm.group(1))


def write_flux_dose_corrected(src: Path, dst: Path, i_src: float) -> None:
    with src.open(encoding="utf-8-sig", newline="") as fin, dst.open(
        "w", encoding="utf-8", newline=""
    ) as fout:
        r = csv.DictReader(fin)
        if not r.fieldnames:
            raise SystemExit(f"空表头: {src}")
        fn = list(r.fieldnames) + [EXTRA_FLUX_GAMMA, EXTRA_FLUX_NEUTRON]
        w = csv.DictWriter(fout, fieldnames=fn)
        w.writeheader()
        for row in r:
            out = {k: row.get(k, "") for k in r.fieldnames}
            g = n = ""
            try:
                gs = str(out.get(FLUX_KEYS[0], "")).strip()
                if gs != "":
                    g = f"{float(gs) * i_src:.12g}"
            except ValueError:
                pass
            try:
                ns = str(out.get(FLUX_KEYS[1], "")).strip()
                if ns != "":
                    n = f"{float(ns) * i_src:.12g}"
            except ValueError:
                pass
            out[EXTRA_FLUX_GAMMA] = g
            out[EXTRA_FLUX_NEUTRON] = n
            w.writerow(out)


def write_spectrum_corrected(src: Path, dst: Path, n_events: int) -> None:
    """按能道合并全部探测器：相同能量上下限的行合并为一行并求和。"""
    acc: defaultdict[tuple[float, float], list[float]] = defaultdict(lambda: [0.0, 0.0])
    with src.open(encoding="utf-8-sig", newline="") as fin:
        r = csv.DictReader(fin)
        for row in r:
            el = float(row["能量下限_MeV"]) * 1000.0
            eh = float(row["能量上限_MeV"]) * 1000.0
            t = float(row["径迹长度_厘米每初级粒子"])
            cum = t * n_events
            key = (el, eh)
            acc[key][0] += t
            acc[key][1] += cum

    merged_name = "c9_outer_ALL_能道合计"
    fieldnames = (
        "探测器名称",
        "能量下限_keV",
        "能量上限_keV",
        "径迹长度_各探测器求和_厘米每初级粒子",
        "能谱计数_各探测器求和_径迹累计_cm",
    )
    with dst.open("w", encoding="utf-8", newline="") as fout:
        w = csv.DictWriter(fout, fieldnames=fieldnames)
        w.writeheader()
        for (el, eh) in sorted(acc.keys(), key=lambda k: (k[0], k[1])):
            s0, s1 = acc[(el, eh)]
            w.writerow(
                {
                    "探测器名称": merged_name,
                    "能量下限_keV": f"{el:.12g}",
                    "能量上限_keV": f"{eh:.12g}",
                    "径迹长度_各探测器求和_厘米每初级粒子": f"{s0:.12g}",
                    "能谱计数_各探测器求和_径迹累计_cm": f"{s1:.12g}",
                }
            )


def main() -> None:
    ap = argparse.ArgumentParser(description="c9 外表面通量/能谱 CSV 校正输出（_校正 后缀）")
    ap.add_argument(
        "--build-dir",
        type=Path,
        default=Path(__file__).resolve().parent / "build",
        help="含 CSV 与 需求参数输出.txt 的 build 目录",
    )
    args = ap.parse_args()
    build = args.build_dir.resolve()
    req = build / "需求参数输出.txt"
    n_events, i_src = parse_requirements(req)

    flux_in = build / "c9外表面探测器通量与剂量.csv"
    flux_out = build / "c9外表面探测器通量与剂量_校正.csv"
    g_in = build / "c9外表面探测器能谱_伽马.csv"
    g_out = build / "c9外表面探测器能谱_伽马_校正.csv"
    n_in = build / "c9外表面探测器能谱_中子.csv"
    n_out = build / "c9外表面探测器能谱_中子_校正.csv"

    for p in (flux_in, g_in, n_in):
        if not p.is_file():
            raise SystemExit(f"缺少输入文件: {p}")

    write_flux_dose_corrected(flux_in, flux_out, i_src)
    write_spectrum_corrected(g_in, g_out, n_events)
    write_spectrum_corrected(n_in, n_out, n_events)

    print(f"运行事件数={n_events}, 源强(1/s)={i_src}")
    print("通量：原两列保留；已追加 伽马/中子通量_再乘源强_数值。")
    print(f"已写入: {flux_out}")
    print(f"已写入（能谱按能道合并）: {g_out}")
    print(f"已写入（能谱按能道合并）: {n_out}")


if __name__ == "__main__":
    main()
