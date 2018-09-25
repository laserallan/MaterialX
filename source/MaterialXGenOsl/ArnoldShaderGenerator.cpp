#include <MaterialXGenOsl/ArnoldShaderGenerator.h>

namespace MaterialX
{

const string ArnoldShaderGenerator::TARGET = "arnold";


ArnoldShaderGenerator::ArnoldShaderGenerator() 
    : OslShaderGenerator() 
{
    const StringSet restrictedNames = { "translucent", "empirical_bssrdf", "randomwalk_bssrdf", "volume_absorption",
                                        "volume_emission", "volume_henyey_greenstein", "volume_matte" };
    _syntax->registerRestrictedNames(restrictedNames);

    // Remap struct shader outputs due issues with Arnold renderer. 
    // Remove this when this issue has been addressed.
    _remapShaderOutput = true;
}

Shader::VDirection ArnoldShaderGenerator::getTargetVDirection() const
{
    return Shader::VDirection::DOWN;
}

}
