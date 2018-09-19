#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>
#include <MaterialXGenGlsl/Nodes/PositionGlsl.h>
#include <MaterialXGenGlsl/Nodes/NormalGlsl.h>
#include <MaterialXGenGlsl/Nodes/TangentGlsl.h>
#include <MaterialXGenGlsl/Nodes/BitangentGlsl.h>
#include <MaterialXGenGlsl/Nodes/TexCoordGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomColorGlsl.h>
#include <MaterialXGenGlsl/Nodes/GeomAttrValueGlsl.h>
#include <MaterialXGenGlsl/Nodes/FrameGlsl.h>
#include <MaterialXGenGlsl/Nodes/TimeGlsl.h>
#include <MaterialXGenGlsl/Nodes/ViewDirectionGlsl.h>
#include <MaterialXGenGlsl/Nodes/SurfaceGlsl.h>
#include <MaterialXGenGlsl/Nodes/SurfaceShaderGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightCompoundGlsl.h>
#include <MaterialXGenGlsl/Nodes/LightShaderGlsl.h>

#include <MaterialXGenShader/Nodes/SourceCode.h>
#include <MaterialXGenShader/Nodes/Swizzle.h>
#include <MaterialXGenShader/Nodes/Switch.h>
#include <MaterialXGenShader/Nodes/Compare.h>

namespace MaterialX
{

namespace
{
    const char* VDIRECTION_FLIP =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n\n";

    const char* VDIRECTION_NOOP =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n\n";
}

const string GlslShaderGenerator::LANGUAGE = "sx-glsl";
const string GlslShaderGenerator::TARGET = "glsl_v4.0";
const string GlslShaderGenerator::VERSION = "400";
const string GlslShaderGenerator::LIGHT_DIR = "L";
const string GlslShaderGenerator::VIEW_DIR = "V";
const string GlslShaderGenerator::INCIDENT = "incident";
const string GlslShaderGenerator::OUTGOING = "outgoing";
const string GlslShaderGenerator::NORMAL = "normal";
const string GlslShaderGenerator::EVAL = "eval";

GlslShaderGenerator::GlslShaderGenerator()
    : ParentClass(GlslSyntax::create())
{
    //
    // Create the node contexts used by this generator
    //

    // BSDF context
    SgNodeContextPtr ctxBsdf = createNodeContext(NODE_CONTEXT_BSDF);
    ctxBsdf->addArgument(Argument("vec3", INCIDENT));
    ctxBsdf->addArgument(Argument("vec3", OUTGOING));

    // BSDF-IBL context
    SgNodeContextPtr ctxBsdfIbl = createNodeContext(NODE_CONTEXT_BSDF_IBL);
    ctxBsdfIbl->addArgument(Argument("vec3", OUTGOING));
    ctxBsdfIbl->setFunctionSuffix("_ibl");

    // EDF context
    SgNodeContextPtr ctxEdf = createNodeContext(NODE_CONTEXT_EDF);
    ctxEdf->addArgument(Argument("vec3", NORMAL));
    ctxEdf->addArgument(Argument("vec3", EVAL));

    //
    // Register all custom node implementation classes
    //

    // <!-- <compare> -->
    registerImplementation("IM_compare_float_sx_glsl", Compare::create);
    registerImplementation("IM_compare_color2_sx_glsl", Compare::create);
    registerImplementation("IM_compare_color3_sx_glsl", Compare::create);
    registerImplementation("IM_compare_color4_sx_glsl", Compare::create);
    registerImplementation("IM_compare_vector2_sx_glsl", Compare::create);
    registerImplementation("IM_compare_vector3_sx_glsl", Compare::create);
    registerImplementation("IM_compare_vector4_sx_glsl", Compare::create);

    // <!-- <switch> -->
    // <!-- 'which' type : float -->
    registerImplementation("IM_switch_float_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color2_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color3_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color4_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector2_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector3_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector4_sx_glsl", Switch::create);
    // <!-- 'which' type : integer -->
    registerImplementation("IM_switch_floatI_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color2I_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color3I_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color4I_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector2I_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector3I_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector4I_sx_glsl", Switch::create);
    // <!-- 'which' type : boolean -->
    registerImplementation("IM_switch_floatB_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color2B_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color3B_sx_glsl", Switch::create);
    registerImplementation("IM_switch_color4B_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector2B_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector3B_sx_glsl", Switch::create);
    registerImplementation("IM_switch_vector4B_sx_glsl", Switch::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_float_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_float_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_float_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_float_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_float_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : color2 -->
    registerImplementation("IM_swizzle_color2_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color2_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color3_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_color4_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector2_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector3_vector4_sx_glsl", Swizzle::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_color2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_color3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_color4_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_vector2_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_vector3_sx_glsl", Swizzle::create);
    registerImplementation("IM_swizzle_vector4_vector4_sx_glsl", Swizzle::create);

    // <!-- <position> -->
    registerImplementation("IM_position_vector3_sx_glsl", PositionGlsl::create);
    // <!-- <normal> -->
    registerImplementation("IM_normal_vector3_sx_glsl", NormalGlsl::create);
    // <!-- <tangent> -->
    registerImplementation("IM_tangent_vector3_sx_glsl", TangentGlsl::create);
    // <!-- <bitangent> -->
    registerImplementation("IM_bitangent_vector3_sx_glsl", BitangentGlsl::create);
    // <!-- <texcoord> -->
    registerImplementation("IM_texcoord_vector2_sx_glsl", TexCoordGlsl::create);
    registerImplementation("IM_texcoord_vector3_sx_glsl", TexCoordGlsl::create);
    // <!-- <geomcolor> -->
    registerImplementation("IM_geomcolor_float_sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor_color2_sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor_color3_sx_glsl", GeomColorGlsl::create);
    registerImplementation("IM_geomcolor_color4_sx_glsl", GeomColorGlsl::create);
    // <!-- <geomattrvalue> -->
    registerImplementation("IM_geomattrvalue_integer_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_boolean_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_string_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_float_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_color2_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_color3_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_color4_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_vector2_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_vector3_sx_glsl", GeomAttrValueGlsl::create);
    registerImplementation("IM_geomattrvalue_vector4_sx_glsl", GeomAttrValueGlsl::create);

    // <!-- <frame> -->
    registerImplementation("IM_frame_float_sx_glsl", FrameGlsl::create);
    // <!-- <time> -->
    registerImplementation("IM_time_float_sx_glsl", TimeGlsl::create);
    // <!-- <viewdirection> -->
    registerImplementation("IM_viewdirection_vector3_sx_glsl", ViewDirectionGlsl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_sx_glsl", SurfaceGlsl::create);
    // <!-- <light> -->
    registerImplementation("IM_light_sx_glsl", LightGlsl::create);

    // <!-- <pointlight> -->
    registerImplementation("IM_pointlight_sx_glsl", LightShaderGlsl::create);
    // <!-- <directionallight> -->
    registerImplementation("IM_directionallight_sx_glsl", LightShaderGlsl::create);
    // <!-- <spotlight> -->
    registerImplementation("IM_spotlight_sx_glsl", LightShaderGlsl::create);
}

ShaderPtr GlslShaderGenerator::generate(const string& shaderName, ElementPtr element, const SgOptions& options)
{
    HwShaderPtr shaderPtr = std::make_shared<HwShader>(shaderName);
    shaderPtr->initialize(element, *this, options);

    HwShader& shader = *shaderPtr;

    //
    // Emit code for vertex shader stage
    //

    shader.setActiveStage(HwShader::VERTEX_STAGE);

    // Create required variables for vertex stage
    shader.createAppData(Type::VECTOR3, "i_position");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_worldMatrix");
    shader.createUniform(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS, Type::MATRIX44, "u_viewProjectionMatrix");

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add all private uniforms
    const Shader::VariableBlock& vsPrivateUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (vsPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + vsPrivateUniforms.name);
        for (const Shader::Variable* uniform : vsPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add any public uniforms
    const Shader::VariableBlock& vsPublicUniforms = shader.getUniformBlock(HwShader::VERTEX_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (vsPublicUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + vsPublicUniforms.name);
        for (const Shader::Variable* uniform : vsPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all app data inputs
    const Shader::VariableBlock& appDataBlock = shader.getAppDataBlock();
    if (appDataBlock.variableOrder.size())
    {
        shader.addComment("Application data block: " + appDataBlock.name);
        for (const Shader::Variable* input : appDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(input->type);
            shader.addLine("in " + type + " " + input->name);
        }
        shader.newLine();
    }

    // Add vertex data block
    const Shader::VariableBlock& vertexDataBlock = shader.getVertexDataBlock();
    if (vertexDataBlock.variableOrder.size())
    {
        shader.addLine("out VertexData", false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* output : vertexDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(output->type);
            shader.addLine(type + " " + output->name);
        }
        shader.endScope(false, false);
        shader.addStr(" " + vertexDataBlock.instance + ";\n");
        shader.newLine();
    }

    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    shader.addLine("vec4 hPositionWorld = u_worldMatrix * vec4(i_position, 1.0)");
    shader.addLine("gl_Position = u_viewProjectionMatrix * hPositionWorld");
    emitFunctionCalls(*_defaultNodeContext, shader);
    shader.endScope();
    shader.newLine();

    //
    // Emit code for pixel shader stage
    //

    shader.setActiveStage(HwShader::PIXEL_STAGE);

    // Add version directive
    shader.addLine("#version " + getVersion(), false);
    shader.newLine();

    // Add global constants and type definitions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_defines.glsl", *this);
    shader.addLine("#define MAX_LIGHT_SOURCES " + std::to_string(getMaxActiveLightSources()), false);
    shader.newLine();
    emitTypeDefs(shader);

    // Add all private uniforms
    const Shader::VariableBlock& psPrivateUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PRIVATE_UNIFORMS);
    if (psPrivateUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + psPrivateUniforms.name);
        for (const Shader::Variable* uniform : psPrivateUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    // Add all public uniforms
    const Shader::VariableBlock& psPublicUniforms = shader.getUniformBlock(HwShader::PIXEL_STAGE, HwShader::PUBLIC_UNIFORMS);
    if (psPublicUniforms.variableOrder.size())
    {
        shader.addComment("Uniform block: " + psPublicUniforms.name);
        for (const Shader::Variable* uniform : psPublicUniforms.variableOrder)
        {
            emitUniform(*uniform, shader);
        }
        shader.newLine();
    }

    bool lighting = shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE);

    // Add light data block if needed
    if (lighting)
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
        shader.addLine("uniform " + lightData.name + " " + lightData.instance + "[MAX_LIGHT_SOURCES]");
        shader.newLine();
    }

    // Add vertex data block
    if (vertexDataBlock.variableOrder.size())
    {
        shader.addLine("in VertexData", false);
        shader.beginScope(Shader::Brackets::BRACES);
        for (const Shader::Variable* input : vertexDataBlock.variableOrder)
        {
            const string& type = _syntax->getTypeName(input->type);
            shader.addLine(type + " " + input->name);
        }
        shader.endScope(false, false);
        shader.addStr(" " + vertexDataBlock.instance + ";\n");
        shader.newLine();
    }

    // Add the pixel shader output. This needs to be a vec4 for rendering
    // and upstream connection will be converted to vec4 if needed in emitFinalOutput()
    shader.addComment("Data output by the pixel shader");
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();
    shader.addLine("out vec4 " + outputSocket->name);
    shader.newLine();

    // Emit common math functions
    shader.addInclude("sxpbrlib/sx-glsl/lib/sx_math.glsl", *this);
    shader.newLine();

    if (lighting)
    {
        // Emit lighting functions
        shader.addInclude("sxpbrlib/sx-glsl/lib/sx_lighting.glsl", *this);
        shader.newLine();
    }

    // Add all functions for node implementations
    emitFunctionDefinitions(shader);

    // Add main function
    shader.addLine("void main()", false);
    shader.beginScope(Shader::Brackets::BRACES);
    emitFunctionCalls(*_defaultNodeContext, shader);
    emitFinalOutput(shader);
    shader.endScope();
    shader.newLine();

    return shaderPtr;
}

void GlslShaderGenerator::emitFunctionDefinitions(Shader& shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

        // Emit function for handling texture coords v-flip 
        // as needed by the v-direction set by the user
        shader.addBlock(shader.getRequestedVDirection() != getTargetVDirection() ? VDIRECTION_FLIP : VDIRECTION_NOOP, *this);

        // For surface shaders we need light shaders
        if (shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
        {
            // Emit functions for all bound light shaders
            for (auto lightShader : getBoundLightShaders())
            {
                lightShader.second->emitFunctionDefinition(*SgNode::NONE, *this, shader);
            }

            // Emit active light count function
            shader.addLine("int numActiveLightSources()", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("return min(u_numActiveLightSources, MAX_LIGHT_SOURCES)");
            shader.endScope();
            shader.newLine();

            // Emit light sampler function with all bound light types
            shader.addLine("void sampleLightSource(LightData light, vec3 position, out lightshader result)", false);
            shader.beginScope(Shader::Brackets::BRACES);
            shader.addLine("result.intensity = vec3(0.0)");
            string ifstatement = "if ";
            for (auto lightShader : getBoundLightShaders())
            {
                shader.addLine(ifstatement + "(light.type == " + std::to_string(lightShader.first) + ")", false);
                shader.beginScope(Shader::Brackets::BRACES);
                lightShader.second->emitFunctionCall(*SgNode::NONE, *_defaultNodeContext, *this, shader);
                shader.endScope();
                ifstatement = "else if ";
            }
            shader.endScope();
            shader.newLine();
        }

    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)

    // Call parent to emit all other functions
    ParentClass::emitFunctionDefinitions(shader);
}

void GlslShaderGenerator::emitFunctionCalls(const SgNodeContext& context, Shader &shader)
{
    BEGIN_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)
        // For vertex stage just emit all function calls in order
        // and ignore conditional scope.
        for (SgNode* node : shader.getNodeGraph()->getNodes())
        {
            shader.addFunctionCall(node, context, *this);
        }
    END_SHADER_STAGE(shader, HwShader::VERTEX_STAGE)

    BEGIN_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
        // For pixel stage surface shaders need special handling
        if (shader.hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
        {
            // Handle all texturing nodes. These are inputs to any
            // closure/shader nodes and need to be emitted first.
            emitTextureNodes(shader);

            // Emit function calls for all surface shader nodes
            for (SgNode* node : shader.getNodeGraph()->getNodes())
            {
                if (node->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
                {
                    shader.addFunctionCall(node, context, *this);
                }
            }
        }
        else
        {
            // No surface shader, fallback to parent class
            ParentClass::emitFunctionCalls(context, shader);
        }
    END_SHADER_STAGE(shader, HwShader::PIXEL_STAGE)
}

void GlslShaderGenerator::emitFinalOutput(Shader& shader) const
{
    const SgOutputSocket* outputSocket = shader.getNodeGraph()->getOutputSocket();

    // Early out for the rare case where the whole graph is just a single value
    if (!outputSocket->connection)
    {
        string outputValue = outputSocket->value ? _syntax->getValue(outputSocket->type, *outputSocket->value) : _syntax->getDefaultValue(outputSocket->type);
        if (!outputSocket->type->isFloat4())
        {
            string finalOutput = outputSocket->name + "_tmp";
            shader.addLine(_syntax->getTypeName(outputSocket->type) + " " + finalOutput + " = " + outputValue);
            toVec4(outputSocket->type, finalOutput);
            shader.addLine(outputSocket->name + " = " + finalOutput);
        }
        else
        {
            shader.addLine(outputSocket->name + " = " + outputValue);
        }
        return;
    }

    string finalOutput = outputSocket->connection->name;

    if (shader.hasClassification(SgNode::Classification::SURFACE))
    {
        shader.addComment("TODO: How should we output transparency?");
        shader.addLine("float outAlpha = 1.0 - sx_max_component(" + finalOutput + ".transparency)");
        shader.addLine(outputSocket->name + " = vec4(" + finalOutput + ".color, outAlpha)");
    }
    else
    {
        if (!outputSocket->type->isFloat4())
        {
            toVec4(outputSocket->type, finalOutput);
        }
        shader.addLine(outputSocket->name + " = " + finalOutput);
    }
}

void GlslShaderGenerator::addNodeContextIDs(SgNode* node) const
{
    if (node->hasClassification(SgNode::Classification::BSDF))
    {
        node->addContextID(NODE_CONTEXT_BSDF);
        node->addContextID(NODE_CONTEXT_BSDF_IBL);
    }
    else if (node->hasClassification(SgNode::Classification::BSDF))
    {
        node->addContextID(NODE_CONTEXT_EDF);
    }
    else
    {
        ParentClass::addNodeContextIDs(node);
    }
}

void GlslShaderGenerator::emitTextureNodes(Shader& shader)
{
    // Emit function calls for all texturing nodes
    bool found = false;
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::TEXTURE) && !node->referencedConditionally())
        {
            shader.addFunctionCall(node, *_defaultNodeContext, *this);
            found = true;
        }
    }

    if (found)
    {
        shader.newLine();
    }
}

void GlslShaderGenerator::emitBsdfNodes(const SgNode& shaderNode, const string& incident, const string& outgoing, Shader& shader, string& bsdf)
{
    SgNodeContext context(NODE_CONTEXT_BSDF);

    // Set extra arguments according to the given directions
    context.addArgument(Argument("vec3", incident));
    context.addArgument(Argument("vec3", outgoing));

    SgNode* last = nullptr;

    // Emit function calls for all BSDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::BSDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, context, *this);
            last = node;
        }
    }

    if (last)
    {
        bsdf = last->getOutput()->name;
    }
}

void GlslShaderGenerator::emitBsdfNodesIBL(const SgNode& shaderNode, const string& outgoing, Shader& shader, string& radiance)
{
    SgNodeContext context(NODE_CONTEXT_BSDF_IBL);

    // Set extra arguments according to the given directions
    context.addArgument(Argument("vec3", outgoing));
    context.setFunctionSuffix("_ibl");

    SgNode* last = nullptr;

    // Emit function calls for all BSDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::BSDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, context, *this);
            last = node;
        }
    }

    if (last)
    {
        radiance = last->getOutput()->name;
    }
}

void GlslShaderGenerator::emitEdfNodes(const SgNode& shaderNode, const string& orientDir, const string& evalDir, Shader& shader, string& edf)
{
    SgNodeContext context(NODE_CONTEXT_EDF);

    // Set extra arguments according to the given directions
    context.addArgument(Argument("vec3", orientDir));
    context.addArgument(Argument("vec3", evalDir));

    edf = "vec3(0.0)";

    SgNode* last = nullptr;

    // Emit function calls for all EDF nodes used by this shader
    // The last node will hold the final result
    for (SgNode* node : shader.getNodeGraph()->getNodes())
    {
        if (node->hasClassification(SgNode::Classification::EDF) && shaderNode.isUsedClosure(node))
        {
            shader.addFunctionCall(node, context, *this);
            last = node;
        }
    }

    if (last)
    {
        edf = last->getOutput()->name;
    }
}

void GlslShaderGenerator::toVec4(const TypeDesc* type, string& variable)
{
    if (type->isFloat3())
    {
        variable = "vec4(" + variable + ", 1.0)";
    }
    else if (type->isFloat2())
    {
        variable = "vec4(" + variable + ", 0.0, 1.0)";
    }
    else if (type == Type::FLOAT || type == Type::INTEGER)
    {
        variable = "vec4(" + variable + ", " + variable + ", " + variable + ", 1.0)";
    }
    else
    {
        // Can't understand other types. Just return black.
        variable = "vec4(0.0, 0.0, 0.0, 1.0)";
    }
}

void GlslShaderGenerator::emitUniform(const Shader::Variable& uniform, Shader& shader)
{
    // A file texture input needs special handling on GLSL
    if (uniform.type == Type::FILENAME)
    {
        shader.addLine("uniform sampler2D " + uniform.name);
    }
    else
    {
        const string& type = _syntax->getTypeName(uniform.type);
        string line = "uniform " + type + " " + uniform.name;
        if (uniform.semantic.length())
            line += " : " + uniform.semantic;
        if (uniform.value)
            line += " = " + _syntax->getValue(uniform.type, *uniform.value, true);
        else
            line += " = " + _syntax->getDefaultValue(uniform.type, true);
        shader.addLine(line);
    }
}

SgImplementationPtr GlslShaderGenerator::createCompoundImplementation(NodeGraphPtr impl)
{
    NodeDefPtr nodeDef = impl->getNodeDef();
    if (!nodeDef)
    {
        throw ExceptionShaderGenError("Error creating compound implementation. Given nodegraph '" + impl->getName() + "' has no nodedef set");
    }
    if (TypeDesc::get(nodeDef->getType()) == Type::LIGHTSHADER)
    {
        return LightCompoundGlsl::create();
    }
    return ParentClass::createCompoundImplementation(impl);
}


const string GlslImplementation::SPACE = "space";
const string GlslImplementation::WORLD = "world";
const string GlslImplementation::OBJECT = "object";
const string GlslImplementation::MODEL = "model";
const string GlslImplementation::INDEX = "index";
const string GlslImplementation::ATTRNAME = "attrname";

const string& GlslImplementation::getLanguage() const
{
    return GlslShaderGenerator::LANGUAGE;
}

const string& GlslImplementation::getTarget() const
{
    return GlslShaderGenerator::TARGET;
}

}