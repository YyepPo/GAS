# Combat Lock-On System in Unreal Engine 5 (C++)

## 1. System Overview

A combat lock-on system is a targeting mode that lets the player bind camera control, facing logic, movement behavior, and UI feedback to a selected enemy. In a third-person melee game, lock-on exists to improve combat readability and reduce camera ambiguity during close-range encounters.

The system described here follows this input model:

- `Tab` pressed while unlocked -> acquire a target
- `Tab` pressed while locked -> unlock
- `Caps Lock` pressed while locked -> switch to another valid target

The main design goals are:

- Keep the camera pointed at the active enemy without hard snapping.
- Preserve player control while making target focus explicit.
- Make melee spacing easier by converting movement into strafe-relative motion.
- Ensure target selection is predictable by using screen-space scoring instead of arbitrary overlap order.

In your project, the current implementation already has the core building blocks:

- `UCombatLockOnComponent`
- `AGAS_AuroraCharacter` input integration
- spring arm and camera references
- screen-space target selection
- death-event listening through `ICombatInterface`

That makes the system a strong foundation. The main remaining work is tightening validation, switching direction logic, camera stability, and UI presentation.

## 2. Architecture

### Recommended class ownership

Use the following ownership model:

- `UCombatLockOnComponent`
  - Owns target detection, validation, acquisition, switching, loss handling, and lock state.
- `AGAS_AuroraCharacter`
  - Owns input binding and movement remapping while locked.
- `USpringArmComponent` and `UCameraComponent`
  - Provide camera pivot, view direction, lock offset behavior, and smooth interpolation.
- `APlayerController`
  - Projects world positions into screen space through `ProjectWorldLocationToScreen(...)`.
- UI widget or HUD layer
  - Renders the lock marker and optional target highlight.

### Component responsibilities

`UCombatLockOnComponent` should be responsible for:

- Scanning for nearby candidates.
- Filtering invalid targets.
- Scoring candidates.
- Setting and clearing `CurrentTarget`.
- Broadcasting target updates.
- Updating camera-facing behavior while locked.
- Reacquiring or exiting lock when a target becomes invalid.

`AGAS_AuroraCharacter` should be responsible for:

- Binding `LockAction` and `SwitchLockAction`.
- Routing `Tab` to lock/unlock.
- Routing `Caps Lock` to switch target.
- Remapping movement input during lock-on.
- Disabling free-look semantics while lock-on is active.

### Suggested component state

A clean `ULockOnComponent` or `UCombatLockOnComponent` typically stores:

```cpp
UPROPERTY()
TObjectPtr<AActor> CurrentTarget;

UPROPERTY()
TArray<TObjectPtr<AActor>> CachedCandidates;

UPROPERTY(EditAnywhere, Category="LockOn")
float MaxLockDistance = 1500.f;

UPROPERTY(EditAnywhere, Category="LockOn")
float TargetRefreshInterval = 0.05f;

UPROPERTY(EditAnywhere, Category="LockOn")
bool bRequireLineOfSight = true;

UPROPERTY(EditAnywhere, Category="LockOn")
float BreakLockDotThreshold = 0.0f;

UPROPERTY(EditAnywhere, Category="LockOn")
FVector SpringArmLockOffset = FVector(0.f, 60.f, 20.f);

UPROPERTY(EditAnywhere, Category="LockOn")
float CameraRotationInterpSpeed = 10.f;
```

Your current component already stores:

- `CurrentTarget`
- `CachedCamera`
- `CachedSpringArm`
- `SpringArmOffsetOnLock`
- interpolation speeds
- pre-lock spring arm state

That is the correct general direction.

## 3. Target Detection Algorithm

The target detection flow should always be deterministic and score-based.

### Step 1: Gather nearby enemies

Use a sphere overlap around the player:

```cpp
UKismetSystemLibrary::SphereOverlapActors(
    GetWorld(),
    OwnerLocation,
    MaxLockDistance,
    ObjectTypes,
    ActorClassFilter,
    ActorsToIgnore,
    OverlappedActors
);
```

Your current implementation already does this in `GetLockableTargets()`.

### Step 2: Filter valid targets

Each candidate should pass the following checks:

- Not null
- Not the owner
- Implements `ICombatInterface` or another targetable interface
- `ICombatInterface::Execute_IsDead(Target) == false`
- Within max distance
- In front of the camera or at least inside a configurable view cone
- Visible if line-of-sight is required

Recommended validation helper:

```cpp
bool UCombatLockOnComponent::IsValidLockTarget(AActor* Target) const
{
    if (!IsValid(Target) || Target == GetOwner())
    {
        return false;
    }

    if (!Target->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
    {
        return false;
    }

    if (ICombatInterface::Execute_IsDead(Target))
    {
        return false;
    }

    const float DistanceSq =
        FVector::DistSquared(Target->GetActorLocation(), GetOwner()->GetActorLocation());

    if (DistanceSq > FMath::Square(MaxLockDistance))
    {
        return false;
    }

    if (!IsTargetInViewCone(Target))
    {
        return false;
    }

    if (bRequireLineOfSight && !HasLineOfSightToTarget(Target))
    {
        return false;
    }

    return true;
}
```

### Step 3: Project world position to screen space

For every valid candidate, project a representative point into screen space:

```cpp
FVector TargetPoint = Target->GetActorLocation();
TargetPoint.Z += CapsuleHalfHeight * 0.5f;

FVector2D ScreenPos;
const bool bProjected =
    PlayerController->ProjectWorldLocationToScreen(TargetPoint, ScreenPos, true);
```

Prefer projecting the enemy capsule center or upper torso point, not raw actor origin, because the actor root may be near the feet.

### Step 4: Compute distance from screen center

Lock acquisition should prefer the target nearest the center of the viewport:

```cpp
int32 ViewportX, ViewportY;
PlayerController->GetViewportSize(ViewportX, ViewportY);

const FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);
const float ScreenDistSq = FVector2D::DistSquared(ScreenPos, ScreenCenter);
```

Choose the lowest score.

### Acquisition pseudo-code

```cpp
AcquireTarget():
    Candidates = GatherNearbyActors()
    BestTarget = nullptr
    BestScore = FLT_MAX

    for Candidate in Candidates:
        if !IsValidLockTarget(Candidate):
            continue

        if !ProjectToScreen(Candidate, ScreenPos):
            continue

        Score = DistSquared(ScreenPos, ScreenCenter)

        if Score < BestScore:
            BestScore = Score
            BestTarget = Candidate

    return BestTarget
```

### Notes on your current implementation

Your current `SelectTargetClosestToMiddleOfTheScreen(...)` already follows the correct center-of-screen scoring model. The most important missing improvement is explicit dead-state filtering inside `GetLockableTargets()`.

## 4. Target Switching Logic

Switching targets is different from initial acquisition. Initial acquisition wants "closest to center". Switching wants "closest valid alternative in the requested screen direction".

Since your input model uses a single `Caps Lock` button rather than separate left/right inputs, you have two viable designs:

- Cycle-style switching: pick the next best candidate excluding the current target.
- Contextual directional switching: infer direction from the target layout in screen space.

### Recommended switching model for a single key

For a single switch key, a practical approach is:

1. Gather all valid targets.
2. Exclude the current target.
3. Project all remaining targets to screen space.
4. Compute horizontal offset relative to the current target.
5. Prefer the smallest positive horizontal offset first.
6. If none exist on that side, wrap to the opposite side.

This produces a stable cycle around the screen.

### Left/right switching variant

If later you split switching into left and right inputs, then use:

- Left switch: choose the closest valid target with `TargetScreenX < CurrentScreenX`
- Right switch: choose the closest valid target with `TargetScreenX > CurrentScreenX`

### Pseudo-code

```cpp
SwitchTarget():
    if CurrentTarget == nullptr:
        return

    Candidates = GatherValidTargets()
    Remove CurrentTarget

    CurrentScreenPos = Project(CurrentTarget)

    RightSideCandidates = []
    LeftSideCandidates = []

    for Candidate in Candidates:
        CandidateScreenPos = Project(Candidate)
        DeltaX = CandidateScreenPos.X - CurrentScreenPos.X

        if DeltaX > 0:
            RightSideCandidates.Add(Candidate, Abs(DeltaX))
        else if DeltaX < 0:
            LeftSideCandidates.Add(Candidate, Abs(DeltaX))

    if RightSideCandidates not empty:
        return candidate with smallest Abs(DeltaX)

    if LeftSideCandidates not empty:
        return candidate with smallest Abs(DeltaX)

    return nullptr
```

### Stronger score for switching

Horizontal distance alone is often not enough. A better switch score is:

```cpp
Score = Abs(DeltaX) + VerticalPenalty + DistancePenalty
```

Example:

```cpp
float Score =
    FMath::Abs(DeltaX) +
    FMath::Abs(DeltaY) * 0.35f +
    WorldDistance * 0.0025f;
```

This reduces bad switches to targets that are technically closest in X but far above, below, or very distant.

### Notes on your current implementation

Your current `SwitchTarget()` chooses the candidate with the smallest absolute X offset from the current target. That is a useful first pass, but it does not:

- distinguish left vs right
- avoid ambiguous overlaps well
- include dead filtering explicitly
- handle wrap order in a deliberate way

## 5. Target Validation System

Lock-on must continuously validate the active target. A target that was valid one frame ago may no longer be valid because of movement, death, camera angle, or obstruction.

### Required validation conditions

The current target should remain locked only if:

- it still exists
- it is alive
- it is still within `MaxLockDistance`
- it is still inside the allowed field of view
- it still passes line-of-sight checks if LOS is required

### Continuous validation strategy

You have two common choices:

- Validate every tick
- Validate on a short timer, such as every `0.03` to `0.1` seconds

For lock-on, every tick is usually fine because the validation itself is lightweight if the candidate set is small. If you expect many enemies, using an interval timer for candidate refresh can be more scalable.

### Suggested update loop

```cpp
void UCombatLockOnComponent::TickComponent(...)
{
    Super::TickComponent(...);

    if (!bLockStarted)
    {
        return;
    }

    if (!IsValidLockTarget(CurrentTarget))
    {
        TryReacquireOrUnlock();
        return;
    }

    UpdateCameraAndFacing(DeltaTime);
}
```

### Target loss behavior

When a target becomes invalid:

1. Search for a replacement using the normal acquisition logic.
2. Prefer candidates near the screen center.
3. If no valid target exists, stop lock-on cleanly.

### Death handling

Your current code already listens to `ICombatInterface::GetOnDeathDelegate()`. That is a good optimization because death is event-driven rather than poll-driven.

However, death should not be the only invalidation path. Range, visibility, and FOV loss should also trigger reacquisition or unlock.

## 6. Camera System

The camera is the most important part of how lock-on feels. A technically correct target system can still feel bad if the camera snaps, jitters, or over-rotates.

### Desired camera behavior while locked

While locked on:

- The camera should rotate to keep both player and target compositionally readable.
- The spring arm should shift sideways for an over-the-shoulder combat framing.
- The camera should slightly tilt upward if needed to keep target upper body visible.
- Rotation and offset changes should be interpolated, not snapped.

### Look-at rotation

Compute a look-at vector from the player or spring arm pivot to the target:

```cpp
const FVector ToTarget = TargetLocation - OwnerLocation;
const FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToTarget).Rotator();
```

Your current implementation rotates the spring arm toward the target each tick using `RInterpTo`, which is correct:

```cpp
FRotator NewRot =
    FMath::RInterpTo(CurrentRot, LookAtRotation, DeltaTime, SpringRotateInterpSpeed);
```

### Spring arm offset

To create the combat framing:

- move the spring arm slightly to one side
- optionally add a small upward Z offset

Example:

```cpp
FVector TargetOffset = FVector(0.f, 65.f, 20.f);
FVector NewOffset =
    FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, SpringOffsetInterpSpeed);
```

Your component already does this with `SpringArmOffsetOnLock`.

### Character facing

While locked, the character should rotate to face the target:

```cpp
FRotator CurrentActorRot = GetOwner()->GetActorRotation();
FRotator DesiredActorRot = FMath::RInterpTo(
    CurrentActorRot,
    LookAtRotation,
    DeltaTime,
    MeshRotateInterpSpeed
);

GetOwner()->SetActorRotation(FRotator(0.f, DesiredActorRot.Yaw, 0.f));
```

That is already close to what your current code does.

### Avoiding jitter

Jitter typically comes from one of these:

- rotating both controller and spring arm against each other
- using unstable target points near the feet
- line-of-sight checks rapidly toggling on geometry edges
- target point noise from animation-driven root motion

To reduce jitter:

- disable `bUsePawnControlRotation` while locked
- avoid applying both free-look input and forced spring arm rotation simultaneously
- target a stable point such as capsule center or chest socket
- keep interpolation speeds high enough to feel responsive but not instantaneous
- use hysteresis before breaking lock on FOV/LOS loss

### Recommended camera ownership

A clean model is:

- Controller free-look active when unlocked
- Spring arm follows controller when unlocked
- Spring arm driven by lock-on component when locked
- Controller look input either ignored or heavily reduced while locked

## 7. Player Movement Changes

When unlocked, third-person movement is usually camera-relative. When locked, movement should become target-relative.

### Strafe behavior

While locked:

- forward input moves toward or away from the target
- right input circles around the target
- character faces the target instead of facing movement direction

### Movement basis vectors

Use the target direction as the local forward axis:

```cpp
FVector ForwardAxis = CurrentTarget->GetActorLocation() - GetActorLocation();
ForwardAxis.Z = 0.f;
ForwardAxis.Normalize();

FVector RightAxis = FVector::CrossProduct(FVector::UpVector, ForwardAxis);
```

Then remap input:

```cpp
AddMovementInput(ForwardAxis, InputY);
AddMovementInput(RightAxis, InputX);
```

Your `AGAS_AuroraCharacter::Move(...)` already does this. That is the correct movement model for melee lock-on.

### Character orientation settings

While locked:

- disable movement-facing rotation
- disable controller-desired rotation if the camera is being forced by the component
- rotate actor toward the target manually

Typical settings:

```cpp
CharacterMovement->bOrientRotationToMovement = false;
CharacterMovement->bUseControllerDesiredRotation = false;
SpringArm->bUsePawnControlRotation = false;
```

When unlocking, restore the normal free movement settings.

### Look input while locked

Because free-look conflicts with forced target framing, you usually want one of these:

- ignore look input while locked
- only allow small pitch adjustment
- blend manual input with lock direction

For a Souls-like lock-on feel, disabling horizontal free-look while locked is usually the cleanest option.

## 8. UI System

The UI layer should make the active target obvious without cluttering the screen.

### Lock-on widget

The most common setup is a screen-space reticle placed over the target. Each frame:

1. Choose a target anchor point.
2. Project it to screen space.
3. Move the widget to that screen position.
4. Hide it if projection fails or lock ends.

### Anchor point

Do not place the marker at `GetActorLocation()` unless that location is already centered visually. Better anchors are:

- capsule center
- chest socket
- head socket with a small downward offset

Example:

```cpp
UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent();
FVector Anchor = Target->GetActorLocation();
Anchor.Z += Capsule->GetScaledCapsuleHalfHeight();
```

### Screen positioning

```cpp
FVector2D ScreenPos;
if (PlayerController->ProjectWorldLocationToScreen(Anchor, ScreenPos, true))
{
    LockWidget->SetPositionInViewport(ScreenPos, true);
    LockWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}
else
{
    LockWidget->SetVisibility(ESlateVisibility::Hidden);
}
```

### Widget ownership

A good pattern is:

- `UCombatLockOnComponent` broadcasts `OnLockTargetUpdated`
- Player HUD or widget controller listens
- UI creates, updates, or hides the lock marker

That keeps UI logic out of the gameplay component.

### Optional cosmetic additions

- Highlight material on the current target
- Crosshair morphing while locked
- Health bar emphasis for the locked enemy
- Small on-target pulse when switching

## 9. Edge Cases

### Target dies mid-combat

This should be event-driven when possible. Your current use of `GetOnDeathDelegate()` is correct. On death:

- unbind old target
- reacquire another valid target if possible
- otherwise unlock

### Target goes out of range

This is currently not handled continuously in your component and should be added. If the target exceeds `MaxLockDistance`, either:

- reacquire another nearby target
- or unlock if none qualify

### Target leaves field of view

Breaking immediately on a single bad frame can feel harsh. Prefer hysteresis:

- allow brief FOV loss
- break only after a short timeout or repeated failed checks

### Multiple targets overlap on screen

When enemies overlap, screen-center scoring alone can become ambiguous. Solve this with weighted scoring:

- center distance
- world distance
- LOS status
- horizontal separation from current target

### Rapid switch spam

Without guarding input, very fast repeated switching can feel unstable. Add a short cooldown:

```cpp
UPROPERTY(EditAnywhere)
float SwitchCooldown = 0.15f;
```

Track `LastSwitchTime` and ignore switch input until cooldown expires.

### Projection failures

If `ProjectWorldLocationToScreen(...)` fails, do not treat the target as valid for acquisition or switching. A failed projection usually means the target is behind the camera or otherwise outside a usable view state.

### Network considerations

For multiplayer, lock-on is mostly a local camera/input feature, but it can still affect gameplay.

Recommended authority model:

- local client owns lock state for camera and UI
- server validates gameplay actions that depend on the target
- target actor references used by abilities or attacks should be confirmed server-side

If lock-on affects attack direction, aim assistance, or gameplay ability targeting, do not trust client target selection blindly.

## 10. Pros and Cons of This Approach

### Pros

- Very readable for close-range melee combat.
- Predictable target acquisition through screen-center scoring.
- Good player control once movement is converted to strafing.
- Camera framing becomes more cinematic and combat-focused.
- Works naturally with animation-driven melee systems.

### Cons

- More camera complexity than free-look systems.
- Requires careful handling to avoid jitter and motion sickness.
- Target switching can feel inconsistent if scoring is too simplistic.
- UI and gameplay can diverge if lock state is not validated continuously.
- Multiplayer gameplay interactions need server validation.

## 11. Optional Improvements

### Soft lock vs hard lock

Hard lock:

- Camera is strongly attached to the target.
- Player orientation is forced.
- Best for one-on-one melee.

Soft lock:

- Target is prioritized, but camera remains more free.
- Aim assist and attack targeting bias toward the selected enemy.
- Better for faster action games with many targets.

### Aim assist

Even without full lock-on, you can bias:

- attack traces
- camera yaw
- movement-facing rotation

toward nearby enemies in front of the player.

### Target priority weighting

Instead of using only screen-center distance, build a weighted score:

```cpp
Score =
    ScreenCenterDistanceSq * 1.0f +
    WorldDistanceSq * 0.0005f +
    AnglePenalty * 5000.f +
    OcclusionPenalty;
```

Where:

- `ScreenCenterDistanceSq` rewards what the player is looking at
- `WorldDistanceSq` prefers closer targets
- `AnglePenalty` discourages targets near the edge of the cone
- `OcclusionPenalty` heavily penalizes blocked targets

### Distance + angle scoring system

A robust target acquisition score often combines:

```cpp
float ScreenScore = FVector2D::DistSquared(ScreenPos, ScreenCenter);
float DistanceScore = FVector::DistSquared(TargetLocation, OwnerLocation) * 0.001f;
float DotScore = (1.f - DotToCameraForward) * 10000.f;

float FinalScore = ScreenScore + DistanceScore + DotScore;
```

This usually feels better than using any one metric alone.

### Better target anchor points

Use sockets if available:

- `"spine_03"`
- `"head"`
- `"lockon"`

A dedicated `"lockon"` socket gives designers precise control over where the reticle appears.

### Separate validation cadence from camera cadence

Do camera interpolation every tick, but refresh candidate lists on a short interval such as `0.05s`. That reduces overlap and trace work while keeping the camera smooth.

## Recommended Implementation Notes for Your Current Codebase

Your existing `UCombatLockOnComponent` is already close to a production-friendly structure. The most important upgrades are:

1. Add explicit `IsDead()` filtering inside `GetLockableTargets()`.
2. Continuously validate `CurrentTarget` for range, FOV, and LOS loss while locked.
3. Prevent free-look camera input from fighting spring arm lock rotation.
4. Improve `SwitchTarget()` scoring to support directional or wrap-based switching.
5. Use capsule center or a socket instead of raw actor origin for both projection and UI.
6. Add a small switch cooldown.
7. Move UI widget positioning to a dedicated HUD or widget controller listener.

## Summary

An Unreal Engine 5 lock-on system is fundamentally a coordination system between target acquisition, camera control, movement remapping, and UI. The best results come from treating those as one cohesive mode rather than isolated features.

The recommended flow is:

1. Detect nearby enemies with sphere overlap.
2. Filter by alive state, distance, FOV, and optionally LOS.
3. Project candidates to screen space.
4. Acquire the target closest to the screen center.
5. While locked, rotate camera and character smoothly toward the target.
6. Convert movement into target-relative strafing.
7. Continuously validate the current target.
8. Reacquire or unlock when the target becomes invalid.

Your current project already implements much of this structure. The next step is refining the validation and switching rules so the system feels stable, readable, and deliberate in combat.
