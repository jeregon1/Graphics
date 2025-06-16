# Scene YAML Format (as parsed by Scene::fromYAML)

This project loads scenes from a simple, flat YAML-like file. Each line defines an element. No nested YAML or indentation is supported.

## Usage

```cpp
auto result = Scene::fromYAML("path/to/scene.yaml");
if (result) {
    auto& [scene, camera] = *result;
    // camera is std::optional<PinholeCamera> - may be empty
}
```

## File Format

- Each line starts with a keyword: `background:`, `material:`, `sphere:`, `plane:`, `light:`, or `camera:`
- Only one material is active at a time (applies to subsequent objects)
- No nested properties for material (only `material: R G B` is supported)
- All keywords must be at the start of the line (no indentation)
- Lines starting with `#` are comments
- Extra whitespace and empty lines are ignored

### Supported Elements

#### Background Color
```
background: R G B
```
- Example: `background: 0.1 0.1 0.2`

#### Material
```
material: R G B
```
- Only RGB diffuse values are supported (no specular, transparency, etc.)
- Example: `material: 0.8 0.2 0.2`

#### Sphere
```
sphere: X Y Z RADIUS
```
- Example: `sphere: 0 0 1 0.5`

#### Plane
```
plane: NX NY NZ D
```
- D is distance (cast to int in implementation)
- Example: `plane: 0 1 0 1.0`

#### Light
```
light: X Y Z R G B
```
- Example: `light: 0 2 0 2 2 2`

#### Camera (optional)
```
camera:
```
- Camera parsing is handled by `parseCamera(file)` function
- Implementation details depend on the parseCamera function
- If present, camera will be available in the returned optional

## Example Scene File
```
# Simple scene with background, materials, objects and light
background: 0.1 0.1 0.2

material: 0.8 0.2 0.2
sphere: 0 0 1 0.5

material: 0.2 0.8 0.2
sphere: 1 0 1 0.5

material: 0.5 0.5 0.5
plane: 0 1 0 1

light: 0 2 0 2 2 2
```

## Notes
- Only the above elements are supported
- Loader is case-sensitive and expects the exact keywords shown above
- No nested YAML or complex indentation (flat format)
- Comments start with `#`
- Extra whitespace and empty lines are ignored
- Unknown keywords are silently ignored
- Function returns `std::optional<std::pair<Scene, std::optional<PinholeCamera>>>`
