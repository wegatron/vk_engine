#include <framework/scene/scene.h>
#include <queue>

// entt reference: https://skypjack.github.io/entt/md_docs_md_entity.html
// https://github.com/skypjack/entt/wiki/Crash-Course:-core-functionalities#introduction
namespace vk_engine {

entt::entity
Scene::createRenderableEntity(const std::string &name,
                              const std::shared_ptr<TransformRelationship> &tr,
                              const std::shared_ptr<Material> &material,
                              const std::shared_ptr<StaticMesh> &mesh) {
  entt::entity entity = renderable_manager_.create();
  renderable_manager_.emplace<std::string>(entity, name); // name
  renderable_manager_.emplace<std::shared_ptr<TransformRelationship>>(
      entity, tr); // node transform index
  renderable_manager_.emplace<std::shared_ptr<Material>>(
      entity, material); // material index
  renderable_manager_.emplace<std::shared_ptr<StaticMesh>>(entity,
                                                           mesh); // mesh index
  return entity;
}

entt::entity
Scene::createCameraEntity(const std::string &name,
                          const std::shared_ptr<TransformRelationship> &tr,
                          const Camera &camera) {
  entt::entity entity = camera_manager_.create();
  camera_manager_.emplace<std::string>(entity, name); // name
  camera_manager_.emplace<std::shared_ptr<TransformRelationship>>(entity,
                                                                  tr); // node transform index
  camera_manager_.emplace<Camera>(entity, camera); // camera index
  return entity;
}

void Scene::update(const float seconds) {
  root_tr_->aabb.setEmpty();
  //// update rt
  std::queue<std::shared_ptr<TransformRelationship>> q;
  // add root->node's children
  auto rch = root_tr_->child;
  rch->gtransform = rch->ltransform;  
  root_tr_->aabb.extend(rch->aabb.transformed(Eigen::Affine3f(rch->gtransform)));
  while (rch != nullptr) // add child nodes
  {
    q.emplace(rch);
    rch = rch->sibling;
  }
  while (!q.empty()) {
    auto node = q.front();
    q.pop();
    node->gtransform = node->parent->gtransform * node->ltransform;
    root_tr_->aabb.extend(node->aabb.transformed(Eigen::Affine3f(node->gtransform)));
    // add child nodes
    auto ch = node->child;
    while (ch != nullptr) {
      q.emplace(ch);
      ch = ch->sibling;
    }
  }  
}

} // namespace vk_engine