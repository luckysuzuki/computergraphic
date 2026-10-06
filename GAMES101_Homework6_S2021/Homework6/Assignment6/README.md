# 作业 6：BVH 加速与 SAH

## 涉及的问题

- 每条光线遍历所有三角形会产生大量求交，如何用空间层次结构减少检查？
- 如何正确处理 AABB 的进入、离开区间，以及光线方向正负？
- BVH 左右子树都命中时，如何选择最近表面？
- 如何使用 SAH 在构建树时权衡包围盒面积与图元数量？

## 代码中的处理

Bounds3 提供轴对齐包围盒求交。BVH 的内部节点先检查包围盒，叶节点执行物体精确求交，再返回最近命中。普通模式按最长轴排序并中分；SAH 模式在三个轴上使用前缀、后缀包围盒，尝试有效分割位置并选择最低成本，遇到退化情况回退中分。

注意本作业 dirIsNeg 的现有约定为方向分量 > 0，构造标记与包围盒实现保持一致即可，不应直接复制作业 7 的相反约定。场景级 BVH 选择 SAH；是否每个网格内部都使用 SAH，应按 MeshTriangle 的实际构造参数判断。

## 成果

源码包含基础 BVH 求交和 SAH 构建分支；归档已有 1280×960 bunny 渲染输出。尚未记录统一条件下的中分/SAH 计时对比，因此不声称具体加速倍数。

![BVH bunny 渲染](images/bunny.png)

## 运行

需要 C++17 和 CMake，不依赖 OpenCV。

```powershell
cmake -S . -B build
cmake --build build
cd build
.\RayTracing.exe
```

从 build 目录启动以匹配 `../models/bunny/bunny.obj`。输出 `binary.ppm`。
