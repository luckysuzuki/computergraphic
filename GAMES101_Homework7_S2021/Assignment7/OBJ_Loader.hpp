// OBJ_Loader.h：单头文件 OBJ 模型加载器
// 本加载器由 Robert Smith 创建。
// https://github.com/Bly7/OBJ-Loader
// 使用 MIT 许可证。

#pragma once

#include <optional>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>

// 加载大型模型时，在控制台打印进度
// #define OBJL_CONSOLE_OUTPUT // 启用加载进度输出（当前已停用）

// 命名空间：OBJL
//
// 说明：此命名空间包含
// OBJ 模型加载器使用的所有内容
namespace objl
{
    // 结构体：Vector2
    //
    // 说明：保存位置数据的二维向量
    struct Vector2
    {
        // 默认构造函数
        Vector2()
        {
            X = 0.0f;
            Y = 0.0f;
        }
        // 通过参数设置成员的构造函数
        Vector2(float X_, float Y_)
        {
            X = X_;
            Y = Y_;
        }
        // 相等比较运算符重载
        bool operator==(const Vector2& other) const
        {
            return (this->X == other.X && this->Y == other.Y);
        }
        // 不等比较运算符重载
        bool operator!=(const Vector2& other) const
        {
            return !(this->X == other.X && this->Y == other.Y);
        }
        // 加法运算符重载
        Vector2 operator+(const Vector2& right) const
        {
            return Vector2(this->X + right.X, this->Y + right.Y);
        }
        // 减法运算符重载
        Vector2 operator-(const Vector2& right) const
        {
            return Vector2(this->X - right.X, this->Y - right.Y);
        }
        // 浮点数乘法运算符重载
        Vector2 operator*(const float& other) const
        {
            return Vector2(this->X *other, this->Y * other);
        }

        // 位置分量
        float X;
        float Y;
    };

    // 结构体：Vector3
    //
    // 说明：保存位置数据的三维向量
    struct Vector3
    {
        // 默认构造函数
        Vector3()
        {
            X = 0.0f;
            Y = 0.0f;
            Z = 0.0f;
        }
        // 通过参数设置成员的构造函数
        Vector3(float X_, float Y_, float Z_)
        {
            X = X_;
            Y = Y_;
            Z = Z_;
        }
        // 相等比较运算符重载
        bool operator==(const Vector3& other) const
        {
            return (this->X == other.X && this->Y == other.Y && this->Z == other.Z);
        }
        // 不等比较运算符重载
        bool operator!=(const Vector3& other) const
        {
            return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z);
        }
        // 加法运算符重载
        Vector3 operator+(const Vector3& right) const
        {
            return Vector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z);
        }
        // 减法运算符重载
        Vector3 operator-(const Vector3& right) const
        {
            return Vector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
        }
        // 浮点数乘法运算符重载
        Vector3 operator*(const float& other) const
        {
            return Vector3(this->X * other, this->Y * other, this->Z * other);
        }
        // 浮点数除法运算符重载
        Vector3 operator/(const float& other) const
        {
            return Vector3(this->X / other, this->Y / other, this->Z / other);
        }

        // 位置分量
        float X;
        float Y;
        float Z;
    };

    // 结构体：Vertex
    //
    // 说明：模型顶点对象，保存
    // 位置、法线和纹理坐标
    struct Vertex
    {
        // 位置向量
        Vector3 Position;

        // 法线向量
        Vector3 Normal;

        // 纹理坐标向量
        Vector2 TextureCoordinate;
    };

    struct Material
    {
        Material()
        {
            name = "";
            Ns = 0.0f;
            Ni = 0.0f;
            d = 0.0f;
            illum = 0;
        }

        // 材质名称
        std::string name;
        // 环境光颜色
        Vector3 Ka;
        // 漫反射颜色
        Vector3 Kd;
        // 镜面反射颜色
        Vector3 Ks;
        // 镜面反射指数
        float Ns;
        // 光学密度（此处为折射率）
        float Ni;
        // 不透明度
        float d;
        // 光照模型编号
        int illum;
        // 环境光纹理贴图
        std::string map_Ka;
        // 漫反射纹理贴图
        std::string map_Kd;
        // 镜面反射纹理贴图
        std::string map_Ks;
        // 镜面高光指数贴图
        std::string map_Ns;
        // 透明度贴图
        std::string map_d;
        // 凹凸贴图
        std::string map_bump;
    };

    // 结构体：Mesh
    //
    // 说明：简单网格对象，保存
    // 名称、顶点列表和索引列表
    struct Mesh
    {
        // 默认构造函数
        Mesh()
        {

        }
        // 通过参数设置成员的构造函数
        Mesh(std::vector<Vertex>& _Vertices, std::vector<unsigned int>& _Indices)
        {
            Vertices = _Vertices;
            Indices = _Indices;
            MeshMaterial = std::nullopt;
        }
        // 网格名称
        std::string MeshName;
        // 顶点列表
        std::vector<Vertex> Vertices;
        // 索引列表
        std::vector<unsigned int> Indices;

        // 材质
        std::optional<Material> MeshMaterial;
    };

    // 命名空间：Math
    //
    // 说明：此命名空间包含
    // OBJL 所需的数学函数
    namespace math
    {
        // 三维向量叉积
        Vector3 CrossV3(const Vector3 a, const Vector3 b)
        {
            return Vector3(a.Y * b.Z - a.Z * b.Y,
                           a.Z * b.X - a.X * b.Z,
                           a.X * b.Y - a.Y * b.X);
        }

        // 计算三维向量的长度
        float MagnitudeV3(const Vector3 in)
        {
            return (sqrtf(powf(in.X, 2) + powf(in.Y, 2) + powf(in.Z, 2)));
        }

        // 三维向量点积
        float DotV3(const Vector3 a, const Vector3 b)
        {
            return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
        }

        // 两个三维向量之间的夹角
        float AngleBetweenV3(const Vector3 a, const Vector3 b)
        {
            float angle = DotV3(a, b);
            angle /= (MagnitudeV3(a) * MagnitudeV3(b));
            return angle = acosf(angle);
        }

        // 计算向量 a 在向量 b 上的投影
        Vector3 ProjV3(const Vector3 a, const Vector3 b)
        {
            Vector3 bn = b / MagnitudeV3(b);
            return bn * DotV3(a, bn);
        }
    }

    // 命名空间：Algorithm
    //
    // 说明：此命名空间包含
    // OBJL 所需的算法
    namespace algorithm
    {
        // 三维向量乘法运算符重载
        Vector3 operator*(const float& left, const Vector3& right)
        {
            return Vector3(right.X * left, right.Y * left, right.Z * left);
        }

        // 判断点 P1 与 P2 是否位于线段 ab 的同一侧
        bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b)
        {
            Vector3 cp1 = math::CrossV3(b - a, p1 - a);
            Vector3 cp2 = math::CrossV3(b - a, p2 - a);

            if (math::DotV3(cp1, cp2) >= 0)
                return true;
            else
                return false;
        }

        // 通过叉积计算三角形法线
        Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3)
        {
            Vector3 u = t2 - t1;
            Vector3 v = t3 - t1;

            Vector3 normal = math::CrossV3(u,v);

            return normal;
        }

        // 判断一个三维点是否位于三个顶点确定的三角形内
        bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3)
        {
            // 判断点是否位于以三角形为底面的无限棱柱内
            bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) && SameSide(point, tri2, tri1, tri3)
                                     && SameSide(point, tri3, tri1, tri2);

            // 若不在该棱柱内，则不可能位于三角形上
            if (!within_tri_prisim)
                return false;

            // 计算三角形法线
            Vector3 n = GenTriNormal(tri1, tri2, tri3);

            // 将点投影到该法线方向
            Vector3 proj = math::ProjV3(point, n);

            // 若点到三角形平面的距离为 0，
            // 则该点位于三角形上
            if (math::MagnitudeV3(proj) == 0)
                return true;
            else
                return false;
        }

        // 按照给定分隔符将字符串拆分为字符串数组
        inline void split(const std::string &in,
                          std::vector<std::string> &out,
                          std::string token)
        {
            out.clear();

            std::string temp;

            for (int i = 0; i < int(in.size()); i++)
            {
                std::string test = in.substr(i, token.size());

                if (test == token)
                {
                    if (!temp.empty())
                    {
                        out.push_back(temp);
                        temp.clear();
                        i += (int)token.size() - 1;
                    }
                    else
                    {
                        out.push_back("");
                    }
                }
                else if (i + token.size() >= in.size())
                {
                    temp += in.substr(i, token.size());
                    out.push_back(temp);
                    break;
                }
                else
                {
                    temp += in[i];
                }
            }
        }

        // 获取首个词元及其后空格之后的剩余字符串
        inline std::string tail(const std::string &in)
        {
            size_t token_start = in.find_first_not_of(" \t");
            size_t space_start = in.find_first_of(" \t", token_start);
            size_t tail_start = in.find_first_not_of(" \t", space_start);
            size_t tail_end = in.find_last_not_of(" \t");
            if (tail_start != std::string::npos && tail_end != std::string::npos)
            {
                return in.substr(tail_start, tail_end - tail_start + 1);
            }
            else if (tail_start != std::string::npos)
            {
                return in.substr(tail_start);
            }
            return "";
        }

        // 获取字符串的首个词元
        inline std::string firstToken(const std::string &in)
        {
            if (!in.empty())
            {
                size_t token_start = in.find_first_not_of(" \t");
                size_t token_end = in.find_first_of(" \t", token_start);
                if (token_start != std::string::npos && token_end != std::string::npos)
                {
                    return in.substr(token_start, token_end - token_start);
                }
                else if (token_start != std::string::npos)
                {
                    return in.substr(token_start);
                }
            }
            return "";
        }

        // 获取指定索引处的元素（支持 OBJ 的负索引）
        template <class T>
        inline const T & getElement(const std::vector<T> &elements, std::string &index)
        {
            int idx = std::stoi(index);
            if (idx < 0)
                idx = int(elements.size()) + idx;
            else
                idx--;
            return elements[idx];
        }
    }

    // 类：Loader
    //
    // 说明：OBJ 模型加载器
    class Loader
    {
    public:
        // 默认构造函数
        Loader()
        {

        }
        ~Loader()
        {
            LoadedMeshes.clear();
        }

        // 将文件读入加载器
        //
        // 若文件加载成功，则返回 true
        //
        // 若找不到文件，
        // 或无法加载，则返回 false
        bool LoadFile(std::string Path)
        {
            // 若文件不是 .obj 文件，则返回 false
            if (Path.substr(Path.size() - 4, 4) != ".obj")
                return false;


            std::ifstream file(Path);

            if (!file.is_open())
                return false;

            LoadedMeshes.clear();
            LoadedVertices.clear();
            LoadedIndices.clear();

            std::vector<Vector3> Positions;
            std::vector<Vector2> TCoords;
            std::vector<Vector3> Normals;

            std::vector<Vertex> Vertices;
            std::vector<unsigned int> Indices;

            std::vector<std::string> MeshMatNames;

            bool listening = false;
            std::string meshname;

            Mesh tempMesh;

#ifdef OBJL_CONSOLE_OUTPUT
            const unsigned int outputEveryNth = 1000;
            unsigned int outputIndicator = outputEveryNth;
#endif

            std::string curline;
            while (std::getline(file, curline))
            {
#ifdef OBJL_CONSOLE_OUTPUT
                if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
                {
                    if (!meshname.empty())
                    {
                        std::cout
                                << "\r- " << meshname
                                << "\t| vertices > " << Positions.size()
                                << "\t| texcoords > " << TCoords.size()
                                << "\t| normals > " << Normals.size()
                                << "\t| triangles > " << (Vertices.size() / 3)
                                << (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
                    }
                }
#endif

                // 生成网格对象，或准备创建新的对象
                if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g" || curline[0] == 'g')
                {
                    if (!listening)
                    {
                        listening = true;

                        if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
                        {
                            meshname = algorithm::tail(curline);
                        }
                        else
                        {
                            meshname = "unnamed";
                        }
                    }
                    else
                    {
                        // 生成待放入数组的网格

                        if (!Indices.empty() && !Vertices.empty())
                        {
                            // 创建网格
                            tempMesh = Mesh(Vertices, Indices);
                            tempMesh.MeshName = meshname;

                            // 插入网格
                            LoadedMeshes.push_back(tempMesh);

                            // 清理临时数据
                            Vertices.clear();
                            Indices.clear();
                            meshname.clear();

                            meshname = algorithm::tail(curline);
                        }
                        else
                        {
                            if (algorithm::firstToken(curline) == "o" || algorithm::firstToken(curline) == "g")
                            {
                                meshname = algorithm::tail(curline);
                            }
                            else
                            {
                                meshname = "unnamed";
                            }
                        }
                    }
#ifdef OBJL_CONSOLE_OUTPUT
                    std::cout << std::endl;
                    outputIndicator = 0;
#endif
                }
                // 读取顶点位置
                if (algorithm::firstToken(curline) == "v")
                {
                    std::vector<std::string> spos;
                    Vector3 vpos;
                    algorithm::split(algorithm::tail(curline), spos, " ");

                    vpos.X = std::stof(spos[0]);
                    vpos.Y = std::stof(spos[1]);
                    vpos.Z = std::stof(spos[2]);

                    Positions.push_back(vpos);
                }
                // 读取顶点纹理坐标
                if (algorithm::firstToken(curline) == "vt")
                {
                    std::vector<std::string> stex;
                    Vector2 vtex;
                    algorithm::split(algorithm::tail(curline), stex, " ");

                    vtex.X = std::stof(stex[0]);
                    vtex.Y = std::stof(stex[1]);

                    TCoords.push_back(vtex);
                }
                // 读取顶点法线
                if (algorithm::firstToken(curline) == "vn")
                {
                    std::vector<std::string> snor;
                    Vector3 vnor;
                    algorithm::split(algorithm::tail(curline), snor, " ");

                    vnor.X = std::stof(snor[0]);
                    vnor.Y = std::stof(snor[1]);
                    vnor.Z = std::stof(snor[2]);

                    Normals.push_back(vnor);
                }
                // 生成一个面（顶点与索引）
                if (algorithm::firstToken(curline) == "f")
                {
                    // 生成顶点
                    std::vector<Vertex> vVerts;
                    GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

                    // 添加顶点
                    for (int i = 0; i < int(vVerts.size()); i++)
                    {
                        Vertices.push_back(vVerts[i]);

                        LoadedVertices.push_back(vVerts[i]);
                    }

                    std::vector<unsigned int> iIndices;

                    VertexTriangluation(iIndices, vVerts);

                    // 添加索引
                    for (int i = 0; i < int(iIndices.size()); i++)
                    {
                        unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
                        Indices.push_back(indnum);

                        indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
                        LoadedIndices.push_back(indnum);

                    }
                }
                // 获取网格的材质名称
                if (algorithm::firstToken(curline) == "usemtl")
                {
                    MeshMatNames.push_back(algorithm::tail(curline));

                    // 若同一组内的材质发生变化，则创建新网格
                    if (!Indices.empty() && !Vertices.empty())
                    {
                        // 创建网格
                        tempMesh = Mesh(Vertices, Indices);
                        tempMesh.MeshName = meshname;
                        int i = 2;
                        while(1) {
                            tempMesh.MeshName = meshname + "_" + std::to_string(i);

                            for (auto &m : LoadedMeshes)
                                if (m.MeshName == tempMesh.MeshName)
                                    continue;
                            break;
                        }

                        // 插入网格
                        LoadedMeshes.push_back(tempMesh);

                        // 清理临时数据
                        Vertices.clear();
                        Indices.clear();
                    }

#ifdef OBJL_CONSOLE_OUTPUT
                    outputIndicator = 0;
#endif
                }
                // 加载材质
                if (algorithm::firstToken(curline) == "mtllib")
                {
                    // 生成已加载材质记录

                    // 生成材质文件路径
                    std::vector<std::string> temp;
                    algorithm::split(Path, temp, "/");

                    std::string pathtomat = "";

                    if (temp.size() != 1)
                    {
                        for (int i = 0; i < temp.size() - 1; i++)
                        {
                            pathtomat += temp[i] + "/";
                        }
                    }


                    pathtomat += algorithm::tail(curline);

#ifdef OBJL_CONSOLE_OUTPUT
                    std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
#endif

                    // 加载材质
                    LoadMaterials(pathtomat);
                }
            }

#ifdef OBJL_CONSOLE_OUTPUT
            std::cout << std::endl;
#endif

            // 处理最后一个网格

            if (!Indices.empty() && !Vertices.empty())
            {
                // 创建网格
                tempMesh = Mesh(Vertices, Indices);
                tempMesh.MeshName = meshname;

                // 插入网格
                LoadedMeshes.push_back(tempMesh);
            }

            file.close();

            // 为每个网格设置材质
            for (int i = 0; i < MeshMatNames.size(); i++)
            {
                std::string matname = MeshMatNames[i];

                // 在已加载材质中查找对应的材质名称，
                // 找到后将材质参数复制到网格材质中
                for (int j = 0; j < LoadedMaterials.size(); j++)
                {
                    if (LoadedMaterials[j].name == matname)
                    {
                        LoadedMeshes[i].MeshMaterial = LoadedMaterials[j];
                        break;
                    }
                }
            }

            if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty())
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        // 已加载的网格对象
        std::vector<Mesh> LoadedMeshes;
        // 已加载的顶点对象
        std::vector<Vertex> LoadedVertices;
        // 已加载的索引
        std::vector<unsigned int> LoadedIndices;
        // 已加载的材质对象
        std::vector<Material> LoadedMaterials;

    private:
        // 根据位置列表、
        // 纹理坐标、法线以及面定义行生成顶点
        void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
                                   const std::vector<Vector3>& iPositions,
                                   const std::vector<Vector2>& iTCoords,
                                   const std::vector<Vector3>& iNormals,
                                   std::string icurline)
        {
            std::vector<std::string> sface, svert;
            Vertex vVert;
            algorithm::split(algorithm::tail(icurline), sface, " ");
            bool noNormal = false;

            // 遍历给定的每个顶点
            for (int i = 0; i < int(sface.size()); i++)
            {
                // 判断顶点的格式类型
                int vtype;

                algorithm::split(sface[i], svert, "/");

                // 判断是否只有位置：v1
                if (svert.size() == 1)
                {
                    // 仅包含位置
                    vtype = 1;
                }

                // 判断是否包含位置和纹理坐标：v1/vt1
                if (svert.size() == 2)
                {
                    // 位置与纹理坐标
                    vtype = 2;
                }

                // 判断是否包含位置、纹理坐标和法线：v1/vt1/vn1
                // 或仅包含位置和法线：v1//vn1
                if (svert.size() == 3)
                {
                    if (svert[1] != "")
                    {
                        // 位置、纹理坐标与法线
                        vtype = 4;
                    }
                    else
                    {
                        // 位置与法线
                        vtype = 3;
                    }
                }

                // 计算并存储顶点
                switch (vtype)
                {
                    case 1: // P：位置
                    {
                        vVert.Position = algorithm::getElement(iPositions, svert[0]);
                        vVert.TextureCoordinate = Vector2(0, 0);
                        noNormal = true;
                        oVerts.push_back(vVert);
                        break;
                    }
                    case 2: // P/T：位置与纹理坐标
                    {
                        vVert.Position = algorithm::getElement(iPositions, svert[0]);
                        vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
                        noNormal = true;
                        oVerts.push_back(vVert);
                        break;
                    }
                    case 3: // P//N：位置与法线
                    {
                        vVert.Position = algorithm::getElement(iPositions, svert[0]);
                        vVert.TextureCoordinate = Vector2(0, 0);
                        vVert.Normal = algorithm::getElement(iNormals, svert[2]);
                        oVerts.push_back(vVert);
                        break;
                    }
                    case 4: // P/T/N：位置、纹理坐标与法线
                    {
                        vVert.Position = algorithm::getElement(iPositions, svert[0]);
                        vVert.TextureCoordinate = algorithm::getElement(iTCoords, svert[1]);
                        vVert.Normal = algorithm::getElement(iNormals, svert[2]);
                        oVerts.push_back(vVert);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            // 处理缺失的法线
            // 计算结果可能并不完全准确，但这是
            // 在模型没有提供法线时采用的替代方案
            if (noNormal)
            {
                Vector3 A = oVerts[0].Position - oVerts[1].Position;
                Vector3 B = oVerts[2].Position - oVerts[1].Position;

                Vector3 normal = math::CrossV3(A, B);

                for (int i = 0; i < int(oVerts.size()); i++)
                {
                    oVerts[i].Normal = normal;
                }
            }
        }

        // 将面中的顶点列表三角化，生成
        // 对应各个三角形的顶点索引
        void VertexTriangluation(std::vector<unsigned int>& oIndices,
                                 const std::vector<Vertex>& iVerts)
        {
            // 若顶点数不超过 2，
            // 则无法构成三角形，
            // 因此直接返回
            if (iVerts.size() < 3)
            {
                return;
            }
            // 若已经是三角形，则无需进一步三角化
            if (iVerts.size() == 3)
            {
                oIndices.push_back(0);
                oIndices.push_back(1);
                oIndices.push_back(2);
                return;
            }

            // 创建顶点列表
            std::vector<Vertex> tVerts = iVerts;

            while (true)
            {
                // 遍历每个顶点
                for (int i = 0; i < int(tVerts.size()); i++)
                {
                    // pPrev：列表中的前一个顶点
                    Vertex pPrev;
                    if (i == 0)
                    {
                        pPrev = tVerts[tVerts.size() - 1];
                    }
                    else
                    {
                        pPrev = tVerts[i - 1];
                    }

                    // pCur：当前顶点
                    Vertex pCur = tVerts[i];

                    // pNext：列表中的后一个顶点
                    Vertex pNext;
                    if (i == tVerts.size() - 1)
                    {
                        pNext = tVerts[0];
                    }
                    else
                    {
                        pNext = tVerts[i + 1];
                    }

                    // 检查是否只剩下 3 个顶点，
                    // 若是，则构成最后一个三角形
                    if (tVerts.size() == 3)
                    {
                        // 使用 pCur、pPrev、pNext 创建三角形
                        for (int j = 0; j < int(tVerts.size()); j++)
                        {
                            if (iVerts[j].Position == pCur.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == pPrev.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == pNext.Position)
                                oIndices.push_back(j);
                        }

                        tVerts.clear();
                        break;
                    }
                    if (tVerts.size() == 4)
                    {
                        // 使用 pCur、pPrev、pNext 创建三角形
                        for (int j = 0; j < int(iVerts.size()); j++)
                        {
                            if (iVerts[j].Position == pCur.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == pPrev.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == pNext.Position)
                                oIndices.push_back(j);
                        }

                        Vector3 tempVec;
                        for (int j = 0; j < int(tVerts.size()); j++)
                        {
                            if (tVerts[j].Position != pCur.Position
                                && tVerts[j].Position != pPrev.Position
                                && tVerts[j].Position != pNext.Position)
                            {
                                tempVec = tVerts[j].Position;
                                break;
                            }
                        }

                        // 使用 pCur、pPrev、pNext 创建三角形
                        for (int j = 0; j < int(iVerts.size()); j++)
                        {
                            if (iVerts[j].Position == pPrev.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == pNext.Position)
                                oIndices.push_back(j);
                            if (iVerts[j].Position == tempVec)
                                oIndices.push_back(j);
                        }

                        tVerts.clear();
                        break;
                    }

                    // 若该顶点不是内侧顶点
                    float angle = math::AngleBetweenV3(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359);
                    if (angle <= 0 && angle >= 180)
                        continue;

                    // 检查是否有其他顶点位于此三角形内
                    bool inTri = false;
                    for (int j = 0; j < int(iVerts.size()); j++)
                    {
                        if (algorithm::inTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
                            && iVerts[j].Position != pPrev.Position
                            && iVerts[j].Position != pCur.Position
                            && iVerts[j].Position != pNext.Position)
                        {
                            inTri = true;
                            break;
                        }
                    }
                    if (inTri)
                        continue;

                    // 使用 pCur、pPrev、pNext 创建三角形
                    for (int j = 0; j < int(iVerts.size()); j++)
                    {
                        if (iVerts[j].Position == pCur.Position)
                            oIndices.push_back(j);
                        if (iVerts[j].Position == pPrev.Position)
                            oIndices.push_back(j);
                        if (iVerts[j].Position == pNext.Position)
                            oIndices.push_back(j);
                    }

                    // 从列表中移除 pCur
                    for (int j = 0; j < int(tVerts.size()); j++)
                    {
                        if (tVerts[j].Position == pCur.Position)
                        {
                            tVerts.erase(tVerts.begin() + j);
                            break;
                        }
                    }

                    // 将 i 重置到起点
                    // 设为 -1，因为循环会再将其加 1
                    i = -1;
                }

                // 若未创建任何三角形
                if (oIndices.size() == 0)
                    break;

                // 若已没有剩余顶点
                if (tVerts.size() == 0)
                    break;
            }
        }

        // 从 .mtl 文件加载材质
        bool LoadMaterials(std::string path)
        {
            // 若不是材质文件，则返回 false
            if (path.substr(path.size() - 4, path.size()) != ".mtl")
                return false;

            std::ifstream file(path);

            // 若找不到文件，则返回 false
            if (!file.is_open())
                return false;

            Material tempMaterial;

            bool listening = false;

            // 逐行查找材质参数
            std::string curline;
            while (std::getline(file, curline))
            {
                // 新材质及其名称
                if (algorithm::firstToken(curline) == "newmtl")
                {
                    if (!listening)
                    {
                        listening = true;

                        if (curline.size() > 7)
                        {
                            tempMaterial.name = algorithm::tail(curline);
                        }
                        else
                        {
                            tempMaterial.name = "none";
                        }
                    }
                    else
                    {
                        // 生成材质

                        // 将已加载材质追加到列表
                        LoadedMaterials.push_back(tempMaterial);

                        // 清空临时材质记录
                        tempMaterial = Material();

                        if (curline.size() > 7)
                        {
                            tempMaterial.name = algorithm::tail(curline);
                        }
                        else
                        {
                            tempMaterial.name = "none";
                        }
                    }
                }
                // 环境光颜色
                if (algorithm::firstToken(curline) == "Ka")
                {
                    std::vector<std::string> temp;
                    algorithm::split(algorithm::tail(curline), temp, " ");

                    if (temp.size() != 3)
                        continue;

                    tempMaterial.Ka.X = std::stof(temp[0]);
                    tempMaterial.Ka.Y = std::stof(temp[1]);
                    tempMaterial.Ka.Z = std::stof(temp[2]);
                }
                // 漫反射颜色
                if (algorithm::firstToken(curline) == "Kd")
                {
                    std::vector<std::string> temp;
                    algorithm::split(algorithm::tail(curline), temp, " ");

                    if (temp.size() != 3)
                        continue;

                    tempMaterial.Kd.X = std::stof(temp[0]);
                    tempMaterial.Kd.Y = std::stof(temp[1]);
                    tempMaterial.Kd.Z = std::stof(temp[2]);
                }
                // 镜面反射颜色
                if (algorithm::firstToken(curline) == "Ks")
                {
                    std::vector<std::string> temp;
                    algorithm::split(algorithm::tail(curline), temp, " ");

                    if (temp.size() != 3)
                        continue;

                    tempMaterial.Ks.X = std::stof(temp[0]);
                    tempMaterial.Ks.Y = std::stof(temp[1]);
                    tempMaterial.Ks.Z = std::stof(temp[2]);
                }
                // 镜面反射指数
                if (algorithm::firstToken(curline) == "Ns")
                {
                    tempMaterial.Ns = std::stof(algorithm::tail(curline));
                }
                // 光学密度（此处为折射率）
                if (algorithm::firstToken(curline) == "Ni")
                {
                    tempMaterial.Ni = std::stof(algorithm::tail(curline));
                }
                // 不透明度
                if (algorithm::firstToken(curline) == "d")
                {
                    tempMaterial.d = std::stof(algorithm::tail(curline));
                }
                // 光照模型编号
                if (algorithm::firstToken(curline) == "illum")
                {
                    tempMaterial.illum = std::stoi(algorithm::tail(curline));
                }
                // 环境光纹理贴图
                if (algorithm::firstToken(curline) == "map_Ka")
                {
                    tempMaterial.map_Ka = algorithm::tail(curline);
                }
                // 漫反射纹理贴图
                if (algorithm::firstToken(curline) == "map_Kd")
                {
                    tempMaterial.map_Kd = algorithm::tail(curline);
                }
                // 镜面反射纹理贴图
                if (algorithm::firstToken(curline) == "map_Ks")
                {
                    tempMaterial.map_Ks = algorithm::tail(curline);
                }
                // 镜面高光指数贴图
                if (algorithm::firstToken(curline) == "map_Ns")
                {
                    tempMaterial.map_Ns = algorithm::tail(curline);
                }
                // 透明度贴图
                if (algorithm::firstToken(curline) == "map_d")
                {
                    tempMaterial.map_d = algorithm::tail(curline);
                }
                // 凹凸贴图
                if (algorithm::firstToken(curline) == "map_Bump" || algorithm::firstToken(curline) == "map_bump" || algorithm::firstToken(curline) == "bump")
                {
                    tempMaterial.map_bump = algorithm::tail(curline);
                }
            }

            // 处理最后一个材质

            // 将已加载材质追加到列表
            LoadedMaterials.push_back(tempMaterial);

            // 检查是否加载了任何材质
            // 若没有，则返回 false
            if (LoadedMaterials.empty())
                return false;
                // 若已加载，则返回 true
            else
                return true;
        }
    };
}
