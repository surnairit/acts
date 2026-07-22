#!/usr/bin/env python3
"""Minimal test for TelescopeDetectorGen3 with visualization
Usage examples:
    python telescopeGen3_test.py
    python telescopeGen3_test.py --graphviz telescope-gen3.dot
    python telescopeGen3_test.py --visualize
    python telescopeGen3_test.py --visualize output/telescope-gen3.obj
    python telescopeGen3_test.py --visualize --show-portals --show-volumes
    python telescopeGen3_test.py --graphviz telescope_plane-gen3.dot --visualize output/telescope_plane-gen3.obj --surface-type plane

"""

from __future__ import annotations

import argparse
from pathlib import Path

import acts
import acts.examples as examples


def prepare_output_file(path: Path, option_name: str) -> Path:
    """Validate an output filename and create its parent directory."""
    path = path.expanduser()

    if path.exists() and path.is_dir():
        raise ValueError(
            f"{option_name} expects a file path, but '{path}' is a directory"
        )

    path.parent.mkdir(parents=True, exist_ok=True)
    return path


def make_geometry_context(detector: examples.DetectorBase):
    """Create and decorate an ACTS geometry context."""
    event_store = examples.WhiteBoard(
        name="TelescopeGen3GeometryVisualization",
        level=acts.logging.INFO,
    )

    context = examples.AlgorithmContext(
        0,  # algorithm number
        0,  # event number
        event_store,
        0,  # thread number
    )

    for decorator in detector.contextDecorators():
        result = decorator.decorate(context)
        if result != examples.ProcessCode.SUCCESS:
            raise RuntimeError(
                f"Geometry-context decorator failed with result: {result}"
            )

    return context


def export_geometry_obj(
    detector: examples.DetectorBase,
    output_file: Path,
    *,
    show_portals: bool = False,
    show_volumes: bool = False,
) -> None:
    """Export the detector tracking geometry as a Wavefront OBJ file."""
    output_file = prepare_output_file(output_file, "--visualize")

    tracking_geometry = detector.trackingGeometry()
    if tracking_geometry is None:
        raise RuntimeError("The detector did not construct a tracking geometry")

    context = make_geometry_context(detector)
    visualization = acts.ObjVisualization3D()

    tracking_geometry.visualize(
        visualization,
        context.geoContext,
        portalViewConfig=acts.ViewConfig(visible=show_portals),
        sensitiveViewConfig=acts.ViewConfig(visible=True),
        viewConfig=acts.ViewConfig(visible=show_volumes),
    )

    visualization.write(output_file)
    print(f"OBJ geometry written to: {output_file.resolve()}")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Construct a minimal ACTS TelescopeDetectorGen3 and optionally "
            "export its Blueprint graph and 3D geometry."
        )
    )

    parser.add_argument(
        "--surface-type",
        choices=("disc", "plane"),
        default="disc",
        help="Sensor shape and Gen3 layer-volume type (default: disc).",
    )

    parser.add_argument(
        "--graphviz",
        type=Path,
        metavar="FILE.dot",
        help="Write the Gen3 Blueprint hierarchy as a Graphviz DOT file.",
    )

    parser.add_argument(
        "--visualize",
        nargs="?",
        const=Path("telescope-gen3.obj"),
        default=None,
        type=Path,
        metavar="FILE.obj",
        help=(
            "Export the tracking geometry as OBJ. When FILE.obj is omitted, "
            "the output is ./telescope-gen3.obj."
        ),
    )

    parser.add_argument(
        "--show-portals",
        action="store_true",
        help="Include navigation portals in the OBJ export.",
    )

    parser.add_argument(
        "--show-volumes",
        action="store_true",
        help="Include volume surfaces in the OBJ export.",
    )

    return parser.parse_args()


def main() -> None:
    args = parse_arguments()

    units = acts.UnitConstants

    config = examples.TelescopeDetectorGen3.Config()
    config.positions = [
        0.0 * units.mm,
        30.0 * units.mm,
        60.0 * units.mm,
        90.0 * units.mm,
        120.0 * units.mm,
        150.0 * units.mm,
        180.0 * units.mm,
    ]
    config.discBounds = [5.0 * units.mm, 25.0 * units.mm]
    config.planeBounds = [25.0 * units.mm, 25.0 * units.mm]
    #config.bounds = [5.0 * units.mm, 25.0 * units.mm]
    config.surfaceType = (
        examples.TelescopeDetectorGen3.SurfaceType.Disc
        if args.surface_type == "disc"
        else examples.TelescopeDetectorGen3.SurfaceType.Plane
    )
    config.thickness = 80.0 * units.um
    config.layerEnvelope = 1.0 * units.mm
    config.logLevel = acts.logging.INFO

    if args.graphviz is not None:
        try:
            graphviz_file = prepare_output_file(args.graphviz, "--graphviz")
        except ValueError as error:
            raise SystemExit(f"error: {error}") from error
        config.graphvizFile = graphviz_file

    with examples.TelescopeDetectorGen3(config) as detector:
        tracking_geometry = detector.trackingGeometry()
        if tracking_geometry is None:
            raise RuntimeError(
                "TelescopeDetectorGen3 returned no tracking geometry"
            )

        print("test TelescopeDetectorGen3")
        print(
            "tracking geometry constructed:",
            type(tracking_geometry).__name__,
        )
        print("surface type:", args.surface_type)
        print("sensor layers:", len(config.positions))

        if args.graphviz is not None:
            print(
                "Blueprint graph written to:",
                args.graphviz.resolve(),
            )

        if args.visualize is not None:
            try:
                export_geometry_obj(
                    detector,
                    args.visualize,
                    show_portals=args.show_portals,
                    show_volumes=args.show_volumes,
                )
            except ValueError as error:
                raise SystemExit(f"error: {error}") from error


if __name__ == "__main__":
    main()
