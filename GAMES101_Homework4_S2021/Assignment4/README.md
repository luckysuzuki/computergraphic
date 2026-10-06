# 作业 4：贝塞尔曲线

## 涉及的问题

- 如何通过四个控制点构造一条三次贝塞尔曲线？
- 怎样将 de Casteljau 算法转化为递归程序？
- 连续的曲线坐标怎样落到离散的图像像素上？

## 代码中的处理

`recursive_bezier` 对相邻控制点进行 `(1-t)*P_i + t*P_(i+1)` 线性插值，再对新点列表递归，直到只剩一个点。`bezier` 在 [0,1] 内采样 t，把曲线绘制到图像。程序用 OpenCV 获取鼠标点击的四个控制点。

文件保留 `naive_bezier` 的三次多项式计算作为参考，当前主流程调用递归版本。当前绘制使用像素取整，尚未实现抗锯齿扩展。

## 成果

归档原运行生成的 700×700 图片，展示四个控制点和绿色贝塞尔曲线。

![贝塞尔曲线](images/bezier.png)

## 运行

需要 C++17 和 OpenCV；CMake 中 OpenCV_DIR 是本机路径，换机器时需调整。

```powershell
cmake -S . -B build
cmake --build build
cd build
.\BezierCurve.exe
```

点击四个控制点后生成 `my_bezier_curve.png`。
