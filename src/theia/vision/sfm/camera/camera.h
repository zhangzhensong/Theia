// Copyright (C) 2014 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)
//         Torsten Sattler (sattlert@inf.ethz.ch)

#ifndef THEIA_VISION_SFM_CAMERA_CAMERA_H_
#define THEIA_VISION_SFM_CAMERA_CAMERA_H_

#include <Eigen/Core>
#include <vector>

#include "theia/vision/sfm/camera/camera_pose.h"
#include "theia/alignment/alignment.h"

namespace theia {

// Base class for all cameras. This class (or derived classes) should include
// things like the camera pose, features, descriptors, sensor measurements,
// etc.
struct Camera {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Camera() {}
  explicit Camera(const CameraPose& pose) : pose_(pose) {}

  // The camera pose describing the position, orientation, and intrinsics of the
  // camera.
  CameraPose pose_;

  // Width of the camera in pixels.
  int width_;
  // Height of the camera in pixels.
  int height_;

  // Original measured feature positions.
  std::vector<Eigen::Vector2d> feature_position_2D_distorted_;
  // Feature positions after correcting for radial distortion.
  std::vector<Eigen::Vector2d> feature_position_2D_;

  // Descriptors.
  std::vector<Eigen::VectorXf> descriptors_;
  std::vector<Eigen::BinaryVectorX> binary_descriptors_;

  // 3D feature IDs. This assumes that a container of 3D feature positions is
  // owned elsewhere. This is essentially the track ID, and the main usage is
  // for reconstructions where many 2D points correspond to a single 3D point.
  std::vector<size_t> feature_3D_ids_;

  // If the 3D positions are stored locally, then we can keep them in a
  // container here.
  std::vector<Eigen::Vector3d> feature_positions_3D_;
};

}  // namespace theia

#endif  // THEIA_VISION_SFM_CAMERA_CAMERA_H_
