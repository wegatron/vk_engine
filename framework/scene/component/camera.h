#pragma once

#include <Eigen/Dense>

namespace vk_engine {
class Camera {
public:
  Camera() = default;

  ~Camera() = default;

  const std::string& getName() const noexcept { return name_; }

  void setName(const std::string &name) { name_ = name; }

  // default z: front, x: right, y: up
  void setLookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &up,
                 const Eigen::Vector3f &center) {
    view_mat_.block<3, 1>(0, 3) = -eye;
    Eigen::Vector3f ny = up.normalized();
    Eigen::Vector3f nz = (center - eye).normalized();
    Eigen::Vector3f nx = ny.cross(nz).normalized();
    view_mat_.block<3, 1>(0, 0) = nx;
    view_mat_.block<3, 1>(1, 0) = ny;
    view_mat_.block<3, 1>(2, 0) = nz;
    }

    // default: z: front, x: right, y: up
    void setRotationEuraXYZ(const Eigen::Vector3f &eura_xyz)
    {
        Eigen::Matrix3f rotation_mat;
        rotation_mat = Eigen::AngleAxisf(eura_xyz.x(), Eigen::Vector3f::UnitX())
                     * Eigen::AngleAxisf(eura_xyz.y(), Eigen::Vector3f::UnitY())
                     * Eigen::AngleAxisf(eura_xyz.z(), Eigen::Vector3f::UnitZ());
        view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
    }

    void setRotationQuat(const Eigen::Quaternionf &quat)
    {
        Eigen::Matrix3f rotation_mat = quat.toRotationMatrix();
        view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
    }

    /**
     * \param near The near clipping plane distance from the camera > 0
     * \param far The far clipping plane distance from the camera > 0
    */
    void setClipPlanes(float near, float far)
    {
        near_ = -near; // to coordinate system of camera
        far_ = -far; // to coordinate system of camera
        dirty_proj_ = true;
    }

    void setFovy(float fovy) // width / focalLength
    {
        fovy_ = fovy;
        dirty_proj_ = true;
    }

    void setAspect(float aspect) // width / height
    {
        aspect_ = aspect;
        dirty_proj_ = true;
    }

    const Eigen::Matrix4f &getViewMatrix() const noexcept { return view_mat_; }
    
    /**
     * \brief Get the projection matrix of the camera, right-handed coordinate system
     * depth: [0, 1]
     * \return The projection matrix
    */
    const Eigen::Matrix4f &getProjectionMatrix() { 
        if (!dirty_proj_) return proj_mat_;
        float f = 1.0 / tan(fovy_ * 0.5);
        float r = 1.0 / (far_ - near_);
        proj_mat_ << 
            f/aspect_, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, far_ * r, -far_ * near_ * r,
            0.0f, 0.0f, -1.0f, 0.0f;
        dirty_proj_ = false;
    }

private:
  std::string name_;
  bool dirty_proj_{true};
  float near_{0.1f};
  float far_{1000.0f};
  float fovy_{60.0f};
  float aspect_{1.0f};

  Eigen::Matrix4f proj_mat_;
  Eigen::Matrix4f view_mat_{Eigen::Matrix4f::Identity()};
};
} // namespace vk_engine