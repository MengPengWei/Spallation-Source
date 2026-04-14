#版本号1.0.0,  初步实现分块化模式

完成活化片的构建，包括
ActivationFoilBuilder:包含材料，结构。
ActivationFoilManager:调整活化片
ActivationFoilScorer：获取目标信息


================================================================================
版本号 1.1.0  新增钽靶 + 70 MeV 均匀质子束模块
================================================================================

一、新增内容
-----------
1. 钽靶几何（TantalumTarget）
   文件：
     include/detectors/TantalumTarget.hh
     src/detectors/TantalumTarget.cc
     include/constants/TtConstants.hh

   几何层次（外 → 内，均为同轴圆柱，无重叠）：
     TT_Water   冷却水筒   G4_WATER           R=25 mm, HalfL=55 mm
     TT_Clad    结构包壳   G4_STAINLESS-STEEL R=22 mm, HalfL=52 mm (壁厚 2 mm)
     TT_Target  钽靶芯     G4_Ta              R=20 mm, HalfL=50 mm

   参数（可在 TtConstants.hh 中修改后重新编译）：
     C_TT_Target_R        钽靶芯半径         20 mm
     C_TT_Target_HalfL    钽靶芯半长         50 mm
     C_TT_Clad_Thickness  包壳厚度            2 mm
     C_TT_Water_Thickness 冷却水层厚度        3 mm
     C_TT_Beam_Energy     质子束动能         70 MeV
     C_TT_Beam_Radius     束流均匀分布半径   18 mm

2. 质子束流（PrimaryGeneratorAction 扩展）
   - 粒子：proton
   - 动能：70 MeV（C_TT_Beam_Energy）
   - 横向分布：在半径 C_TT_Beam_Radius（18 mm）内均匀圆面采样
     公式：r = R * sqrt(U[0,1))，phi = 2π * U[0,1)，保证面积均匀
   - 方向：+Z 轴
   - 起点：靶面前方 1 mm 处（z = -56 mm，当 C_TT_Pos=(0,0,0) 时）
   - 束流强度：由 /run/beamOn 控制事件数；实际物理强度典型参考值 1e13 p/s，
     请在后处理时结合权重（events/s）换算。

二、如何启用钽靶模式
--------------------
步骤 1：修改编译开关
  打开 include/constants/Constants.hh，将
    constexpr G4bool is_TT = false;
  改为
    constexpr G4bool is_TT = true;

步骤 2：重新编译
  mkdir build && cd build
  cmake ..
  make -j$(nproc)

步骤 3：运行钽靶示例宏
  ./SS_V0_1_1 -m macros/tantalum_proton.mac

  宏内容简介（macros/tantalum_proton.mac）：
    /run/initialize
    /run/beamOn 1000      # 发射 1000 个 70 MeV 质子

  控制台将打印类似信息：
    [TantalumTarget] 冷却水外筒: R=25 mm, HalfL=55 mm
    [TantalumTarget] 结构包壳(SS316L): R=22 mm, HalfL=52 mm, 壁厚=2 mm
    [TantalumTarget] 钽靶芯(G4_Ta): R=20 mm, HalfL=50 mm
    [PrimaryGeneratorAction] 钽靶模式已激活
      粒子:   proton
      动能:   70 MeV
      起始Z:  -56 mm
      束流半径(均匀圆面): 18 mm

三、几何验证
-----------
- Geant4 的 G4PVPlacement 在第 7 个参数传入 true 时自动执行重叠检查。
  若有重叠，运行时会输出 WARNING: overlap detected。
- 本模块所有 G4PVPlacement 均已开启重叠检查（checkOverlaps=true）。
- 可用可视化宏 macros/init_vis.mac 在图形界面下目视确认几何层次。

四、参数调整示例
--------------
修改钽靶半径为 15 mm、包壳厚度为 3 mm：
  在 TtConstants.hh 中：
    constexpr G4double C_TT_Target_R      = 15.0 * mm;
    constexpr G4double C_TT_Clad_Thickness = 3.0 * mm;
  重新编译后即生效。

修改束流能量为 100 MeV（不改变代码，使用宏覆盖）：
  在宏文件中可添加（需 is_TT=false 时使用默认粒子枪模式）：
    /gun/energy 100 MeV

五、与其他组件的兼容性
----------------------
- is_TT=true 时可与 is_AF=true、is_CSS=true 同时启用；
  各组件位置：
    TT  (0, 0, 0)      – 钽靶，束流沿 +Z 方向
    AF  (0, 0, 100 mm) – 活化片胶囊，旋转 90° 沿 X 轴
    CSS (1 m, 0, 100 mm) – 康普顿抑制探测系统
  三者不重叠。
- is_TT=true 时束流自动切换为 70 MeV 质子；
  is_TT=false 时保持原有 1 MeV 中子束流，行为不变。
