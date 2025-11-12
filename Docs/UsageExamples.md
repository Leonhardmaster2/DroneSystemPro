# DroneSystemPro - Usage Examples

This document provides practical examples of how to use the DroneSystemPro plugin in your project.

## Table of Contents
1. [Basic Setup](#basic-setup)
2. [Movement Input Examples](#movement-input-examples)
3. [Marking Enemies](#marking-enemies)
4. [Vision Modes](#vision-modes)
5. [HUD Integration](#hud-integration)
6. [Camera Effects](#camera-effects)
7. [AI Drone Setup](#ai-drone-setup)
8. [Multiplayer Examples](#multiplayer-examples)

---

## Basic Setup

### Creating a Drone in C++

```cpp
// In your GameMode or Player Controller

void AMyGameMode::SpawnDrone(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn drone
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    ADroneBase* Drone = World->SpawnActor<ADroneBase>(Location, FRotator::ZeroRotator, SpawnParams);

    if (Drone)
    {
        // Load and apply configuration
        UDroneConfig* Config = LoadObject<UDroneConfig>(nullptr,
            TEXT("/Game/DroneSystemPro/Configs/DA_DroneConfig_Default.DA_DroneConfig_Default"));

        if (Config)
        {
            Drone->SetDroneConfig(Config);
        }

        // Possess drone
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->Possess(Drone);
        }
    }
}
```

### Creating a Drone in Blueprint

1. Create Blueprint Class based on `DroneBase` (BP_MyDrone)
2. In the Blueprint:
   - Add/configure Static Mesh for drone body
   - Assign DroneConfig DataAsset
   - Configure camera settings
3. Place in level or spawn dynamically

---

## Movement Input Examples

### Method 1: Using Standard Input Functions (Recommended)

```cpp
// In your PlayerController or Pawn class

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind movement axes
    InputComponent->BindAxis("MoveForward", this, &AMyPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &AMyPlayerController::MoveRight);
    InputComponent->BindAxis("MoveUp", this, &AMyPlayerController::MoveUp);
}

void AMyPlayerController::MoveForward(float Value)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (Drone && Value != 0.0f)
    {
        // Use AddMovementInput like a Character
        FVector Forward = Drone->GetActorForwardVector();
        Drone->AddMovementInput(Forward, Value);
    }
}

void AMyPlayerController::MoveRight(float Value)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (Drone && Value != 0.0f)
    {
        FVector Right = Drone->GetActorRightVector();
        Drone->AddMovementInput(Right, Value);
    }
}

void AMyPlayerController::MoveUp(float Value)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (Drone && Value != 0.0f)
    {
        FVector Up = FVector::UpVector;
        Drone->AddMovementInput(Up, Value);
    }
}
```

### Method 2: Using Controller Input

```cpp
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Bind look axes
    InputComponent->BindAxis("LookUp", this, &AMyPlayerController::LookUp);
    InputComponent->BindAxis("Turn", this, &AMyPlayerController::Turn);
}

void AMyPlayerController::LookUp(float Value)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (Drone)
    {
        Drone->AddControllerPitchInput(Value);
    }
}

void AMyPlayerController::Turn(float Value)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (Drone)
    {
        Drone->AddControllerYawInput(Value);
    }
}
```

### Blueprint Example

```
Event Tick
  → Get Player Pawn → Cast to DroneBase
  → Get Input Axis Value "MoveForward"
  → Get Actor Forward Vector
  → Add Movement Input (World Direction, Scale Value)
```

---

## Marking Enemies

### C++ Example - Mark Target in Crosshair

```cpp
void AMyPlayerController::MarkTargetInCrosshair()
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneMarkingComponent* MarkingComp = Drone->GetDroneMarking();
    if (MarkingComp)
    {
        // Automatically finds and marks target in center of screen
        MarkingComp->MarkTargetInCrosshair();
    }
}

// Or mark specific actor
void AMyPlayerController::MarkSpecificActor(AActor* Target)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone || !Target) return;

    UDroneMarkingComponent* MarkingComp = Drone->GetDroneMarking();
    if (MarkingComp)
    {
        MarkingComp->MarkTarget(Target);
    }
}

// Get all marked targets
void AMyPlayerController::DisplayMarkedTargets()
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneMarkingComponent* MarkingComp = Drone->GetDroneMarking();
    if (MarkingComp)
    {
        TArray<AActor*> MarkedTargets = MarkingComp->GetMarkedTargets();
        for (AActor* Target : MarkedTargets)
        {
            UE_LOG(LogTemp, Log, TEXT("Marked: %s"), *Target->GetName());
        }
    }
}
```

### Blueprint Example

```
On "R" Key Pressed
  → Get Drone Marking Component
  → Mark Target In Crosshair
```

---

## Vision Modes

### C++ Example - Cycle Vision Modes

```cpp
void AMyPlayerController::CycleVisionMode()
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
    if (VisionComp)
    {
        VisionComp->CycleVisionMode();
    }
}

// Set specific vision mode
void AMyPlayerController::SetThermalVision()
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
    if (VisionComp)
    {
        VisionComp->SetVisionMode(EDroneVisionMode::Thermal);
    }
}

// Get thermal detections
void AMyPlayerController::ScanForHeatSignatures()
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneVisionComponent* VisionComp = Drone->GetDroneVision();
    if (VisionComp)
    {
        TArray<FThermalDetection> Detections = VisionComp->GetThermalDetections();
        for (const FThermalDetection& Detection : Detections)
        {
            UE_LOG(LogTemp, Log, TEXT("Heat source at: %s, Intensity: %f"),
                *Detection.Location.ToString(), Detection.HeatSignature);
        }
    }
}
```

---

## HUD Integration

### C++ Example - Create HUD Widget

```cpp
// In your PlayerController or HUD class

void AMyPlayerController::CreateDroneHUD()
{
    if (!DroneHUDWidgetClass) return;

    // Create widget
    DroneHUDWidget = CreateWidget<UDroneHUDWidget>(this, DroneHUDWidgetClass);
    if (DroneHUDWidget)
    {
        // Set drone reference
        ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
        DroneHUDWidget->SetDrone(Drone);

        // Add to viewport
        DroneHUDWidget->AddToViewport();
    }
}

// Update HUD elements
void AMyPlayerController::UpdateHUD()
{
    if (!DroneHUDWidget) return;

    // Get battery percentage
    float BatteryPercent = DroneHUDWidget->GetBatteryPercent();

    // Get vision mode
    FString VisionMode = DroneHUDWidget->GetVisionModeText();

    // Get marked target count
    int32 MarkedCount = DroneHUDWidget->GetMarkedTargetCount();

    // Display warning if battery low
    if (DroneHUDWidget->IsBatteryCritical())
    {
        // Show critical battery warning
    }
}
```

### Blueprint Example - HUD Widget

Create BP_DroneHUD widget (based on UDroneHUDWidget):

**Designer**:
```
- Canvas Panel
  - Text: Battery (Bind → GetBatteryPercent)
  - Text: Speed (Bind → GetSpeedText)
  - Text: Altitude (Bind → GetAltitudeText)
  - Text: Vision Mode (Bind → GetVisionModeText)
  - Overlay: Marked Targets
    - For Each (GetMarkedTargets)
      - Image: Target Indicator
```

**Event Graph**:
```
Event Construct
  → Get Owning Player Pawn
  → Cast to DroneBase
  → Set Drone

Event Tick
  → Get Battery Percent
  → If < 20%
    → Play Low Battery Animation
```

---

## Camera Effects

### C++ Example - Camera Shake

```cpp
// Play camera shake on collision
void ADroneBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (DroneCameraEffects)
    {
        float ImpactStrength = NormalImpulse.Size() / 1000.0f;
        DroneCameraEffects->PlayCollisionShake(ImpactStrength);
    }
}

// Speed-based FOV
void UpdateSpeedFOV()
{
    if (!DroneCameraEffects || !DroneMovement) return;

    float CurrentSpeed = DroneMovement->GetCurrentSpeed();
    float MaxSpeed = DroneConfig ? DroneConfig->MaxSpeedHigh : 1200.0f;

    // FOV increases with speed
    DroneCameraEffects->ApplySpeedFOV(CurrentSpeed, MaxSpeed);
}

// Manual FOV control
void SetCustomFOV(float NewFOV)
{
    if (DroneCameraEffects)
    {
        DroneCameraEffects->SetFOVOverride(NewFOV, 0.5f); // 0.5s blend time
    }
}
```

---

## AI Drone Setup

### C++ Example - Spawn AI Drone

```cpp
void AMyGameMode::SpawnAIDrone(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn drone
    ADroneBase* Drone = World->SpawnActor<ADroneBase>(DroneClass, Location, FRotator::ZeroRotator);

    // Spawn AI controller
    ADroneAIController* AI = World->SpawnActor<ADroneAIController>();

    if (Drone && AI)
    {
        // Load behavior profile
        UDroneBehaviorProfile* BehaviorProfile = LoadObject<UDroneBehaviorProfile>(nullptr,
            TEXT("/Game/DroneSystemPro/AI/DA_Behavior_Patrol.DA_Behavior_Patrol"));

        if (BehaviorProfile)
        {
            AI->SetBehaviorProfile(BehaviorProfile);
        }

        // Possess drone
        AI->Possess(Drone);

        // Set patrol points
        AI->AddPatrolPoint(Location + FVector(1000, 0, 0));
        AI->AddPatrolPoint(Location + FVector(1000, 1000, 0));
        AI->AddPatrolPoint(Location + FVector(0, 1000, 0));
        AI->AddPatrolPoint(Location);
    }
}

// Change AI behavior dynamically
void SwitchToAttackMode(ADroneAIController* AI)
{
    if (AI)
    {
        AI->SetBehaviorType(EDroneBehaviorType::AttackMark);
    }
}
```

---

## Multiplayer Examples

### Server-Side Drone Management

```cpp
// Server spawns drone for client
void AMyGameMode::SpawnDroneForPlayer_Server(APlayerController* PC)
{
    if (!HasAuthority()) return;

    FVector SpawnLocation = GetPlayerStartLocation(PC);
    ADroneBase* Drone = GetWorld()->SpawnActor<ADroneBase>(DroneClass, SpawnLocation, FRotator::ZeroRotator);

    if (Drone && PC)
    {
        PC->Possess(Drone);
    }
}

// Client requests to mark target
void AMyPlayerController::RequestMarkTarget_Client(AActor* Target)
{
    ADroneBase* Drone = Cast<ADroneBase>(GetPawn());
    if (!Drone) return;

    UDroneMarkingComponent* MarkingComp = Drone->GetDroneMarking();
    if (MarkingComp)
    {
        // This automatically sends RPC to server
        MarkingComp->MarkTarget(Target);
    }
}
```

### Testing Multiplayer in PIE

```cpp
// In Editor:
// 1. Play → Net Mode: Listen Server
// 2. Number of Players: 3 (1 server + 2 clients)

// Spawn drones for all players
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // Spawn drones for each player
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (PC)
            {
                SpawnDroneForPlayer_Server(PC);
            }
        }
    }
}
```

---

## Advanced Examples

### Custom Battery Drain

```cpp
// Add custom battery drain for a feature
void AMyDrone::ActivateSpecialAbility()
{
    UDroneBatteryComponent* Battery = GetDroneBattery();
    if (!Battery) return;

    if (Battery->GetBatteryLevel() >= 20.0f)
    {
        // Drain 20% battery
        Battery->SetBatteryLevel(Battery->GetBatteryLevel() - 20.0f);

        // Activate ability
        // ...
    }
}
```

### Jamming Field Setup

```cpp
// Create jamming field actor
AActor* CreateJammingField(FVector Location, float Radius)
{
    AActor* JammerActor = GetWorld()->SpawnActor<AActor>(Location, FRotator::ZeroRotator);

    UJammingComponent* Jammer = NewObject<UJammingComponent>(JammerActor);
    Jammer->RegisterComponent();

    Jammer->SetJammingEnabled(true);
    Jammer->SetJamRadius(Radius);
    Jammer->SetJamStrength(1.0f);

    return JammerActor;
}
```

### Hacking Terminal

```cpp
// Player attempts to hack terminal
void AMyCharacter::AttemptHack(AActor* TerminalActor)
{
    UHackingComponent* HackingComp = FindComponentByClass<UHackingComponent>();
    if (!HackingComp) return;

    // Start hacking
    bool bStarted = HackingComp->StartHack(TerminalActor, 5.0f); // 5 second hack

    if (bStarted)
    {
        // Bind to completion event
        HackingComp->OnHackCompleted.AddDynamic(this, &AMyCharacter::OnHackSuccess);
    }
}

void AMyCharacter::OnHackSuccess(AActor* Hacker, AActor* Target)
{
    // Terminal hacked!
    UE_LOG(LogTemp, Log, TEXT("Successfully hacked: %s"), *Target->GetName());
}
```

---

## Input Mapping Reference

Add these to your project's input settings:

**Axis Mappings**:
```
MoveForward: W (1.0), S (-1.0), Gamepad Left Thumbstick Y
MoveRight: D (1.0), A (-1.0), Gamepad Left Thumbstick X
MoveUp: E (1.0), Q (-1.0), Gamepad Right/Left Shoulder
LookUp: Mouse Y, Gamepad Right Thumbstick Y
Turn: Mouse X, Gamepad Right Thumbstick X
```

**Action Mappings**:
```
ToggleSpeed: Left Shift, Gamepad Left Thumbstick
ToggleFlashlight: F, Gamepad D-Pad Down
ToggleVision: V, Gamepad D-Pad Up
Mark: R, Gamepad Face Button Right (B/Circle)
```

---

## Performance Tips

1. **Limit Marked Targets**: Set reasonable mark duration (10-30 seconds)
2. **Thermal Scan Interval**: Adjust scan interval based on needs (0.5-2.0s)
3. **Camera Shake**: Disable or reduce intensity for performance
4. **Post-Process Materials**: Use LOD or distance-based activation
5. **Network Bandwidth**: Configure replication rate per game needs

---

## Debugging

### Enable Drone Debug Logging

```cpp
// In your project's log categories
DECLARE_LOG_CATEGORY_EXTERN(LogDroneSystem, Log, All);

// Use in code
UE_LOG(LogDroneSystem, Log, TEXT("Battery: %f%%"), BatteryPercent);
```

### Console Commands

```
// Show network stats
stat net

// Show FPS
stat fps

// Show drone debug
showdebug drone (if implemented)

// Toggle custom depth visualization
r.CustomDepth.Order 1
```

---

This usage guide covers the main features of DroneSystemPro. For more details, see the API reference in README.md.
