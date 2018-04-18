#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxSyntax.h>

namespace MaterialX
{

namespace
{
    // Semantics used by OgsFx
    static const std::unordered_map<string,string> OGSFX_DEFAULT_SEMANTICS_MAP =
    {
        { "i_position", "POSITION"},
        { "i_normal", "NORMAL" },
        { "i_tangent", "TANGENT" },
        { "i_bitangent", "BITANGENT" },

        { "i_texcoord_0", "TEXCOORD0" },
        { "i_texcoord_1", "TEXCOORD1" },
        { "i_texcoord_2", "TEXCOORD2" },
        { "i_texcoord_3", "TEXCOORD3" },
        { "i_texcoord_4", "TEXCOORD4" },
        { "i_texcoord_5", "TEXCOORD5" },
        { "i_texcoord_6", "TEXCOORD6" },
        { "i_texcoord_7", "TEXCOORD7" },

        { "i_color_0", "COLOR0" },

        { "u_worldMatrix", "World" },
        { "u_worldInverseMatrix", "WorldInverse" },
        { "u_worldTransposeMatrix", "WorldTranspose" },
        { "u_worldInverseTransposeMatrix", "WorldInverseTranspose" },

        { "u_viewMatrix", "View" },
        { "u_viewInverseMatrix", "ViewInverse" },
        { "u_viewTransposeMatrix", "ViewTranspose" },
        { "u_viewInverseTransposeMatrix", "ViewInverseTranspose" },

        { "u_projectionMatrix", "Projection" },
        { "u_projectionInverseMatrix", "ProjectionInverse" },
        { "u_projectionTransposeMatrix", "ProjectionTranspose" },
        { "u_projectionInverseTransposeMatrix", "ProjectionInverseTranspose" },

        { "u_worldViewMatrix", "WorldView" },
        { "u_viewProjectionMatrix", "ViewProjection" },
        { "u_worldViewProjectionMatrix", "WorldViewProjection" },

        { "u_viewDirection", "ViewDirection" },
        { "u_viewPosition", "ViewPosition" },
        { "u_frame", "Frame" },
        { "u_time", "Time" }
    };
}

OgsFxShader::OgsFxShader(const string& name) 
    : ParentClass(name)
{
    _stages.push_back(Stage("FinalFx"));

    // Create default uniform blocks for final fx stage
    createUniformBlock(FINAL_FX_STAGE, PRIVATE_UNIFORMS, "prv");
    createUniformBlock(FINAL_FX_STAGE, PUBLIC_UNIFORMS, "pub");
}

void OgsFxShader::createUniform(size_t stage, const string& block, const string& type, const string& name, const string& semantic, ValuePtr value)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createUniform(stage, block, type, name, it->second, value);
            return;
        }
    }
    ParentClass::createUniform(stage, block, type, name, semantic, value);
}

void OgsFxShader::createAppData(const string& type, const string& name, const string& semantic)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createAppData(type, name, it->second);
            return;
        }
    }
    ParentClass::createAppData(type, name, semantic);
}

void OgsFxShader::createVertexData(const string& type, const string& name, const string& semantic)
{
    // If no semantic is given check if we have 
    // an OgsFx semantic that should be used
    if (semantic.empty())
    {
        auto it = OGSFX_DEFAULT_SEMANTICS_MAP.find(name);
        if (it != OGSFX_DEFAULT_SEMANTICS_MAP.end())
        {
            HwShader::createVertexData(type, name, it->second);
            return;
        }
    }
    ParentClass::createVertexData(type, name, semantic);
}


const string OgsFxShaderGenerator::TARGET = "ogsfx";

OgsFxShaderGenerator::OgsFxShaderGenerator()
    : ParentClass()
{
    _syntax = OgsFxSyntax::creator();
}

ShaderPtr OgsFxShaderGenerator::generate(const string& shaderName, ElementPtr element)
{
    OgsFxShaderPtr shaderPtr = std::make_shared<OgsFxShader>(shaderName);
    shaderPtr->initialize(element, *this);

    OgsFxShader& shader = *shaderPtr;

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(OgsFxShader::VERTEX_STAGE);

    // Create required variables
    shader.createAppData(DataType::VECTOR3, "i_position");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_worldMatrix");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, DataType::MATRIX4, "u_viewProjectionMatrix");

    shader.addComment("---------------------------------- Vertex shader ----------------------------------------\n");
    shader.addLine("GLSLShader VS", false);
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(shader);
    shader.endScope();
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(OgsFxShader::PIXEL_STAGE);
    
    shader.addComment("---------------------------------- Pixel shader ----------------------------------------\n");
    shader.addStr("GLSLShader PS\n");
    shader.beginScope(Shader::Brackets::BRACES);

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitFunctionCalls(shader);
    emitFinalOutput(shader);
    shader.endScope();
    shader.endScope();
    shader.newLine();

    //
    // Assemble the final effects shader
    //

    shader.setActiveStage(size_t(OgsFxShader::FINAL_FX_STAGE));

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("sxpbrlib/sx-glsl/defines.glsl", *this);
    shader.addLine("#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    shader.newLine();
    emitTypeDefs(shader);

    shader.addComment("Data from application to vertex shader");
    shader.addLine("attribute AppData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    for (const Shader::Variable* input : appDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(input->type);
        shader.addLine(type + " " + input->name + (!input->semantic.empty() ? " : " + input->semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    shader.addComment("Data passed from vertex shader to pixel shader");
    shader.addLine("attribute VertexData", false);
    shader.beginScope(Shader::Brackets::BRACES);
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    for (const Shader::Variable* output : vertexDataBlock.variableOrder)
    {
        const string& type = _syntax->getTypeName(output->type);
        shader.addLine(type + " " + output->name + (!output->semantic.empty() ? " : " + output->semantic : ""));
    }
    shader.endScope(true);
    shader.newLine();

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    shader.addLine("attribute PixelOutput", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 " + outputSocket->name);
    shader.endScope(true);
    shader.newLine();

    // Add all private vertex shader uniforms
    const Shader::VariableBlock& vsPrivateUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (vsPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Vertex stage uniform block: " + vsPrivateUniforms.name);
        for (const Shader::Variable* uniform : vsPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public vertex shader uniforms
    const Shader::VariableBlock& vsPublicUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (vsPublicUniforms.variableOrder.size())
    {
        shader.addComment("Vertex stage uniform block: " + vsPublicUniforms.name);
        for (const Shader::Variable* uniform : vsPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all private pixel shader uniforms
    const Shader::VariableBlock& psPrivateUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (psPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Pixel stage uniform block: " + psPrivateUniforms.name);
        for (const Shader::Variable* uniform : psPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public pixel shader uniforms
    const Shader::VariableBlock& psPublicUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (psPublicUniforms.variableOrder.size())
    {
        shader.addComment("Pixel stage uniform block: " + psPublicUniforms.name);
        for (const Shader::Variable* uniform : psPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add light data block if needed
    if (shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
    {
        const Shader::VariableBlock& lightData = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::LIGHT_DATA_BLOCK);
        shader.addLine("struct " + lightData.name, false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* uniform : lightData.variableOrder)
        {
            const string& type = _syntax->getTypeName(uniform->type);
            shader.addLine(type + " " + uniform->name);
        }
        shader.endScope(true);
        shader.newLine();
        shader.addLine("uniform " + lightData.name + " " + lightData.instance);
        shader.newLine();
    }

    // Emit common math functions
    shader.addLine("GLSLShader MathFunctions", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addInclude("sxpbrlib/sx-glsl/math.glsl", *this);
    shader.endScope();
    shader.newLine();

    // Add code blocks from the vertex and pixel shader stages generated above
    shader.addBlock(shader.getSourceCode(OgsFxShader::VERTEX_STAGE));
    shader.addBlock(shader.getSourceCode(OgsFxShader::PIXEL_STAGE));

    // Add Main technique block
    shader.addLine("technique Main", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("pass p0", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("VertexShader(in AppData, out VertexData vd) = { MathFunctions, VS }");
    shader.addLine("PixelShader(in VertexData vd, out PixelOutput) = { MathFunctions, PS }");
    shader.endScope();
    shader.endScope();
    shader.newLine();

    return shaderPtr;
}

void OgsFxShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == DataType::FILENAME)
    {
        std::stringstream str;
        str << "uniform texture2D " << uniform.name << "_texture : SourceTexture;\n";
        str << "uniform sampler2D " << uniform.name << " = sampler_state\n";
        str << "{\n    Texture = <" << uniform.name << "_texture>;\n};\n";
        shader.addBlock(str.str());
    }
    else if (!uniform.semantic.empty())
    {
        const string& type = _syntax->getTypeName(uniform.type);
        shader.addLine("uniform " + type + " " + uniform.name + " : " + uniform.semantic);
    }
    else
    {
        const string& type = _syntax->getTypeName(uniform.type);
        const string initStr = (uniform.value ? 
            _syntax->getValue(*uniform.value, uniform.type, true) : 
            _syntax->getTypeDefault(uniform.type, true));
        shader.addLine("uniform " + type + " " + uniform.name + (initStr.empty() ? "" : " = " + initStr));
    }
}

} // namespace MaterialX
