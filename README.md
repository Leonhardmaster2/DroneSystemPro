# DroneSystemPro

## Overview

DroneSystemPro is a complete, production-ready Unreal Engine 5.6 C++ plugin providing a comprehensive multiplayer drone system with AI, networking, vision modes, battery management, and environmental interactions. This plugin is marketplace-ready and designed for professional game development.

## Features

### Core Systems
- **Advanced Movement System**: Client prediction with server reconciliation for smooth multiplayer experience
- **Battery Management**: Realistic battery drain based on feature usage with auto-recharge at docking stations
- **Vision Modes**: Normal, Night, and Thermal vision with efficient network transmission
- **Target Marking**: Mark enemies with outline effect through walls, networked with optimizations
- **AI Behavior System**: Data-driven behaviors (Patrol, Follow, Scan, Attack-Mark) using DataAssets

### Environmental Interactions
- **Jamming System**: Degrades drone sensors and increases battery drain in affected radius
- **Hacking System**: Networked hacking minigame for interactive terminals
- **Docking Stations**: Auto-recharge and recall system for drones
- **Terminal Interaction**: Hackable terminals with server-validated progress

### Networking Features
- **Client Prediction**: Smooth movement with server reconciliation
- **Bandwidth Optimization**: Quantized data, delta compression, relevancy management
- **Security**: Server-side validation of all inputs and actions
- **Replication**: Optimized replication with distance-based prioritization

## Installation

### Prerequisites
- Unreal Engine 5.6
- Visual Studio 2022 (Windows) or Xcode (macOS)
- C++ project (not Blueprint-only)

### Installation Steps

1. **Copy Plugin to Project**
   ```
   Copy DroneSystemPro folder to: YourProject/Plugins/
   ```

2. **Add Plugin to .uproject**
   Add to your project's .uproject file:
   ```json
   "Plugins": [
       {
           "Name": "DroneSystemPro",
           "Enabled": true
       }
   ]
   ```

3. **Add Module Dependency**
   Add to your game module's Build.cs file:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] { "DroneSystemPro" });
   ```

4. **Generate Project Files**
   - Right-click .uproject → Generate Visual Studio project files
   - Or run: `UnrealBuildTool.exe -projectfiles -project="YourProject.uproject"`

5. **Compile**
   - Open Visual Studio solution
   - Build in Development Editor configuration
   - Launch editor

## Quick Start

### Creating a Drone in Blueprint

1. **Create Drone Blueprint**
   - Content Browser → Add → Blueprint Class
   - Parent Class: DroneBase
   - Name: BP_MyDrone

2. **Configure Drone**
   - Open BP_MyDrone
   - Add/configure mesh in DroneMesh component
   - Create or assign DroneConfig DataAsset
   - Set default values

3. **Create DroneConfig DataAsset**
   - Content Browser → Add → Miscellaneous → Data Asset
   - Class: DroneConfig
   - Configure movement, battery, sensors, networking values

### Setting Up Input

Add these input mappings to your project:
```
Axis Mappings:
- MoveForward: W/S, Gamepad Left Thumbstick Y
- MoveRight: D/A, Gamepad Left Thumbstick X
- MoveUp: E/Q, Gamepad Right Shoulder/Left Shoulder
- LookUp: Mouse Y, Gamepad Right Thumbstick Y
- Turn: Mouse X, Gamepad Right Thumbstick X

Action Mappings:
- ToggleSpeed: Left Shift
- ToggleFlashlight: F
- ToggleVision: V
- Mark: R, Gamepad Face Button Right
```

### Placing a Docking Station

1. Create Blueprint based on Actor
2. Add DroneDockingComponent
3. Configure docking range and auto-recall settings
4. Place in level

### Adding Jamming Field

1. Create Blueprint based on Actor
2. Add JammingComponent
3. Configure jam strength and radius
4. Set enabled state
5. Place in level

## API Reference

### Core Classes

#### ADroneBase
Main drone pawn class with all components.

```cpp
// Get components
UDroneMovementComponent* GetDroneMovement();
UDroneBatteryComponent* GetDroneBattery();
UDroneVisionComponent* GetDroneVision();
UDroneMarkingComponent* GetDroneMarking();

// Configuration
void SetDroneConfig(UDroneConfig* NewConfig);
UDroneConfig* GetDroneConfig();
```

#### UDroneMovementComponent
Handles movement with client prediction and server reconciliation.

```cpp
// Movement
void SetMovementInput(FVector InInput);
void SetLookInput(FVector2D InInput);
void SetSpeedMode(EDroneSpeedMode NewMode);
FVector GetVelocity();
```

#### UDroneBatteryComponent
Manages battery with feature-based drain rates.

```cpp
// Battery
float GetBatteryLevel();
float GetBatteryPercent();
void StartRecharging();
void StopRecharging();

// Features
void SetFlashlightActive(bool bActive);
void SetVisionMode(EDroneVisionMode Mode);
```

#### UDroneVisionComponent
Manages vision modes with thermal detection.

```cpp
// Vision
void SetVisionMode(EDroneVisionMode NewMode);
void CycleVisionMode();
TArray<FThermalDetection> GetThermalDetections();
```

#### UDroneMarkingComponent
Handles target marking with networked timeouts.

```cpp
// Marking
void MarkTarget(AActor* Target);
void UnmarkTarget(AActor* Target);
TArray<AActor*> GetMarkedTargets();
```

### AI System

#### ADroneAIController
AI controller with data-driven behaviors.

```cpp
// Behavior
void SetBehaviorProfile(UDroneBehaviorProfile* NewProfile);
void SetBehaviorType(EDroneBehaviorType NewType);

// Patrol
void SetPatrolCenter(FVector Center);
void AddPatrolPoint(FVector Point);

// Follow
void SetFollowTarget(AActor* Target);
```

#### UDroneBehaviorProfile (DataAsset)
Configure AI behavior parameters.

```cpp
EDroneBehaviorType BehaviorType;
float PatrolRadius;
float FollowDistance;
float ScanRadius;
bool bAutoMarkEnemies;
```

### Environmental Systems

#### UJammingComponent
Affects drones in radius.

```cpp
void SetJammingEnabled(bool bEnabled);
void SetJamStrength(float Strength);
float GetJammingIntensityAtLocation(FVector Location);
```

#### UHackingComponent
Networked hacking system.

```cpp
bool StartHack(AActor* Target, float Duration);
void CancelHack();
float GetHackProgress();
```

#### UDroneDockingComponent
Recharge and recall system.

```cpp
bool DockDrone(ADroneBase* Drone);
void UndockDrone();
void RecallDrone(ADroneBase* Drone);
```

## Networking

### Client Prediction
The movement system implements client prediction with server reconciliation:
1. Client simulates movement locally
2. Client sends input to server
3. Server validates and simulates authoritatively
4. Server sends corrections when needed
5. Client reconciles and replays inputs

### Bandwidth Optimization
- Quantized floats for position/rotation
- Delta compression for state changes
- Distance-based relevancy
- Conditional replication based on change significance

### Security
- Server validates all inputs (speed, distance, state changes)
- Client requests validated on server
- No client-authoritative actions
- Hack progress calculated server-side

## Performance Guidelines

### Recommended Settings
- NetUpdateFrequency: 20Hz (configurable per DroneConfig)
- NetCullDistance: 15000 units
- RelevancyCheckInterval: 0.5 seconds
- Max Drones per Server: 50+ (depends on game complexity)

### Optimization Tips
1. Use DataAssets for configuration (no recompilation needed)
2. Adjust NetUpdateFrequency based on gameplay needs
3. Use relevancy distance to limit replication
4. Disable tick on inactive drones
5. Pool thermal detection queries

## Testing

### PIE Multiplayer Testing
1. Editor → Play → Net Mode: Listen Server
2. Number of Players: 3 (1 server + 2 clients)
3. Spawn drones on server and clients
4. Test movement, battery, marking, jamming

### Automation Tests
Run from Session Frontend or command line:
```
RunTests DroneSystemPro
```

Tests include:
- Battery drain and recharge
- Movement speed modes
- Marking replication
- Jamming effects
- Docking functionality

## Troubleshooting

### Common Issues

**Plugin not showing in editor**
- Verify .uplugin file exists
- Check UE version compatibility (5.6)
- Regenerate project files

**Compile errors**
- Ensure all dependencies in Build.cs
- Clean and rebuild solution
- Check UE 5.6 API compatibility

**Replication not working**
- Verify bReplicates = true on actors
- Check server/client roles
- Enable net logging: `log LogNet Verbose`

**Battery not draining**
- Ensure drone is active
- Check DroneConfig values
- Verify battery component is ticking

## Examples

The plugin includes example content:
- BP_ExampleDrone: Complete drone setup
- DA_DroneConfig_Default: Default configuration
- BP_DockingStation: Working docking station
- BP_JamField: Jamming field example
- BP_HackableTerminal: Terminal with hacking

### Example Level Setup
1. Place BP_ExampleDrone in level
2. Place BP_DockingStation nearby
3. Add BP_JamField for interference
4. Place BP_HackableTerminal for interaction
5. PIE with multiplayer to test

## Marketplace Submission

This plugin is ready for Unreal Marketplace submission:
- ✓ Compatible with UE 5.6
- ✓ Clean compile with no warnings
- ✓ Multiplayer tested and validated
- ✓ Example content included
- ✓ Complete documentation
- ✓ MIT License

See MARKETPLACE_CHECKLIST.md for submission guidelines.

## Support & Contributing

### Bug Reports
Please include:
- UE version
- Plugin version
- Reproduction steps
- Expected vs actual behavior
- Logs if applicable

### Feature Requests
Submit detailed descriptions of desired features with use cases.

## License

MIT License - See LICENSE file for details.

## Credits

DroneSystemPro - Professional Drone System Plugin for Unreal Engine 5.6

## Version History

See CHANGELOG.md for detailed version history.

---

**Version**: 1.0.0
**Engine**: Unreal Engine 5.6
**Last Updated**: 2025
