# Transformed Geometry Shaders

`transformed.hlsl` is the first 360 shader pair for the software-transformed
Re-Volt vertex path. It preserves screen-space `POSITION0`, diffuse/specular
colors, and two UV sets.

Compiled with the installed XeDK 7645 `fxc.exe`:

```text
vs_3_0 /E transformed_vs -> transformed_vs.xvu
ps_3_0 /E transformed_ps -> transformed_ps.xvu
```

The binaries are checked in as intermediate bring-up artifacts. The final
resource pipeline should package them with the renderer resources rather than
load them from a host path.
