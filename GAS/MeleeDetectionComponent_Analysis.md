# MeleeDetectionComponent Technical Analysis

## 1. System Overview

`UMeleeDetectionComponent` is a tick-driven melee hit detection component for Unreal Engine 5. Its purpose is to detect weapon hits reliably during an attack window, especially when the weapon moves too fast for simple per-frame overlap checks.

Instead of checking only the weapon's current position, the component stores the previous frame's socket positions and compares them against the current frame's socket positions. It then performs collision sweeps across that movement path. This is a form of continuous melee collision detection.

In this implementation, the trace is activated with `StartTrace(...)` and deactivated with `StopTrace()`. While active, `TickComponent(...)` calls `DoTrace()` every frame.

The system currently uses a **capsule sweep** through `SweepMultiByChannel(...)`, but the same design pattern could be adapted to use sphere sweeps or line traces depending on the weapon type.

### High-level flow

1. Cache the previous socket positions when tracing starts.
2. Read the current socket positions each frame.
3. Interpolate several sub-steps between the previous and current positions.
4. Sweep a collision shape through each sub-step.
5. Collect hits and ignore duplicate actors during the same swing.

### Why previous and current positions matter

If a sword tip moves a large distance in one frame, a target can exist between the previous position and the current position without ever overlapping either endpoint. A sweep between those positions covers the actual traveled path and reduces missed collisions.

## 2. Core Algorithm Breakdown

### Activation

When `StartTrace(...)` is called, the component:

1. Sets `bStartTrace = true`.
2. Stores the socket names for the weapon segment.
3. Stores the capsule radius and half-height.
4. Resolves the owner's skeletal mesh.
5. Captures the initial socket positions into:
   - `PreviousStartLocation`
   - `PreviousEndLocation`

This initial snapshot is necessary so the first tick has a valid "previous state".

### Per-frame detection

Each frame while tracing is active:

1. `TickComponent(...)` calls `DoTrace()`.
2. `DoTrace()` validates the world, owner, and mesh.
3. It reads the current socket positions:
   - `CurrentStart`
   - `CurrentEnd`
4. It creates a capsule collision shape with:
   - `FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight)`
5. It builds collision query params and ignores the owner.
6. It generates **4 interpolated sub-steps** between the previous and current positions.
7. It calls `SweepMultiByChannel(...)` for each sub-step.
8. It filters valid hits and suppresses repeat hits against the same actor.
9. It updates the previous positions to the current positions.

### How previous positions are stored

The component stores previous positions in two members:

```cpp
FVector PreviousStartLocation;
FVector PreviousEndLocation;
```

These represent the traced weapon segment from the last frame.

### How current positions are calculated

Current positions come directly from skeletal mesh sockets:

```cpp
const FVector CurrentStart = Mesh->GetSocketLocation(TraceStartSocket);
const FVector CurrentEnd = Mesh->GetSocketLocation(TraceEndSocket);
```

This makes the trace follow the actual animation pose rather than a detached timer or overlap volume.

### How sub-steps are generated

The current implementation uses fixed interpolation:

```cpp
for (int i = 0; i < 4; ++i)
{
    const float Alpha = (i + 1) / static_cast<float>(4);
    const FVector StepStart = FMath::Lerp(PreviousStartLocation, CurrentStart, Alpha);
    const FVector StepEnd = FMath::Lerp(PreviousEndLocation, CurrentEnd, Alpha);
}
```

This divides one frame of movement into four smaller trace segments.

### Pseudo-code

```cpp
StartTrace():
    bTracing = true
    PreviousStart = Mesh.GetSocketLocation(StartSocket)
    PreviousEnd = Mesh.GetSocketLocation(EndSocket)

Tick():
    if bTracing:
        DoTrace()

DoTrace():
    CurrentStart = Mesh.GetSocketLocation(StartSocket)
    CurrentEnd = Mesh.GetSocketLocation(EndSocket)

    for substep in 1..4:
        alpha = substep / 4.0
        StepStart = Lerp(PreviousStart, CurrentStart, alpha)
        StepEnd = Lerp(PreviousEnd, CurrentEnd, alpha)

        Hits = SweepMultiByChannel(StepStart, StepEnd, CapsuleShape)

        for hit in Hits:
            if actor is valid and not already hit:
                record actor
                broadcast hit event

    PreviousStart = CurrentStart
    PreviousEnd = CurrentEnd

StopTrace():
    bTracing = false
    clear hit actor set
```

## 3. Sub-Stepping Explained

Sub-stepping in this system means dividing the weapon's movement during a single frame into smaller interpolated segments and tracing each one separately.

Without sub-stepping, the component could perform only one sweep based on the full frame's movement. That is often not enough for fast swings, low frame rates, or narrow targets.

### Why sub-stepping is needed

- Fast weapons can travel a large distance between two frames.
- Low FPS increases the distance traveled per frame.
- A single trace may miss contact that happens between two sampled states.
- Rotating weapons often sweep curved paths that are only approximated by one coarse check.

### What this implementation does

This component uses 4 sub-steps per frame:

- 25% of the motion
- 50% of the motion
- 75% of the motion
- 100% of the motion

That increases temporal sampling density inside the frame.

### Difference from standard per-frame detection

A standard per-frame approach usually does one of the following:

- one overlap at the current socket position
- one trace from one current pose to another current pose
- one timer callback every fixed interval

This sub-stepped approach instead tracks motion continuity across time and queries the intermediate path, not just the endpoints.

## 4. Why Sub-Stepping Is Better Than Timer-Based Detection

Timer-based melee detection usually uses an `FTimerHandle` to run traces at a fixed interval. That can work for simple systems, but it has real limitations.

### Problems with timer-based detection

- The timer may run less often than the frame rate.
- Large gaps can exist between timer callbacks.
- High-speed weapons can move through thin targets between checks.
- Timer frequency may not match animation peaks or hit windows accurately.
- Detection quality becomes heavily dependent on the chosen timer interval.

### Why this sub-stepped tick approach is better

- It stays synchronized with the animation-driven socket transforms.
- It traces every frame while the hit window is open.
- It further subdivides the per-frame movement into smaller segments.
- It reduces the spatial gap between collision queries.

### Practical result

Compared with timer-driven checks, this approach is usually:

- more accurate
- more consistent
- less sensitive to animation speed
- less likely to miss fast impacts

### Important nuance

This is still a tick-based system, not a perfect continuous solver. If frame time becomes extremely large, four sub-steps may still be insufficient. Even so, it is far more robust than a lower-frequency timer approach.

## 5. Pros and Cons

### Pros

- High accuracy for moving melee weapons.
- Better resistance to tunneling than single-frame or timer-based detection.
- Cleaner alignment with attack windows driven by anim notifies or abilities.
- One-hit-per-swing behavior through `HitActors`.
- Good fit for animation-driven combat systems.

### Cons

- More sweeps per frame means higher cost.
- More implementation complexity than overlaps or timers.
- Requires tuning of radius, half-height, collision channel, and sub-step count.
- Fixed 4-step subdivision is not adaptive to weapon speed or frame time.

## 6. Edge Cases and Considerations

### Multiple hits on the same target

The component prevents duplicate hits during one active trace session:

```cpp
if (HitActors.Contains(HitActor))
{
    continue;
}
HitActors.Add(HitActor);
```

This is the right behavior for most melee swings. `StopTrace()` clears the set so the next attack can hit the same actor again.

### Preventing duplicate damage

Because the component performs multiple sweeps per frame, duplicate damage prevention is essential. The `HitActors` set ensures the same actor is only processed once per active hit window.

### Multiplayer considerations

For multiplayer, the main question is where authoritative tracing happens.

- Server-authoritative tracing is the safest model.
- Client-side tracing can improve responsiveness, but damage should still be validated by the server.
- Socket-driven traces depend on animation state, so server and client animation playback must remain reasonably synchronized.
- Hit windows triggered by notifies or gameplay events must align across network authority boundaries.

In a GAS-based setup, a common pattern is:

1. Client predicts the attack and montage.
2. Server runs the authoritative melee trace.
3. Server applies the gameplay effect or damage.
4. Clients receive the replicated result.

### Performance tuning

Useful optimization levers include:

- limiting sub-step count
- tracing only during short attack windows
- using narrow collision channels
- using the cheapest acceptable trace shape
- ignoring the owner and irrelevant actors aggressively
- switching to single-hit queries if multi-hit results are unnecessary

The current component traces on `ECC_GameTraceChannel1`, which is a good design choice if that channel is dedicated to melee-relevant targets.

### Important implementation caveats in the current code

#### Capsule orientation is fixed

The sweep currently uses `FQuat::Identity`, so the capsule is not rotated to match the weapon direction. For a capsule sweep, that matters. A capsule aligned to the weapon segment is generally more accurate than one using a fixed orientation.

The ability-task variant in the codebase does this better by computing:

```cpp
FQuat::FindBetweenNormals(FVector::UpVector, (CurrentTip - CurrentRoot).GetSafeNormal())
```

That aligns the capsule with the blade direction.

#### `HitResults` should be cleared per sub-step

`HitResults` is declared before the sub-step loop and reused. For clarity and safety, it should be reset inside the loop before each sweep:

```cpp
for (int i = 0; i < NumSubsteps; ++i)
{
    HitResults.Reset();
    World->SweepMultiByChannel(...);
}
```

Even though `HitActors` prevents repeat damage, clearing the array per sub-step avoids stale result handling.

#### Capsule half-height is externally configured

The component takes `CapsuleHalfHeight` as input rather than deriving it from the current socket distance. That gives flexibility, but it also means the caller must choose appropriate values. If the weapon pose changes a lot, a fixed half-height may not always match the blade geometry well.

#### The trace is an approximation of rotational motion

Interpolating socket endpoints and sweeping between them is a practical solution, but it is still an approximation. Very fast or highly curved rotational motion may still benefit from more sub-steps or a more advanced trace representation.

## 7. When to Use This System

This system is a strong fit when:

- the weapon moves quickly
- the attack animation covers a wide arc
- frame rate can vary
- reliable melee hit registration matters
- overlap-only solutions are not accurate enough

Typical examples:

- sword slashes
- polearm swings
- heavy attacks with large motion arcs
- boss melee attacks where missed hits feel broken

### When a simpler system is enough

A simpler timer or single-trace solution may be sufficient when:

- the attack is slow
- targets are large
- the combat is intentionally forgiving
- the weapon path is short and simple
- the game does not need high-precision melee contact

Typical examples:

- short punch attacks
- simple prototype combat
- stylized melee with generous hitboxes
- low-stakes NPC attacks

## Summary

`UMeleeDetectionComponent` is a practical continuous-style melee tracing system built around:

- previous-frame socket caching
- per-tick tracing
- sub-stepped interpolation
- capsule sweeps
- duplicate-hit suppression

Its main strength is that it traces the weapon's traveled path instead of checking only isolated points in time. That makes it significantly more reliable than a timer-based melee detection approach for fast, animation-driven combat.

The biggest improvement opportunities in the current implementation are:

1. Rotate the capsule to match the weapon segment.
2. Clear `HitResults` per sub-step.
3. Make sub-step count adaptive to distance or frame time.
4. Use server-authoritative tracing for multiplayer if combat is networked.
