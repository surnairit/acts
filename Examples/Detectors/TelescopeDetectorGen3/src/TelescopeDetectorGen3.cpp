// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3.hpp"

#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeBuilderGen3.hpp"

#include <algorithm>
#include <stdexcept>

namespace ActsExamples {

TelescopeDetectorGen3::TelescopeDetectorGen3(const Config& cfg)
    : Detector(
          Acts::getDefaultLogger("TelescopeDetectorGen3", cfg.logLevel)),
      m_cfg(cfg) {
  if (m_cfg.positions.empty()) {
    throw std::invalid_argument(
        "TelescopeDetectorGen3 requires at least one position");
  }

  if (m_cfg.stereos.empty()) {
    m_cfg.stereos.assign(m_cfg.positions.size(), 0.);
  } else if (m_cfg.positions.size() != m_cfg.stereos.size()) {
    throw std::invalid_argument(
        "The number of provided positions must match the number of "
        "provided stereo angles.");
  }

  const auto nonIncreasing = std::adjacent_find(
      m_cfg.positions.begin(), m_cfg.positions.end(),
      [](double lhs, double rhs) { return lhs >= rhs; });
  if (nonIncreasing != m_cfg.positions.end()) {
    throw std::invalid_argument(
        "TelescopeDetectorGen3 positions must be strictly increasing");
  }

  if (m_cfg.axis < 0 || m_cfg.axis > 2) {
    throw std::invalid_argument("The axis value must be 0, 1, or 2.");
  }
  
  switch (m_cfg.surfaceType) {
    case SurfaceType::Disc:
      if (m_cfg.discBounds[0] < 0. ||
          m_cfg.discBounds[0] >= m_cfg.discBounds[1]) {
        throw std::invalid_argument(
            "TelescopeDetectorGen3 discBounds must satisfy "
            "0 <= minR < maxR");
      }
      break;
    case SurfaceType::Plane:
      if (m_cfg.planeBounds[0] <= 0. || m_cfg.planeBounds[1] <= 0.) {
        throw std::invalid_argument(
            "TelescopeDetectorGen3 planeBounds half-lengths must be positive");
      }
      break;
    default:
      throw std::invalid_argument(
          "TelescopeDetectorGen3 received an unknown surface type");
  }

  if (m_cfg.thickness <= 0.) {
    throw std::invalid_argument(
        "TelescopeDetectorGen3 thickness must be positive");
  }
  if (m_cfg.layerEnvelope <= 0.) {
    throw std::invalid_argument(
        "TelescopeDetectorGen3 layerEnvelope must be positive");
  }

  m_nominalGeometryContext =
      Acts::GeometryContext::dangerouslyDefaultConstruct();
  m_trackingGeometry = buildTelescopeDetectorGen3(
      m_nominalGeometryContext, m_detectorStore, m_cfg, logger());

  if (m_trackingGeometry == nullptr) {
    throw std::runtime_error(
        "TelescopeDetectorGen3 failed to build a tracking geometry");
  }
}

}  // namespace ActsExamples
