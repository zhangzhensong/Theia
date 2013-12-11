// Copyright (C) 2013 The Regents of the University of California (Regents).
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

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/SVD>
#include <glog/logging.h>
#include "gtest/gtest.h"

#include "theia/math/util.h"
#include "theia/test/benchmark.h"
#include "theia/util/random.h"
#include "theia/vision/sfm/pose/eight_point_fundamental_matrix.h"
#include "theia/vision/sfm/pose/util.h"
#include "theia/vision/sfm/projection_matrix.h"
#include "theia/vision/sfm/triangulation/triangulation.h"

namespace theia {
namespace {
using Eigen::AngleAxisd;
using Eigen::Matrix3d;
using Eigen::Quaterniond;
using Eigen::Vector3d;

// Creates a test scenario from ground truth 3D points and ground truth rotation
// and translation. Projection (i.e., image) noise is optional (set to 0 for no
// noise). The fundamental matrix is computed to ensure that the reprojection
// errors are sufficiently small.
void GenerateImagePoints(const std::vector<Vector3d>& points_3d,
                         const double projection_noise_std_dev,
                         const Quaterniond& expected_rotation,
                         const Vector3d& expected_translation,
                         std::vector<Vector3d>* image_1_points,
                         std::vector<Vector3d>* image_2_points) {
  image_1_points->reserve(points_3d.size());
  image_2_points->reserve(points_3d.size());
  for (int i = 0; i < points_3d.size(); i++) {
    image_1_points->push_back(points_3d[i].normalized());
    image_2_points->push_back(
        (expected_rotation * points_3d[i] + expected_translation).normalized());
  }

  if (projection_noise_std_dev) {
    for (int i = 0; i < points_3d.size(); i++) {
      AddNoiseToProjection(projection_noise_std_dev, &((*image_1_points)[i]));
      AddNoiseToProjection(projection_noise_std_dev, &((*image_2_points)[i]));
    }
  }
}

// Check that the reprojection error is small.
void CheckReprojectionError(const std::vector<Vector3d>& image_1_points,
                            const std::vector<Vector3d>& image_2_points,
                            const Matrix3d& fundamental_matrix,
                            const double max_reprojection_error) {
  // Compute the right projection matrix.
  Eigen::JacobiSVD<Matrix3d> f_svd(fundamental_matrix, Eigen::ComputeFullU);
  const Vector3d epipole_right = f_svd.matrixU().rightCols<1>();
  ProjectionMatrix right_projection;
  right_projection.col(3) = epipole_right;
  right_projection.block<3, 3>(0, 0) =
      CrossProductMatrix(epipole_right) * fundamental_matrix;

  // The left projection matrix is the identity.
  const ProjectionMatrix identity_transformation =
      TransformationMatrix::Identity().matrix();

  for (int i = 0; i < image_1_points.size(); i++) {
    // Triangulate the world point.
    const Eigen::Vector4d world_homog_point =
        Triangulate(identity_transformation, right_projection,
                    image_1_points[i], image_2_points[i]);
    const Vector3d left_point =
        world_homog_point.head<3>() / world_homog_point[3];
    const Vector3d right_point = right_projection * world_homog_point;

    // Compute reprojection error.
    const double img_1_error = (image_1_points[i] / image_1_points[i].z() -
                                left_point / left_point.z()).squaredNorm();
    const double img_2_error = (image_2_points[i] / image_2_points[i].z() -
                                right_point / right_point.z()).squaredNorm();

    EXPECT_LT(img_1_error, max_reprojection_error);
    EXPECT_LT(img_2_error, max_reprojection_error);
  }
}

void EightPointNormalizedWithNoiseTest(const std::vector<Vector3d>& points_3d,
                                       const double projection_noise_std_dev,
                                       const Quaterniond& expected_rotation,
                                       const Vector3d& expected_translation,
                                       const double kMaxReprojectionError) {
  InitRandomGenerator();
  std::vector<Vector3d> image_1_points;
  std::vector<Vector3d> image_2_points;
  GenerateImagePoints(points_3d, projection_noise_std_dev, expected_rotation,
                      expected_translation, &image_1_points, &image_2_points);
  // Compute fundamental matrix.
  Matrix3d fundamental_matrix;
  EXPECT_TRUE(NormalizedEightPoint(image_1_points, image_2_points,
                                   &fundamental_matrix));

  CheckReprojectionError(image_1_points, image_2_points, fundamental_matrix,
                         kMaxReprojectionError);
}

void BasicNormalizedTest() {
  const std::vector<Vector3d> points_3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0),
                                            Vector3d(-1.0, -3.0, 2.0),
                                            Vector3d(1.0, -2.0, 1.0),
                                            Vector3d(-1.0, 4.0, 2.0),
                                            Vector3d(-2.0, 2.0, 3.0)
  };

  const Quaterniond soln_rotation(
      AngleAxisd(Radians(13.0), Vector3d(0.0, 0.0, 1.0)));
  const Vector3d soln_translation(1.0, 0.5, 1.5);
  const double kNoise = 0.0 / 512.0;
  const double kMaxReprojectionError = 1e-12;

  EightPointNormalizedWithNoiseTest(
      points_3d, kNoise, soln_rotation, soln_translation,
      kMaxReprojectionError);
}

TEST(NormalizedEightPoint, BasicTest) {
  BasicNormalizedTest();
}

BENCHMARK(NormalizedEightPoint, Benchmark, 100, 1000) {
  BasicNormalizedTest();
}

TEST(NormalizedEightPoint, NoiseTest) {
  const std::vector<Vector3d> points_3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0),
                                            Vector3d(-1.0, -3.0, 2.0),
                                            Vector3d(1.0, -2.0, 1.0),
                                            Vector3d(-1.0, 4.0, 2.0),
                                            Vector3d(-2.0, 2.0, 3.0)
  };

  const Quaterniond soln_rotation(
      AngleAxisd(Radians(13.0), Vector3d(0.0, 0.0, 1.0)));
  const Vector3d soln_translation(1.0, 0.5, 0.0);
  const double kNoise = 1.0 / 512.0;
  const double kMaxReprojectionError = 1e-4;

  EightPointNormalizedWithNoiseTest(points_3d, kNoise, soln_rotation,
                                    soln_translation, kMaxReprojectionError);
}

void EightPointGoldStandardWithNoiseTest(const std::vector<Vector3d>& points_3d,
                                       const double projection_noise_std_dev,
                                       const Quaterniond& expected_rotation,
                                       const Vector3d& expected_translation,
                                       const double kMaxReprojectionError) {
  InitRandomGenerator();
  std::vector<Vector3d> image_1_points;
  std::vector<Vector3d> image_2_points;
  GenerateImagePoints(points_3d, projection_noise_std_dev, expected_rotation,
                      expected_translation, &image_1_points, &image_2_points);
  // Compute fundamental matrix.
  Matrix3d fundamental_matrix;
  EXPECT_TRUE(GoldStandardEightPoint(image_1_points, image_2_points,
                                   &fundamental_matrix));

  CheckReprojectionError(image_1_points, image_2_points, fundamental_matrix,
                         kMaxReprojectionError);
}

void BasicGoldStandardTest() {
  const std::vector<Vector3d> points_3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0),
                                            Vector3d(-1.0, -3.0, 2.0),
                                            Vector3d(1.0, -2.0, 1.0),
                                            Vector3d(-1.0, 4.0, 2.0),
                                            Vector3d(-2.0, 2.0, 3.0)
  };

  const Quaterniond soln_rotation(
      AngleAxisd(Radians(13.0), Vector3d(0.0, 0.0, 1.0)));
  const Vector3d soln_translation(1.0, 0.5, 1.5);
  const double kNoise = 0.0 / 512.0;
  const double kMaxReprojectionError = 1e-12;

  EightPointGoldStandardWithNoiseTest(
      points_3d, kNoise, soln_rotation, soln_translation,
      kMaxReprojectionError);
}

TEST(GoldStandardEightPoint, BasicTest) {
  BasicGoldStandardTest();
}

BENCHMARK(GoldStandardEightPoint, Benchmark, 100, 1000) {
  BasicGoldStandardTest();
}

TEST(GoldStandardEightPoint, NoiseTest) {
  const std::vector<Vector3d> points_3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0),
                                            Vector3d(-1.0, -3.0, 2.0),
                                            Vector3d(1.0, -2.0, 1.0),
                                            Vector3d(-1.0, 4.0, 2.0),
                                            Vector3d(-2.0, 2.0, 3.0)
  };

  const Quaterniond soln_rotation(
      AngleAxisd(Radians(13.0), Vector3d(0.0, 0.0, 1.0)));
  const Vector3d soln_translation(1.0, 0.5, 0.0);
  const double kNoise = 1.0 / 512.0;
  const double kMaxReprojectionError = 1e-5;

  EightPointGoldStandardWithNoiseTest(points_3d, kNoise, soln_rotation,
                                    soln_translation, kMaxReprojectionError);
}

}  // namespace
}  // namespace theia