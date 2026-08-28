#!/usr/bin/env python3
"""
ACTS TelescopeDetectorGen3 alignment test with particle gun, Fatras,
digitization, and visualization.

Usage ->

Nominal:
    python telescopeGen3_alignment_test.py --configuration nominal

Misaligned:
    python telescopeGen3_alignment_test.py --configuration misaligned

The digitization resolution default values taken from
Examples/Configs/telescope-digi-smearing-config.json:
    sigma(local0) = 25 um
    sigma(local1) = 100 um
    
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import acts
import acts.examples as examples

from acts.examples.alignment import (
    AlignmentDecorator,
    AlignmentGeneratorGlobalShift,
    GeoIdAlignmentStore,
)

from acts.examples.simulation import (
    EtaConfig,
    MomentumConfig,
    ParticleConfig,
    PhiConfig,
    addDigitization,
    addFatras,
    addParticleGun,
)


u = acts.UnitConstants

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Planar TelescopeDetectorGen3 with optional injected misalignment and digitization"
        )
    )

    parser.add_argument(
        "--configuration",
        choices=("nominal", "misaligned"),
        required=True,
        help="Run nominal geometry or inject one lateral sensor shift.",
    )

    parser.add_argument(
        "--events",
        type=int,
        default=1000,
        help="Number of muon events, must be >0 (default: 1000)",
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path.cwd() / "telescopeGen3_alignment_test",
        help="Top-level output directory.",
    )

    parser.add_argument(
        "--visualize",
        action="store_true",
        help="Write the detector geometry used by this run as a Wavefront OBJ file.",
    )

    parser.add_argument(
        "--show-portals",
        action="store_true",
        help="Include Gen3 navigation portals in the OBJ visualization.",
    )

    parser.add_argument(
        "--show-volumes",
        action="store_true",
        help="Include volume surfaces in the OBJ visualization.",
    )

    return parser.parse_args()

def translation(transform):
    """Return Transform3 translation as x,y,z."""
    tr = transform.translation
    tr = tr() if callable(tr) else tr
    return float(tr[0]), float(tr[1]), float(tr[2])


def make_visualization_context(detector, alignment_decorator=None):
    """
    Create the GeometryContext used for OBJ visualization.

    In the misaligned case, apply the same AlignmentDecorator that is attached
    to the Sequencer so visualize() sees the same shifted sensor transform.
    """
    board = examples.WhiteBoard(
        name="TelescopeGen3Visualization",
        level=acts.logging.WARNING,
    )

    context = examples.AlgorithmContext(
        0,  # algorithm number
        0,  # event number
        board,
        0,  # thread number
    )

    for decorator in detector.contextDecorators():
        result = decorator.decorate(context)
        if result != examples.ProcessCode.SUCCESS:
            raise RuntimeError(
                "Detector context decorator failed during visualization"
            )

    if alignment_decorator is not None:
        result = alignment_decorator.decorate(context)
        if result != examples.ProcessCode.SUCCESS:
            raise RuntimeError(
                "AlignmentDecorator failed during visualization"
            )

    return context.geoContext

def export_geometry_obj(
    detector,
    tracking_geometry,
    output_file: Path,
    alignment_decorator=None,
    *,
    show_portals: bool = False,
    show_volumes: bool = False,
) -> None:
    """Export the nominal or contextually misaligned Gen3 geometry as OBJ."""
    output_file.parent.mkdir(parents=True, exist_ok=True)

    gctx = make_visualization_context(
        detector,
        alignment_decorator,
    )

    visualization = acts.ObjVisualization3D()

    tracking_geometry.visualize(
        visualization,
        gctx,
        portalViewConfig=acts.ViewConfig(visible=show_portals),
        sensitiveViewConfig=acts.ViewConfig(visible=True),
        viewConfig=acts.ViewConfig(visible=show_volumes),
    )

    visualization.write(output_file)
    print(f"OBJ geometry written to: {output_file.resolve()}")


def write_digitization_config(
    path: Path,
) -> None:
    """
    Minimal ACTS smearing digitization configuration
    Based on acts/Examples/Configs/telescope-digi-smearing-config.json 
    But no hardcoded volume selection
    """
    sigma_loc0_mm = 0.025
    sigma_loc1_mm = 0.1
    if sigma_loc0_mm <= 0.0 or sigma_loc1_mm <= 0.0:
        raise ValueError("Digitization stddevs must be positive")

    config = {
        "acts-geometry-hierarchy-map": {
            "format-version": 0,
            "value-identifier": "digitization-configuration",
        },
        "entries": [
            {
                "value": {
                    "smearing": [
                        {
                            "index": 0,
                            "mean": 0.0,
                            "stddev": sigma_loc0_mm,
                            "type": "Gauss",
                        },
                        {
                            "index": 1,
                            "mean": 0.0,
                            "stddev": sigma_loc1_mm,
                            "type": "Gauss",
                        },
                    ]
                }
            }
        ],
    }

    path.parent.mkdir(parents=True, exist_ok=True)

    with path.open("w") as stream:
        json.dump(config, stream, indent=4)
        stream.write("\n")


def main() -> None:
    args = parse_args()

    if args.events <= 0:
        raise SystemExit("--events must be positive")

    # Planar TelescopeDetectorGen3 along global Z

    positions = [
        0.0,
        30.0,
        60.0,
        90.0,
        120.0,
    ]

    detector = examples.TelescopeDetectorGen3(
        surfaceType=examples.TelescopeDetectorGen3.SurfaceType.Plane,
        planeBounds=[20.0, 20.0],
        positions=positions,

        # Omit stereos: TelescopeDetectorGen3 fills zeros automatically.
        axis=2,

        thickness=80.0 * u.um,
        layerEnvelope=1.0 * u.mm,
        logLevel=acts.logging.INFO,
    )

    trackingGeometry = detector.trackingGeometry()

    if trackingGeometry is None:
        raise RuntimeError(
            "TelescopeDetectorGen3 returned no TrackingGeometry"
        )

    # Get sensitive surfaces and order them along Z

    nominalGeometryContext = (
        acts.GeometryContext.dangerouslyDefaultConstruct()
    )

    selector = examples.StructureSelector(trackingGeometry)

    nominalTransforms = selector.selectedTransforms(
        nominalGeometryContext,
        acts.GeometryIdentifier(),
    )

    layers = sorted(
        nominalTransforms.items(),
        key=lambda item: translation(item[1])[2],
    )

    if not layers:
        raise RuntimeError(
            "No sensitive surfaces were found in TelescopeDetectorGen3"
        )

    # Select layer to misalign E.g., the third layer (index=2).
    indx = 2
    layerToBump = layers[indx][0]

    print("Sensitive layers:")
    for index, (geoId, transform) in enumerate(layers):
        x, y, z = translation(transform)

        marker = (
            " <-- selected for shift"
            if index == indx
            else ""
        )

        print(
            f"  {index}: {geoId} "
            f"center=({x:.3f}, {y:.3f}, {z:.3f}) mm"
            f"{marker}"
        )

    # Inject a known misalignment
    alignDeco = None

    if args.configuration == "misaligned":
        layerShift = AlignmentGeneratorGlobalShift()

        dx = 200
        shift = dx * 1.0e-3 * u.mm # 200 um shift (exaggerate to ~5000 um to visualize in OBJ)

        # Lateral shift: +global X.
        layerShift.shift = acts.Vector3(
            shift,
            0.0,
            0.0,
        )

        alignDecoConfig = AlignmentDecorator.Config()

        alignDecoConfig.nominalStore = GeoIdAlignmentStore(
            selector.selectedTransforms(
                nominalGeometryContext,
                layerToBump,
            )
        )

        # Same misalignment for every event.
        alignDecoConfig.iovGenerators = [
            ((0, 10_000_000), layerShift)
        ]

        alignDeco = AlignmentDecorator(
            alignDecoConfig,
            acts.logging.WARNING,
        )

        print()
        print(
            f"Injecting +{dx:.1f} um global-X shift "
            f"into layer {indx}"
        )
        print(f"GeometryIdentifier: {layerToBump}")

    else:
        print()
        print("Nominal configuration: no alignment shift injected.")


    # Output directories

    runOutput = (args.output / args.configuration).resolve()
    simulationOutput = runOutput / "simulation"
    digitizationOutput = runOutput / "digitization"
    geometryOutput = runOutput / "geometry"

    simulationOutput.mkdir(parents=True, exist_ok=True)
    digitizationOutput.mkdir(parents=True, exist_ok=True)

    if args.visualize:
        geometryOutput.mkdir(parents=True, exist_ok=True)


    # Minimal Gen3 digitization
    # write a global hierarchy-map entry, not restricted to a specific volume

    digiConfigFile = runOutput / "telescope-gen3-digi-smearing.json"

    write_digitization_config(
        digiConfigFile,
    )

    # OBJ visualization
    # For the misaligned case, the same AlignmentDecorator used by the
    # sequencer is applied to this one-off visualization context

    if args.visualize:
        geometryFile = geometryOutput / (
            f"telescope-gen3-{args.configuration}.obj"
        )

        export_geometry_obj(
            detector,
            trackingGeometry,
            geometryFile,
            alignDeco,
            show_portals=args.show_portals,
            show_volumes=args.show_volumes,
        )

    # Sequencer

    rnd = examples.RandomNumbers(seed=1729)

    s = examples.Sequencer(
        events=args.events,
        numThreads=1,
        outputDir=str(runOutput),
        logLevel=acts.logging.INFO,
    )

    # Simulation and digitization both see the same aligned/misaligned GeometryContext

    if alignDeco is not None:
        s.addContextDecorator(alignDeco)

    # Particle gun to fire muons at the telescope
    # The first sensor is at z=0.  layerEnvelope=1 mm means z=-0.5 mm is
    # inside its Gen3 layer volume but upstream of the sensitive surface

    vertexGenerator = examples.GaussianVertexGenerator(
        mean=acts.Vector4(
            0.0,
            0.0,
            -0.5 * u.mm,
            0.0,
        ),
        stddev=acts.Vector4(
            5.0 * u.mm,
            5.0 * u.mm,
            0.0 * u.mm,
            0.0 * u.ns,
        ),
    )

    addParticleGun(
        s,
        MomentumConfig(
            10.0 * u.GeV,
            10.0 * u.GeV,
        ),
        EtaConfig(
            3.0,
            3.0,
        ),
        PhiConfig(
            45.0 * u.degree,
            45.0 * u.degree,
        ),
        ParticleConfig(
            1,
            acts.PdgParticle.eMuon,
            randomizeCharge=False,
        ),
        multiplicity=1,
        vtxGen=vertexGenerator,
        rnd=rnd,
        outputDirCsv=simulationOutput,
    )

    # Fatras
    # Disable interactions for a clean alignment demonstration,
    # identical generated tracks in nominal and misaligned runs.

    field = acts.NullBField()

    addFatras(
        s,
        trackingGeometry,
        field,
        rnd=rnd,
        enableInteractions=False,
        outputDirCsv=simulationOutput,
        logLevel=acts.logging.INFO,
    )


    # Minimal digitization, following the Millepede example
    # outputDirCsv adds CsvMeasurementWriter, which writes local0/local1.

    addDigitization(
        s,
        trackingGeometry,
        field,
        digiConfigFile=digiConfigFile,
        outputDirRoot=None,
        outputDirCsv=digitizationOutput,
        rnd=rnd,
        logLevel=acts.logging.INFO,
    )

    print()
    print(f"Configuration     : {args.configuration}")
    print(f"Events            : {args.events}")
    print(f"Output            : {runOutput}")
    print()

    s.run()

    print()
    print("Digitized local measurements are in:")
    print(
        "  "
        + str(
            digitizationOutput
            / "eventXXXXXXXXX-measurements.csv"
        )
    )
    print()
    print("Relevant CSV columns:")
    print(
        "  geometry_id, local0, local1, "
        "var_local0, var_local1"
    )

    if args.configuration == "misaligned":
        print()
        print(
            "For the shifted sensor, a +X surface displacement "
            "appears approximately as a -local0 measurement shift "
            "when the plane local-X direction is aligned with global X."
        )


if __name__ == "__main__":
    main()