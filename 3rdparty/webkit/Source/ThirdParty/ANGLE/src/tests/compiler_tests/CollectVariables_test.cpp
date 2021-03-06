//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CollectVariables_test.cpp:
//   Some tests for shader inspection
//

#include <memory>

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorGLSL.h"

using namespace sh;

#define EXPECT_GLENUM_EQ(expected, actual) \
    EXPECT_EQ(static_cast<::GLenum>(expected), static_cast<::GLenum>(actual))

namespace
{

std::string DecorateName(const char *name)
{
    return std::string("_u") + name;
}

}  // anonymous namespace

class CollectVariablesTest : public testing::Test
{
  public:
    CollectVariablesTest(::GLenum shaderType) : mShaderType(shaderType) {}

  protected:
    void SetUp() override
    {
        ShBuiltInResources resources;
        InitBuiltInResources(&resources);
        resources.MaxDrawBuffers = 8;

        initTranslator(resources);
    }

    virtual void initTranslator(const ShBuiltInResources &resources)
    {
        mTranslator.reset(
            new TranslatorGLSL(mShaderType, SH_GLES3_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT));
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    // For use in the gl_DepthRange tests.
    void validateDepthRangeShader(const std::string &shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        ASSERT_TRUE(mTranslator->compile(shaderStrings, 1, SH_VARIABLES));

        const std::vector<Uniform> &uniforms = mTranslator->getUniforms();
        ASSERT_EQ(1u, uniforms.size());

        const Uniform &uniform = uniforms[0];
        EXPECT_EQ("gl_DepthRange", uniform.name);
        ASSERT_TRUE(uniform.isStruct());
        ASSERT_EQ(3u, uniform.fields.size());

        bool foundNear = false;
        bool foundFar = false;
        bool foundDiff = false;

        for (const auto &field : uniform.fields)
        {
            if (field.name == "near")
            {
                EXPECT_FALSE(foundNear);
                foundNear = true;
            }
            else if (field.name == "far")
            {
                EXPECT_FALSE(foundFar);
                foundFar = true;
            }
            else
            {
                ASSERT_EQ("diff", field.name);
                EXPECT_FALSE(foundDiff);
                foundDiff = true;
            }

            EXPECT_FALSE(field.isArray());
            EXPECT_FALSE(field.isStruct());
            EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, field.precision);
            EXPECT_TRUE(field.staticUse);
            EXPECT_GLENUM_EQ(GL_FLOAT, field.type);
        }

        EXPECT_TRUE(foundNear && foundFar && foundDiff);
    }

    // For use in tests for output varibles.
    void validateOutputVariableForShader(const std::string &shaderString,
                                         unsigned int varIndex,
                                         const char *varName,
                                         const OutputVariable **outResult)
    {
        const char *shaderStrings[] = {shaderString.c_str()};
        ASSERT_TRUE(mTranslator->compile(shaderStrings, 1, SH_VARIABLES))
            << mTranslator->getInfoSink().info.str();

        const auto &outputVariables = mTranslator->getOutputVariables();
        ASSERT_LT(varIndex, outputVariables.size());
        const OutputVariable &outputVariable = outputVariables[varIndex];
        EXPECT_EQ(-1, outputVariable.location);
        EXPECT_TRUE(outputVariable.staticUse);
        EXPECT_EQ(varName, outputVariable.name);
        *outResult = &outputVariable;
    }

    void compile(const std::string &shaderString, ShCompileOptions compileOptions)
    {
        const char *shaderStrings[] = {shaderString.c_str()};
        ASSERT_TRUE(mTranslator->compile(shaderStrings, 1, SH_VARIABLES | compileOptions));
    }

    void compile(const std::string &shaderString) { compile(shaderString, 0u); }

    ::GLenum mShaderType;
    std::unique_ptr<TranslatorGLSL> mTranslator;
};

class CollectVertexVariablesTest : public CollectVariablesTest
{
  public:
    CollectVertexVariablesTest() : CollectVariablesTest(GL_VERTEX_SHADER) {}
};

class CollectFragmentVariablesTest : public CollectVariablesTest
{
  public:
      CollectFragmentVariablesTest() : CollectVariablesTest(GL_FRAGMENT_SHADER) {}
};

class CollectVariablesTestES31 : public CollectVariablesTest
{
  public:
    CollectVariablesTestES31(sh::GLenum shaderType) : CollectVariablesTest(shaderType) {}

  protected:
    void initTranslator(const ShBuiltInResources &resources) override
    {
        mTranslator.reset(
            new TranslatorGLSL(mShaderType, SH_GLES3_1_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT));
        ASSERT_TRUE(mTranslator->Init(resources));
    }
};

class CollectVariablesOESGeometryShaderTest : public CollectVariablesTestES31
{
  public:
    CollectVariablesOESGeometryShaderTest(sh::GLenum shaderType)
        : CollectVariablesTestES31(shaderType)
    {
    }

  protected:
    void SetUp() override
    {
        ShBuiltInResources resources;
        InitBuiltInResources(&resources);
        resources.OES_geometry_shader = 1;

        initTranslator(resources);
    }
};

class CollectGeometryVariablesTest : public CollectVariablesOESGeometryShaderTest
{
  public:
    CollectGeometryVariablesTest() : CollectVariablesOESGeometryShaderTest(GL_GEOMETRY_SHADER_OES)
    {
    }

  protected:
    void compileGeometryShaderWithInputPrimitive(const std::string &inputPrimitive,
                                                 const std::string &inputVarying,
                                                 const std::string &functionBody)
    {
        std::ostringstream sstream;
        sstream << "#version 310 es\n"
                << "#extension GL_OES_geometry_shader : require\n"
                << "layout (" << inputPrimitive << ") in;\n"
                << "layout (points, max_vertices = 2) out;\n"
                << inputVarying << functionBody;

        compile(sstream.str());
    }
};

class CollectFragmentVariablesOESGeometryShaderTest : public CollectVariablesOESGeometryShaderTest
{
  public:
    CollectFragmentVariablesOESGeometryShaderTest()
        : CollectVariablesOESGeometryShaderTest(GL_FRAGMENT_SHADER)
    {
    }

  protected:
    void initTranslator(const ShBuiltInResources &resources)
    {
        mTranslator.reset(
            new TranslatorGLSL(mShaderType, SH_GLES3_1_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT));
        ASSERT_TRUE(mTranslator->Init(resources));
    }
};

class CollectVertexVariablesES31Test : public CollectVariablesTestES31
{
  public:
    CollectVertexVariablesES31Test() : CollectVariablesTestES31(GL_VERTEX_SHADER) {}
};

class CollectFragmentVariablesES31Test : public CollectVariablesTestES31
{
  public:
    CollectFragmentVariablesES31Test() : CollectVariablesTestES31(GL_FRAGMENT_SHADER) {}
};

TEST_F(CollectFragmentVariablesTest, SimpleOutputVar)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out_fragColor;\n"
        "void main() {\n"
        "   out_fragColor = vec4(1.0);\n"
        "}\n";

    compile(shaderString);

    const auto &outputVariables = mTranslator->getOutputVariables();
    ASSERT_EQ(1u, outputVariables.size());

    const OutputVariable &outputVariable = outputVariables[0];

    EXPECT_FALSE(outputVariable.isArray());
    EXPECT_EQ(-1, outputVariable.location);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable.precision);
    EXPECT_TRUE(outputVariable.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable.type);
    EXPECT_EQ("out_fragColor", outputVariable.name);
}

TEST_F(CollectFragmentVariablesTest, LocationOutputVar)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location=5) out vec4 out_fragColor;\n"
        "void main() {\n"
        "   out_fragColor = vec4(1.0);\n"
        "}\n";

    compile(shaderString);

    const auto &outputVariables = mTranslator->getOutputVariables();
    ASSERT_EQ(1u, outputVariables.size());

    const OutputVariable &outputVariable = outputVariables[0];

    EXPECT_FALSE(outputVariable.isArray());
    EXPECT_EQ(5, outputVariable.location);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable.precision);
    EXPECT_TRUE(outputVariable.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable.type);
    EXPECT_EQ("out_fragColor", outputVariable.name);
}

TEST_F(CollectVertexVariablesTest, LocationAttribute)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "layout(location=5) in vec4 in_Position;\n"
        "void main() {\n"
        "   gl_Position = in_Position;\n"
        "}\n";

    compile(shaderString);

    const std::vector<Attribute> &attributes = mTranslator->getAttributes();
    ASSERT_EQ(1u, attributes.size());

    const Attribute &attribute = attributes[0];

    EXPECT_FALSE(attribute.isArray());
    EXPECT_EQ(5, attribute.location);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, attribute.precision);
    EXPECT_TRUE(attribute.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, attribute.type);
    EXPECT_EQ("in_Position", attribute.name);
}

TEST_F(CollectVertexVariablesTest, SimpleInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform b {\n"
        "  float f;\n"
        "};"
        "void main() {\n"
        "   gl_Position = vec4(f, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_FALSE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("b", interfaceBlock.name);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, field.precision);
    EXPECT_TRUE(field.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, field.type);
    EXPECT_EQ("f", field.name);
    EXPECT_FALSE(field.isRowMajorLayout);
    EXPECT_TRUE(field.fields.empty());
}

TEST_F(CollectVertexVariablesTest, SimpleInstancedInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform b {\n"
        "  float f;\n"
        "} blockInstance;"
        "void main() {\n"
        "   gl_Position = vec4(blockInstance.f, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_FALSE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("b", interfaceBlock.name);
    EXPECT_EQ("blockInstance", interfaceBlock.instanceName);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, field.precision);
    EXPECT_TRUE(field.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, field.type);
    EXPECT_EQ("f", field.name);
    EXPECT_FALSE(field.isRowMajorLayout);
    EXPECT_TRUE(field.fields.empty());
}

TEST_F(CollectVertexVariablesTest, StructInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct st { float f; };"
        "uniform b {\n"
        "  st s;\n"
        "};"
        "void main() {\n"
        "   gl_Position = vec4(s.f, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_FALSE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("b", interfaceBlock.name);
    EXPECT_EQ(DecorateName("b"), interfaceBlock.mappedName);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_TRUE(field.isStruct());
    EXPECT_TRUE(field.staticUse);
    EXPECT_EQ("s", field.name);
    EXPECT_EQ(DecorateName("s"), field.mappedName);
    EXPECT_FALSE(field.isRowMajorLayout);

    const ShaderVariable &member = field.fields[0];

    // NOTE: we don't currently mark struct members as statically used or not
    EXPECT_FALSE(member.isStruct());
    EXPECT_EQ("f", member.name);
    EXPECT_EQ(DecorateName("f"), member.mappedName);
    EXPECT_GLENUM_EQ(GL_FLOAT, member.type);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, member.precision);
}

TEST_F(CollectVertexVariablesTest, StructInstancedInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct st { float f; };"
        "uniform b {\n"
        "  st s;\n"
        "} instanceName;"
        "void main() {\n"
        "   gl_Position = vec4(instanceName.s.f, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_FALSE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("b", interfaceBlock.name);
    EXPECT_EQ(DecorateName("b"), interfaceBlock.mappedName);
    EXPECT_EQ("instanceName", interfaceBlock.instanceName);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_TRUE(field.isStruct());
    EXPECT_TRUE(field.staticUse);
    EXPECT_EQ("s", field.name);
    EXPECT_EQ(DecorateName("s"), field.mappedName);
    EXPECT_FALSE(field.isRowMajorLayout);

    const ShaderVariable &member = field.fields[0];

    // NOTE: we don't currently mark struct members as statically used or not
    EXPECT_FALSE(member.isStruct());
    EXPECT_EQ("f", member.name);
    EXPECT_EQ(DecorateName("f"), member.mappedName);
    EXPECT_GLENUM_EQ(GL_FLOAT, member.type);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, member.precision);
}

TEST_F(CollectVertexVariablesTest, NestedStructRowMajorInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct st { mat2 m; };"
        "layout(row_major) uniform b {\n"
        "  st s;\n"
        "};"
        "void main() {\n"
        "   gl_Position = vec4(s.m);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_TRUE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("b", interfaceBlock.name);
    EXPECT_EQ(DecorateName("b"), interfaceBlock.mappedName);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_TRUE(field.isStruct());
    EXPECT_TRUE(field.staticUse);
    EXPECT_EQ("s", field.name);
    EXPECT_EQ(DecorateName("s"), field.mappedName);
    EXPECT_TRUE(field.isRowMajorLayout);

    const ShaderVariable &member = field.fields[0];

    // NOTE: we don't currently mark struct members as statically used or not
    EXPECT_FALSE(member.isStruct());
    EXPECT_EQ("m", member.name);
    EXPECT_EQ(DecorateName("m"), member.mappedName);
    EXPECT_GLENUM_EQ(GL_FLOAT_MAT2, member.type);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, member.precision);
}

TEST_F(CollectVertexVariablesTest, VaryingInterpolation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "centroid out float vary;\n"
        "void main() {\n"
        "   gl_Position = vec4(1.0);\n"
        "   vary = 1.0;\n"
        "}\n";

    compile(shaderString);

    const std::vector<Varying> &varyings = mTranslator->getOutputVaryings();
    ASSERT_EQ(2u, varyings.size());

    const Varying *varying = &varyings[0];

    if (varying->name == "gl_Position")
    {
        varying = &varyings[1];
    }

    EXPECT_FALSE(varying->isArray());
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, varying->precision);
    EXPECT_TRUE(varying->staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, varying->type);
    EXPECT_EQ("vary", varying->name);
    EXPECT_EQ(DecorateName("vary"), varying->mappedName);
    EXPECT_EQ(INTERPOLATION_CENTROID, varying->interpolation);
}

// Test for builtin uniform "gl_DepthRange" (Vertex shader)
TEST_F(CollectVertexVariablesTest, DepthRange)
{
    const std::string &shaderString =
        "attribute vec4 position;\n"
        "void main() {\n"
        "   gl_Position = position + vec4(gl_DepthRange.near, gl_DepthRange.far, gl_DepthRange.diff, 1.0);\n"
        "}\n";

    validateDepthRangeShader(shaderString);
}

// Test for builtin uniform "gl_DepthRange" (Fragment shader)
TEST_F(CollectFragmentVariablesTest, DepthRange)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(gl_DepthRange.near, gl_DepthRange.far, gl_DepthRange.diff, 1.0);\n"
        "}\n";

    validateDepthRangeShader(shaderString);
}

// Test that gl_FragColor built-in usage in ESSL1 fragment shader is reflected in the output
// variables list.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1FragColor)
{
    const std::string &fragColorShader =
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(1.0);\n"
        "}\n";

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(fragColorShader, 0u, "gl_FragColor", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);
}

// Test that gl_FragData built-in usage in ESSL1 fragment shader is reflected in the output
// variables list.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1FragData)
{
    const std::string &fragDataShader =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.0);\n"
        "   gl_FragData[1] = vec4(0.5);\n"
        "}\n";

    ShBuiltInResources resources       = mTranslator->getResources();
    resources.EXT_draw_buffers         = 1;
    const unsigned int kMaxDrawBuffers = 3u;
    resources.MaxDrawBuffers = kMaxDrawBuffers;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(fragDataShader, 0u, "gl_FragData", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    ASSERT_EQ(1u, outputVariable->arraySizes.size());
    EXPECT_EQ(kMaxDrawBuffers, outputVariable->arraySizes.back());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);
}

// Test that gl_FragDataEXT built-in usage in ESSL1 fragment shader is reflected in the output
// variables list. Also test that the precision is mediump.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1FragDepthMediump)
{
    const std::string &fragDepthShader =
        "#extension GL_EXT_frag_depth : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragDepthEXT = 0.7;"
        "}\n";

    ShBuiltInResources resources = mTranslator->getResources();
    resources.EXT_frag_depth = 1;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(fragDepthShader, 0u, "gl_FragDepthEXT", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);
}

// Test that gl_FragDataEXT built-in usage in ESSL1 fragment shader is reflected in the output
// variables list. Also test that the precision is highp if user requests it.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1FragDepthHighp)
{
    const std::string &fragDepthHighShader =
        "#extension GL_EXT_frag_depth : require\n"
        "void main() {\n"
        "   gl_FragDepthEXT = 0.7;"
        "}\n";

    ShBuiltInResources resources    = mTranslator->getResources();
    resources.EXT_frag_depth        = 1;
    resources.FragmentPrecisionHigh = 1;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(fragDepthHighShader, 0u, "gl_FragDepthEXT", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, outputVariable->precision);
}

// Test that gl_FragData built-in usage in ESSL3 fragment shader is reflected in the output
// variables list. Also test that the precision is highp.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL3FragDepthHighp)
{
    const std::string &fragDepthHighShader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragDepth = 0.7;"
        "}\n";

    ShBuiltInResources resources = mTranslator->getResources();
    resources.EXT_frag_depth = 1;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(fragDepthHighShader, 0u, "gl_FragDepth", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, outputVariable->precision);
}

// Test that gl_SecondaryFragColorEXT built-in usage in ESSL1 fragment shader is reflected in the
// output variables list.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1EXTBlendFuncExtendedSecondaryFragColor)
{
    const char *secondaryFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(1.0);\n"
        "   gl_SecondaryFragColorEXT = vec4(1.0);\n"
        "}\n";

    const unsigned int kMaxDrawBuffers = 3u;
    ShBuiltInResources resources       = mTranslator->getResources();
    resources.EXT_blend_func_extended  = 1;
    resources.EXT_draw_buffers         = 1;
    resources.MaxDrawBuffers           = kMaxDrawBuffers;
    resources.MaxDualSourceDrawBuffers = resources.MaxDrawBuffers;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(secondaryFragColorShader, 0u, "gl_FragColor", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);

    outputVariable = nullptr;
    validateOutputVariableForShader(secondaryFragColorShader, 1u, "gl_SecondaryFragColorEXT",
                                    &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    EXPECT_FALSE(outputVariable->isArray());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);
}

// Test that gl_SecondaryFragDataEXT built-in usage in ESSL1 fragment shader is reflected in the
// output variables list.
TEST_F(CollectFragmentVariablesTest, OutputVarESSL1EXTBlendFuncExtendedSecondaryFragData)
{
    const char *secondaryFragDataShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.0);\n"
        "   gl_FragData[1] = vec4(0.5);\n"
        "   gl_SecondaryFragDataEXT[0] = vec4(1.0);\n"
        "   gl_SecondaryFragDataEXT[1] = vec4(0.8);\n"
        "}\n";
    const unsigned int kMaxDrawBuffers = 3u;
    ShBuiltInResources resources       = mTranslator->getResources();
    resources.EXT_blend_func_extended  = 1;
    resources.EXT_draw_buffers         = 1;
    resources.MaxDrawBuffers           = kMaxDrawBuffers;
    resources.MaxDualSourceDrawBuffers = resources.MaxDrawBuffers;
    initTranslator(resources);

    const OutputVariable *outputVariable = nullptr;
    validateOutputVariableForShader(secondaryFragDataShader, 0u, "gl_FragData", &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    ASSERT_EQ(1u, outputVariable->arraySizes.size());
    EXPECT_EQ(kMaxDrawBuffers, outputVariable->arraySizes.back());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);

    outputVariable = nullptr;
    validateOutputVariableForShader(secondaryFragDataShader, 1u, "gl_SecondaryFragDataEXT",
                                    &outputVariable);
    ASSERT_NE(outputVariable, nullptr);
    ASSERT_EQ(1u, outputVariable->arraySizes.size());
    EXPECT_EQ(kMaxDrawBuffers, outputVariable->arraySizes.back());
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, outputVariable->type);
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, outputVariable->precision);
}

static khronos_uint64_t SimpleTestHash(const char *str, size_t len)
{
    return static_cast<uint64_t>(len);
}

class CollectHashedVertexVariablesTest : public CollectVertexVariablesTest
{
  protected:
    void SetUp() override
    {
        // Initialize the translate with a hash function
        ShBuiltInResources resources;
        sh::InitBuiltInResources(&resources);
        resources.HashFunction = SimpleTestHash;
        initTranslator(resources);
    }
};

TEST_F(CollectHashedVertexVariablesTest, InstancedInterfaceBlock)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform blockName {\n"
        "  float field;\n"
        "} blockInstance;"
        "void main() {\n"
        "   gl_Position = vec4(blockInstance.field, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const std::vector<InterfaceBlock> &interfaceBlocks = mTranslator->getInterfaceBlocks();
    ASSERT_EQ(1u, interfaceBlocks.size());

    const InterfaceBlock &interfaceBlock = interfaceBlocks[0];

    EXPECT_EQ(0u, interfaceBlock.arraySize);
    EXPECT_FALSE(interfaceBlock.isRowMajorLayout);
    EXPECT_EQ(BLOCKLAYOUT_SHARED, interfaceBlock.layout);
    EXPECT_EQ("blockName", interfaceBlock.name);
    EXPECT_EQ("blockInstance", interfaceBlock.instanceName);
    EXPECT_EQ("webgl_9", interfaceBlock.mappedName);
    EXPECT_TRUE(interfaceBlock.staticUse);

    ASSERT_EQ(1u, interfaceBlock.fields.size());

    const InterfaceBlockField &field = interfaceBlock.fields[0];

    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, field.precision);
    EXPECT_TRUE(field.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, field.type);
    EXPECT_EQ("field", field.name);
    EXPECT_EQ("webgl_5", field.mappedName);
    EXPECT_FALSE(field.isRowMajorLayout);
    EXPECT_TRUE(field.fields.empty());
}

TEST_F(CollectHashedVertexVariablesTest, StructUniform)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct sType {\n"
        "  float field;\n"
        "};"
        "uniform sType u;"
        "void main() {\n"
        "   gl_Position = vec4(u.field, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const auto &uniforms = mTranslator->getUniforms();
    ASSERT_EQ(1u, uniforms.size());

    const Uniform &uniform = uniforms[0];

    EXPECT_FALSE(uniform.isArray());
    EXPECT_EQ("u", uniform.name);
    EXPECT_EQ("webgl_1", uniform.mappedName);
    EXPECT_TRUE(uniform.staticUse);

    ASSERT_EQ(1u, uniform.fields.size());

    const ShaderVariable &field = uniform.fields[0];

    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, field.precision);
    // EXPECT_TRUE(field.staticUse); // we don't yet support struct static use
    EXPECT_GLENUM_EQ(GL_FLOAT, field.type);
    EXPECT_EQ("field", field.name);
    EXPECT_EQ("webgl_5", field.mappedName);
    EXPECT_TRUE(field.fields.empty());
}

// Test a uniform declaration with multiple declarators.
TEST_F(CollectFragmentVariablesTest, MultiDeclaration)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out_fragColor;\n"
        "uniform float uA, uB;\n"
        "void main()\n"
        "{\n"
        "    vec4 color = vec4(uA, uA, uA, uB);\n"
        "    out_fragColor = color;\n"
        "}\n";

    compile(shaderString);

    const auto &uniforms = mTranslator->getUniforms();
    ASSERT_EQ(2u, uniforms.size());

    const Uniform &uniform = uniforms[0];
    EXPECT_FALSE(uniform.isArray());
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, uniform.precision);
    EXPECT_TRUE(uniform.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, uniform.type);
    EXPECT_EQ("uA", uniform.name);

    const Uniform &uniformB = uniforms[1];
    EXPECT_FALSE(uniformB.isArray());
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, uniformB.precision);
    EXPECT_TRUE(uniformB.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, uniformB.type);
    EXPECT_EQ("uB", uniformB.name);
}

// Test a uniform declaration starting with an empty declarator.
TEST_F(CollectFragmentVariablesTest, EmptyDeclarator)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out_fragColor;\n"
        "uniform float /* empty declarator */, uB;\n"
        "void main()\n"
        "{\n"
        "    out_fragColor = vec4(uB, uB, uB, uB);\n"
        "}\n";

    compile(shaderString);

    const auto &uniforms = mTranslator->getUniforms();
    ASSERT_EQ(1u, uniforms.size());

    const Uniform &uniformB = uniforms[0];
    EXPECT_FALSE(uniformB.isArray());
    EXPECT_GLENUM_EQ(GL_MEDIUM_FLOAT, uniformB.precision);
    EXPECT_TRUE(uniformB.staticUse);
    EXPECT_GLENUM_EQ(GL_FLOAT, uniformB.type);
    EXPECT_EQ("uB", uniformB.name);
}

// Test collecting variables from an instanced multiview shader that has an internal ViewID_OVR
// varying.
TEST_F(CollectVertexVariablesTest, ViewID_OVR)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0.0);\n"
        "}\n";

    ShBuiltInResources resources = mTranslator->getResources();
    resources.OVR_multiview      = 1;
    resources.MaxViewsOVR        = 4;
    initTranslator(resources);

    compile(shaderString, SH_INITIALIZE_BUILTINS_FOR_INSTANCED_MULTIVIEW |
                              SH_SELECT_VIEW_IN_NV_GLSL_VERTEX_SHADER);

    // The internal ViewID_OVR varying is not exposed through the ShaderVars interface.
    const auto &varyings = mTranslator->getOutputVaryings();
    ASSERT_EQ(1u, varyings.size());
    const Varying *varying = &varyings[0];
    EXPECT_EQ("gl_Position", varying->name);
}

// Test all the fields of gl_in can be collected correctly in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectGLInFields)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require

        layout (points) in;
        layout (points, max_vertices = 2) out;

        void main()
        {
            vec4 value = gl_in[0].gl_Position;
            vec4 value2 = gl_in[0].gl_Position;
            gl_Position = value + value2;
            EmitVertex();
        })";

    compile(shaderString);

    EXPECT_EQ(1u, mTranslator->getOutputVaryings().size());
    EXPECT_TRUE(mTranslator->getInputVaryings().empty());

    const auto &inBlocks = mTranslator->getInBlocks();
    ASSERT_EQ(1u, inBlocks.size());

    const InterfaceBlock *inBlock = &inBlocks[0];
    EXPECT_EQ("gl_PerVertex", inBlock->name);
    EXPECT_EQ("gl_in", inBlock->instanceName);
    EXPECT_TRUE(inBlock->staticUse);
    EXPECT_TRUE(inBlock->isBuiltIn());

    ASSERT_EQ(1u, inBlock->fields.size());

    const ShaderVariable &glPositionField = inBlock->fields[0];
    EXPECT_EQ("gl_Position", glPositionField.name);
    EXPECT_FALSE(glPositionField.isArray());
    EXPECT_FALSE(glPositionField.isStruct());
    EXPECT_TRUE(glPositionField.staticUse);
    EXPECT_TRUE(glPositionField.isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, glPositionField.precision);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, glPositionField.type);
}

// Test the collected array size of gl_in matches the input primitive declaration.
TEST_F(CollectGeometryVariablesTest, GLInArraySize)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    const GLuint kArraySizeForInputPrimitives[] = {1u, 2u, 4u, 3u, 6u};

    const std::string &functionBody =
        R"(void main()
        {
            gl_Position = gl_in[0].gl_Position;
        })";

    for (size_t i = 0; i < kInputPrimitives.size(); ++i)
    {
        compileGeometryShaderWithInputPrimitive(kInputPrimitives[i], "", functionBody);

        const auto &inBlocks = mTranslator->getInBlocks();
        ASSERT_EQ(1u, inBlocks.size());

        const InterfaceBlock *inBlock = &inBlocks[0];
        ASSERT_EQ("gl_in", inBlock->instanceName);
        EXPECT_EQ(kArraySizeForInputPrimitives[i], inBlock->arraySize);
    }
}

// Test collecting gl_PrimitiveIDIn in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectPrimitiveIDIn)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = vec4(gl_PrimitiveIDIn);
            EmitVertex();
        })";

    compile(shaderString);

    EXPECT_EQ(1u, mTranslator->getOutputVaryings().size());
    ASSERT_TRUE(mTranslator->getInBlocks().empty());

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(1u, inputVaryings.size());

    const Varying *varying = &inputVaryings[0];
    EXPECT_EQ("gl_PrimitiveIDIn", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting gl_InvocationID in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectInvocationID)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require
        layout (points, invocations = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = vec4(gl_InvocationID);
            EmitVertex();
        })";

    compile(shaderString);

    EXPECT_EQ(1u, mTranslator->getOutputVaryings().size());
    ASSERT_TRUE(mTranslator->getInBlocks().empty());

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(1u, inputVaryings.size());

    const Varying *varying = &inputVaryings[0];
    EXPECT_EQ("gl_InvocationID", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting gl_in in a geometry shader when gl_in is indexed by an expression.
TEST_F(CollectGeometryVariablesTest, CollectGLInIndexedByExpression)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require
        layout (triangles, invocations = 2) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
            gl_Position = gl_in[gl_InvocationID + 1].gl_Position;
            EmitVertex();
        })";

    compile(shaderString);

    EXPECT_EQ(1u, mTranslator->getOutputVaryings().size());

    const auto &inBlocks = mTranslator->getInBlocks();
    ASSERT_EQ(1u, inBlocks.size());
    const InterfaceBlock *inBlock = &inBlocks[0];
    EXPECT_EQ("gl_PerVertex", inBlock->name);
    EXPECT_EQ("gl_in", inBlock->instanceName);

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(1u, inputVaryings.size());
    const Varying *glInvocationID = &inputVaryings[0];
    EXPECT_EQ("gl_InvocationID", glInvocationID->name);
}

// Test collecting gl_Position in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectPosition)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0.1, 0.2, 0.3, 1);\n"
        "}\n";

    compile(shaderString);

    ASSERT_TRUE(mTranslator->getInputVaryings().empty());
    ASSERT_TRUE(mTranslator->getInBlocks().empty());

    const auto &outputVaryings = mTranslator->getOutputVaryings();
    ASSERT_EQ(1u, outputVaryings.size());

    const Varying *varying = &outputVaryings[0];
    EXPECT_EQ("gl_Position", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, varying->precision);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, varying->type);
}

// Test collecting gl_PrimitiveID in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectPrimitiveID)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_PrimitiveID = 100;\n"
        "}\n";

    compile(shaderString);

    ASSERT_TRUE(mTranslator->getInputVaryings().empty());
    ASSERT_TRUE(mTranslator->getInBlocks().empty());

    const auto &OutputVaryings = mTranslator->getOutputVaryings();
    ASSERT_EQ(1u, OutputVaryings.size());

    const Varying *varying = &OutputVaryings[0];
    EXPECT_EQ("gl_PrimitiveID", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting gl_Layer in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectLayer)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_Layer = 2;\n"
        "}\n";

    compile(shaderString);

    ASSERT_TRUE(mTranslator->getInputVaryings().empty());
    ASSERT_TRUE(mTranslator->getInBlocks().empty());

    const auto &OutputVaryings = mTranslator->getOutputVaryings();
    ASSERT_EQ(1u, OutputVaryings.size());

    const Varying *varying = &OutputVaryings[0];
    EXPECT_EQ("gl_Layer", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting gl_PrimitiveID in a fragment shader.
TEST_F(CollectFragmentVariablesOESGeometryShaderTest, CollectPrimitiveID)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require

        out int my_out;

        void main()
        {
            my_out = gl_PrimitiveID;
        })";

    compile(shaderString);

    ASSERT_TRUE(mTranslator->getOutputVaryings().empty());

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(1u, inputVaryings.size());

    const Varying *varying = &inputVaryings[0];
    EXPECT_EQ("gl_PrimitiveID", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting gl_Layer in a fragment shader.
TEST_F(CollectFragmentVariablesOESGeometryShaderTest, CollectLayer)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require

        out int my_out;

        void main()
        {
            my_out = gl_Layer;
        })";

    compile(shaderString);

    ASSERT_TRUE(mTranslator->getOutputVaryings().empty());

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(1u, inputVaryings.size());

    const Varying *varying = &inputVaryings[0];
    EXPECT_EQ("gl_Layer", varying->name);
    EXPECT_FALSE(varying->isArray());
    EXPECT_FALSE(varying->isStruct());
    EXPECT_TRUE(varying->staticUse);
    EXPECT_TRUE(varying->isBuiltIn());
    EXPECT_GLENUM_EQ(GL_HIGH_INT, varying->precision);
    EXPECT_GLENUM_EQ(GL_INT, varying->type);
}

// Test collecting the location of vertex shader outputs.
TEST_F(CollectVertexVariablesES31Test, CollectOutputWithLocation)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "out vec4 v_output1;\n"
        "layout (location = 1) out vec4 v_output2;\n"
        "void main()\n"
        "{\n"
        "}\n";

    compile(shaderString);

    const auto &outputVaryings = mTranslator->getOutputVaryings();
    ASSERT_EQ(2u, outputVaryings.size());

    const Varying *varying1 = &outputVaryings[0];
    EXPECT_EQ("v_output1", varying1->name);
    EXPECT_EQ(-1, varying1->location);

    const Varying *varying2 = &outputVaryings[1];
    EXPECT_EQ("v_output2", varying2->name);
    EXPECT_EQ(1, varying2->location);
}

// Test collecting the location of fragment shader inputs.
TEST_F(CollectFragmentVariablesES31Test, CollectInputWithLocation)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 f_input1;\n"
        "layout (location = 1) in vec4 f_input2;\n"
        "layout (location = 0) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    o_color = f_input2;\n"
        "}\n";

    compile(shaderString);

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(2u, inputVaryings.size());

    const Varying *varying1 = &inputVaryings[0];
    EXPECT_EQ("f_input1", varying1->name);
    EXPECT_EQ(-1, varying1->location);

    const Varying *varying2 = &inputVaryings[1];
    EXPECT_EQ("f_input2", varying2->name);
    EXPECT_EQ(1, varying2->location);
}

// Test collecting the inputs of a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectInputs)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_OES_geometry_shader : require
        layout (points) in;
        layout (points, max_vertices = 2) out;
        in vec4 texcoord1[];
        in vec4 texcoord2[1];
        void main()
        {
            gl_Position = texcoord1[0];
            gl_Position += texcoord2[0];
            EmitVertex();
        })";

    compile(shaderString);

    EXPECT_EQ(1u, mTranslator->getOutputVaryings().size());

    const auto &inputVaryings = mTranslator->getInputVaryings();
    ASSERT_EQ(2u, inputVaryings.size());

    const std::string kVaryingName[] = {"texcoord1", "texcoord2"};

    for (size_t i = 0; i < inputVaryings.size(); ++i)
    {
        const Varying &varying = inputVaryings[i];

        EXPECT_EQ(kVaryingName[i], varying.name);
        EXPECT_TRUE(varying.isArray());
        EXPECT_FALSE(varying.isStruct());
        EXPECT_TRUE(varying.staticUse);
        EXPECT_FALSE(varying.isBuiltIn());
        EXPECT_GLENUM_EQ(GL_HIGH_FLOAT, varying.precision);
        EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, varying.type);
        EXPECT_FALSE(varying.isInvariant);
        ASSERT_EQ(1u, varying.arraySizes.size());
        EXPECT_EQ(1u, varying.arraySizes.back());
    }
}

// Test that the unsized input of a geometry shader can be correctly collected.
TEST_F(CollectGeometryVariablesTest, CollectInputArraySizeForUnsizedInput)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    const GLuint kArraySizeForInputPrimitives[] = {1u, 2u, 4u, 3u, 6u};

    const std::string &kVariableDeclaration = "in vec4 texcoord[];\n";
    const std::string &kFunctionBody =
        R"(void main()
        {
            gl_Position = texcoord[0];
        })";

    for (size_t i = 0; i < kInputPrimitives.size(); ++i)
    {
        compileGeometryShaderWithInputPrimitive(kInputPrimitives[i], kVariableDeclaration,
                                                kFunctionBody);

        const auto &inputVaryings = mTranslator->getInputVaryings();
        ASSERT_EQ(1u, inputVaryings.size());

        const Varying *varying = &inputVaryings[0];
        EXPECT_EQ("texcoord", varying->name);
        ASSERT_EQ(1u, varying->arraySizes.size());
        EXPECT_EQ(kArraySizeForInputPrimitives[i], varying->arraySizes.back());
    }
}

// Test collecting inputs using interpolation qualifiers in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectInputsWithInterpolationQualifiers)
{
    const std::string &kHeader =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n";
    const std::string &kLayout =
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n";

    const std::array<std::string, 3> kInterpolationQualifiers = {{"flat", "smooth", "centroid"}};

    const std::array<InterpolationType, 3> kInterpolationType = {
        {INTERPOLATION_FLAT, INTERPOLATION_SMOOTH, INTERPOLATION_CENTROID}};

    const std::string &kFunctionBody =
        R"(void main()
        {
            gl_Position = texcoord[0];
            EmitVertex();
        })";

    for (size_t i = 0; i < kInterpolationQualifiers.size(); ++i)
    {
        const std::string &qualifier = kInterpolationQualifiers[i];

        std::ostringstream stream1;
        stream1 << kHeader << kLayout << qualifier << " in vec4 texcoord[];\n" << kFunctionBody;
        compile(stream1.str());

        const auto &inputVaryings = mTranslator->getInputVaryings();
        ASSERT_EQ(1u, inputVaryings.size());
        const Varying *varying = &inputVaryings[0];
        EXPECT_EQ("texcoord", varying->name);
        EXPECT_EQ(kInterpolationType[i], varying->interpolation);
    }
}

// Test collecting outputs using interpolation qualifiers in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectOutputsWithInterpolationQualifiers)
{
    const std::string &kHeader =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n";

    const std::array<std::string, 4> kInterpolationQualifiers = {
        {"", "flat", "smooth", "centroid"}};

    const std::array<InterpolationType, 4> kInterpolationType = {
        {INTERPOLATION_SMOOTH, INTERPOLATION_FLAT, INTERPOLATION_SMOOTH, INTERPOLATION_CENTROID}};

    const std::string &kFunctionBody =
        "void main()\n"
        "{\n"
        "    texcoord = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    for (size_t i = 0; i < kInterpolationQualifiers.size(); ++i)
    {
        const std::string &qualifier = kInterpolationQualifiers[i];
        std::ostringstream stream;
        stream << kHeader << qualifier << " out vec4 texcoord;\n" << kFunctionBody;

        compile(stream.str());
        const auto &outputVaryings = mTranslator->getOutputVaryings();
        ASSERT_EQ(1u, outputVaryings.size());

        const Varying *varying = &outputVaryings[0];
        EXPECT_EQ("texcoord", varying->name);
        EXPECT_EQ(kInterpolationType[i], varying->interpolation);
        EXPECT_FALSE(varying->isInvariant);
    }
}

// Test collecting outputs using 'invariant' qualifier in a geometry shader.
TEST_F(CollectGeometryVariablesTest, CollectOutputsWithInvariant)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_OES_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "invariant out vec4 texcoord;\n"
        "void main()\n"
        "{\n"
        "    texcoord = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    compile(shaderString);

    const auto &outputVaryings = mTranslator->getOutputVaryings();
    ASSERT_EQ(1u, outputVaryings.size());

    const Varying *varying = &outputVaryings[0];
    EXPECT_EQ("texcoord", varying->name);
    EXPECT_TRUE(varying->isInvariant);
}
