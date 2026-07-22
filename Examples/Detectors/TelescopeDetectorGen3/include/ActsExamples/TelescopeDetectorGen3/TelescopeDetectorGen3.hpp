// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "ActsExamples/DetectorCommons/Detector.hpp"

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

namespace ActsExamples {

/// Minimal Gen3 telescope detector for Blueprint/Python tests.
///
/// The detector can construct either disc sensors in cylindrical layer volumes
/// or rectangular plane sensors in cuboid layer volumes. In both modes the
/// sensors are stacked along the global z-axis.
class TelescopeDetectorGen3 final : public Detector {
 public:
  enum class SurfaceType { Disc, Plane };
  
  struct Config {
    /// Sensor and layer-volume shape.
    SurfaceType surfaceType{SurfaceType::Disc};

    /// Global z positions of the telescope sensors.
    std::vector<double> positions{0., 30., 60.};

    /// Disc bounds: {minimum radius, maximum radius}.
    std::array<double, 2> discBounds{5., 25.};

    /// Rectangle bounds: {half-length x, half-length y}.
    std::array<double, 2> planeBounds{25., 15.};

    /// Sensitive-surface thickness.
    double thickness{80. * Acts::UnitConstants::um};

    /// Extra z-space around every layer volume.
    double layerEnvelope{1. * Acts::UnitConstants::mm};

    /// Optional Blueprint graph in Graphviz DOT format.
    std::optional<std::filesystem::path> graphvizFile;

    Acts::Logging::Level logLevel{Acts::Logging::WARNING};
  };

  explicit TelescopeDetectorGen3(const Config& cfg);

 private:
  Config m_cfg;
};

}  // namespace ActsExamples
