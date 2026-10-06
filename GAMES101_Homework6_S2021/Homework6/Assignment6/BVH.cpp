#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // 计算当前 BVH 节点中所有图元的整体包围盒
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // 创建叶子 BVHBuildNode
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        const auto sortByAxis = [](std::vector<Object*>& values, int axis) {
            std::sort(values.begin(), values.end(), [axis](Object* lhs, Object* rhs) {
                const Vector3f lhsCentroid = lhs->getBounds().Centroid();
                const Vector3f rhsCentroid = rhs->getBounds().Centroid();
                if (axis == 0)
                    return lhsCentroid.x < rhsCentroid.x;
                if (axis == 1)
                    return lhsCentroid.y < rhsCentroid.y;
                return lhsCentroid.z < rhsCentroid.z;
            });
        };

        size_t splitIndex = objects.size() / 2;
        int splitAxis = 0;
        bool foundSahSplit = false;

        if (splitMethod == SplitMethod::SAH) {
            const double parentArea = bounds.SurfaceArea();
            double bestCost = std::numeric_limits<double>::infinity();
            std::vector<Object*> bestOrdering;

            if (parentArea > 0.0 && std::isfinite(parentArea)) {
                // 精确 SAH：在 X、Y、Z 三个轴上尝试每一个有效切分位置。
                for (int axis = 0; axis < 3; ++axis) {
                    std::vector<Object*> ordered = objects;
                    sortByAxis(ordered, axis);

                    const size_t count = ordered.size();
                    std::vector<Bounds3> leftBounds(count);
                    std::vector<Bounds3> rightBounds(count);

                    leftBounds[0] = ordered[0]->getBounds();
                    for (size_t i = 1; i < count; ++i)
                        leftBounds[i] = Union(leftBounds[i - 1], ordered[i]->getBounds());

                    rightBounds[count - 1] = ordered[count - 1]->getBounds();
                    for (size_t i = count - 1; i > 0; --i)
                        rightBounds[i - 1] = Union(rightBounds[i], ordered[i - 1]->getBounds());

                    for (size_t i = 0; i + 1 < count; ++i) {
                        const size_t leftCount = i + 1;
                        const size_t rightCount = count - leftCount;
                        const double leftArea = leftBounds[i].SurfaceArea();
                        const double rightArea = rightBounds[i + 1].SurfaceArea();

                        // 假设一次节点遍历和一次图元求交的单位成本都为 1。
                        const double cost =
                            1.0 +
                            (leftArea / parentArea) * static_cast<double>(leftCount) +
                            (rightArea / parentArea) * static_cast<double>(rightCount);

                        if (cost < bestCost) {
                            bestCost = cost;
                            splitAxis = axis;
                            splitIndex = leftCount;
                            bestOrdering = ordered;
                            foundSahSplit = true;
                        }
                    }
                }

                if (foundSahSplit)
                    objects = std::move(bestOrdering);
            }
        }

        // 普通 BVH，或者 SAH 遇到退化包围盒时，回退到最长轴中点划分。
        if (!foundSahSplit) {
            Bounds3 centroidBounds;
            for (Object* object : objects)
                centroidBounds = Union(centroidBounds, object->getBounds().Centroid());

            splitAxis = centroidBounds.maxExtent();
            splitIndex = objects.size() / 2;
            sortByAxis(objects, splitAxis);
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + splitIndex;
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->splitAxis = splitAxis;
        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection noHit;

    if (node == nullptr)
        return noHit;

    // 框架中的 dirIsNeg 实际记录方向分量是否大于 0，
    // Bounds3::IntersectP 会据此选择每个轴上的进入面和离开面。
    const std::array<int, 3> dirIsNeg = {
        static_cast<int>(ray.direction.x > 0),
        static_cast<int>(ray.direction.y > 0),
        static_cast<int>(ray.direction.z > 0)
    };

    // 没有击中当前节点的包围盒，它的整棵子树都不可能被击中。
    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg))
        return noHit;

    // 叶子节点保存真正的场景物体，直接进行精确求交。
    if (node->left == nullptr && node->right == nullptr) {
        if (node->object != nullptr)
            return node->object->getIntersection(ray);
        return noHit;
    }

    // 内部节点分别查询左右子树。
    const Intersection leftHit = getIntersection(node->left, ray);
    const Intersection rightHit = getIntersection(node->right, ray);

    // 两边都命中时返回距离光线起点更近的交点。
    if (leftHit.happened && rightHit.happened)
        return leftHit.distance <= rightHit.distance ? leftHit : rightHit;

    if (leftHit.happened)
        return leftHit;
    if (rightHit.happened)
        return rightHit;

    return noHit;
}
