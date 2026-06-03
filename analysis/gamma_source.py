# gamma_source.py 最终无错版
import matplotlib
matplotlib.use('Agg')  # 屏蔽3D冲突 + 无界面运行
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# ====================== 路径 ======================
excel_path = "中子能谱数据.xlsx"

# ====================== 读取并清洗数据 ======================
df = pd.read_excel(excel_path)
df.columns = df.columns.str.strip()

# 只保留纯数字行（去掉“总”这一行）
df = df[pd.to_numeric(df["能群边界(MeV)"], errors='coerce').notna()]

# 读取数据（已匹配你的真实列名）
energy = pd.to_numeric(df["能群边界(MeV)"], errors='coerce')
flux_empty = pd.to_numeric(df["无材料"], errors='coerce')
err_empty = pd.to_numeric(df["统计偏差"], errors='coerce')
flux_clf1 = pd.to_numeric(df["CLF-1（5*5*5cm）"], errors='coerce')
err_clf1 = pd.to_numeric(df["统计偏差.1"], errors='coerce')

# 计算绝对误差
yerr_empty = flux_empty * err_empty
yerr_clf1 = flux_clf1 * err_clf1

# ====================== 绘图 ======================
plt.rcParams["axes.unicode_minus"] = False
plt.rcParams["figure.dpi"] = 300

fig, ax = plt.subplots(figsize=(10, 6))

ax.errorbar(energy, flux_empty, yerr=yerr_empty,
            label="无材料", marker="o", capsize=3, color="#1f77b4")
ax.errorbar(energy, flux_clf1, yerr=yerr_clf1,
            label="CLF-1 5×5×5cm", marker="s", capsize=3, color="#ff7f0e")

ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlabel("Energy (MeV)")
ax.set_ylabel("Flux")
ax.set_title("Neutron / Gamma Spectrum")
ax.legend()
ax.grid(alpha=0.3, linestyle='--')

plt.tight_layout()
plt.savefig("spectrum.png", bbox_inches="tight")
print("✅ 绘图完成！已生成：spectrum.png")