#include "Triangle.hpp"
#include "Scene.hpp"
#include <stdexcept>
#include <iostream>

const float EPSILON = 0.00001f;

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

bool hits(const Bounds3& box, const Ray& ray)
{
    return box.IntersectP(ray, ray.direction_inv,
        {ray.direction.x < 0, ray.direction.y < 0, ray.direction.z < 0});
}

int main()
{
    try {
        const Bounds3 box(Vector3f(0, 0, 0), Vector3f(1, 1, 1));
        require(hits(box, Ray(Vector3f(-1, .5f, .5f), Vector3f(1, 0, 0))), "positive direction");
        require(hits(box, Ray(Vector3f(2, .5f, .5f), Vector3f(-1, 0, 0))), "negative direction");
        require(hits(box, Ray(Vector3f(.5f), Vector3f(0, 0, 1))), "origin inside");
        require(hits(box, Ray(Vector3f(-1, 0, 0), Vector3f(1, 0, 0))), "parallel on boundary");
        require(!hits(box, Ray(Vector3f(-1, 2, 0), Vector3f(1, 0, 0))), "parallel outside");
        require(hits(box, Ray(Vector3f(-1, 1, .5f), Vector3f(1, -1, 0))), "grazing interval equality");
        require(!hits(box, Ray(Vector3f(-1, .5f, .5f), Vector3f(-1, 0, 0))), "box behind ray");
        Ray segment(Vector3f(-1, .5f, .5f), Vector3f(1, 0, 0));
        segment.t_max = .5;
        require(!hits(box, segment), "finite shadow segment");

        Material material;
        material.Kd = Vector3f(.5f);
        Triangle nearTri(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), &material);
        Triangle farTri(Vector3f(0, 0, -2), Vector3f(1, 0, -2), Vector3f(0, 1, -2), &material);
        const Ray ray(Vector3f(.2f, .2f, 1), Vector3f(0, 0, -1));
        const Intersection hit = nearTri.getIntersection(ray);
        require(hit.happened && std::fabs(hit.distance - 1) < 1e-5, "triangle hit distance");
        require(hit.obj == &nearTri && hit.m == &material && std::fabs(hit.coords.z) < 1e-5, "intersection metadata");
        require(!nearTri.getIntersection(Ray(Vector3f(.2f, .2f, -1), Vector3f(0, 0, 1))).happened, "backface rejection");
        require(!nearTri.getIntersection(Ray(Vector3f(.8f, .8f, 1), Vector3f(0, 0, -1))).happened, "outside barycentric coordinates");
        Ray shortRay = ray;
        shortRay.t_max = .5;
        require(!nearTri.getIntersection(shortRay).happened, "triangle beyond segment");
        BVHAccel tree({&farTri, &nearTri});
        require(tree.Intersect(ray).obj == &nearTri, "BVH nearest primitive");
        BVHAccel empty({});
        require(!empty.Intersect(ray).happened, "empty BVH");

        // 两个等面积三角形应等概率被选中，内部点的均值应接近重心。
        Triangle other(Vector3f(10, 0, 0), Vector3f(11, 0, 0), Vector3f(10, 1, 0), &material);
        BVHAccel samples({&nearTri, &other});
        int nearCount = 0;
        double localX = 0, localY = 0;
        constexpr int count = 20000;
        for (int i = 0; i < count; ++i) {
            Intersection sample;
            float pdf;
            samples.Sample(sample, pdf);
            require(sample.happened && std::fabs(pdf - 1.f) < 1e-5, "area PDF normalization");
            const bool isNear = sample.coords.x < 5;
            nearCount += isNear;
            localX += sample.coords.x - (isNear ? 0 : 10);
            localY += sample.coords.y;
        }
        require(std::fabs(double(nearCount) / count - .5) < .025, "uniform area selection");
        require(std::fabs(localX / count - 1. / 3) < .02 && std::fabs(localY / count - 1. / 3) < .02,
                "triangle area centroid");

        Material lamp(DIFFUSE, Vector3f(1));
        Triangle light1(Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(0, 1, 0), &lamp);
        Triangle light2(Vector3f(10, 0, 0), Vector3f(12, 0, 0), Vector3f(10, 1, 0), &lamp);
        Scene scene(1, 1);
        scene.Add(&light1);
        scene.Add(&light2);
        for (int i = 0; i < 100; ++i) {
            Intersection sample;
            float pdf;
            scene.sampleLight(sample, pdf);
            require(sample.happened && std::fabs(pdf - 1.f / 1.5f) < 1e-5, "multiple light joint PDF");
        }
        scene.buildBVH();
        require(scene.castRay(ray, 0).x == 1, "camera sees emission");
        require(scene.castRay(ray, 1).x == 0, "avoid counting light twice");
        Scene dark(1, 1);
        Intersection sample;
        float pdf = 1;
        dark.sampleLight(sample, pdf);
        require(!sample.happened && pdf == 0, "scene without lights");
        require(dark.castRay(ray, 0).x == 0, "empty scene radiance");
        std::cout << "Geometry, visibility intervals, sampling, and emission checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
