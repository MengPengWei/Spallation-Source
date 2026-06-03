#!/usr/bin/env bash
set -euo pipefail

exe="${1:-./build/Hadr07}"
events="${2:-20000}"

if [[ ! -x "${exe}" ]]; then
  echo "Executable not found: ${exe}" >&2
  exit 1
fi

rm -f wall_scan_results.csv
echo "wall_thickness_cm,gamma_dose_Sv_per_h,neutron_dose_Sv_per_h,total_dose_Sv_per_h" \
  > wall_scan_results.csv
mkdir -p wall_scan_outputs

for t in 80 100 120 140 160 180 200 220 240; do
  cat > run_wall_scan_tmp.mac <<EOF
/control/verbose 0
/run/verbose 1
/event/verbose 0
/tracking/verbose 0
/shield/doorOpen false
/shield/hotcellWallThickness ${t} cm
/run/initialize
/shield/gun/setDefault
/shield/gun/beamRadius 25 mm
/shield/gun/sourceIntensity 1.05238e15
/shield/gun/irradiationHours 1000
/run/beamOn ${events}
EOF

  "${exe}" run_wall_scan_tmp.mac > "wall_scan_${t}.log" 2>&1
  if [[ -f probe_points.csv ]]; then
    cp -f probe_points.csv "wall_scan_outputs/probe_points_${t}cm.csv"
    row="$(awk -F',' '$1=="hotcell_outer_30cm" {print $2","$3","$4}' probe_points.csv)"
    if [[ -n "${row}" ]]; then
      echo "${t},${row}" >> wall_scan_results.csv
    fi
  fi
done

rm -f run_wall_scan_tmp.mac
python3 ./postprocess_results.py || true
python3 - <<'PY'
import csv
from pathlib import Path

path = Path("wall_scan_results.csv")
rows = []
if path.exists():
    with path.open("r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

out = ["# 墙厚扫描摘要", ""]
if not rows:
    out.append("- 未获取到扫描结果。")
else:
    out.append("| 墙厚(cm) | Gamma(Sv/h) | Neutron(Sv/h) | Total(Sv/h) |")
    out.append("|---:|---:|---:|---:|")
    for r in rows:
        out.append(
            f"| {r['wall_thickness_cm']} | {r['gamma_dose_Sv_per_h']} | "
            f"{r['neutron_dose_Sv_per_h']} | {r['total_dose_Sv_per_h']} |"
        )
    best = min(rows, key=lambda r: float(r["total_dose_Sv_per_h"]))
    out.append("")
    out.append(
        f"- 最小总剂量对应墙厚: **{best['wall_thickness_cm']} cm**, "
        f"Total = **{best['total_dose_Sv_per_h']} Sv/h**"
    )

Path("wall_scan_summary.md").write_text("\n".join(out) + "\n", encoding="utf-8")
print("Wrote wall_scan_summary.md")
PY
echo "Wall scan completed: wall_scan_results.csv"
