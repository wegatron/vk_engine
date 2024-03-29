#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <framework/functional/scene/scene.h>
#include <framework/functional/global/app_context.h>
#include <framework/utils/vk/vk_driver.h>
#include <framework/functional/component/light.h>

namespace vk_engine {

class CommandBuffer;
class StagePool;
class GPUAssetManager;
class Camera;

// todo Static Mesh, Material TransformRelationship memory management
class AssimpLoader final {
public:
  AssimpLoader() = default;

  void loadScene(const std::string &path, Scene &scene,
                 const std::shared_ptr<CommandBuffer> &cmd_buf);

private:
  std::shared_ptr<TransformRelationship>
  processNode(const std::shared_ptr<TransformRelationship> &parent,
              aiNode *node, const aiScene *a_scene, Scene &scene,
              std::vector<std::shared_ptr<StaticMesh>> &meshes,
              std::vector<std::shared_ptr<Material>> &materials);

  std::vector<std::shared_ptr<StaticMesh>>
  processMeshs(const aiScene *a_scene,
               const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<std::shared_ptr<Material>>
  processMaterials(const aiScene *a_scene, const std::string &dir, const std::shared_ptr<CommandBuffer> &cmd_buf);

  std::vector<Camera> processCameras(const aiScene *a_scene);

  Lights processLight(const aiScene *a_scene);
  // void loadAndSet(const std::string &dir, const aiScene *a_scene, aiMaterial *a_mat,
  //                 const std::shared_ptr<CommandBuffer> &cmd_buf,
  //                 aiTextureType ttype, const char *pKey, unsigned int vtype,
  //                 unsigned int idx, const char *shader_texture_name,
  //                 const char *shader_color_name,
  //                 std::shared_ptr<PbrMaterial> &mat);
};
} // namespace vk_engine