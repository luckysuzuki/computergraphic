# 作业 7 加分项与 CLion 使用

已实现多线程 Ray Generation，以及各向同性 GGX Microfacet 材质。基础漫反射版本保留，通过参数切换两个箱子的材质；墙面和顶灯保持原来的材质。

## 在 CLion 中直接运行

推荐打开 `D:\games101\GAMES101_Homework_S2021\cg` 为项目。该目录已有顶层 CMakeLists.txt 和 CMakePresets.json，不需要寻找作业子目录；默认构建作业 7，不引入其他作业的 OpenCV / OpenGL 依赖。

1. 在 **File → Open** 选择 `cg`，以 CMake 项目打开。此前打开整个 GAMES101 目录的配置也已指向 cg。
2. **Settings → Build, Execution, Deployment → Toolchains** 使用现有 MinGW：`D:/games101/toolchains/gcc-11.2/mingw64`。已有工具链名称 `GCC 11.2 OpenCV` 也可使用，本作业不依赖 OpenCV。
3. **Settings → Build, Execution, Deployment → CMake** 启用 `clion-debug`、`clion-release` 两个预设，执行 Reload CMake Project。它们使用 Ninja、GCC 11.2 和 C++17。
4. 右上角选择以下运行配置，点 Run；调试 Preview / Tests 可点 Debug。

| 运行配置 | 构建 | 行为 |
| --- | --- | --- |
| Homework7 Preview | Debug | 128×128、16 SPP、自动线程数、Microfacet，输出 preview.ppm |
| Homework7 Final | Release | 512×512、256 SPP、自动线程数、Microfacet，输出 microfacet.ppm |
| Homework7 Tests | Debug | 几何、BRDF、采样、线程一致性及工作线程异常检查 |

配置保存在 cg/.run，工作目录为 cg/cmake-build-debug 或 cg/cmake-build-release。输出日志会打印图片的绝对路径；PPM 是输出图像，程序不弹出 OpenCV 窗口。可以用支持 PPM 的查看器打开，仓库中的成果另外提供 PNG。

模型路径由 CMake 在编译时指定，换运行工作目录也能找到模型。运行配置把 MinGW bin 加到 PATH，以找到运行库。Debug 预设提供调试符号，GDB 位于 `D:/games101/toolchains/gcc-11.2/mingw64/bin/gdb.exe`。

如果 CLion 没有自动显示共享配置，可在 Run → Edit Configurations 新增 **CMake Application**，Target 选 RayTracing，程序参数填：

```text
--size 512 --spp 256 --threads 0 --material microfacet --roughness 0.4 --metallic 0.85 --seed 42 --output microfacet.ppm
```

工作目录设到对应的构建目录。项目也可以单独打开 Assignment7/CMakeLists.txt，使用子目录中的两个预设；共享运行配置则针对 cg 根项目。

## 参数与命令行

| 参数 | 默认值 | 含义 |
| --- | --- | --- |
| --size | 784 | 正方形图片边长 |
| --spp | 16 | 每像素路径样本数 |
| --threads | 0 | 0 使用硬件并发数，1 单线程，其他值指定线程数；最大 256，实际不超过图像行数 |
| --material | diffuse | diffuse 或 microfacet；只切换两个箱子 |
| --roughness | 0.4 | 感知粗糙度 [0.05,1]，越低高光越集中 |
| --metallic | 0.85 | 金属度 [0,1] |
| --seed | 42 | 固定像素种子；同一程序和参数下不同线程数生成相同图像 |
| --output | binary.ppm | 输出路径，相对路径按运行工作目录解释 |
| --check-scene | 关闭 | 仅验证模型加载和 BVH 初始化 |

在 cg 根目录：

```powershell
cmake --preset clion-release
cmake --build --preset clion-release --parallel
ctest --test-dir cmake-build-release --output-on-failure

.\cmake-build-release\GAMES101_Homework7_S2021\Assignment7\RayTracing.exe --size 512 --spp 256 --threads 0 --material microfacet --output microfacet.ppm
```

或在作业子目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\run.ps1 -Size 512 -Spp 256 -Threads 0 -Material microfacet -Roughness 0.4 -Metallic 0.85
```

## 多线程实现与冲突处理

Renderer 使用原子行计数器分配工作，一行只被领取一次。线程在局部变量中累加一个像素的全部样本，再一次性写入独占的 framebuffer 元素。Scene、BVH、几何与材质在渲染期间只读。

每线程使用 thread_local mt19937；按“全局种子 + 像素编号”混合后为每个像素重新播种，消除工作调度对随机序列的影响。共享进度条用互斥锁保护。线程全部 join 后才写 PPM。

工作线程异常经 exception_ptr 保存，通知其他线程停止，全部 join 后在主线程重新抛出；创建线程失败也会先回收已启动线程。不会让工作线程的异常直接触发 std::terminate。

回归检查验证 1 与 4 线程的输出逐字节一致；真实 Cornell Box 的单 / 多线程对照与计时保存在 images/bonus-validation.json。

## Microfacet 实现

使用 Cook–Torrance 反射项：

```text
V = normalize(-wi)        # 框架 wi 是射入表面的传播方向
L = normalize(wo)
H = normalize(V + L)
alpha = roughness²

D_GGX = alpha² / [π * ((N·H)² * (alpha² - 1) + 1)²]
G1(c) = 2c / [c + sqrt(alpha² + (1-alpha²)c²)]
G = G1(N·V) * G1(N·L)
F0 = (1-metallic) * Ks + metallic * Kd
F = F0 + (1-F0) * (1-V·H)^5

specular = D * F * G / [4 * (N·V) * (N·L)]
diffuse = (1-F) * (1-metallic) * Kd / π
BRDF = specular + diffuse
```

Ks 默认 0.04；金属度 1 时不加入漫反射项。将感知粗糙度映射为 alpha=roughness² 是本实现的参数约定。法线和方向归一化，背面返回零，关键分母使用 double，并限制最小粗糙度以处理近镜面数值问题。

保留 sample / pdf / eval 的原接口。Microfacet 与 DIFFUSE 共用均匀半球 sample 和方向 PDF=1/(2π)，这是作业允许的简化。未实现 GGX 重要性采样、MIS 和理想镜面 delta 材质；很低粗糙度时需要更多样本，可能出现较高噪声。

回归覆盖法向入射解析值、BRDF 互易性、粗糙度对高光的影响、方向 / PDF、一组参数的积分能量与掠射角有限性。积分能量测试是数值抽查，不代表对全部参数做严格证明。

理论参考：[PBRT：微表面理论](https://pbr-book.org/4ed/Reflection_Models/Roughness_Using_Microfacet_Theory)、[Walter 等：Microfacet Models for Refraction through Rough Surfaces](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf)。实现使用 GGX/Smith 反射与 Schlick 近似，不是该论文的完整透射模型。

## 成果

正式微表面结果：512×512、256 SPP，粗糙度 0.4、金属度 0.85、种子 42，金色短箱与银色高箱。参数、线程数和测得耗时见 images/bonus-validation.json。

![Microfacet 渲染](images/microfacet_512_256spp.png)

![较粗糙的 Microfacet 渲染](images/microfacet_rough_512_256spp.png)

第二张采用粗糙度 0.7，其他主要参数相同，用于比较高光宽度和表面反射外观。基础 DIFFUSE 图片仍保留在 README 中。

测试、构建和命令行/GDB 验证会记录实际结果；CLion 首次打开后的窗口重载与界面状态仍需要在 IDE 中确认。
