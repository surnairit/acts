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
/// The detector consists of disc-shaped sensitive surfaces stacked along the
/// global z-axis. Plane layers to be added.
class TelescopeDetectorGen3 final : public Detector {
 public:
  struct Config {
    /// Global z positions of the telescope discs.
    std::vector<double> positions{0., 30., 60.};

    /// Disc bounds: {minimum radius, maximum radius}.
    std::array<double, 2> bounds{5., 25.};

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
