# Thermal Vision Setup Guide

## Overview
This guide explains how to create a thermal vision post-process effect for the DroneSystemPro plugin. The thermal vision mode uses post-process materials to create a heat-based visualization.

## Creating the Thermal Vision Material

### Step 1: Create Post Process Material

1. **Create New Material**:
   - Content Browser → Right Click → Material
   - Name: `M_ThermalVision` or `PP_ThermalVision`
   - Open the material

2. **Material Settings**:
   - Material Domain: **Post Process**
   - Blendable Location: **Before Tonemapping**
   - Shading Model: **Unlit**

### Step 2: Material Graph - Basic Thermal Effect

#### Simple Heat Map Approach:

```
Scene Color → Desaturate → [Heat Ramp]
```

**Detailed Steps:**

1. **Get Scene Color**:
```
SceneTexture:PostProcessInput0 → Store as SceneColor
```

2. **Calculate Luminance** (brightness as heat proxy):
```
SceneColor.RGB → Dot Product with (0.299, 0.587, 0.114) → Store as Luminance
```

3. **Apply Heat Gradient**:
```
Luminance → Power(2.0) to amplify hot spots → Clamp(0, 1)
Create gradient:
  - If Luminance < 0.2: Blue (0, 0, 1)
  - If Luminance 0.2-0.4: Cyan (0, 1, 1)
  - If Luminance 0.4-0.6: Green (0, 1, 0)
  - If Luminance 0.6-0.8: Yellow (1, 1, 0)
  - If Luminance > 0.8: Red (1, 0, 0)
```

4. **Output**:
```
HeatColor → Multiply by ThermalIntensity → Emissive Color
```

### Step 3: Advanced Thermal with Object Detection

For more realistic thermal vision that highlights living entities:

```cpp
// Pseudocode for material logic

// Sample scene
SceneColor = SceneTexture:PostProcessInput0
CustomDepth = SceneTexture:CustomDepth
CustomStencil = SceneTexture:CustomStencil

// Check if pixel is a detected entity (set by DroneVisionComponent)
IsEntity = (CustomStencil >= 100 && CustomStencil < 200) ? 1 : 0
IsHighHeat = (CustomStencil >= 150 && CustomStencil < 200) ? 1 : 0

// Base heat from luminance
BaseLuminance = Dot(SceneColor.RGB, float3(0.299, 0.587, 0.114))

// Boost heat for detected entities
HeatValue = Lerp(BaseLuminance, 1.0, IsEntity * 0.7)
HeatValue = Lerp(HeatValue, 1.0, IsHighHeat * 0.3)

// Apply thermal color gradient
if (HeatValue < 0.2)
    ThermalColor = float3(0, 0, 0.5) // Dark Blue
else if (HeatValue < 0.4)
    ThermalColor = float3(0, 0.5, 1) // Blue
else if (HeatValue < 0.6)
    ThermalColor = float3(0, 1, 0) // Green
else if (HeatValue < 0.8)
    ThermalColor = float3(1, 1, 0) // Yellow
else
    ThermalColor = float3(1, 0, 0) // Red

// Add scan lines
ScanLine = frac(UV.y * 100 + Time) < 0.5 ? 0.9 : 1.0
ThermalColor *= ScanLine

// Add noise for realism
Noise = SimplexNoise(UV * 50 + Time * 0.1) * 0.1
ThermalColor += Noise

// Output
EmissiveColor = ThermalColor
```

### Step 4: Material Parameters

Add these parameters for runtime control:

```
- ThermalIntensity: Scalar (Default: 1.0)
- HeatSensitivity: Scalar (Default: 1.5)
- ScanLineSpeed: Scalar (Default: 1.0)
- NoiseIntensity: Scalar (Default: 0.1)
- ColorTemperatureCold: Color (Blue)
- ColorTemperatureMedium: Color (Green)
- ColorTemperatureHot: Color (Red)
```

## Heat Gradient Texture

Create a 1D gradient texture for smooth color transitions:

1. **Create Texture**:
   - Content Browser → Right Click → Texture → Render Target
   - Set to 256x1 pixels
   - Name: `T_ThermalGradient`

2. **Gradient Colors** (Left to Right):
   - 0%: Black (0,0,0)
   - 10%: Dark Blue (0,0,0.3)
   - 25%: Blue (0,0,1)
   - 40%: Cyan (0,1,1)
   - 55%: Green (0,1,0)
   - 70%: Yellow (1,1,0)
   - 85%: Orange (1,0.5,0)
   - 100%: White (1,1,1)

3. **Use in Material**:
```
Luminance → Clamp(0,1) → Sample T_ThermalGradient (UV.x = Luminance, UV.y = 0.5)
```

## Night Vision Material

Create a companion night vision material:

### M_NightVision Setup:

```
SceneColor → Desaturate → Multiply by Green (0, 1, 0)
Add grain/noise
Add vignette
Add scan lines
Output green-tinted image
```

**Material Graph:**

```
// Get scene
SceneColor = SceneTexture:PostProcessInput0

// Desaturate to grayscale
Gray = Dot(SceneColor.RGB, float3(0.299, 0.587, 0.114))

// Amplify brightness (night vision boost)
Amplified = Gray * NightVisionGain (default 2.5)

// Apply green tint
GreenTint = float3(0, Amplified, 0)

// Add grain
Grain = Random(UV + Time) * GrainIntensity
GreenTint += Grain

// Add vignette
Center = float2(0.5, 0.5)
DistFromCenter = length(UV - Center)
Vignette = 1.0 - saturate(DistFromCenter * VignetteStrength)
GreenTint *= Vignette

// Add scan lines
ScanLine = step(0.5, frac(UV.y * ScanLineCount + Time * ScanLineSpeed))
GreenTint *= lerp(0.9, 1.0, ScanLine)

// Output
EmissiveColor = GreenTint
```

## Integration with DroneVisionComponent

The DroneVisionComponent automatically applies vision mode effects:

```cpp
// In your game code or blueprint
void AMyPlayerController::OnPossess(APawn* InPawn)
{
    ADroneBase* Drone = Cast<ADroneBase>(InPawn);
    if (Drone)
    {
        // Get camera
        UCameraComponent* Camera = Drone->FindComponentByClass<UCameraComponent>();
        if (Camera)
        {
            // Load thermal material
            UMaterialInterface* ThermalMat = LoadObject<UMaterialInterface>(nullptr,
                TEXT("/Game/DroneSystemPro/Materials/PP_ThermalVision.PP_ThermalVision"));

            // Load night vision material
            UMaterialInterface* NightMat = LoadObject<UMaterialInterface>(nullptr,
                TEXT("/Game/DroneSystemPro/Materials/PP_NightVision.PP_NightVision"));

            // Create dynamic instances
            ThermalMaterialInstance = UMaterialInstanceDynamic::Create(ThermalMat, this);
            NightMaterialInstance = UMaterialInstanceDynamic::Create(NightMat, this);

            // Listen to vision mode changes
            UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
            if (VisionComp)
            {
                VisionComp->OnVisionModeChanged.AddDynamic(this, &AMyPlayerController::OnVisionModeChanged);
            }
        }
    }
}

void AMyPlayerController::OnVisionModeChanged(EDroneVisionMode NewMode)
{
    UCameraComponent* Camera = GetPawn()->FindComponentByClass<UCameraComponent>();
    if (!Camera) return;

    // Clear existing post process materials
    Camera->PostProcessSettings.WeightedBlendables.Array.Empty();

    // Apply appropriate material
    switch (NewMode)
    {
    case EDroneVisionMode::Thermal:
        Camera->PostProcessSettings.WeightedBlendables.Array.Add(
            FWeightedBlendable(1.0f, ThermalMaterialInstance));
        break;

    case EDroneVisionMode::Night:
        Camera->PostProcessSettings.WeightedBlendables.Array.Add(
            FWeightedBlendable(1.0f, NightMaterialInstance));
        break;

    case EDroneVisionMode::Normal:
    default:
        // No post process for normal vision
        break;
    }
}
```

## Blueprint Implementation

Create a Blueprint widget that responds to vision mode changes:

**BP_DroneVisionOverlay**:

1. **Event Graph**:
```
Event Construct
  → Get Owning Player Pawn
  → Cast to DroneBase
  → Get Drone Vision Component
  → Bind Event to OnVisionModeChanged

OnVisionModeChanged (Event)
  → Switch on Vision Mode
    → Normal: Hide all overlays
    → Night: Show night vision overlay
    → Thermal: Show thermal vision overlay
```

2. **Designer**:
   - Overlay widget
   - Image for thermal scan lines
   - Border for vignette effect
   - Text for mode indicator

## Additional Effects

### Thermal Noise

Add animated noise for realism:

```
// Simplex noise function or use Noise texture
NoiseUV = UV * NoiseScale + Time * NoiseSpeed
Noise = SampleTexture(T_Noise, NoiseUV)
ThermalColor += (Noise - 0.5) * NoiseIntensity
```

### Thermal Bloom

Add bloom to hot objects:

```
if (HeatValue > 0.7)
{
    Bloom = (HeatValue - 0.7) / 0.3
    ThermalColor += Bloom * BloomColor
}
```

### Distance Haze

Objects further away should be cooler:

```
SceneDepth = SceneTexture:SceneDepth
DepthFade = saturate(SceneDepth / MaxThermalDistance)
HeatValue *= (1.0 - DepthFade * 0.5)
```

## Performance Optimization

1. **LOD System**:
   - Reduce thermal effect quality with distance
   - Disable for objects beyond sensor range

2. **Conditional Activation**:
   - Only enable post-process when in thermal mode
   - Cache material instances

3. **Mobile Optimization**:
   - Use simpler gradient lookup
   - Reduce noise samples
   - Lower resolution for thermal pass

## Testing Checklist

- [ ] Thermal vision activates when mode is set to Thermal
- [ ] Living entities (Characters) show as hot (red/yellow)
- [ ] Environment shows as cool (blue/green)
- [ ] Scan lines animate smoothly
- [ ] Noise adds realistic texture
- [ ] Performance is acceptable (check with stat fps)
- [ ] Multiplayer: Vision mode is client-side only

## Troubleshooting

**Thermal effect not showing**:
- Verify post-process material is added to camera
- Check material domain is "Post Process"
- Ensure CustomStencil values are set correctly

**Too bright/dark**:
- Adjust ThermalIntensity parameter
- Modify HeatSensitivity

**Performance issues**:
- Reduce noise resolution
- Simplify gradient calculations
- Use lower resolution render targets

---

**Note**: The thermal detection itself is handled by `UDroneVisionComponent::PerformThermalDetection()`, which identifies heat sources. This material provides the visual representation of that data.
