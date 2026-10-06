//
// 由 LEI XU 创建，日期：5/16/19。
//

#ifndef RAYTRACING_BVH_H
#define RAYTRACING_BVH_H

#include <atomic>
#include <vector>
#include <memory>
#include <ctime>
#include "Object.hpp"
#include "Ray.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"

struct BVHBuildNode;
// BVHAccel 所需类型的前置声明
struct BVHPrimitiveInfo;

// BVHAccel 相关声明
inline int leafNodes, totalLeafNodes, totalPrimitives, interiorNodes;
class BVHAccel {

public:
    // BVHAccel 的公开类型
    enum class SplitMethod { NAIVE, SAH };

    // BVHAccel 的公开方法
    BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);
    Bounds3 WorldBound() const;
    ~BVHAccel();
    BVHAccel(const BVHAccel&) = delete;
    BVHAccel& operator=(const BVHAccel&) = delete;

    Intersection Intersect(const Ray &ray) const;
    Intersection getIntersection(BVHBuildNode* node, const Ray& ray)const;
    bool IntersectP(const Ray &ray) const;
    BVHBuildNode* root = nullptr;

    // BVHAccel 的内部方法（原框架未用 private 限制访问）
    BVHBuildNode* recursiveBuild(std::vector<Object*>objects);

    // BVHAccel 的内部数据（原框架未用 private 限制访问）
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<Object*> primitives;

    void getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf);
    void Sample(Intersection &pos, float &pdf);
};

struct BVHBuildNode {
    Bounds3 bounds;
    BVHBuildNode *left;
    BVHBuildNode *right;
    Object* object;
    float area;

public:
    int splitAxis=0, firstPrimOffset=0, nPrimitives=0;
    // BVHBuildNode 的公开方法
    BVHBuildNode(){
        bounds = Bounds3();
        left = nullptr;right = nullptr;
        object = nullptr;
    }
};




#endif // 结束头文件保护：RAYTRACING_BVH_H
