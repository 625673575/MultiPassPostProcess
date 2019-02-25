#pragma once
#include "Falcor.h"
using namespace Falcor;

#define DEF_VAR(vec) std::map<std::string, vec> param_##vec;
#define DEF_INSERT_FUNC(vec) void insert_##vec(const std::string& var_name,const vec value) { param_##vec.emplace(var_name, value); }\
const vec& get_##vec(const std::string& var_name){return param_##vec[var_name];};

class MaterialInstance :std::enable_shared_from_this<MaterialInstance>
{
public:
    using SharedPtr = std::shared_ptr<MaterialInstance>;
    static const std::string vsEntry;
    static const std::string psEntry;
public:
    MaterialInstance() = default;
    MaterialInstance(const std::string& shader, const Program::DefineList& programDefines, const std::string& name);
    MaterialInstance(const std::string& name);
    //MaterialInstance(Program::SharedPtr program, const Program::DefineList& programDefines);
    ~MaterialInstance() = default;
    static MaterialInstance::SharedPtr create(const std::string& shader, const Program::DefineList& programDefines, const std::string& _name="");
    //static MaterialInstance::SharedPtr create(Program::SharedPtr& program, const Program::DefineList& programDefines);
private:

public:
    /*PerFrame 的Buffer名称是DCB，Static的是SCB*/
    enum class ECBType {
        PerFrame,
        Static
    };
    enum class EParamType {
        Bool,
        Int,
        Int2,
        Int3,
        Int4,
        Float,
        Float2,
        Float3,
        Float4,
        Float2x2,
        Float3x3,
        Float4x4,
        Texture2D
    };
private:
    DEF_VAR(bool);
    DEF_VAR(int);
    DEF_VAR(ivec2);
    DEF_VAR(ivec3);
    DEF_VAR(ivec4);
    DEF_VAR(float);
    DEF_VAR(vec2);
    DEF_VAR(vec3);
    DEF_VAR(vec4);
    DEF_VAR(mat2);
    DEF_VAR(mat3);
    DEF_VAR(mat4);
    std::map<std::string, Texture::SharedPtr> param_texture2D;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    std::string mName;
public:
    DEF_INSERT_FUNC(bool);
    DEF_INSERT_FUNC(int);
    DEF_INSERT_FUNC(ivec2);
    DEF_INSERT_FUNC(ivec3);
    DEF_INSERT_FUNC(ivec4);
    DEF_INSERT_FUNC(float);
    DEF_INSERT_FUNC(vec2);
    DEF_INSERT_FUNC(vec3);
    DEF_INSERT_FUNC(vec4);
    DEF_INSERT_FUNC(mat2);
    DEF_INSERT_FUNC(mat3);
    DEF_INSERT_FUNC(mat4);
    void set_texture2D(const std::string& var_name, const Texture::SharedPtr& tex) { param_texture2D[var_name] = tex; }
    Texture::SharedPtr& get_texture2D(const std::string& var_name) { return param_texture2D[var_name]; }
    ConstantBuffer::SharedPtr get_constantbuffer(ECBType t);
    void onMaterialGui(Gui *p);
    void onRender(RenderContext* pRenderContext);
    static Texture::SharedPtr BlankTexture;
    const std::string& getName() { return mName; }
};

#undef DEF_VAR
#undef DEF_INSERT_FUNC
