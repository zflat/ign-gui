# Ignition Visualization Marker Example

This example demonstrates how to create, modify, and delete visualization
markers in Ignition GUI.

## Build Instructions

From this directory:

    mkdir build
    cd build
    cmake ..
    make

## Execute Instructions

Launch the `markers` example config:

    ign gui -c examples/config/markers.config

Then from the build directory above:

    ./marker

The terminal will output messages indicating visualization changes that
will occur in Ignition GUI's render window.
