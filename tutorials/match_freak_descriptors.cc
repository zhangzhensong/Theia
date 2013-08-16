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

#include <glog/logging.h>
#include <gflags/gflags.h>
#include <time.h>
#include <string>
#include <vector>

#include "image/image.h"
#include "image/image_canvas.h"
#include "image/descriptor/freak_descriptor.h"
#include "image/keypoint_detector/keypoint.h"
#include "image/keypoint_detector/brisk_detector.h"
#include "vision/matching/distance.h"
#include "vision/matching/brute_force_matcher.h"
#include "vision/matching/image_matcher.h"

DEFINE_string(img_input_dir, "input", "Directory of two input images.");
DEFINE_string(img_output_dir, "output", "Name of output image file.");

using theia::Descriptor;
using theia::FreakDescriptorExtractor;
using theia::BriskDetector;
using theia::BruteForceImageMatcher;
using theia::BruteForceMatcher;
using theia::GrayImage;
using theia::Hamming;
using theia::ImageCanvas;
using theia::Keypoint;

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  GrayImage left_image(FLAGS_img_input_dir + std::string("/img1.png"));
  GrayImage right_image(FLAGS_img_input_dir + std::string("/img2.png"));

  // Detect keypoints.
  VLOG(0) << "detecting keypoints";
  BriskDetector brisk_detector(70, 4);
  std::vector<Keypoint*> left_keypoints;
  brisk_detector.DetectKeypoints(left_image, &left_keypoints);
  VLOG(0) << "detected " << left_keypoints.size()
          << " keypoints in left image.";

  VLOG(0) << "detecting keypoints";
  std::vector<Keypoint*> right_keypoints;
  brisk_detector.DetectKeypoints(right_image, &right_keypoints);
  VLOG(0) << "detected " << left_keypoints.size()
          << " keypoints in left image.";

  // Extract descriptors.
  VLOG(0) << "extracting descriptors.";
  FreakDescriptorExtractor freak_extractor;
  freak_extractor.Initialize();

  std::vector<Descriptor*> left_pruned_descriptors;
  freak_extractor.ComputeDescriptorsPruned(left_image,
                                           left_keypoints,
                                           &left_pruned_descriptors);
  VLOG(0) << "pruned descriptors size = " << left_pruned_descriptors.size();

  std::vector<Descriptor*> right_pruned_descriptors;
  freak_extractor.ComputeDescriptorsPruned(right_image,
                                           right_keypoints,
                                           &right_pruned_descriptors);
  VLOG(0) << "pruned descriptors size = " << right_pruned_descriptors.size();

  // Match descriptors!
  BruteForceImageMatcher<Hamming> brute_force_image_matcher;
  std::vector<theia::FeatureMatch<int> > matches;
  clock_t t = clock();
  brute_force_image_matcher.MatchSymmetricAndDistanceRatio(
      left_pruned_descriptors,
      right_pruned_descriptors,
      &matches,
      0.8,
      128);
  t = clock() - t;
  VLOG(0) << "It took " << (static_cast<float>(t)/CLOCKS_PER_SEC)
          << " to match FREAK descriptors";

  // Get an image canvas to draw the features on.
  ImageCanvas image_canvas;
  image_canvas.AddImage(left_image);
  image_canvas.AddImage(right_image);
  image_canvas.DrawMatchedFeatures(0, left_pruned_descriptors,
                                   1, right_pruned_descriptors,
                                   matches,
                                   0.1);

  image_canvas.Write(FLAGS_img_output_dir +
                     std::string("/freak_descriptors.png"));
}
