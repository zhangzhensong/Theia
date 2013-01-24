// Copyright (C) 2013  Chris Sweeney <cmsweeney@cs.ucsb.edu>
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
//     * Neither the name of the University of California, Santa Barbara nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL CHRIS SWEENEY BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "vision/pose/five_point_relative_pose.h"

#include <Eigen/Core>
#include <vector>
#include <iostream>
#include <ctime>

#include "vision/models/essential_matrix.h"
#include "gtest/gtest.h"

namespace vision {
namespace pose {
using Eigen::Matrix3d;
using vision::models::EssentialMatrix;

TEST(FivePointRelativePose, Sanity) {
  // Ground truth essential matrix.
  Matrix3d essential_mat;
  essential_mat << 21.36410238362200, -6.45974435705903, -12.57571428668080,
      -7.57644257286238, -20.76739671331773, 28.55199769513539,
      23.52168039400320, -21.32072430358359, 1.00000000000000;

  // corresponding points
  double m1[5][2] = {{-0.06450996083506, 0.13271495297568},
                     {0.12991134106070, 0.25636930852309},
                     {-0.01417169260989, -0.00201819071777},
                     {0.02077599827229, 0.14974189941659},
                     {0.15970376410206, 0.17782283760921}};
  double m2[5][2] = {{0.01991821260977, 0.13855920297370},
                     {0.29792009989498, 0.21684093037950},
                     {0.14221131487001, 0.03902000959363},
                     {0.09012597508721, 0.11407993279726},
                     {0.15373963107017, 0.02622710108650}};

  // clock_t t = clock();
  std::vector<EssentialMatrix> essential_matrices =
      FivePointRelativePose(m1, m2);
  // t = clock() - t;
  // printf("My version took me %d clicks (%f seconds).\n",
  // t,
  // ((float)t)/CLOCKS_PER_SEC);

  std::cout << "Num solutions found = " <<
      essential_matrices.size() << std::endl;
  for (int i = 0; i < essential_matrices.size(); i++) {
    const Matrix3d& essential_matrix = essential_matrices[i].GetMatrix();
    std::cout << "Essential Matrix =\n" << essential_matrix << std::endl;
    // Use Map.
    Eigen::Vector3d u(m1[i][0], m1[i][1], 1.0);
    Eigen::Vector3d u_prime(m2[i][0], m2[i][1], 1.0);
    double epipolar_constraint = (u_prime.transpose())*essential_matrix*u;
    std::cout << "Epipolar constraint = " << epipolar_constraint << std::endl;
  }
}

}  // namespace pose
}  // namespace vision
