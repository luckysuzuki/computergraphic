#include <stdexcept>
//
// 由 Göksu Güvendiren 创建，日期：2019-05-14。
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return bvh ? bvh->Intersect(ray) : Intersection();
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    pos = Intersection();
    pdf = 0;
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    if (emit_area_sum <= 0) return;
    const float totalArea = emit_area_sum;
    float p = get_random_float() * totalArea;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                // 多光源情况下，联合 PDF = 物体选择概率 × 物体内部面积 PDF。
                pdf *= objects[k]->getArea() / totalArea;
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// 路径追踪实现
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    const Intersection hit = intersect(ray);
    if (!hit.happened || !hit.m) return Vector3f();
    // 相机直接看到光源时计入自发光；后续反弹的光源项已由直接光采样估计。
    if (hit.m->hasEmission())
        return depth == 0 ? hit.m->getEmission() : Vector3f();

    const Vector3f p = hit.coords;
    const Vector3f normal = normalize(hit.normal);
    const Vector3f incoming = ray.direction;
    // Cornell Box 的尺寸约为 555，采用小幅法线偏移避免浮点误差造成自相交。
    const float rayOffset = 0.001f;
    const Vector3f origin = p + normal * rayOffset;
    Vector3f direct, indirect;

    Intersection lightPoint;
    float lightPdf = 0;
    sampleLight(lightPoint, lightPdf);
    if (lightPoint.happened && lightPdf > 0) {
        const Vector3f toLight = lightPoint.coords - p;
        const float distanceSquared = dotProduct(toLight, toLight);
        if (distanceSquared > rayOffset * rayOffset) {
            const Vector3f ws = normalize(toLight);
            const float cosSurface = std::max(0.f, dotProduct(normal, ws));
            const float cosLight = std::max(0.f, dotProduct(lightPoint.normal, -ws));
            if (cosSurface > 0 && cosLight > 0) {
                const Vector3f shadowDelta = lightPoint.coords - origin;
                const float shadowDistance = std::sqrt(dotProduct(shadowDelta, shadowDelta));
                Ray shadowRay(origin, normalize(shadowDelta));
                // 截止到光源之前，只检查这段线段内的遮挡物。
                shadowRay.t_max = shadowDistance - rayOffset;
                if (!intersect(shadowRay).happened) {
                    direct = lightPoint.emit * hit.m->eval(incoming, ws, normal)
                           * (cosSurface * cosLight / distanceSquared / lightPdf);
                }
            }
        }
    }

    // 仅终止间接项；存活路径除以继续概率，补偿被丢弃路径的期望贡献。
    const float survival = RussianRoulette;
    if (survival <= 0 || survival >= 1)
        throw std::invalid_argument("RussianRoulette must be in (0, 1)");
    if (get_random_float() < survival) {
        const Vector3f nextDirection = normalize(hit.m->sample(incoming, normal));
        const float directionPdf = hit.m->pdf(incoming, nextDirection, normal);
        const float cosine = std::max(0.f, dotProduct(normal, nextDirection));
        if (directionPdf > 0 && cosine > 0) {
            indirect = castRay(Ray(origin, nextDirection), depth + 1)
                     * hit.m->eval(incoming, nextDirection, normal)
                     * (cosine / directionPdf / survival);
        }
    }
    return direct + indirect;
}
