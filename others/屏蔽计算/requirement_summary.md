# 2026屏蔽计算需求对照摘要

本报告由 `postprocess_results.py` 基于本次运行输出自动生成。

## 1) 打靶过程外部剂量率（点位）

| 点位 | Gamma (Sv/h) | Neutron (Sv/h) | Total (Sv/h) |
|---|---:|---:|---:|
| shield_outer_30cm | 0 | 0 | 0 |
| hotcell_outer_30cm | 0 | 0 | 0 |
| channel_22_exit | 0 | 0 | 0 |
| channel_23_exit | 0 | 0 | 0 |
| channel_24_exit | 0 | 0 | 0 |
| door_operation_point | 0 | 0 | 0 |

## 7) 屏蔽体内辐射热

- 全系统热沉积功率合计: **6.951480e+16 W**

| 区域 | 功率 (W) |
|---|---:|
| shield_iron_inner | 6.861960e+16 |
| target_air_core | 6.703780e+14 |
| shield_graphite | 9.349390e+13 |
| shield_bpe | 6.634730e+13 |
| shield_lead_inner | 6.498390e+13 |

## 2/3/4/5/6/8) 活化与三废核素（按1年冷却活度排序）

### 固废/屏蔽构件
| 核素 | 1年冷却活度 (Bq) |
|---|---:|
| Fe55 | 1.161210e+12 |
| C12 | 0 |
| deuteron | 0 |
| proton | 0 |
| C13 | 0 |
| Fe56 | 0 |
| Mn55 | 0 |
| Pb206 | 0 |
| Pb207 | 0 |
| Pb208 | 0 |

### 废液/冷却水
- 无记录。

### 废气/空气通道
- 无记录。

## 9/10) 热室源项与墙厚建议

- 可基于 `probe_points.csv` 的 `hotcell_outer_30cm` 点位进行墙厚参数扫描决策。
- 建议配合多次运行（不同墙厚）并汇总成 `wall_scan_results.csv` 后再确定推荐厚度。

