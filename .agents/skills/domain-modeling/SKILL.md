---
name: domain-modeling
description: >-
  Use this skill to model domain concepts into strongly-typed C++17 entities, enforce ubiquitous language, and eliminate Primitive Obsession across all current and future codebase modules.
---

# Domain Modeling & Ubiquitous Language

This skill establishes a disciplined, type-driven domain modeling workflow that applies universally across the **entire codebase**—including all existing libraries (`libs/ShooterCore`, `libs/SdlAdapter`) as well as **any future or newly added libraries** in this repository.

It actively translates game specifications, physics rules, and transformation state mechanics into strongly-typed C++17 entities, maintains a unified ubiquitous language, and eliminates **Primitive Obsession** while adhering strictly to MISRA C++:2023 and SEI CERT C++ safety standards.

---

## 1. Core Principles

1. **Eliminate Primitive Obsession:**
   - Raw primitives (`std::uint8_t`, `int`, `float`, `double`, `std::string`) must never be passed freely across domain or API boundaries.
   - Wrap raw values into strongly-typed value objects with domain semantics and invariant checks.
2. **Make Illegal States Unrepresentable:**
   - Enforce domain invariants upon construction. If a value is invalid, reject it immediately or make out-of-range states unconstructible.
3. **Zero Dynamic Allocation (CERT ERR55-CPP):**
   - Invariants and value objects on real-time loops, collision checks, and weapon targeting must be `noexcept` and execute with zero heap allocations. Use `std::string_view`, stack arrays, or pre-allocated buffers.
4. **Explicit Typing & Conversions (MISRA C++:2023 Rule 7.0.6 & SEI CERT INT31-C):**
   - Fixed-width integers must explicitly include `<cstdint>` and use `std::` qualification.
   - Conversions between units, coordinate frames, or integer ranges must be explicit, clamped, and range-verified via `static_cast`.

---

## 2. Onboarding New Libraries & Future Domains

Whenever a new library or module is added to `libs/`, this domain modeling protocol must be applied from day one:

```
+-------------------------------------------------------------------------------+
|                    NEW LIBRARY DOMAIN ONBOARDING PROTOCOL                     |
|                                                                               |
|   New Library (`libs/<Name>`)                                                 |
|         │                                                                     |
|         ▼                                                                     |
|   1. Classify Architectural Tier                                              |
|         ├── Core Library (Default): Zero external deps, pure C++17, 0-heap    |
|         └── Platform Adapter (`*Adapter`): SDL2/3 hardware wrappers           |
|         │                                                                     |
|         ▼                                                                     |
|   2. Extract Domain Concepts & Ubiquitous Language                           |
|         • Identify domain nouns, measurements, units, and ranges              |
|         • Forbid raw primitives (wrap energy, coordinates, velocities, modes) |
|         │                                                                     |
|         ▼                                                                     |
|   3. Verify Invariant Bounds & Zero-Heap Fast Paths                           |
+-------------------------------------------------------------------------------+
```

---

## 3. Ubiquitous Language & Canonical Domain Entities

### A. Player & Transformation (`libs/ShooterCore`)
| Domain Term | Canonical Type | Allowed Invariants | Purpose |
| :--- | :--- | :--- | :--- |
| `TransformMode` | `enum class TransformMode` | `Robot`, `Jet`, `ToRobot`, `ToJet` | Player mechanical configuration state. |
| `EnergyPoints` | `class EnergyPoints` | Clamped integer $[0, 100]$ | Primary energy reserve for shield & laser. |
| `ShieldState` | `enum class ShieldState` | `Inactive`, `Active`, `Depleted` | Barrier invulnerability status. |

### B. Physics & Collision (`libs/ShooterCore`)
| Domain Term | Canonical Type | Allowed Invariants | Purpose |
| :--- | :--- | :--- | :--- |
| `WorldPos` | `struct WorldPos` | Explicit float `x`, `y` | Continuous 2D world coordinate. |
| `TileCoord` | `struct TileCoord` | Non-negative integer `col`, `row` | Discrete tilemap grid cell. |
| `Velocity` | `struct Velocity` | Bounded vector $(dx, dy)$ | Rate of position change per tick. |
| `Hitbox` | `struct Hitbox` | $w > 0$, $h > 0$ relative to origin | Axis-aligned bounding box. |

### C. Weapons & Targeting (`libs/ShooterCore`)
| Domain Term | Canonical Type | Allowed Invariants | Purpose |
| :--- | :--- | :--- | :--- |
| `TargetEntityId`| `class TargetEntityId` | Positive handle / index | Entity locked by homing laser beam. |
| `LaserBeam` | `struct LaserBeam` | Origin `WorldPos`, Target `WorldPos` | Signature continuous beam rendering data. |
| `DamageValue` | `class DamageValue` | Integer $\ge 0$ | Damage applied per tick to target. |

### D. Hardware-Decoupled Commands (`libs/ShooterCore`)
| Domain Term | Canonical Type | Allowed Invariants | Purpose |
| :--- | :--- | :--- | :--- |
| `RenderCommand` | `struct RenderCommand` | Fixed-size POD variant | Decoupled draw call for platform adapter. |
| `AudioCommand` | `struct AudioCommand` | SFX enum and volume byte | Decoupled audio trigger for sound engine. |

---

## 4. Modeling Workflow for Any Subsystem

When authoring or modifying code in any library (existing or future):

1. **Extract Domain Concepts:**
   - Review game specifications and mathematical models.
   - Distinguish Value Objects from Stateful Entities.
2. **Define Invariants and Preconditions:**
   - Are bounding dimensions positive? Are coordinates in bounds? Are transform transitions legal?
3. **Formulate Strongly-Typed Value Objects:**
   - Use explicit constructors (`explicit constexpr ... noexcept`).
   - Eliminate implicit type conversions.
   - Mark accessors `[[nodiscard]] noexcept`.
4. **Audit MISRA & CERT Compliance:**
   - Integer types: explicitly qualify `std::uint8_t`, `std::uint32_t`, `std::size_t` with `<cstdint>` and `<cstddef>`.
   - Floating-point calculations: verify absence of undefined casts.
   - Fast paths: 0-heap allocations in `noexcept` functions (CERT ERR55-CPP).
