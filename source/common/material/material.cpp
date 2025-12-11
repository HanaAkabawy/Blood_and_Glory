#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

namespace our {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        //TODO: (Req 7) Write this function
        pipelineState.setup();
        shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        if(data.contains("pipelineState")){
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint 
    void TintedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        Material::setup();
        shader->set("tint", tint);
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json& data){
        Material::deserialize(data);
        if(!data.is_object()) return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex" 
    void TexturedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        TintedMaterial::setup();
        shader->set("alphaThreshold", alphaThreshold);
        
        // Bind texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        texture->bind();
        // Bind sampler to texture unit 0
        if(sampler)
        sampler->bind(0);
        // Send texture unit index to shader
        shader->set("tex", 0);
    }
    void LitMaterial::setup() const{
        TexturedMaterial::setup();
        shader->set("shininess", shininess);
        if(specularMap){
            glActiveTexture(GL_TEXTURE1);
            specularMap->bind();
            shader->set("specular_tex", 1);
            shader->set("has_specular_map", true);
        }else
        {
            shader->set("has_specular_map", false);
        }
     if(emissiveMap) {
        glActiveTexture(GL_TEXTURE2);
        emissiveMap->bind();
        shader->set("emissive_tex", 2);
        shader->set("has_emissive_map", true);
    } else {
        shader->set("has_emissive_map", false);
    }
    }
    void LitMaterial::deserialize(const nlohmann::json& data){
        TexturedMaterial::deserialize(data);
        if(!data.is_object()){
            return;
        }
        shininess = data.value("shininess", 32.0f);
        if(data.contains("specularMap")){
            specularMap = AssetLoader<Texture2D>::get(data["specularMap"].get<std::string>());
        }
        if(data.contains("emissiveMap")){
    emissiveMap = AssetLoader<Texture2D>::get(data["emissiveMap"].get<std::string>());
}
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

}