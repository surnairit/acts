// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// Must be on top to avoid some conflict between forward declare and typedef
// Needed until https://gitlab.cern.ch/GeoModelDev/GeoModel/-/merge_requests/351
// is deployed
// clang-format off
#include <GeoModelRead/ReadGeoModel.h>
// clang-format on

#include "Acts/Detector/CylindricalContainerBuilder.hpp"
#include "Acts/Geometry/ITrackingGeometryBuilder.hpp"
#include "Acts/Plugins/GeoModel/GeoModelBlueprintCreater.hpp"
#include "Acts/Plugins/GeoModel/GeoModelConverters.hpp"
#include "Acts/Plugins/GeoModel/GeoModelDetectorElement.hpp"
#include "Acts/Plugins/GeoModel/GeoModelDetectorElementITk.hpp"
#include "Acts/Plugins/GeoModel/GeoModelDetectorObjectFactory.hpp"
#include "Acts/Plugins/GeoModel/GeoModelReader.hpp"
#include "Acts/Plugins/GeoModel/GeoModelTree.hpp"
#include "Acts/Plugins/GeoModel/IGeoShapeConverter.hpp"
#include "Acts/Plugins/Python/Utilities.hpp"
#include "Acts/Surfaces/AnnulusBounds.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "ActsExamples/GeoModelDetector/GeoModelDetector.hpp"
#include "ActsExamples/GeoModelDetector/GeoModelMuonMockupBuilder.hpp"
#include "ActsExamples/ITkModuleSplitting/ITkModuleSplitting.hpp"
#include "ActsExamples/MuonSpectrometerMockupDetector/GeoMuonMockupExperiment.hpp"

#include <string>

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;

namespace Acts::Python {

void addGeoModel(Context& ctx) {
  auto m = ctx.get("main");

  auto gm = m.def_submodule("geomodel");

  py::class_<GeoModelTree::FpvConstLink>(gm, "GeoModelTree::FpvConstLink")
      .def(py::init<>())
      .def("get", &GeoModelTree::FpvConstLink::get,
           py::return_value_policy::reference);

  py::class_<Acts::GeoModelTree>(gm, "GeoModelTree").def(py::init<>());

  gm.def("readFromDb", &Acts::GeoModelReader::readFromDb);

  py::class_<Acts::GeoModelDetectorElement,
             std::shared_ptr<Acts::GeoModelDetectorElement>>(
      gm, "GeoModelDetectorElement")
      .def("logVolName", &Acts::GeoModelDetectorElement::logVolName)
      .def("databaseEntryName",
           &Acts::GeoModelDetectorElement::databaseEntryName)
      .def("surface", [](Acts::GeoModelDetectorElement self) {
        return self.surface().getSharedPtr();
      });

  {
    auto f = py::class_<ActsExamples::GeoModelDetector, ActsExamples::Detector,
                        std::shared_ptr<ActsExamples::GeoModelDetector>>(
                 gm, "GeoModelDetector")
                 .def(py::init<const ActsExamples::GeoModelDetector::Config&>())
                 .def("buildTrackingGeometry",
                      &ActsExamples::GeoModelDetector::buildTrackingGeometry);

    auto c = py::class_<ActsExamples::GeoModelDetector::Config>(f, "Config")
                 .def(py::init<>());
    ACTS_PYTHON_STRUCT(c, geoModelTree, logLevel, path);

    // patch the constructor
    patchKwargsConstructor(c);
  }
  {
    // GeomodelMuonMockupBuilder
    py::class_<Acts::ITrackingGeometryBuilder,
               std::shared_ptr<Acts::ITrackingGeometryBuilder>>(
        gm, "ITrackingGeometryBuilder");
    auto gmMuonBuilder =
        py::class_<ActsExamples::GeoModelMuonMockupBuilder,
                   Acts::ITrackingGeometryBuilder,
                   std::shared_ptr<ActsExamples::GeoModelMuonMockupBuilder>>(
            gm, "GeoModelMuonMockupBuilder")
            .def(py::init([](const ActsExamples::GeoModelMuonMockupBuilder::
                                 Config& config,
                             const std::string& name,
                             Acts::Logging::Level level) {
              return std::make_shared<ActsExamples::GeoModelMuonMockupBuilder>(
                  config, getDefaultLogger(name, level));
            }))
            .def("trackingGeometry",
                 &ActsExamples::GeoModelMuonMockupBuilder::trackingGeometry);
    auto gmMuonConfig =
        py::class_<ActsExamples::GeoModelMuonMockupBuilder::Config>(
            gmMuonBuilder, "Config")
            .def(py::init<>());
    ACTS_PYTHON_STRUCT(gmMuonConfig, volumeBoxFPVs, stationNames,
                       volumeBoundFactory);
  }

  // Shape converters
  {
    py::class_<Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::IGeoShapeConverter>>(gm,
                                                          "IGeoShapeConverter");

    py::class_<Acts::GeoBoxConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoBoxConverter>>(gm, "GeoBoxConverter")
        .def(py::init<>())
        .def("toSensitiveSurface", &Acts::GeoBoxConverter::toSensitiveSurface)
        .def("toPassiveSurface", &Acts::GeoBoxConverter::toPassiveSurface);

    py::class_<Acts::GeoTrdConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoTrdConverter>>(gm, "GeoTrdConverter")
        .def(py::init<>())
        .def("toSensitiveSurface", &Acts::GeoTrdConverter::toSensitiveSurface)
        .def("toPassiveSurface", &Acts::GeoTrdConverter::toPassiveSurface);

    py::class_<Acts::GeoTubeConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoTubeConverter>>(gm, "GeoTubeConverter")
        .def(py::init<>())
        .def("toSensitiveSurface", &Acts::GeoTubeConverter::toSensitiveSurface)
        .def("toPassiveSurface", &Acts::GeoTubeConverter::toPassiveSurface);

    py::class_<Acts::GeoUnionDoubleTrdConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoUnionDoubleTrdConverter>>(
        gm, "GeoUnionDoubleTrdConverter")
        .def(py::init<>())
        .def("toSensitiveSurface",
             &Acts::GeoUnionDoubleTrdConverter::toSensitiveSurface)
        .def("toPassiveSurface",
             &Acts::GeoUnionDoubleTrdConverter::toPassiveSurface);

    py::class_<Acts::GeoIntersectionAnnulusConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoIntersectionAnnulusConverter>>(
        gm, "GeoIntersectionAnnulusConverter")
        .def(py::init<>())
        .def("toSensitiveSurface",
             &Acts::GeoIntersectionAnnulusConverter::toSensitiveSurface)
        .def("toPassiveSurface",
             &Acts::GeoIntersectionAnnulusConverter::toPassiveSurface);

    py::class_<Acts::GeoShiftConverter, Acts::IGeoShapeConverter,
               std::shared_ptr<Acts::GeoShiftConverter>>(gm,
                                                         "GeoShiftConverter")
        .def(py::init<>())
        .def("toSensitiveSurface", &Acts::GeoShiftConverter::toSensitiveSurface)
        .def("toPassiveSurface", &Acts::GeoShiftConverter::toPassiveSurface);
  }
  {
    // GeoMuonMockupExperiment
    auto f =
        py::class_<ActsExamples::GeoMuonMockupExperiment,
                   std::shared_ptr<ActsExamples::GeoMuonMockupExperiment>>(
            gm, "GeoMuonMockupExperiment")
            .def(py::init(
                [](const ActsExamples::GeoMuonMockupExperiment::Config& config,
                   const std::string& name, Acts::Logging::Level level) {
                  return std::make_shared<
                      ActsExamples::GeoMuonMockupExperiment>(
                      config, getDefaultLogger(name, level));
                }))
            .def("constructMS",
                 &ActsExamples::GeoMuonMockupExperiment::constructMS);
    auto c =
        py::class_<ActsExamples::GeoMuonMockupExperiment::Config>(f, "Config")
            .def(py::init<>());
    ACTS_PYTHON_STRUCT(c,
                       /// General properties
                       dumpTree, dbName,
                       /// Mdt properties
                       innerTubeRadius, tubeWallThickness, nTubeLayers, nTubes,
                       mdtFoamThickness, multiLayerSeparation,
                       /// Rpc properties
                       nRpcGasGaps, nRpcAlongZ, nRpcAlongPhi,
                       /// Station properties
                       barrelRadii, nSectors, nEtaStations, stationDistInZ,
                       stationDistInR, endCapWheelLowR, bigWheelDistZ,
                       buildEndcaps);
  }
  // Volume factory
  {
    auto a =
        py::class_<Acts::GeoModelDetectorObjectFactory,
                   std::shared_ptr<Acts::GeoModelDetectorObjectFactory>>(
            gm, "GeoModelDetectorObjectFactory")
            .def(py::init(
                [](const Acts::GeoModelDetectorObjectFactory::Config& cfg,
                   Acts::Logging::Level level) {
                  return std::make_shared<Acts::GeoModelDetectorObjectFactory>(
                      cfg, Acts::getDefaultLogger(
                               "GeoModelDetectorObjectFactory", level));
                }))
            .def("construct", &Acts::GeoModelDetectorObjectFactory::construct);

    py::class_<Acts::GeoModelDetectorObjectFactory::Config>(a, "Config")
        .def(py::init<>())
        .def_readwrite(
            "convertSubVolumes",
            &Acts::GeoModelDetectorObjectFactory::Config::convertSubVolumes)
        .def_readwrite("nameList",
                       &Acts::GeoModelDetectorObjectFactory::Config::nameList)
        .def_readwrite("convertBox",
                       &Acts::GeoModelDetectorObjectFactory::Config::convertBox)
        .def_readwrite(
            "materialList",
            &Acts::GeoModelDetectorObjectFactory::Config::materialList);

    auto convVol =
        py::class_<Acts::GeoModelDetectorObjectFactory::ConvertedGeoVol>(
            a, "ConvertedGeoVol");

    ACTS_PYTHON_STRUCT(convVol, volume, gen2Volume, fullPhysVol, name,
                       surfaces);
    py::class_<Acts::GeoModelDetectorObjectFactory::Cache>(a, "Cache")
        .def(py::init<>())
        .def_readwrite(
            "sensitiveSurfaces",
            &Acts::GeoModelDetectorObjectFactory::Cache::sensitiveSurfaces)
        .def_readwrite(
            "boundingBoxes",
            &Acts::GeoModelDetectorObjectFactory::Cache::volumeBoxFPVs);

    py::class_<Acts::GeoModelDetectorObjectFactory::Options>(a, "Options")
        .def(py::init<>())
        .def_readwrite("queries",
                       &Acts::GeoModelDetectorObjectFactory::Options::queries);
  }

  {
    py::class_<Acts::GeoModelBlueprintCreater::Blueprint,
               std::shared_ptr<Acts::GeoModelBlueprintCreater::Blueprint>>(
        gm, "Blueprint")
        .def("convertToBuilder",
             [](Acts::GeoModelBlueprintCreater::Blueprint& self,
                Acts::Logging::Level level) {
               // It's a container builder
               return std::make_shared<
                   Acts::Experimental::CylindricalContainerBuilder>(self.node(),
                                                                    level);
             });

    auto bpc =
        py::class_<Acts::GeoModelBlueprintCreater,
                   std::shared_ptr<Acts::GeoModelBlueprintCreater>>(
            gm, "GeoModelBlueprintCreater")
            .def(py::init([](const Acts::GeoModelBlueprintCreater::Config& cfg,
                             Acts::Logging::Level level) {
              return std::make_shared<Acts::GeoModelBlueprintCreater>(
                  cfg,
                  Acts::getDefaultLogger("GeoModelBlueprintCreater", level));
            }))
            .def("create", &Acts::GeoModelBlueprintCreater::create);

    py::class_<Acts::GeoModelBlueprintCreater::Config>(bpc, "Config")
        .def(py::init<>())
        .def_readwrite(
            "detectorSurfaces",
            &Acts::GeoModelBlueprintCreater::Config::detectorSurfaces)
        .def_readwrite("kdtBinning",
                       &Acts::GeoModelBlueprintCreater::Config::kdtBinning);

    py::class_<Acts::GeoModelBlueprintCreater::Options>(bpc, "Options")
        .def(py::init<>())
        .def_readwrite("topEntry",
                       &Acts::GeoModelBlueprintCreater::Options::topEntry)
        .def_readwrite(
            "topBoundsOverride",
            &Acts::GeoModelBlueprintCreater::Options::topBoundsOverride)
        .def_readwrite("table", &Acts::GeoModelBlueprintCreater::Options::table)
        .def_readwrite("dotGraph",
                       &Acts::GeoModelBlueprintCreater::Options::dotGraph);
  }

  gm.def(
      "splitBarrelModule",
      [](const Acts::GeometryContext& gctx,
         std::shared_ptr<const GeoModelDetectorElement> detElement,
         unsigned nSegments, Acts::Logging::Level logLevel) {
        auto logger = Acts::getDefaultLogger("ITkSlitBarrel", logLevel);
        auto name = detElement->databaseEntryName();

        auto factory = [&](const auto& trafo, const auto& bounds) {
          return Acts::GeoModelDetectorElement::createDetectorElement<
              Acts::PlaneSurface, Acts::RectangleBounds>(
              detElement->physicalVolume(), bounds, trafo,
              detElement->thickness());
        };

        return ActsExamples::ITk::splitBarrelModule(gctx, detElement, nSegments,
                                                    factory, name, *logger);
      },
      "gxtx"_a, "detElement"_a, "nSegments"_a,
      "logLevel"_a = Acts::Logging::INFO);

  gm.def(
      "splitDiscModule",
      [](const Acts::GeometryContext& gctx,
         std::shared_ptr<const GeoModelDetectorElement> detElement,
         const std::vector<std::pair<double, double>>& patterns,
         Acts::Logging::Level logLevel) {
        auto logger = Acts::getDefaultLogger("ITkSlitBarrel", logLevel);
        auto name = detElement->databaseEntryName();

        auto factory = [&](const auto& trafo, const auto& bounds) {
          return Acts::GeoModelDetectorElement::createDetectorElement<
              Acts::DiscSurface, Acts::AnnulusBounds>(
              detElement->physicalVolume(), bounds, trafo,
              detElement->thickness());
        };

        return ActsExamples::ITk::splitDiscModule(gctx, detElement, patterns,
                                                  factory, name, *logger);
      },
      "gxtx"_a, "detElement"_a, "splitRanges"_a,
      "logLevel"_a = Acts::Logging::INFO);

  py::class_<Acts::GeoModelDetectorElementITk,
             std::shared_ptr<Acts::GeoModelDetectorElementITk>>(
      gm, "GeoModelDetectorElementITk")
      .def("surface", [](Acts::GeoModelDetectorElementITk& self) {
        return self.surface().getSharedPtr();
      });
  gm.def("convertToItk", &GeoModelDetectorElementITk::convertFromGeomodel);
}

}  // namespace Acts::Python
