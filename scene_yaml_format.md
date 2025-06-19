# Scene YAML Format

This project loads scenes from a simple YAML-like file.

## Usage

```cpp
auto result = Scene::fromYAML("path/to/scene.yaml");
if (result) {
    auto& [scene, camera] = *result;
    // camera is std::optional<PinholeCamera> - may be empty
}
```

## File Format

- Each line starts with a keyword: `background:`, `material:`, `sphere:`, `plane:`, `light:`, `camera:`, `triangle:`, `cone:`, `cylinder:`
- Only **one material** is active at a time (applies to subsequent objects)
- Top-level keywords must be at the start of the line (no indentation)

- Lines starting with `#` are comments

### Supported Elements

### Background Color
```
background: R G B
```

### Material
Simple format (diffuse only):
```
material: R G B
```

Extended format:
```
material:
  diffuse: R G B
  specular: S
  transmittance: T
  emission: R G B
  n: REFRACTIVE_INDEX
```
- All of them are optional.
- For specular and transmittance, a single value is used for all RGB channels

### Sphere
```
sphere: X Y Z  RADIUS
```
- Example: `sphere: 0 0 1  0.5`

### Plane
```
plane: NX NY NZ  D
```
- D is distance to origin
- Example: `plane: 0 1 0 1.2`

### Triangle
```
triangle: AX AY AZ BX BY BZ CX CY CZ
```
- Defines a triangle with vertices at points A, B, and C

### Cone
```
cone:
  base: X Y Z
  axis: X Y Z
  radius: R
  height: H
```

### Cylinder
```
cylinder:
  base: X Y Z
  axis: X Y Z
  radius: R
  height: H
```

### Light
```
light: X Y Z  R G B
```
- X, Y, Z define the position
- R, G, B define the power/intensity
- Example: `light: 0 2 0 2 2 2`

### Camera (optional)
Basic format:
```
camera:
  origin: X Y Z
  fov: ANGLE
  forward: X Y Z
  pixels: WIDTH HEIGHT
```

Extended format:
```
camera:
  origin: X Y Z
  left: X Y Z
  up: X Y Z
  forward: X Y Z
  pixels: WIDTH HEIGHT
```
- If present, camera will be available in the returned optional

## Example Scene File
```
# Simple scene with background, materials, objects and light
background: 0.1 0.1 0.2

material: 0.8 0.2 0.2
sphere: 0 0 1 0.5

material:
  diffuse: 0.2 0.8 0.2
  specular: 0.5
  n: 1.5
sphere: 1 0 1 0.5

material: 0.5 0.5 0.5
plane: 0 1 0 1

triangle: 0 0 0 1 0 0 0 1 0

light: 0 2 0 2 2 2

camera:
  origin: 0 0 -3
  fov: 60
  pixels: 1024 768
```

## Notes
- Loader is case-sensitive and expects the exact keywords shown above
- Both flat and hierarchical YAML styles are supported depending on the element
- Comments start with `#`
- Extra whitespace and empty lines are ignored
- Unknown keywords are silently ignored
- Function returns `std::optional<std::pair<Scene, std::optional<PinholeCamera>>>`
