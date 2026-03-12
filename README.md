[![OpenFOAM version](https://img.shields.io/badge/OpenFOAM-v2412-brightgreen)](https://www.openfoam.com/)
## Overview
TracerHistory is a OpenFOAM functionObject designed to track and export the trajectory and field data of individual Lagrangian particles over time.
It generates a dedicated `.dat` file for every single particle detected in the simulation.

Ensure in `Make/files`:
```
LIB = $(FOAM_USER_LIBBIN)/libtracerHistory
```

## Usage
In system/controlDict:

```
functions
{
    traceData
    {
        type            tracerHistory;
        libs            ("libtracerHistory.so");
        field           U;
        clouds          (kinematicCloud1);
        writeControl    writeTime;
        // Custom output directory (default is postProcessing/<name>)
        directory       "postProcessing/particleTracks";
        
        // Output numerical precision (default is 6)
        precision       8;     
    }
}
```