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

================================================================================
版本号 1.2.0  新增双 source 探测器粒子记录（step/track/event/run）+ ROOT 输出
================================================================================

一、新增功能概述
----------------
在 is_TT=true && PG_Is_ProtonGun=true 模式下，自动在钽靶组件两侧构建两个薄圆盘
探测器（材料：空气，R=5 cm，厚度 10 mm），并记录粒子信息写入 ROOT 文件：

  - 质子探测器（SD_LV_Proton）：位于质子枪上方 1 cm 处
    * 记录质子的逐步位置、动能、能量沉积、动量（Ntuple ProtonDetStep）
    * 运行时填充 TH3D 三维位置热力图 hProton3D

  - 中子探测器（SD_LV_Neutron）：位于钽靶顶面上方 5 cm 处
    * 记录中子的逐步位置、动能、动量（Ntuple NeutronDetStep）
    * 运行时在中子首次进入时填充：
        TH1D 动能谱  hNeutronSpectrum（200 bins, 0–100 MeV）
        TH2D 产额热力图 hNeutronYield2D（x-y, 50×50 bins）

  - 每事件汇总（Ntuple SourceDetEvent）：
      eventId, nProtonSteps, nNeutronEntries, protonEdep_MeV

二、生成的 ROOT 文件结构
------------------------
文件名：activation_output.root（与现有输出合并，位于构建目录）

  TTrees（Ntuples）：
    NeutronEntry       – 原有：中子进入样品
    NeutronInteraction – 原有：样品内中子相互作用步骤
    Secondary          – 原有：次级粒子
    EventSummary       – 原有：每事件每样品汇总
    ProtonDetStep      – 新增：质子探测器质子逐步信息
    NeutronDetStep     – 新增：中子探测器中子逐步信息
    SourceDetEvent     – 新增：每事件 source 探测器汇总

  直方图：
    hNeutronSpectrum   – 1D：中子动能谱（运行时填充）
    hNeutronYield2D    – 2D：中子产额 x-y 热力图（运行时填充）
    hProton3D          – 3D：质子位置热力图（运行时填充）

三、如何运行
------------
步骤 1：确认编译开关（include/constants/Constants.hh）
  constexpr G4bool C_Is_TT = true;

  确认质子枪开关（include/gun/PgConstants.hh）
  constexpr G4bool PG_Is_ProtonGun = true;

步骤 2：重新编译
  mkdir build && cd build
  cmake .. && make -j$(nproc)

步骤 3：运行模拟
  ./SS_V0_1_1 -m macros/tantalum_proton.mac

  运行后在构建目录生成 activation_output.root（或带线程后缀的分文件，
  G4RootAnalysisManager 会自动合并）。

四、分析脚本
------------
分析脚本位于 analysis/ 目录，从 ROOT 文件读取数据并保存图片：

  C++ ROOT macro（推荐）：
    cd build
    root -l -q '../analysis/plot_detectors.C("activation_output.root")'

  Python + PyROOT：
    cd build
    python3 ../analysis/plot_detectors.py activation_output.root

  输出图片（保存在当前工作目录）：
    proton_3D_heatmap.png   – 质子探测器内质子 3D 热力分布（三视图）
    neutron_yield_2D.png    – 中子探测器内中子产额热力图（x-y）
    neutron_spectrum.png    – 中子探测器内中子动能谱（对数纵轴）

  前提：ROOT 已安装且已 source thisroot.sh，或 Geant4 构建时已启用 ROOT 支持。

五、相关代码文件
----------------
  探测器几何：
    src/detectors/SourceDetector.cc     – 逻辑体名称改为 "SD_LV_Proton"/"SD_LV_Neutron"
    src/worldconstruction/DetectorConstruction.cc – 构建并保存 LV 指针

  数据记录（step/track/event/run）：
    src/actions/SteppingAction.cc       – 按 LV 名称识别探测器，填充 Ntuple+直方图
    src/actions/EventAction.cc          – 每事件累加质子步数、中子进入次数
    src/actions/TrackingAction.cc       – 统计钽靶内产生的次级粒子（Run 级别）
    src/actions/RunAction.cc            – BeginOfRunAction 中定义新 Ntuple 和直方图

  接口头文件：
    include/worldconstruction/DetectorConstruction.hh – 新增 Get_LV_ProtonDet/NeutronDet
    include/actions/EventAction.hh      – 新增 AddProtonStep/AddNeutronEntry

