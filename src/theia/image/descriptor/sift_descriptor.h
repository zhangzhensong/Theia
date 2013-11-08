
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

#ifndef THEIA_IMAGE_DESCRIPTOR_SIFT_DESCRIPTOR_H_
#define THEIA_IMAGE_DESCRIPTOR_SIFT_DESCRIPTOR_H_

extern "C" {
#include <vl/sift.h>
}
#include <vector>
#include "theia/image/descriptor/descriptor.h"
#include "theia/image/descriptor/descriptor_extractor.h"
#include "theia/util/util.h"

namespace theia {
template<class T> class Image;
typedef Image<float> GrayImage;
class Keypoint;

class SiftDescriptor : public FloatDescriptor {
 public:
  SiftDescriptor() : FloatDescriptor(128, DescriptorType::SIFT) {}
};

class SiftDescriptorExtractor : public DescriptorExtractor {
 public:
  // Set the feature score threshold and indicate whether nonmax suppression
  // should be used to reduce the number of features. A good value for the
  // threshold is usually 20.
  SiftDescriptorExtractor(int num_octaves, int num_levels, int first_octave)
      : sift_filter_(nullptr),
        num_octaves_(num_octaves),
        num_levels_(num_levels),
        first_octave_(first_octave) {}
  SiftDescriptorExtractor() : SiftDescriptorExtractor(-1, 3, 0) {}
  ~SiftDescriptorExtractor();

  // Computes a descriptor at a single keypoint.
  Descriptor* ComputeDescriptor(const GrayImage& image,
                                const Keypoint& keypoint);

  // Compute multiple descriptors for keypoints from a single image.
  bool ComputeDescriptors(const GrayImage& image,
                          const std::vector<Keypoint*>& keypoints,
                          std::vector<Descriptor*>* descriptors);

  bool DetectAndExtractDescriptors(const GrayImage& image,
                                   std::vector<Descriptor*>* descriptors);

 private:
  VlSiftFilt* sift_filter_;
  int num_octaves_;
  int num_levels_;
  int first_octave_;

  DISALLOW_COPY_AND_ASSIGN(SiftDescriptorExtractor);
};

}  // namespace theia

#endif  // THEIA_IMAGE_DESCRIPTOR_SIFT_DESCRIPTOR_H_