# ⚔️ Aurora — Gameplay Ability System
### An Unreal Engine 5 action RPG showcasing GAS implementation

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-purple?style=flat&logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat&logo=c%2B%2B)
![GAS](https://img.shields.io/badge/Gameplay%20Ability%20System-GAS-brightgreen?style=flat)
![Enhanced Input](https://img.shields.io/badge/Enhanced%20Input-UE5-orange?style=flat)
![License](https://img.shields.io/badge/License-MIT-gray?style=flat)

## Table of Contents
## Table of Contents

1. [Overview](3overview)
2. [Characters](#characters)
   - 2.1 [GAS\_BaseCharacter](#gas_basecharacter)
   - 2.2 [GAS\_AuroraCharacter · Player](#gas_auroracharacter--player)
   - 2.3 [GAS\_Enemy · AI](#gas_enemy--ai)
3. [Attribute System](#attribute-system)
   - 3.1 [Primary Attributes](#primary-attributes)
   - 3.2 [Secondary / Vital Attributes](#secondary--vital-attributes)
   - 3.3 [Meta Attributes](#meta-attributes)
   - 3.4 [IncomingDamage Flow](#incomingdamage-flow)
   - 3.5 [IncomingXP Flow](#incomingxp-flow)
4. [Gameplay Ability System](#gameplay-ability-system)
   - 4.1 [GAS\_AbilitySystemComponent](#gas_abilitysystemcomponent)
   - 4.2 [GAS\_BaseAbility](#gas_baseability)
   - 4.3 [Aurora's Abilities](#auroras-abilities)

---

## Overview

**Aurora** is an Unreal Engine 5 action RPG built to showcase a full, production-grade
implementation of the **Gameplay Ability System (GAS)**. Inspired by the character Aurora
from Epic's *Paragon*, the project recreates her core ability kit — and expands on it —
with original abilities, a full attribute system, XP-based progression, and polished
gameplay feel.

Built entirely in **C++ with Blueprint extensions**, every major system — from damage flow
and ability activation to UI architecture and AI behavior — is designed for scalability
and clarity.

### What this project demonstrates

- Full **GAS pipeline**: attributes, gameplay effects, ability activation, gameplay cues
- **Aurora's ability kit** faithfully reimplemented + custom original abilities
- **AI enemies** with their own ASC, unique ability sets
- **Attribute system**: Health, MaxHealth, Mana, MaxMana, primary & secondary stats
- **XP & leveling**: kill XP, level-up events, attribute and spell point rewards
- **UI architecture**: MVC-style Widget Controller pattern, delegate-driven data binding
- **Damage flow**: execution calculations, gameplay cues, hit react based on direction ,death
- **Gameplay Feel**: screen shake, camera lag, Niagara VFX, hit pause, hit indicator widget, reactive hit-flash material effects on characters,knockback on hit

## 2. Characters

All characters share a common base that wires up GAS and combat behavior. The hierarchy keeps shared logic in one place while letting each character type extend only what it needs.

```
GAS_BaseCharacter
    ├── GAS_AuroraCharacter   (player)
    └── GAS_Enemy             (AI)
```

---

### 2.1 GAS_BaseCharacter

The root of the character hierarchy. Every character — player or AI — derives from this class. It handles two responsibilities at startup: granting the character's default abilities and applying the gameplay effects that initialize their attributes (Health, Mana, etc.) from a data-driven curve table.

#### Implements

| Interface | Purpose |
|-----------|---------|
| `IAbilitySystemInterface` | Exposes the ASC to the GAS framework so the engine can resolve ability queries |
| `ICombatInterface` | Contract for combat-relevant queries shared across all characters |

#### ICombatInterface — functions

| Function | Return | Description |
|----------|--------|-------------|
| `GetHitReactMontage(FGameplayTag)` | `UAnimMontage*` | Returns the hit react montage matching the incoming tag — enables directional or type-specific reactions without hardcoded logic |
| `GetCharacterClass()` | `ECharacterClass` | Returns this character's class enum, used to look up the correct ability set and attribute curve at spawn |
| `Die(FVector DeathImpulse)` | `void` | Triggers the death sequence; the impulse vector drives directional ragdoll knockback on kill |
| `GetOnDeathDelegate()` | `FOnDeath&` | Multicast delegate — the XP system, enemy spawner, and UI all bind here to react to death without the character knowing they exist |
| `GetOnDamageSignature()` | `FOnDamageSignature&` | Delegate broadcast on every hit — drives floating damage numbers and hit markers |
| `IsDead()` | `bool` | State query used by abilities and AI tasks to skip targeting or animating dead characters |
| `GetAvatar()` | `AActor*` | Returns the physical world actor — required when the ASC lives on a separate object such as PlayerState |

> **Design note:** The two delegates (`OnDeath`, `OnDamage`) keep the combat interface decoupled from any specific system. Every listener reacts independently without the character needing any knowledge of what is observing it.

---

### 2.2 GAS_AuroraCharacter

The player character. Derives from GAS_BaseCharacter and adds all player-specific concerns: input handling, progression tracking, and player-only UI feedback.

The **ASC and Attribute Set live on the PlayerState**, not the character itself — this ensures they survive death and respawn without being torn down and re-initialized.

#### Implements

| Interface | Purpose |
|-----------|---------|
| `IAbilitySystemInterface` | Inherited — fetches ASC from PlayerState |
| `ICombatInterface` | Inherited — hit react, death, delegates, avatar |
| `ICharacterInterface` | Player-only contract for progression, rewards, and UI feedback |

#### ICharacterInterface — functions

**Queries**

| Function | Return | Description |
|----------|--------|-------------|
| `GetXP()` | `int32` | Returns the player's current total XP |
| `FindLevelForXP(int32)` | `int32` | Resolves a raw XP value to a character level by looking it up in the level curve table |
| `GetAttributePoints()` | `int32` | Returns unspent attribute points available in the stat menu |
| `GetSpellPoints()` | `int32` | Returns unspent spell points available in the ability menu |
| `GetAttributePointsReward(int32 Level)` | `int32` | Returns how many attribute points to award for reaching a given level |
| `GetSpellPointsReward(int32 Level)` | `int32` | Returns how many spell points to award for reaching a given level |

**Mutations**

| Function | Return | Description |
|----------|--------|-------------|
| `AddToXP(int32)` | `void` | Adds XP after an enemy kill; internally triggers a level-up check |
| `AddToPlayerLevel(int32)` | `void` | Increments the player level — called by the level-up flow after XP threshold is crossed |
| `AddToAttributePoints(int32)` | `void` | Grants attribute points as a level-up reward |
| `AddToSpellPoints(int32)` | `void` | Grants spell points as a level-up reward |
| `LevelUp()` | `void` | Orchestrates the full level-up sequence — fires the level-up cue, updates UI, grants rewards |
| `ShowHitMarker()` | `void` | Fires a delegate the HUD listens to, triggering the hit-confirm indicator on a successful hit |

> **Design note:** Separating queries from mutations makes it clear which functions are safe to call from UI (read-only) and which trigger state changes. `FindLevelForXP` being a pure query also means the XP-to-level relationship is fully data-driven via a curve table — no hardcoded level thresholds anywhere.

#### Input

Movement, camera look, and jump are bound here via **Enhanced Input**. Ability inputs are routed through the ASC using gameplay input tags, keeping key bindings fully decoupled from ability logic.

---

### 2.3 GAS_Enemy · AI

All AI-controlled enemies derive from this class. Unlike Aurora, enemies own their ASC directly on the Character — they have no PlayerState and no need for the ASC to outlive the actor.

Each enemy has a **health bar widget component** rendered in world space, updated in real time via an attribute change listener on their ASC.

Behavior is driven by a **Behavior Tree**. The enemy's `CharacterClass` tag (from ICombatInterface) determines which ability set and attribute curve they are initialized with at spawn — adding a new enemy type requires no code changes, only data.

## 3. Attribute System

All attributes are defined in `GAS_AttributeSetBase`, which derives from `UAttributeSet`. Initial values are applied at spawn via a Gameplay Effect that reads from a **curve table** — no hardcoded defaults anywhere in code.

---

### 3.1 Primary Attributes

Manually assigned by the player through the attribute menu using attribute points earned on level up. Drive secondary attribute values through Gameplay Effects.

| Attribute | Description |
|-----------|-------------|
| `Strength` | Physical power — affects physical damage output and related secondary stats |
| `Intelligence` | Magical aptitude — affects spell damage, mana capacity, and mana regeneration |

---

### 3.2 Secondary / Vital Attributes

Derived from primary attributes via Gameplay Effects and clamped in `PostGameplayEffectExecute`. Updated automatically whenever a primary attribute changes.

| Attribute | Description |
|-----------|-------------|
| `Health` | Current health. Reaches zero → death. |
| `MaxHealth` | Health ceiling. Derived from Strength. |
| `HealthRegenerationRate` | Passive health recovery per second. |
| `Mana` | Current mana pool. Consumed on ability activation. |
| `MaxMana` | Mana ceiling. Derived from Intelligence. |
| `Stamina` | Current stamina. Used for movement actions. |
| `MaxStamina` | Stamina ceiling. |
| `StaminaRegenerationRate` | Passive stamina recovery per second. |

---

### 3.3 Meta Attributes

Meta attributes are **transient** — they have no persistent value and are never replicated. They act as a one-frame scratch pad that Gameplay Effects write into, allowing `PostGameplayEffectExecute` to intercept the value, apply game logic, and zero it out before the next frame.

| Attribute | Description |
|-----------|-------------|
| `IncomingDamage` | Receives raw damage from a Gameplay Effect. Consumed immediately to modify `Health`. |
| `IncomingXP` | Receives an XP reward from a Gameplay Event. Consumed immediately to drive the level-up flow. |

---

### 3.4 IncomingDamage Flow

When a Gameplay Effect writes to `IncomingDamage`, `PostGameplayEffectExecute` fires and runs the following sequence:

```
IncomingDamage set by GE
        │
        ▼
  Health -= IncomingDamage
  IncomingDamage = 0
        │
        ├─ Health <= 0 ──────────────────────────────────────────►  ICombatInterface::Die(DeathImpulse)
        │                                                                      │
        │                                                             SendXPEvent() to instigator
        │
        └─ Health > 0 ───► Knockback impulse applied
                                    │
                                    ▼
                          GetHitReactMontage(HitTag) played
                                    │
                                    ▼
                          ShowHitMarker() fired on instigator HUD
```

---

### 3.5 IncomingXP Flow  (`SendXPEvent` → `HandleIncomingXP`)

XP is not applied directly. Instead it travels through the GAS event system to keep the reward flow decoupled from the damage flow.

**Step 1 — `SendXPEvent` (called on death)**
Reads the dead enemy's `CharacterClass` and `Level` via `ICombatInterface`, then calls `GAS_FunctionLibrary::GetXPRewardForClassAndLevel()` to look up the correct reward from a data table. Packages the result into a `FGameplayEventData` payload tagged `Attributes_Meta_IncomingXP` and dispatches it to the **instigator** (Aurora) via `UAbilitySystemBlueprintLibrary::SendGameplayEventToActor`.

**Step 2 — `HandleIncomingXP` (fires on the instigator's AttributeSet)**
Reads and immediately zeroes `IncomingXP` to prevent double-processing. Then computes whether a level-up has occurred:

```
NewLevel = FindLevelForXP(CurrentXP + IncomingXP)
NumLevelUps = NewLevel - CurrentLevel

if NumLevelUps > 0:
    for each level gained:
        AttributePointsReward += GetAttributePointsReward(level)
        SpellPointsReward     += GetSpellPointsReward(level)

    AddToPlayerLevel(NumLevelUps)
    AddToAttributePoints(AttributePointsReward)
    AddToSpellPoints(SpellPointsReward)
    LevelUp()   ← fires level-up cue, updates UI

AddToXP(IncomingXP)   ← always applied, level-up or not
```

> **Design note:** XP travels as a Gameplay Event tagged `Attributes_Meta_IncomingXP` rather than a direct function call. This means any ability or system can award XP by dispatching the same event — the AttributeSet handles the rest with no additional wiring. Multi-level-up in a single kill is also handled correctly: rewards are accumulated in a loop over each level gained, so no points are lost if the player gains two or more levels at once.

## 4. Gameplay Ability System

The project uses a custom Ability System Component and a custom base ability
class that extend Unreal's GAS framework with project-specific input routing,
ability lifecycle management, and cost/cooldown querying.

---

### 4.1 GAS_AbilitySystemComponent

Derives from `UAbilitySystemComponent`. Centralizes all ability management
for both Aurora and AI enemies — granting, removing, activating, and routing
input to the correct ability via gameplay tags.

#### Ability Management

| Function | Description |
|----------|-------------|
| `AddCharacterAbilities()` | Grants a list of abilities without activating them — used for player abilities that wait for input |
| `AddPassiveAbilities()` | Grants passive abilities — these activate immediately and run in the background |
| `AddCharacterAbilitiesAndActive()` | Grants and immediately activates abilities — used for always-on behaviors like hit react and death |
| `RemoveAllCharacterAbilities()` | Strips all granted abilities — called on death or game state reset |
| `RemoveCharacterAbility()` | Removes a single ability by spec handle — used when unequipping a spell |
| `ForEachAbility()` | Iterates over all granted abilities and fires a delegate — used by the UI to populate the spell menu |

#### Input Routing

Abilities are not bound to keys directly. Instead each ability carries a
`FGameplayTag` input tag. When a key is pressed or released, the Enhanced
Input system calls into the ASC which finds and activates the matching ability.

| Function | Description |
|----------|-------------|
| `OnAbilityInputPressed(FGameplayTag)` | Finds the ability whose input tag matches and tries to activate it |
| `OnAbilityInputReleased(FGameplayTag)` | Notifies the active ability that the input was released — used for hold-to-charge abilities |
| `ActivateAbilityByTag(FGameplayTag)` | Activates an ability directly by its ability tag — used for programmatic triggers |

#### Tag & Spec Helpers

| Function | Description |
|----------|-------------|
| `GetAbilityTagFromSpec()` | Reads the ability's own gameplay tag from a spec |
| `GetInputTagFromSpec()` | Reads the input tag bound to an ability spec — used by the UI to display correct key hints |
| `GetStatusFromSpec()` | Returns the ability's current status tag (locked, eligible, unlocked, equipped) |

#### Delegates

| Delegate | Fired when |
|----------|------------|
| `AbilitiesGivenEvent` | All startup abilities have been granted — the spell menu binds here to know it is safe to populate |
| `AbilityEquippedEvent` | An ability is equipped or swapped in the spell menu — the UI updates input hint displays |

> **Design note:** Routing input through gameplay tags rather than direct ability
> references means rebinding a key or swapping an ability requires no code change —
> only the tag on the spec needs to change. The ASC and the input system never
> need to know about each other directly.

---

### 4.2 GAS_BaseAbility

Derives from `UGameplayAbility`. Every ability in the project inherits from
this class. It adds two things on top of the engine base:

- An **`InputTag`** property that the ASC uses to match input events to the
  correct ability spec at runtime.
- Helper functions to query cost and cooldown at a given level **without
  activating** the ability — used by the UI to display accurate mana cost
  and cooldown values in the spell menu before the player commits to casting.

| Function | Description |
|----------|-------------|
| `GetManaCost(float Level)` | Returns the mana cost of this ability at the given level |
| `GetCooldown(float Level)` | Returns the cooldown duration of this ability at the given level |

---

### 4.3 Aurora's Abilities

All of Aurora's abilities derive from `GAS_BaseAbility`. Each is implemented
in C++ with Blueprint-exposed properties for tuning.

> ⚠️ **Work in progress** — abilities are functional but still being iterated on.

#### GAS_MeleeAttack_Ability

Aurora's basic melee strike. Plays an attack montage and applies a damage
Gameplay Effect to enemies within reach. Serves as the baseline combat
action available at all times regardless of mana.

#### GAS_HoarFrost

Aurora summons a ring of ice spikes around herself. Any enemy that contacts
the spikes takes damage and receives a **Freeze/Stun** gameplay tag, locking
them in place and attaching a visual ice mesh to their character. Aurora
cannot move while the ability is active — movement is blocked for the
duration of the cast animation via a tag-based movement lock.

Key systems involved: Gameplay Tags (freeze/stun), Niagara (ice ring VFX),
skeletal mesh attachment (ice on enemy), animation-driven timing.

#### GAS_FrozenSimulacrum

Aurora leaps in the direction of the player's input, leaving behind an icy
clone of herself at the origin point. The clone has its own health pool and
can absorb damage as a decoy. Leap direction is calculated from the input
vector at the moment of activation.

Key systems involved: root motion leap, actor spawning (icy clone),
independent health component on the clone, input vector sampling.

#### GAS_GlacialCharge

Aurora charges forward, pushing enemies aside and dealing damage. The charge
uses character movement to traverse any terrain. A persistent **icy trail**
is left behind that allies can use. Aurora can manually destroy the trail
at any time to deny enemies the same path.

Key systems involved: movement component override (terrain traversal),
persistent trail actor with player-triggered destruction, overlap-based
push impulse on hit enemies.
