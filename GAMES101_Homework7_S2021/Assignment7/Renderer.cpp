//
// 由 goksu 创建，日期：2/25/20。
//

#include <fstream>
#include <atomic>
#include <thread>
#include <mutex>
#include <exception>
#include <filesystem>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// 主渲染函数：遍历图像中的所有像素，
// 生成主光线并追踪到场景中，
// 最终将帧缓冲区保存到文件。
void Renderer::Render(const Scene& scene)
{
    if (scene.width <= 0 || scene.height <= 0 || scene.spp <= 0)
        throw std::invalid_argument("Image dimensions and spp must be positive");
    std::vector<Vector3f> framebuffer(size_t(scene.width) * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    const int spp = scene.spp;
    if (scene.width <= 0 || scene.height <= 0 || spp <= 0)
        throw std::invalid_argument("Image dimensions and spp must be positive");
    const unsigned int available = std::max(1u, std::thread::hardware_concurrency());
    const unsigned int threadCount = std::min<unsigned int>(scene.height,
        scene.threads == 0 ? available : scene.threads);
    std::cout << "SPP: " << spp << ", threads: " << threadCount
              << ", seed: " << scene.seed << "\n";
    std::atomic<int> nextRow{0};
    std::atomic<bool> stop{false};
    std::mutex statusMutex;
    int completedRows = 0;
    std::exception_ptr error;
    const auto worker = [&]() {
        try {
            while (!stop.load()) {
                const int j = nextRow.fetch_add(1);
                if (j >= scene.height) break;
                for (int i = 0; i < scene.width; ++i) {
                    const size_t pixel = size_t(j) * scene.width + i;
                    // 每像素独立种子：线程调度和线程数不会改变路径的随机序列。
                    unsigned int hash = scene.seed ^ (unsigned(pixel) + 0x9e3779b9u);
                    hash = (hash ^ (hash >> 16)) * 0x85ebca6bu;
                    hash = (hash ^ (hash >> 13)) * 0xc2b2ae35u;
                    seed_random(hash ^ (hash >> 16));
                    const float x = (2 * (i + 0.5f) / scene.width - 1) * imageAspectRatio * scale;
                    const float y = (1 - 2 * (j + 0.5f) / scene.height) * scale;
                    const Vector3f dir = normalize(Vector3f(-x, y, 1));
                    Vector3f radiance;
                    for (int k = 0; k < spp; ++k)
                        radiance += scene.castRay(Ray(eye_pos, dir), 0) / spp;
                    // 行只领取一次，像素写入互不重叠；场景和 BVH 在渲染期间只读。
                    framebuffer[pixel] = radiance;
                }
                std::lock_guard<std::mutex> lock(statusMutex);
                UpdateProgress(++completedRows / float(scene.height));
            }
        } catch (...) {
            std::lock_guard<std::mutex> lock(statusMutex);
            if (!error) error = std::current_exception();
            stop.store(true);
        }
    };
    std::vector<std::thread> workers;
    try {
        for (unsigned int i = 0; i < threadCount; ++i) workers.emplace_back(worker);
    } catch (...) {
        stop.store(true);
        for (auto& thread : workers) thread.join();
        throw;
    }
    for (auto& thread : workers) thread.join();
    if (error) std::rethrow_exception(error);
    UpdateProgress(1.f);

    // 将帧缓冲区保存为 PPM 图像
    const std::filesystem::path output = std::filesystem::u8path(scene.outputPath);
    if (output.has_parent_path()) std::filesystem::create_directories(output.parent_path());
#ifdef _WIN32
    FILE* fp = _wfopen(output.wstring().c_str(), L"wb");
#else
    FILE* fp = fopen(scene.outputPath.c_str(), "wb");
#endif
    if (!fp) throw std::runtime_error("Cannot open output PPM: " + scene.outputPath);
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
    std::cout << "\nSaved: " << std::filesystem::absolute(output).u8string() << '\n';
}
