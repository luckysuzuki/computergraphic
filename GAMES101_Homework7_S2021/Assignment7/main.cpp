#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include <string>

#ifndef HW7_MODEL_DIR
#define HW7_MODEL_DIR "../models/cornellbox"
#endif

// 程序入口负责构建场景（创建物体和光源），
// 并设置渲染参数（图像宽高、
// 最大递归深度、视场角等），然后调用
// 渲染函数。
int main(int argc, char** argv)
{

    // 修改此处的宽高可以调整图像分辨率
    Scene scene(784, 784);
    // 可用 --size N --spp N 调整预览或最终渲染参数。
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--check-scene") continue;
            if ((argument != "--size" && argument != "--spp") || i + 1 >= argc)
                throw std::invalid_argument("Usage: RayTracing [--size N] [--spp N] [--check-scene]");
            const std::string value = argv[++i];
            size_t parsed = 0;
            const int count = std::stoi(value, &parsed);
            if (parsed != value.size() || count <= 0 || count > 8192)
                throw std::invalid_argument("size and spp must be integers in [1, 8192]");
            if (argument == "--size") scene.width = scene.height = count;
            else scene.spp = count;
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    light->Kd = Vector3f(0.65f);

    MeshTriangle floor(HW7_MODEL_DIR "/floor.obj", white);
    MeshTriangle shortbox(HW7_MODEL_DIR "/shortbox.obj", white);
    MeshTriangle tallbox(HW7_MODEL_DIR "/tallbox.obj", white);
    MeshTriangle left(HW7_MODEL_DIR "/left.obj", red);
    MeshTriangle right(HW7_MODEL_DIR "/right.obj", green);
    MeshTriangle light_(HW7_MODEL_DIR "/light.obj", light);

    scene.Add(&floor);
    scene.Add(&shortbox);
    scene.Add(&tallbox);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.buildBVH();

    // 仅检查模型加载及 BVH 构建，便于在求交和路径追踪未完成时验证环境。
    bool checkScene = false;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--check-scene") checkScene = true;
    if (checkScene) {
        std::cout << "Scene check passed: " << scene.objects.size() << " meshes loaded.\n";
        return 0;
    }

    Renderer r;

    auto start = std::chrono::system_clock::now();
    try {
        r.Render(scene);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}
