// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfacePlacementBase.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3.hpp"

#include <array>
#include <memory>

namespace Acts {
class SurfaceBounds;
}

namespace ActsExamples {

/// One disc or rectangular-plane sensitive element.
class TelescopeDetectorGen3Element final : public Acts::SurfacePlacementBase {
 public:
  using Identifier = unsigned long long;

  TelescopeDetectorGen3Element(
      Identifier identifier, std::shared_ptr<Acts::Transform3> transform,
      TelescopeDetectorGen3::SurfaceType surfaceType,
      const std::array<double, 2>& bounds, double thickness);

  ~TelescopeDetectorGen3Element() override = default;

  const Acts::Surface& surface() const final;
  Acts::Surface& surface() final;

  const Acts::Transform3& localToGlobalTransform(
      const Acts::GeometryContext& gctx) const final;

  bool isSensitive() const final { return true; }

  Identifier identifier() const { return m_identifier; }
  double thickness() const { return m_thickness; }

 private:
  Identifier m_identifier;
  std::shared_ptr<Acts::Transform3> m_transform;
  std::shared_ptr<Acts::Surface> m_surface;
  std::shared_ptr<const Acts::SurfaceBounds> m_bounds;
  double m_thickness;
};

inline const Acts::Surface& TelescopeDetectorGen3Element::surface() const {
  return *m_surface;
}

inline Acts::Surface& TelescopeDetectorGen3Element::surface() {
  return *m_surface;
}

inline const Acts::Transform3&
TelescopeDetectorGen3Element::localToGlobalTransform(
    const Acts::GeometryContext& /*gctx*/) const {
  return *m_transform;
}

}  // namespace ActsExamples
