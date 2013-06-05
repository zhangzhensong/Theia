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

#include "image/keypoint_detector/agast_detector.h"

#include <agast/agast5_8.h>
#include <agast/agast7_12d.h>
#include <agast/agast7_12s.h>
#include <agast/cvWrapper.h>
#include <agast/oast9_16.h>
#include <memory>
#include <vector>

#include "image/image.h"
#include "image/keypoint_detector/keypoint.h"

namespace theia {
AgastDetector::AgastDetector(AstPattern pattern,
                             int threshold,
                             bool nonmax_suppression)
    : nonmax_suppression_(nonmax_suppression) {
  switch (pattern) {
    case AGAST5_8:
      ast_detector_.reset(new agast::AgastDetector5_8());
      break;
    case AGAST7_12D:
      ast_detector_.reset(new agast::AgastDetector7_12d());
      break;
    case AGAST7_12S:
      ast_detector_.reset(new agast::AgastDetector7_12s());
      break;
    case OAST9_16:
      ast_detector_.reset(new agast::OastDetector9_16());
      break;
    default:
      break;
  }
  ast_detector_->set_threshold(threshold);
}

bool AgastDetector::DetectKeypoints(const GrayImage& image,
                                    std::vector<Keypoint*>* keypoints) {
  ast_detector_->set_imageSize(image.Cols(), image.Rows());
  // Convert to uchar for algorithm.
  Image<unsigned char> uchar_image = image.ConvertTo<unsigned char>();
  // Detect keypoints.
  ast_detector_->detect(uchar_image.GetData());

  // Perform nonmax suppresion if necessary.
  std::vector<struct agast::CvPoint> ast_keypoints;
  // TODO(cmsweeney): investigate how well the branch predictor is at making
  // this an efficient if statement for repeated calls of DetectKeypoints. If it
  // is wasting too much time, we could use a function pointer here and
  // initialize it in the constructor based on the nonmax_suppresion parameter.
  if (nonmax_suppression_)
    ast_keypoints = ast_detector_->nms(uchar_image.GetData());

  keypoints->reserve(ast_keypoints.size());
  for (struct agast::CvPoint ast_point : ast_keypoints) {
    // TODO(cmsweeney): add score by cornerScore method. This will require
    // moving cornerScore to public. The function takes a pointer so it will be
    // something like ast_detector->cornerScore(image.GetData() + ast_point.x +
    // ast_point.y*image.Cols())
    keypoints->push_back(new Keypoint(ast_point.x,
                                      ast_point.y,
                                      Keypoint::AGAST));
  }
}
void AgastDetector::SetThreshold(int threshold) {
  ast_detector_->set_threshold(threshold);
}
}  // namespace theia