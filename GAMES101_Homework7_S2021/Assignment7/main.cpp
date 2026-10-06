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
    bool useMicrofacet = false;
    bool checkScene = false;
    float roughness = 0.4f, metallic = 0.85f;
    // 所有选项都可直接填入 CLion 的程序参数栏。
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--check-scene") { checkScene = true; continue; }
            if (argument == "--help") {
                std::cout << "RayTracing [--size N] [--spp N] [--threads N (0=auto)]\n"
                          << "  [--material diffuse|microfacet] [--roughness 0.05..1]\n"
                          << "  [--metallic 0..1] [--seed N] [--output path.ppm] [--check-scene]\n";
                return 0;
            }
            if (i + 1 >= argc) throw std::invalid_argument("Missing value for " + argument);
            const std::string value = argv[++i];
            if (argument == "--output") {
                if (value.empty()) throw std::invalid_argument("Output path must not be empty");
                scene.outputPath = value;
                continue;
            }
            if (argument == "--material") {
                if (value != "diffuse" && value != "microfacet")
                    throw std::invalid_argument("Material must be diffuse or microfacet");
                useMicrofacet = value == "microfacet";
                continue;
            }
            size_t parsed = 0;
            if (argument == "--roughness" || argument == "--metallic") {
                const float number = std::stof(value, &parsed);
                const float minimum = argument == "--roughness" ? .05f : 0.f;
                if (parsed != value.size() || !std::isfinite(number) || number < minimum || number > 1)
                    throw std::invalid_argument("Invalid value for " + argument);
                if (argument == "--roughness") roughness = number;
                else metallic = number;
                continue;
            }
            if (argument == "--seed") {
                const auto number = std::stoull(value, &parsed);
                if (parsed != value.size() || number > std::numeric_limits<unsigned int>::max())
                    throw std::invalid_argument("Seed must be an unsigned 32-bit integer");
                scene.seed = static_cast<unsigned int>(number);
                continue;
            }
            if (argument != "--size" && argument != "--spp" && argument != "--threads")
                throw std::invalid_argument("Unknown option: " + argument);
            const int count = std::stoi(value, &parsed);
            const int minimum = argument == "--threads" ? 0 : 1;
            const int maximum = argument == "--threads" ? 256 : 8192;
            if (parsed != value.size() || count < minimum || count > maximum)
                throw std::invalid_argument("Invalid integer value for " + argument);
            if (argument == "--size") scene.width = scene.height = count;
            else if (argument == "--spp") scene.spp = count;
            else scene.threads = count;
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

    // 只替换两个箱子的材质，墙壁保持漫反射，便于观察微表面反射差异。
    Material gold(MICROFACET), silver(MICROFACET);
    gold.Kd = Vector3f(.83f, .63f, .28f);
    silver.Kd = Vector3f(.8f, .82f, .85f);
    gold.roughness = silver.roughness = roughness;
    gold.metallic = silver.metallic = metallic;
    std::cout << "Box material: " << (useMicrofacet ? "microfacet" : "diffuse")
              << ", roughness: " << roughness << ", metallic: " << metallic << '\n';

    MeshTriangle floor(HW7_MODEL_DIR "/floor.obj", white);
    MeshTriangle shortbox(HW7_MODEL_DIR "/shortbox.obj", useMicrofacet ? &gold : white);
    MeshTriangle tallbox(HW7_MODEL_DIR "/tallbox.obj", useMicrofacet ? &silver : white);
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
    if (checkScene) {
        std::cout << "Scene check passed: " << scene.objects.size() << " meshes loaded.\n";
        return 0;
    }

    Renderer r;

    auto start = std::chrono::steady_clock::now();
    try {
        r.Render(scene);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    auto stop = std::chrono::steady_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration<double>(stop - start).count() << " seconds\n";

    return 0;
}
