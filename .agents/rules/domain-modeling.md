---
trigger: always_on
---

# Domain Modeling & Value Objects Standard

This rule mandates type-driven domain modeling across the **entire codebase**—enforcing standards on all current libraries (`libs/ShooterCore`, `libs/SdlAdapter`) and **any future libraries** added to the repository.

---

## 1. Core Principles

1. **Prohibition of Primitive Obsession:**
   - Raw primitives (`std::uint8_t`, `int`, `float`, `double`, `std::string`) must **never** be passed directly across business logic, algorithm, or API boundaries where semantic meaning or domain bounds exist.
   - Wrap primitives into zero-cost, strongly-typed C++17 value objects (e.g., `TransformMode`, `EnergyPoints`, `ShieldGauge`, `WorldPos`, `TileCoord`, `Velocity`, `Hitbox`, `DamageValue`).
2. **Make Illegal States Unrepresentable:**
   - Enforce domain invariants upon construction. If a value falls outside allowable bounds, it must be rejected immediately at construction time or fail compile-time validation.
   - Mutually exclusive states (e.g., Robot crouching while simultaneously executing Jet thrust) must be rejected by the domain layer state machine.
3. **Zero Dynamic Allocation (CERT ERR55-CPP):**
   - Value types and invariant validators on real-time loops, collision checks, and weapon targeting must be `noexcept` and execute with **zero dynamic heap allocations**. Operate on `std::string_view`, stack arrays, or pre-allocated buffers.
4. **Explicit Typing & Conversions (MISRA C++:2023 Rule 7.0.6 & SEI CERT INT31-C):**
   - Fixed-width integers must explicitly include `<cstdint>` and use the `std::` namespace.
   - Coordinate or unit transformations (e.g., world float coordinate to tile grid integer) must be explicit, clamped, and range-verified via `static_cast`.

---

## 2. Architecture: Primitive Obsession vs. Strongly-Typed Domain Modeling

### ASCII Diagram: Primitive Obsession vs. Strong Value Objects

```
+-------------------------------------------------------------------------------+
|                       PRIMITIVE OBSESSION (STRICTLY BANNED)                   |
|                                                                               |
|   // Ambiguous parameters, easily transposed, zero compile-time safety:      |
|   void updatePlayer(float x, float y, float vx, float vy, int energy, int mode);
|   void fireLaser(float srcX, float srcY, float targetX, float targetY, int pwr);
+-------------------------------------------------------------------------------+
|                     STRONG DOMAIN MODELING (REQUIRED)                         |
|                                                                               |
|   // Type-safe, invariant-enforced, zero runtime overhead:                    |
|   void updatePlayer(WorldPos pos, Velocity vel, EnergyPoints energy, Mode mode);
|   void fireLaser(WorldPos origin, WorldPos target, DamageValue power);       |
+-------------------------------------------------------------------------------+
```

### Mermaid Diagram: Strong Value Object Architecture

```mermaid
classDiagram
    direction TB
    
    namespace ShooterDomain {
        class TransformMode {
            <<enumeration>>
            Robot
            Jet
            TransformingToRobot
            TransformingToJet
        }
        class EnergyPoints {
            -std::uint16_t value_
            +explicit EnergyPoints(std::uint16_t val)
            +std::uint16_t value() const noexcept
            +bool hasSufficient(EnergyPoints cost) const noexcept
        }
        class ShieldGauge {
            -std::uint8_t percentage_
            +explicit ShieldGauge(std::uint8_t pct)
            +bool isActive() const noexcept
        }
        class WorldPos {
            +float x
            +float y
            +TileCoord toTileCoord(TileSize tileSize) const noexcept
        }
        class Hitbox {
            +WorldPos min
            +WorldPos max
            +bool intersects(const Hitbox& other) const noexcept
        }
    }
```

---

## 3. Implementation Rules for Value Types

1. **Explicit Constructors:**
   - Every single-argument constructor must be marked `explicit` (e.g. `explicit constexpr EnergyPoints(std::uint16_t val) noexcept`).
2. **Constexpr & Noexcept:**
   - Methods querying value type states must be marked `[[nodiscard]] constexpr ... noexcept` whenever possible.
3. **No Implicit Casts:**
   - Conversion to raw primitive types must be explicit via named accessors (`.value()`, `.raw()`, `.asInt()`), never through implicit conversion operators `operator T()`.
4. **Application to Any Future Library:**
   - Any new module added under `libs/<NewModule>` must identify its core nouns, coordinates, velocities, and resource pools, defining type-safe value objects before authoring business logic.
