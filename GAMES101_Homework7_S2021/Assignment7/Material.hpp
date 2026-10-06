//
// 由 LEI XU 创建，日期：5/16/19。
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include <stdexcept>

enum MaterialType { DIFFUSE};

class Material{
private:

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

    Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }

public:
    MaterialType m_type;
    // Vector3f m_color; // 预留颜色成员（已停用）
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    // Texture tex; // 预留纹理成员（已停用）

    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    // inline Vector3f getColor(); // 预留颜色接口（已停用）
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // 根据材质属性采样一个光线方向
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    // 计算给定方向的概率密度 PDF（相对于立体角）
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // 计算给定方向对应的双向反射分布函数 BRDF
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

};

Material::Material(MaterialType t, Vector3f e){
    m_type = t;
    // m_color = c; // 预留颜色赋值（已停用）
    m_emission = e;
}

MaterialType Material::getType(){return m_type;}
// /Vector3f Material::getColor(){return m_color;} // 预留颜色接口（已停用）
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // 在法线所在的半球上均匀采样方向
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            Vector3f localRay(r*std::cos(phi), r*std::sin(phi), z);
            return toWorld(localRay, N);
            
            break;
        }
    }
    throw std::invalid_argument("Unsupported material type in sample");
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // 半球均匀采样的概率密度为 1 / (2 * PI)
            if (dotProduct(wo, N) > 0.0f)
                return 0.5f / M_PI;
            else
                return 0.0f;
            break;
        }
    }
    throw std::invalid_argument("Unsupported material type in pdf");
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // 计算漫反射模型的 BRDF，即 Kd / PI
            float cosalpha = dotProduct(N, wo);
            if (cosalpha > 0.0f) {
                Vector3f diffuse = Kd / M_PI;
                return diffuse;
            }
            else
                return Vector3f(0.0f);
            break;
        }
    }
    throw std::invalid_argument("Unsupported material type in eval");
}

#endif // 结束头文件保护：RAYTRACING_MATERIAL_H
