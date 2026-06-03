# Geant4 屏蔽计算运行手册

## 1. 工程位置

- 主工程: `examples/extended/hadronic/Hadr07`
- 输入参考: `/home/geant4/LTS/屏蔽计算/test2026.i`
- 需求参考: `/home/geant4/LTS/屏蔽计算/2026屏蔽计算需求.pdf`

## 2. 已实现能力

- 70 MeV 质子源、50 mm 圆形束斑（GPS）。
- `n/gamma` 网格剂量 + 点位剂量 + 点位能谱输出。
- 屏蔽体分区热沉积功率输出。
- 同位素产额与冷却时窗活度输出（EOB、1d、30d、1y、5y）。
- 开门/闭门工况切换：`/shield/doorOpen true|false`。
- 热室墙厚参数扫描变量：`/shield/hotcellWallThickness <value> cm`。

## 3. 关键输出文件

- `prompt_mesh_dose.csv`
- `probe_points.csv`
- `probe_spectra.csv`
- `thermal_load.csv`
- `nuclide_inventory.csv`
- `shielding_summary_requirements.md`
- `requirement_summary.md`（由 `postprocess_results.py` 生成）

## 4. 运行命令

### 4.1 构建

```bash
cmake -S . -B build -DWITH_GEANT4_UIVIS=OFF
cmake --build build -j4
```

### 4.2 工况宏

- Prompt: `./build/Hadr07 run_prompt.mac`
- 残余（闭门）: `./build/Hadr07 run_residual_closed.mac`
- 残余（开门）: `./build/Hadr07 run_residual_open.mac`
- 墙厚扫描: `./run_wall_scan.sh ./build/Hadr07 20000`

### 4.3 后处理

```bash
python3 ./postprocess_results.py
```

## 5. 本机已知环境限制

当前机器 Geant4 CMake 包依赖 Qt5 组件（`Qt5Core/Gui/Widgets/OpenGL/PrintSupport`）。
若系统未安装对应 Qt5 开发包，`cmake` 配置将失败。

建议二选一：

1. 安装 Qt5 开发依赖后重新构建；
2. 使用不依赖 Qt 的 Geant4 安装（或在该安装下重新编译 Geant4）。
