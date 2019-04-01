#pragma once
#include "Falcor.h"
using namespace Falcor;

struct DepthStencilStateBundle {
    bool bWriteDepth = true;
    bool bDepthTest = true;
    ComparisonFunc eDepthTestFunc = ComparisonFunc::LessEqual;
    bool operator ==(const DepthStencilStateBundle& rhs);
    static std::vector<DepthStencilState::SharedPtr> state;
    static bool Get(const DepthStencilStateBundle& bundle, DepthStencilState::SharedPtr& ref);
    static DepthStencilState::SharedPtr Get(const DepthStencilStateBundle& bundle);
};

enum class ERenderQueue :int32_t {
    Background = 1000,
    Geometry = 2000,
    AlphaTest = 2450,
    Transparent = 3000,
    Overlay = 4000
};

#define DEF_VAR(vec) std::map<std::string, vec> param_##vec;\
std::vector<_Bind<vec>> bind_##vec;
#define DEF_INSERT_FUNC(vec) vec& insert_##vec(const std::string& var_name,const vec value) { param_##vec.emplace(var_name, value);return param_##vec[var_name];}\
const vec& get_##vec(const std::string& var_name){return param_##vec[var_name];};\
void bind(vec& value, std::function<vec()> f) {_Bind<vec> r;r.bind(value, f); bind_##vec.push_back(std::move(r)); }
#define BIND_THIS_VAR(var)[this](){return var; }

class MaterialInstance :std::enable_shared_from_this<MaterialInstance>
{
    template <typename T>
    struct _Bind {
    private:
        T* source;
        std::function<T()> func;
    public:
        void bind(T& v, std::function<T()> f) { source = &v; func = f; }
        void execute() { *source = func(); }
    };
public:
    friend class SceneExtend;
    using SharedPtr = std::shared_ptr<MaterialInstance>;
    static const std::string vsEntry;
    static const std::string psEntry;
public:
    MaterialInstance() = default;
    MaterialInstance(const std::string& shader, const Program::DefineList& programDefines, const std::string& name);
    MaterialInstance(const std::string& name, const  Material::SharedPtr& material = nullptr);
    //MaterialInstance(Program::SharedPtr program, const Program::DefineList& programDefines);
    ~MaterialInstance() = default;
    static MaterialInstance::SharedPtr create(const std::string& shader, const Program::DefineList& programDefines, const std::string& _name = "");
    //static MaterialInstance::SharedPtr create(Program::SharedPtr& program, const Program::DefineList& programDefines);
    bool ComaparsionQueue(const MaterialInstance* rhs)const;
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
    enum class EBlendMode {
        Opaque,
        Transparent
    };
    enum class ERasterizeMode {
        Wire,
        CullNone,
        CullBack,
        CullFront
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
    std::map<std::string, Texture::SharedPtr> param_textureCube;
    GraphicsProgram::SharedPtr mpProgram = nullptr;
    GraphicsState::SharedPtr mpState = nullptr;
    GraphicsVars::SharedPtr mpProgramVars = nullptr;
    DepthStencilState::SharedPtr mpDepthStencilState = nullptr;

    Material::SharedPtr mpMaterial = nullptr;
    bool bUseMaterial = false;
    std::string mName;
    std::string mShaderName;
    uint64_t mResId;
    bool renderOutline = false;
    DepthStencilStateBundle depthStencilBundle;
    EBlendMode blendMode;
    ERasterizeMode rasterizeMode;
    int32_t renderQueue = int32_t(ERenderQueue::Geometry);
public:
    std::string get_resName() { return mName + "res id:" + std::to_string(mResId); }
    const GraphicsProgram::SharedPtr& get_program() { return mpProgram; }
    const GraphicsState::SharedPtr& get_state() { return mpState; }
    const GraphicsVars::SharedPtr& get_programVars() { return mpProgramVars; }
    MaterialInstance& set_program(const  GraphicsProgram::SharedPtr& prog, const std::string& shaderName, bool use_default_material = false, uint32_t shaderModel = ShadingModelSpecGloss, bool resetState = true);
    MaterialInstance& set_blendMode(EBlendMode mode);
    MaterialInstance& set_rasterizeMode(ERasterizeMode mode);
    MaterialInstance& set_depthStencilTest(const DepthStencilStateBundle& bundle);
    void set_renderOutline(bool render) { renderOutline = render; }
    bool get_renderOutline() { return renderOutline; }
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
    void clear();
    void set_texture2D(const std::string& var_name, const Texture::SharedPtr& tex) { param_texture2D[var_name] = tex; }
    Texture::SharedPtr& get_texture2D(const std::string& var_name) { return param_texture2D[var_name]; }
    void set_textureCube(const std::string& var_name, const Texture::SharedPtr& tex) { param_textureCube[var_name] = tex; }
    Texture::SharedPtr& get_textureCube(const std::string& var_name) { return param_textureCube[var_name]; }

    ConstantBuffer::SharedPtr get_constantbuffer(ECBType t);
    void onMaterialGui(Gui* p);
    void onRender(RenderContext* pRenderContext);
    const std::string& getName() { return mName; }
    const std::string& getInstanceName() { return mName; }

    MaterialInstance& setWriteDepth(bool write) { depthStencilBundle.bWriteDepth = write; return *this; }
    MaterialInstance& setDepthTestFunc(ComparisonFunc func) { depthStencilBundle.eDepthTestFunc = func; return *this; }
    MaterialInstance& setRenderQueue(ERenderQueue queue) { renderQueue = uint32_t(queue); return *this; }
    MaterialInstance& setRenderQueue(uint32_t queue) { renderQueue = queue; return *this; }

    static const Gui::DropdownList kBlendModeDropDownList;
    static const Gui::DropdownList kRasterizeModeDropDownList;
    static const Gui::DropdownList kDepthTestFuncDropDownList;
    static Texture::SharedPtr WhiteTexture, BlackTexture, RedTexture, GreenTexture, BlueTexture;
    static Texture::SharedPtr pTextureNoise;
    static Texture::SharedPtr pTextureNoiseRGB;
    static Texture::SharedPtr pTextureSmoke;
    static Texture::SharedPtr pTextureWoodFloor;
    static Texture::SharedPtr pTextureSelectedFromFile;

    static RasterizerState::SharedPtr pRasterizerState_Wire;
    static RasterizerState::SharedPtr pRasterizerState_Solid_Back;
    static RasterizerState::SharedPtr pRasterizerState_Solid_Front;
    static RasterizerState::SharedPtr pRasterizerState_Solid_None;
    static BlendState::SharedPtr pBlenderState_Opaque;
    static BlendState::SharedPtr pBlenderState_Transparent;
    static void loadStaticData();
};

#undef DEF_VAR
#undef DEF_INSERT_FUNC
