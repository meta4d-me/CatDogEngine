# Terrain User Guide
This document outlines the basic usage of the terrain generator found in the editor where user may provide various terrain parameters and then click "Update Terrain" to generate the resources needed to render the terrain.

## Terrain Mesh
Currently terrain mesh will be composed of quads where each contain 4 vertices and two triangles. Each vertex contain position and UV coordinates that can be tailed. See the parameter section for details on to control this.

## Terrain Parameters
In the editor, user can edit the following parameters that affects the way terrain mesh and associated textures are created:
1. Sector Count X - the number of sectors in the X-axis direction. Must be at least 1. A sector is a single draw call with vertices and indices.
2. Sector Count Z - the number of sectors in the Z-axis direction. Must be at least 1. A sector is a single draw call with vertices and indices.
3. Quads Count X - the number of quads in the X-axis direction. Must be at least 1. Each quad is composed of two triangles and 4 vertices.
4. Quads Count Z - the number of quads in the Z-axis direction. Must be at least 1. Each quad is composed of two triangles and 4 vertices.
5. Quads Length X - the length between each vertices in the X-axis direction. Must be at least 1. Note each integer unit will contain an elevation value.
5. Quads Length X - the length between each vertices in the Z-axis direction. Must be at least 1. Note each integer unit will contain an elevation value.
6. Min Elevation - the minimum elevation of the terrain. This can be negative but must be smaller than Max Elevation.
7. Max Elevation - the maximum elevation of the terrain. This can be negative but must be larger than Min Elevation.
8. Power - a smoothing factor used to raise the elevation by a specific power. Note this is applied before min/max elevation is calculated.
9. Octaves - Each octave contain a seed, frequency and weight. Seed is used for random number generation that feeds into Simplex2D noise at given 2D position (x, z). Frequency is used to determine how "spiky" the noise generation is. The higher the number, the more "spiky" it is. The lower the number the more smooth the terrain will be but with larget bumps. This number should be powers of 2 such as 4, 8, 16, 32 etc. Finally weight tells the generator how to combine multiple octaves. 