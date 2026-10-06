# 作业 7：路径追踪实现与学习指南

基础版本已实现：三角形求交、AABB 求交、BVH 遍历、直接光照、间接反射和俄罗斯轮盘赌。保留中文注释及学习材料。此版本使用 DIFFUSE 材质，未实现多线程和 Microfacet 加分项。

## 1. 构建与运行

本机使用 MinGW GCC 11.2 + Ninja，工具链位于 `D:/games101/toolchains/gcc-11.2/mingw64/bin`，源码采用 C++17 和 UTF-8。无需 OpenCV、Eigen、OpenGL。

在 `D:\games101\GAMES101_Homework_S2021\GAMES101_Homework7_S2021\Assignment7` 运行：

```powershell
# 默认 784×784、16 SPP，Release 渲染
powershell -ExecutionPolicy Bypass -File .\run.ps1

# 512×512、64 SPP
powershell -ExecutionPolicy Bypass -File .\run.ps1 -Size 512 -Spp 64

# 仅验证模型与 BVH 初始化
powershell -ExecutionPolicy Bypass -File .\run.ps1 -CheckScene

# 调试构建
powershell -ExecutionPolicy Bypass -File .\run.ps1 -Configuration Debug -Size 128 -Spp 8
```

脚本自动设置工具链 PATH，输出到对应 `cmake-build-debug/binary.ppm` 或 `cmake-build-release/binary.ppm`。模型目录由 CMake 指定，不依赖启动位置。程序不弹出窗口，PPM 是输出图片格式。

直接构建和回归检查：

```powershell
cmake --preset clion-release
cmake --build cmake-build-release --parallel
ctest --test-dir cmake-build-release --output-on-failure

# exe 直接运行时需要 MinGW bin 在终端 PATH 中
.\cmake-build-release\RayTracing.exe --size 512 --spp 64
```

手动运行时 binary.ppm 位于终端当前目录。CLion 重新加载 CMake，选择 `Homework7` / `RayTracing` 和 `clion-debug` 或 `clion-release`；程序参数可填 `--size 128 --spp 8` 预览。已有工具链名为 `GCC 11.2 OpenCV`，本作业本身不使用 OpenCV。

## 2. 已完成的实现

| 文件与函数 | 实现方式 |
| --- | --- |
| Triangle::getIntersection | 复用已有 Möller–Trumbore 计算，校验 t 区间，填写坐标、距离、法线、物体、材质和发光值 |
| Bounds3::IntersectP | slab 区间求交，明确使用 direction < 0 标记；处理零方向和平行边界，保留擦边命中 |
| BVHAccel::getIntersection | 包围盒剔除、叶节点求交、递归选择左右子树最近命中 |
| Scene::castRay | 黑色未命中、相机可见自发光、光源面积采样、有限长度阴影射线、半球间接反射、俄罗斯轮盘赌 |
| BVHAccel::Sample | 修正叶节点选择量，使用均匀面积区间而非平方根分布 |
| Scene::sampleLight | 无光源时安全返回；多光源 PDF 包含物体选择概率 |

其他修正：随机数引擎使用 thread_local，避免每次采样重建；初始化空 BVH 根节点与场景 BVH 指针；实现 BVH 析构函数释放树节点；材质非法类型有显式异常；PPM 打开失败会报错。

BVH 不拥有传入的 Object。现有 Scene 和 MeshTriangle 仍沿用框架的原始指针生命周期管理，进程结束后由系统回收未显式释放的场景资源。旧 bool intersect 接口和 Sphere::evalDiffuseColor 不属于当前 Cornell Box 的执行路径；后者仍保留未实现异常。

## 你接下来要做的事情

- [ ] 对照下面类说明阅读实现，确认四个核心函数的输入与返回值。
- [ ] 理解面积 PDF 与方向 PDF 为什么不能混用。
- [ ] 改变 SPP，对比噪声；改变 Kd，观察颜色反弹。
- [ ] 补充姓名、学号及自己的理解到文末报告模板。
- [ ] 如需要加分，另行实现多线程或 Microfacet；注意随机数与像素写入竞争。
- [ ] 若提交到课程指定环境，再在对应虚拟机中编译验证。
- [ ] 打包前排除构建目录、日志、IDE 配置和临时备份，包含所有所需源码、模型与图片。

## 3. 类说明

| 类 / 结构 | 文件 | 作用和重要接口 |
| --- | --- | --- |
| `Vector3f` | `Vector.hpp` | 三维位置、方向、RGB；点积、叉积、归一化。向量 `*` 是逐分量相乘，点积使用 `dotProduct` |
| `Vector2f` | `Vector.hpp` | 二维向量，主要用于纹理坐标 |
| `Ray` | `Ray.hpp` | origin、direction、方向倒数；`ray(t)` 得到位置。direction 归一化时求交参数 t 才等于距离；成员 `Ray::t` 是单独的时间参数 |
| `Intersection` | `Intersection.hpp` | 求交或采样结果：happened、coords、normal、distance、obj、m、emit 等 |
| `Object` | `Object.hpp` | 抽象物体接口，统一求交、面积、表面采样与发光判断；路径追踪主要用 getIntersection / Sample / hasEmit |
| `Triangle` | `Triangle.hpp` | 单三角形，保存顶点、边、法线、面积、材质；getIntersection 待补齐，Sample 已提供重心坐标采样 |
| `MeshTriangle` | `Triangle.hpp` | 从 OBJ 加载三角形集合，构建内部 BVH；Sample 会填写采样交点的 emit |
| `Sphere` | `Sphere.hpp` | 球体解析求交和采样；当前默认场景未使用 |
| `Bounds3` | `Bounds3.hpp` | 轴对齐包围盒 AABB；并集、中心、范围；IntersectP 支持 BVH 剔除 |
| `BVHBuildNode` | `BVH.hpp` | 包围盒、左右子节点、叶节点图元和子树面积；area 用于采样 |
| `BVHAccel` | `BVH.hpp` / `BVH.cpp` | recursiveBuild 构建树、getIntersection 遍历求交、Sample 对图元集合采样 |
| `Material` | `Material.hpp` | Kd、发光值及 sample / pdf / eval；当前仅 DIFFUSE，PDF 和 BRDF 是不同量 |
| `Scene` | `Scene.hpp` / `Scene.cpp` | 物体管理、场景 BVH、光源采样、路径追踪；重点 intersect、sampleLight、castRay 和 RussianRoulette |
| `Renderer` | `Renderer.hpp` / `Renderer.cpp` | 相机光线、每像素 SPP 次累加、写 PPM；相机位置 `(278,273,-800)` |
| `Light` | `Light.hpp` | 旧光源基类，保存位置与强度；当前 Scene::lights 未使用 |
| `AreaLight` | `AreaLight.hpp` | 旧面积光源接口；本场景使用发光网格 light_，不要替代 sampleLight |
| `objl::Vector2` / `objl::Vector3` | `OBJ_Loader.hpp` | 加载器向量，与渲染器 Vector3f 不同，读取 X/Y/Z 后转换 |
| `objl::Vertex` | `OBJ_Loader.hpp` | 模型顶点的位置、法线和纹理坐标 |
| `objl::Material` | `OBJ_Loader.hpp` | MTL 参数与贴图路径，与渲染器 Material 不同；当前场景直接指定渲染材质 |
| `objl::Mesh` | `OBJ_Loader.hpp` | 名称、顶点列表、索引及加载材质 |
| `objl::Loader` | `OBJ_Loader.hpp` | OBJ/MTL 解析、网格生成、多边形三角化；一般无需改动 |

`global.hpp`：随机数、clamp、二次方程和进度条。`main.cpp`：创建红、绿、白材质和顶灯，加载六个模型，调用渲染。

```text
main 创建材质与 MeshTriangle
  → 网格加载 OBJ，构建各自内部 BVH
  → Scene::buildBVH 构建场景 BVH
  → Renderer::Render 生成相机光线
  → Scene::castRay
      → Scene::intersect → 场景 BVH → 网格 BVH → Triangle::getIntersection
      → Scene::sampleLight → 发光网格 Sample → BVH::Sample → Triangle::Sample
      → Material::sample / pdf / eval
      → 间接反射递归 castRay
  → 保存 binary.ppm
```

## 4. 光照公式与代码对照

令 p 为表面点、N 为其法线、x 为光源采样点、Nl 为光源法线，`ws = normalize(x-p)`。

```text
L_direct = emit * BRDF * max(0,N·ws) * max(0,Nl·(-ws))
           / |x-p|² / pdf_area

L_indirect = castRay(next_ray) * BRDF * max(0,N·next_dir)
             / pdf_direction / P_RR
```

当前漫反射 BRDF 为 Kd/π，均匀半球 PDF 为 1/(2π)。BRDF 不自带余弦，只在积分里乘一次。面积采样公式中的两个余弦和距离平方用于几何转换，不要再次转换 PDF。

`ray.direction` 是光线传播方向；朝向上一顶点的是它的相反方向。当前 DIFFUSE 不依赖 eval 的第一个方向参数，但第二个参数必须位于法线正半球。

castRay 对每次命中先算直接光，再以默认 0.8 的概率继续间接路径；存活贡献除以 0.8。P_RR 必须在 (0,1) 内，避免永不终止或除零。maxDepth 保留为旧参数，不用于截断当前积分。

相机直接看到发光表面时返回自发光。递归路径直接命中光源时返回零，因为该项已经通过当前顶点的直接光源采样估计，避免重复计数；这属于本作业基础估计器，未引入 MIS。

阴影射线从 `p + N*0.001` 出发，重新计算到光源点的方向与距离，t_max 截止到光源之前。求交函数遵守 t 区间，线段内存在最近交点就认为遮挡。偏移数值针对 Cornell Box 约 555 的尺寸，换成不同尺度的模型时应重新评估。

Triangle::Sample 中平方根用于三角形内部均匀面积采样，应保留；BVH::Sample 中叶节点面积选择不使用平方根。这两种采样是不同步骤。

多光源时联合 PDF 为“选择该物体的概率 × 在物体内部采样的条件密度”。没有光源时直接项为零。

## 5. 验证与结果

Debug 和 Release 的 geometry-and-sampling 回归检查覆盖：正负光线方向、平行边界、擦边、光线后方、有限长度区间、三角形元数据与背面剔除、BVH 最近交点与空树、等面积选择频率、三角形采样重心、多光源联合 PDF、空场景和自发光处理。

采样检查使用 20000 个随机点并设置统计容差，验证采样分布，不仅检查函数能执行。渲染图另外用于检查几何、阴影和红绿反弹；这不等于对整个积分器作严格无偏性证明。

最终结果：512×512、64 SPP，Release，单线程，P_RR=0.8。满足本地作业 PDF 的至少 512×512、至少 8 SPP 参数要求。随机采样仍会产生噪声，增加 SPP 可以减轻噪声。当前相机采样固定于像素中心，未加入像素内抗锯齿抖动。

![Cornell Box 路径追踪结果](images/cornellbox_512_64spp.png)

原始 PPM 与 PNG 同时保存在 images/。渲染时长见 `images/render-summary.txt`，不同机器可能有差异。

常见问题：全黑时核对光源法线和遮挡；亮斑时检查 PDF 和自相交；耗时过长时确认使用 Release。旧接口尚未实现的提示只与扩展使用旧球体流程有关，当前场景不会触发。

## 6. 相关材料

1. [本地作业 PDF](../Assignment7.pdf)：重点读 2.2（作业 6 迁移）、3.2（伪代码）、第 5 节（提交评分）。中文直接提取可能乱码，要求已对照渲染页面核对。
2. [GAMES101 官方课程页](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)：复习第 13 讲加速结构、第 14 讲渲染方程、第 15 讲蒙特卡洛与路径追踪、第 17 讲材质，页面包含课件链接。
3. [第 15 讲课件](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_15.pdf)：对照光源面积采样、方向采样与 PDF。
4. [第 17 讲课件](https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES101_Lecture_17.pdf)：用于可选 Microfacet 扩展。
5. [PBRT 第四版：Path Tracing](https://pbr-book.org/4ed/Light_Transport_I_Surface_Reflection/Path_Tracing)：基础出图后读路径累积与轮盘赌重加权，完整渲染器接口比本作业复杂。

建议顺序：渲染方程 → 蒙特卡洛“贡献除以 PDF” → 面积积分几何项 → 半球采样 → 俄罗斯轮盘赌；基础正确后再学 MIS、重要性采样和微表面模型。

## 7. 提交报告模板

- 姓名 / 学号：待你填写。
- 基础项：已实现三种求交与路径追踪；环境和结果已验证。
- 输出参数：512×512，64 SPP，P_RR=0.8，DIFFUSE，单线程。
- 计算时间：见 images/render-summary.txt。
- 加分项：未实现多线程与 Microfacet。
- 实现理解与学习总结：待你填写。

本地 2021 年 PDF 的基础路径追踪为 45 分、格式与可编译运行为 5 分，多线程及 Microfacet 各为 10 分加分项。评分由课程方决定。

提交包含 CMakeLists.txt、tests.cpp（默认测试目标使用）、所有所需源码与模型、README.md、images/。排除 cmake-build-*、IDE 缓存、日志和备份。PDF 指定包名为 `姓名_Homework7.zip`。Windows 构建验证不能替代课程指定的虚拟机验证；跨平台构建可用普通 CMake，不依赖本机预设。

前次配置阶段的原文件备份在仓库根目录 `.codex-backups/hw7-before-setup`。作业 8 仍未修改，等你提出时再按既定流程配置与翻译。
