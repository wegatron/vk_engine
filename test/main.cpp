#include <iostream>

// #include <assimp/Importer.hpp>
// #include <assimp/postprocess.h>
// #include <assimp/scene.h>
// #include <entt/entt.hpp>
// #include <framework/scene/component/material.h>
// #include <framework/vk/shader_module.h>
#include <Eigen/Dense>

int main(int argc, char const *argv[])
{
    Eigen::Matrix4f ids[2] = {Eigen::Matrix4f::Identity(), Eigen::Matrix4f::Identity() };

    std::cout << sizeof(ids) << std::endl;
    /* code */
    // Assimp::Importer importer;
    // const aiScene * scene = importer.ReadFile("data/buster_drone/scene.gltf", aiProcessPreset_TargetRealtime_Quality);
    
    // if (!scene)
    // {
    //     std::cerr << "Error: " << importer.GetErrorString() << std::endl;
    //     return 1;
    // }

    // entt::registry registry;
   
    //vk_engine::PbrMaterial material;

    std::cout << "done" << std::endl;
    return 0;
}