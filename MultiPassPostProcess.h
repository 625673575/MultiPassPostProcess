/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Falcor.h"
#include "PostProcessMixer.h"
#include "PostProcessVignette.h"
#include "PostProcessSharp.h"
#include "PostProcessGlitch.h"
#include "PostProcessFilmGrain.h"
#include "PostProcessBugTV.h"
#include "PostProcessMotionBlur.h"
#include "PostProcessLut.h"
#include "ShaderToyImplementation.h"
#include "ModelViewer.h"

using namespace Falcor;

class MultiPassPostProcess : public Renderer
{
    struct SLight {
        glm::vec3 color;
        float intensity;
        SLight() {
            color = glm::vec3(1.0f);
            intensity = 1.0f;
        }
    };
   static const int LIGHT_COUNT = 10;
public:
    void onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext) override;
    void onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onShutdown(SampleCallbacks* pSample) override;
    bool onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent) override;
    void onGuiRender(SampleCallbacks* pSample, Gui* pGui) override;
    void onDroppedFile(SampleCallbacks* pSample, const std::string& filename) override;
private:
    Fbo::SharedPtr mpTempFB;
    //VideoDecoder::UniquePtr mpVideoDecoder;
    bool mEnableGaussianBlur = false;
    GaussianBlur::UniquePtr mpGaussianBlur;
    FullScreenPass::UniquePtr mpBlit;

    std::vector<PostProcessBase::UniquePtr> postProcessor;
    std::vector<ShaderToyImplementation::UniquePtr> shaderToy;
    ModelViewer::UniquePtr modelViewer;
  
    GraphicsVars::SharedPtr mpProgVars;

    ComputeProgram::SharedPtr mpComputeProg;
    ComputeState::SharedPtr mpComputeState;
    ComputeVars::SharedPtr mpComputeProgVars;
    //利用ComputeShader变成01范围内的
    Texture::SharedPtr mpDepthLiner01Texture;

    void loadImage(SampleCallbacks* pSample);
    void loadImageFromFile(SampleCallbacks* pSample, std::string filename);
    void loadVideoFromFile(SampleCallbacks* pSample);
    void loadShaderToy();
    void loadPostProcess();
    //testing 
    void onInitializeTesting(SampleCallbacks* pSample) override;
    Texture::SharedPtr pTextureSelectedFromFile = nullptr;
};
