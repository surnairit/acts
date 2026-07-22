#!/usr/bin/env python3
"""Fatras simulation with a planar TelescopeDetectorGen3."""

from __future__ import annotations

from pathlib import Path

import acts
import acts.examples as examples
from acts.examples.simulation import (
    EtaConfig,
    MomentumConfig,
    ParticleConfig,
    PhiConfig,
    addFatras,
    addParticleGun,
)


u = acts.UnitConstants


def main() -> None:
    # Telescope plane positions along global Z (mm)
    positions = [
        30.0,
        60.0,
        90.0,
        120.0,
        150.0,
        180.0,
        210.0,
        240.0,
        270.0,
    ]

    # Extra half-width of each layer volume along Z.
    layer_envelope = 1.0

    detector = examples.TelescopeDetectorGen3(
        surfaceType=examples.TelescopeDetectorGen3.SurfaceType.Plane,

        # 400 mm x 400 mm sensors.
        planeBounds=[200.0, 200.0],

        positions=positions,

        thickness=0.08,

        layerEnvelope=layer_envelope,
        logLevel=acts.logging.INFO,
    )

    tracking_geometry = detector.trackingGeometry()
    if tracking_geometry is None:
        raise RuntimeError(
            "TelescopeDetectorGen3 did not construct a tracking geometry"
        )

    field = acts.ConstantBField(
        acts.Vector3(0.0, 0.0, 2.0 * u.T)
    )

    output_dir = Path.cwd() / "telescopeGen3_simulation"
    fatras_output_dir = output_dir / "fatras"
    fatras_output_dir.mkdir(parents=True, exist_ok=True)

    rndm = examples.RandomNumbers(seed=42)

    sequencer = examples.Sequencer(
        events=1,
        numThreads=1,
        logLevel=acts.logging.INFO,
    )

    # The tracking geometry begins near the first plane. Put the particle
    # vertex inside the first layer volume, slightly upstream of its sensor.
    z_start = positions[0] - 0.5 * layer_envelope

    vertex_generator = examples.GaussianVertexGenerator(
        mean=acts.Vector4(
            0.0,
            0.0,
            z_start,
            0.0,
        ),
        stddev=acts.Vector4(
            0.0,
            0.0,
            0.0,
            0.0,
        ),
    )

    addParticleGun(
        sequencer,
        momentumConfig=MomentumConfig(
            1.0 * u.GeV,
            10.0 * u.GeV,
        ),

        #particles almost parallel to +Z.
        etaConfig=EtaConfig(
            10.0,
            10.0,
        ),

        # fixed phi for test.
        phiConfig=PhiConfig(
            0.0,
            0.0,
        ),

        particleConfig=ParticleConfig(
            100,
            acts.PdgParticle.eMuon,
            False,
        ),

        multiplicity=1,
        vtxGen=vertex_generator,
        rnd=rndm,

        outputDirCsv=fatras_output_dir,
        outputDirRoot=fatras_output_dir,
        printParticles=True,
    )

    addFatras(
        sequencer,
        tracking_geometry,
        field,
        rnd=rndm,

        outputParticles="particles_simulated",
        outputSimHits="simhits",

        outputDirCsv=fatras_output_dir,
        outputDirRoot=fatras_output_dir,

        logLevel=acts.logging.INFO,
    )

    print(f"Starting vertex: (0, 0, {z_start} mm)")
    print(f"Output directory: {fatras_output_dir.resolve()}")

    sequencer.run()


if __name__ == "__main__":
    main()