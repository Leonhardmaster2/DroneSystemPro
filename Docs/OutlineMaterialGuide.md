# Outline Material Setup Guide

## Overview
This guide explains how to create the outline/marking material for the DroneSystemPro plugin. The marking system uses Custom Depth/Stencil rendering to draw outlines around marked enemies, visible through walls.

## Prerequisites
- The DroneMarkingComponent automatically sets marked actors to render in Custom Depth with stencil value 255
- You need to create a Post Process Material in Unreal Editor

## Creating the Outline Material

### Step 1: Enable Custom Depth-Stencil Pass

1. **Project Settings**:
   - Edit → Project Settings
   - Engine → Rendering
   - Enable "Custom Depth-Stencil Pass" = "Enabled with Stencil"

### Step 2: Create Post Process Material

1. **Create Material**:
   - Content Browser → Right Click → Material
   - Name: `M_DroneOutline` or `M_MarkedEnemyOutline`
   - Open the material

2. **Material Settings**:
   - Material Domain: **Post Process**
   - Blendable Location: **After Tonemapping** (for best visibility)
   - Shading Model: **Unlit**

### Step 3: Material Graph Setup

#### Main Nodes:

```
Scene Texture: CustomDepth
  ↓
[Sobel Edge Detection or Custom Edge Detection]
  ↓
Multiply with Outline Color (e.g., Red: 1, 0, 0)
  ↓
Scene Texture: PostProcessInput0
  ↓
Lerp (Alpha = Edge Mask)
  ↓
Emissive Color
```

#### Detailed Material Expression Setup:

**1. Scene Texture Nodes:**
```
Scene Texture: CustomDepth (Scene Texture Id = CustomDepth)
Scene Texture: CustomStencil (Scene Texture Id = CustomStencil)
Scene Texture: PostProcessInput0 (Original scene color)
```

**2. Stencil Mask:**
```
CustomStencil → Equals (Value = 255) → [StencilMask]
```
This creates a mask for pixels with stencil value 255 (marked enemies).

**3. Edge Detection (Sobel Filter):**
```
CustomDepth → [Sample neighbors with UV offsets]
Apply Sobel kernel:
  Horizontal: [-1, 0, 1, -2, 0, 2, -1, 0, 1]
  Vertical: [-1, -2, -1, 0, 0, 0, 1, 2, 1]
Combine: sqrt(Gx² + Gy²)
Threshold to get clean edges
```

**4. Combine and Apply:**
```
EdgeMask → Multiply → StencilMask
Result → Multiply → OutlineColor
Lerp between OriginalScene and OutlineColor using EdgeMask
```

### Step 4: Simplified Material Graph

For a quick implementation, you can use this simplified approach:

**Material Expressions:**

1. **SceneTexture:CustomDepth** → Output to Variable `CustomDepth`
2. **SceneTexture:CustomStencil** → Output to Variable `CustomStencil`
3. **SceneTexture:PostProcessInput0** → Output to Variable `SceneColor`

4. **Edge Detection Approximation:**
```
TexCoord → Add (0.001, 0) → Sample CustomDepth → Subtract CustomDepth → Abs → Store as EdgeX
TexCoord → Add (0, 0.001) → Sample CustomDepth → Subtract CustomDepth → Abs → Store as EdgeY
EdgeX + EdgeY → Multiply by 100 → Clamp (0, 1) → Store as EdgeMask
```

5. **Stencil Check:**
```
CustomStencil → Equals (Const 255) → If (True = 1, False = 0) → Store as IsMarked
```

6. **Final Composite:**
```
EdgeMask → Multiply → IsMarked → Store as FinalMask
FinalMask → Multiply → Color (1, 0, 0) → Store as OutlineColor
Lerp (A = SceneColor, B = OutlineColor, Alpha = FinalMask) → Emissive Color
```

### Step 5: Material Parameters

Add these parameters to make the material customizable:

```
- OutlineColor: Vector3 (Default: Red 1,0,0)
- OutlineThickness: Scalar (Default: 1.0)
- OutlineIntensity: Scalar (Default: 1.0)
- GlowIntensity: Scalar (Default: 0.5)
```

## Advanced: Through-Wall Glow

To make marked enemies glow through walls:

1. **Depth Mask**:
```
SceneDepth → CustomDepth → Subtract
If (Result > 0) → Enemy is behind wall
```

2. **Glow Effect**:
```
IsMarked → Multiply → BehindWallMask
Result → Multiply → GlowColor (Red with reduced alpha)
Add to final composite
```

## Blueprint Material Setup Code

Here's the complete material setup in blueprint-friendly pseudocode:

```
// Get textures
CustomDepth = SceneTexture:CustomDepth
CustomStencil = SceneTexture:CustomStencil
SceneColor = SceneTexture:PostProcessInput0
SceneDepth = SceneTexture:SceneDepth

// Check if pixel is a marked target
IsMarked = (CustomStencil == 255) ? 1 : 0

// Sobel edge detection
UV = TexCoord
PixelSize = 0.001

// Sample neighbors
D0 = SampleCustomDepth(UV + PixelSize * float2(-1, -1))
D1 = SampleCustomDepth(UV + PixelSize * float2(0, -1))
D2 = SampleCustomDepth(UV + PixelSize * float2(1, -1))
D3 = SampleCustomDepth(UV + PixelSize * float2(-1, 0))
D4 = CustomDepth
D5 = SampleCustomDepth(UV + PixelSize * float2(1, 0))
D6 = SampleCustomDepth(UV + PixelSize * float2(-1, 1))
D7 = SampleCustomDepth(UV + PixelSize * float2(0, 1))
D8 = SampleCustomDepth(UV + PixelSize * float2(1, 1))

// Sobel filter
Gx = -D0 + D2 - 2*D3 + 2*D5 - D6 + D8
Gy = -D0 - 2*D1 - D2 + D6 + 2*D7 + D8
EdgeStrength = sqrt(Gx*Gx + Gy*Gy)

// Threshold and combine
Edge = (EdgeStrength > 0.01) ? 1 : 0
FinalMask = Edge * IsMarked * OutlineIntensity

// Through-wall detection
BehindWall = (SceneDepth > CustomDepth) ? 1 : 0
ThroughWallGlow = IsMarked * BehindWall * GlowIntensity

// Final composite
OutlineColor = float3(1, 0, 0) * FinalMask
GlowColor = float3(1, 0.2, 0.2) * ThroughWallGlow
Result = SceneColor + OutlineColor + GlowColor

// Output
EmissiveColor = Result
```

## Applying the Material

### Option 1: Camera Post Process Volume

1. Place a **Post Process Volume** in your level
2. Set it to **Unbound** (affects entire level)
3. Add the material to **Rendering Features → Post Process Materials**

### Option 2: Camera Component

1. Select your drone's Camera component
2. In **Post Process → Post Process Materials**, add your outline material

### Option 3: Via Code

```cpp
// In DroneBase.cpp or custom component
if (Camera)
{
    Camera->PostProcessSettings.WeightedBlendables.Array.Add(
        FWeightedBlendable(1.0f, OutlineMaterialInstance)
    );
}
```

## Testing

1. **Enable Custom Depth Rendering**:
   - Select any actor in the scene
   - Details → Rendering
   - Check "Render CustomDepth Pass"
   - Set "CustomDepth Stencil Value" = 255

2. **Test Without Drone**:
   - The actor should now have an outline
   - Verify it's visible through walls

3. **Test With Drone Marking**:
   - Play in PIE
   - Mark a target using the drone (default: R key)
   - Target should show outline
   - Outline should expire after mark duration

## Color Schemes

**Recommended outline colors:**

- **Enemies**: Red (1, 0, 0)
- **Allies**: Blue (0, 0.5, 1)
- **Objectives**: Yellow (1, 1, 0)
- **Low Battery**: Orange (1, 0.5, 0)

## Performance Considerations

- **Edge Detection Cost**: Sobel filter requires 9 texture samples
- **Optimization**: Use lower pixel size for distant objects
- **LOD**: Disable outline for objects beyond certain distance
- **Mobile**: Use simpler edge detection (2-sample instead of 9)

## Troubleshooting

### Outline Not Showing

1. Verify Custom Depth-Stencil Pass is enabled in Project Settings
2. Check material domain is set to "Post Process"
3. Ensure material is added to camera's post process materials
4. Verify marked actor has CustomDepth enabled (DroneMarkingComponent does this automatically)

### Outline Too Thick

- Reduce `PixelSize` in UV offsets
- Adjust `OutlineThickness` parameter
- Use higher resolution Scene Textures

### Performance Issues

- Reduce edge detection samples (use 4-sample cross instead of 9-sample)
- Implement distance-based culling
- Use material quality switches

## Additional Features

### Pulsing Effect

```
Time → Sine → Multiply by 0.5 → Add 0.5 → Multiply → OutlineIntensity
```

### Color by Distance

```
DistanceToCamera → Divide by MaxDistance → Lerp(NearColor, FarColor)
```

### Threat Level Colors

```
If (StencilValue == 255) → Red (High Threat)
If (StencilValue == 200) → Orange (Medium Threat)
If (StencilValue == 150) → Yellow (Low Threat)
```

---

## Complete Material Function

For easier reuse, create a **Material Function** called `MF_CustomDepthOutline`:

**Inputs:**
- Outline Color (Vector3)
- Thickness (Scalar)
- Intensity (Scalar)
- Stencil Value (Scalar, default 255)

**Outputs:**
- Outline Mask (Scalar)
- Final Color (Vector3)

This function can be reused across multiple post-process materials for different visual effects.

---

**Note**: This material works automatically with the DroneMarkingComponent's `ApplyMarkVisuals()` function, which sets `RenderCustomDepth = true` and `CustomDepthStencilValue = 255` on marked targets.
