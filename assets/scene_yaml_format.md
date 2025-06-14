# Simple YAML-like Scene Loader Format

This project supports loading scenes from a simple YAML-like text file using `Scene::fromYAML(filename)`.

## Usage

To load a scene from a file:
```cpp
Scene scene = Scene::fromYAML('path/to/scene.txt');
```

## File Format

- Each line defines an element: background, material, sphere, plane, or light.
- **Empty lines and extra spaces/tabs are allowed.**
- Only one material is active at a time (applies to subsequent objects).

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
- Example: `material: 0.8 0.2 0.2`
- Applies to all following objects until a new material is set.

#### Sphere
```
sphere: X Y Z RADIUS
```
- Example: `sphere: 0 0 1 0.5`

#### Plane
```
plane: NX NY NZ D
```
- Example: `plane: 0 1 0 1`

#### Light
```
light: X Y Z R G B
```
- Example: `light: 0 2 0 2 2 2`

## Example Scene File
```
background:   0.1   0.1 0.2

material: 0.8 0.2 0.2
sphere: 0 0 1 0.5

material: 0.2 0.8 0.2
sphere: 1 0 1 0.5

material: 0.5 0.5 0.5
plane: 0 1 0 1

light: 0 2 0 2 2 2
```

## Notes
- Only the above elements are supported.
- Loader is case-sensitive and expects the exact keywords shown above.
- **Empty lines and extra whitespace are ignored.**
