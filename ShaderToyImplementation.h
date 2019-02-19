#pragma once
#include "PostProcessBase.h"
class ShaderToyImplementation :
    public PostProcessBase
{
public:
    using UniquePtr = std::unique_ptr<ShaderToyImplementation>;
    ShaderToyImplementation(const std::string& hlsl);
    ShaderToyImplementation(const std::vector<std::string>& hlsl);
    ~ShaderToyImplementation() = default;
    virtual void loadProgram(SampleCallbacks* pSample, RenderContext* pContext, Gui* pGui)override;
    void setTexture(int passIndex, const std::string& bufferName, const std::string& imageFile);
    void setTexture(int passIndex, const std::string& bufferName, Texture::SharedPtr& imageFile);

protected:
    virtual void execute()override;
    virtual void gui()override;
private:
    struct TextureParam {
        std::string file;
        std::string bufferName;
        Texture::SharedPtr pTexture;
        TextureParam(const char* f, const char* b, Texture::SharedPtr& p) { file = f; bufferName = b; pTexture = p; }
    };
    struct PassVar {
        ShaderPass pToy;
        ShaderVar vToy;
        ProgramReflection::BindLocation mToyCBBinding;
        std::vector<TextureParam> tParams;
    };
   std::vector<std::pair<std::string, PassVar>> passes;
public:
    template <typename T>
    void setParameter(const std::string& hlslName, const std::string& parameterName, T value) {
        for (auto&v : passes) {
            if (v.first == hlslName) {
                v.second.vToy[parameterName] = value;
                break;
            }
        }
    }
};
