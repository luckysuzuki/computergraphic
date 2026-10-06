//
// 由 Göksu Güvendiren 创建，日期：2019-05-14。
//

#pragma once

#include <vector>
#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "AreaLight.hpp"
#include "BVH.hpp"
#include "Ray.hpp"


class Scene
{
public:
    // 设置场景与渲染参数
    int width = 1280;
    int height = 960;
    double fov = 40;
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
    int maxDepth = 1;
    float RussianRoulette = 0.8;
    int spp = 16; // 每个像素的路径采样次数
    unsigned int threads = 0; // 0 表示使用硬件并发数，1 表示单线程
    unsigned int seed = 42; // 固定像素种子，使结果不依赖线程调度顺序
    std::string outputPath = "binary.ppm";

    Scene(int w, int h) : width(w), height(h)
    {}

    void Add(Object *object) { objects.push_back(object); }
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }

    const std::vector<Object*>& get_objects() const { return objects; }
    const std::vector<std::unique_ptr<Light> >&  get_lights() const { return lights; }
    Intersection intersect(const Ray& ray) const;
    BVHAccel *bvh = nullptr;
    void buildBVH();
    Vector3f castRay(const Ray &ray, int depth) const;
    void sampleLight(Intersection &pos, float &pdf) const;
    bool trace(const Ray &ray, const std::vector<Object*> &objects, float &tNear, uint32_t &index, Object **hitObject);
    std::tuple<Vector3f, Vector3f> HandleAreaLight(const AreaLight &light, const Vector3f &hitPoint, const Vector3f &N,
                                                   const Vector3f &shadowPointOrig,
                                                   const std::vector<Object *> &objects, uint32_t &index,
                                                   const Vector3f &dir, float specularExponent);

    // 构建场景：保存物体与光源
    std::vector<Object* > objects;
    std::vector<std::unique_ptr<Light> > lights;

    // 计算反射方向
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }



// 根据斯涅尔定律计算折射方向
//
// 需要仔细区分以下两种情况：
//
// －光线位于物体内部
//
// －光线位于物体外部
//
// 若光线在物体外部，令 cosi 为正，即 cosi = -N·I
//
// 若光线在物体内部，交换两侧折射率，并将法线 N 取反
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }



    // 计算菲涅耳方程
//
// \param I 入射观察方向
//
// \param N 交点处的法线
//
// \param ior 材质的折射率
//
// \param[out] kr 反射光所占的比例
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // 根据斯涅尔定律计算折射角的正弦值
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // 全内反射
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // 根据能量守恒，透射比例为：
        // kt = 1 - kr;（透射比例等于 1 减去反射比例）
    }
};
