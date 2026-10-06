#include "Triangle.hpp"
#include "Scene.hpp"
#include <stdexcept>
#include <iostream>

#include "Renderer.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>

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
        seed_random(1234);
        Material microfacet(MICROFACET);
        microfacet.Kd = Vector3f(.8f, .5f, .2f);
        microfacet.metallic = 1;
        microfacet.roughness = .5f;
        const Vector3f N(0, 0, 1), incoming(0, 0, -1), outgoing(0, 0, 1);
        const Vector3f normalBRDF = microfacet.eval(incoming, outgoing, N);
        // 法向入射时 F=F0、G=1，D=1/(PI*alpha²)，可独立核对解析值。
        require(std::fabs(normalBRDF.x - .8f / (4.f * M_PI * .0625f)) < 1e-5,
                "GGX analytic normal incidence");
        const Vector3f oblique = normalize(Vector3f(.5f, .2f, 1));
        const Vector3f a = microfacet.eval(-oblique, outgoing, N);
        const Vector3f b = microfacet.eval(incoming, oblique, N);
        require((a-b).norm() < 1e-5, "BRDF reciprocity");
        require(microfacet.eval(incoming, Vector3f(0,0,-1), N).norm() == 0,
                "microfacet back hemisphere");
        microfacet.roughness = .2f;
        require(microfacet.eval(incoming, outgoing, N).x > normalBRDF.x,
                "lower roughness concentrates highlight");
        microfacet.roughness = .5f;
        double reflected = 0;
        for (int i = 0; i < 30000; ++i) {
            const Vector3f direction = microfacet.sample(incoming, N);
            const float density = microfacet.pdf(incoming, direction, N);
            require(std::fabs(direction.norm() - 1) < 1e-5 && dotProduct(direction,N) >= 0,
                    "hemisphere sample direction");
            require(std::fabs(density - .5f/M_PI) < 1e-6, "microfacet sampling PDF");
            reflected += microfacet.eval(incoming, direction, N).x * direction.z / density;
        }
        require(reflected / 30000 > 0.1 && reflected / 30000 < 1.03,
                "microfacet finite reflected energy");
        const Vector3f grazing = normalize(Vector3f(1,0,.000001f));
        const Vector3f grazingBRDF = microfacet.eval(-grazing, grazing, N);
        require(std::isfinite(grazingBRDF.x) && grazingBRDF.x >= 0,
                "microfacet grazing numerical stability");

        // 同一相机像素仅由一个线程处理，固定种子下各线程数的 PPM 应逐字节一致。
        const auto temp = std::filesystem::temp_directory_path() /
            ("hw7-thread-check-" + std::to_string(std::random_device{}()));
        std::filesystem::create_directories(temp);
        Material white;
        white.Kd = Vector3f(.7f);
        Material source(DIFFUSE, Vector3f(4));
        Triangle floor1(Vector3f(-1000,0,-1000), Vector3f(-1000,0,2000), Vector3f(2000,0,-1000), &white);
        Triangle ceiling(Vector3f(-1000,500,-1000), Vector3f(2000,500,-1000), Vector3f(-1000,500,2000), &source);
        Scene check(16,16);
        check.Add(&floor1); check.Add(&ceiling); check.buildBVH();
        check.spp = 8; check.seed = 31415; check.threads = 1;
        check.outputPath = (temp / "single.ppm").u8string();
        Renderer renderer;
        renderer.Render(check);
        check.threads = 4;
        check.outputPath = (temp / "parallel.ppm").u8string();
        renderer.Render(check);
        const auto read = [](const std::filesystem::path& path) {
            std::ifstream file(path, std::ios::binary);
            return std::string(std::istreambuf_iterator<char>(file), {});
        };
        const std::string single = read(temp / "single.ppm");
        require(single.size() > 16*16*3 && single == read(temp / "parallel.ppm"),
                "single and multithread images identical");
        check.RussianRoulette = 1;
        bool reported = false;
        try { renderer.Render(check); }
        catch (const std::invalid_argument&) { reported = true; }
        require(reported, "worker exception reaches caller without termination");
        require(std::filesystem::equivalent(temp.parent_path(), std::filesystem::temp_directory_path()),
                "temporary output boundary");
        std::filesystem::remove_all(temp);
        std::cout << "Geometry, visibility intervals, sampling, and emission checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
