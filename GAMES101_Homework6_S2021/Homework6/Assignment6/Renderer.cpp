//
// 由 goksu 于 2020/2/25 创建。
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// 主渲染函数：遍历图像中的所有像素，生成主射线并将其投射到场景中，
// 最后把帧缓冲区内容保存到文件。
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(-1, 5, 10);
    int m = 0;
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // 生成主射线方向
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
            // TODO：求当前像素对应的 x、y 坐标，从而得到穿过该像素的方向向量。
            // x、y 都要乘以变量 scale，水平方向的 x 还要乘以 imageAspectRatio。
            Vector3f dir = normalize(Vector3f(x, y, -1.0f));
          framebuffer[m++]=  scene.castRay(Ray(eye_pos, dir), 0);
            // 不要忘记将方向向量归一化！

        }
        UpdateProgress(j / (float)scene.height);
    }
    UpdateProgress(1.f);

    // 将帧缓冲区保存到文件
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].x));
        color[1] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].y));
        color[2] = (unsigned char)(255 * clamp(0, 1, framebuffer[i].z));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
