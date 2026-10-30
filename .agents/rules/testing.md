# Testing & Verification Standards

This rule governs automated testing, hardware-free simulation, and regression testing across the **entire codebase**—encompassing all current test targets and **any newly added libraries** in this repository.

---

## 1. Testing Architecture & Mock Seams

```
+-------------------------------------------------------------------------------+
|                       AUTOMATED TESTING & SEAM ARCHITECTURE                   |
|                                                                               |
|   tests/ (Unit, Regression & Physics Simulation Suites)                       |
|         │                                                                     |
|         ├── [Transform Tests]: Robot <-> Jet mode transitions, hitbox checks  |
|         ├── [Physics & Tile Collision]: Swept AABB, solid & breakable tiles   |
|         ├── [Weapon System Tests]: Homing laser targeting & energy drain      |
|         ├── [Shield Tests]: Barrier activation, projectile deflection         |
|         └── [Entity Tests]: Zero-heap entity pool reuse & lifecycle           |
|         │                                                                     |
|         ▼                                                                     |
|   Deterministic CTest Execution (`ctest --output-on-failure` -> 100% Pass)    |
+-------------------------------------------------------------------------------+
```

```mermaid
flowchart TD
    subgraph TestTargets["CTest Targets (tests/*)"]
        T1["TransformTests<br/>(Robot vs Jet bounds, tunnel check)"]
        T2["TileCollisionTests<br/>(Swept AABB vs tile grid)"]
        T3["WeaponTests<br/>(Homing laser auto-aim & energy burn)"]
        T4["ShieldTests<br/>(Energy barrier deflection & depletion)"]
        T_NEW["Future Engine Tests<br/>(tests/&lt;NewModule&gt;Tests/)"]
    end

    subgraph Seams["Hardware-Free Mock Seams"]
        M1["Synthetic Input Feeds (Deterministic Keypresses)"]
        M2["Render Command Queue Capture (Inspect draw calls)"]
        M3["Audio Command Queue Capture (Inspect SFX triggers)"]
    end

    TestTargets --> Seams
    Seams --> PASS["Zero Hardware Dependency / 100% Deterministic Pass"]
```

---

## 2. Test-Driven Bug Fixing (TDD)

When fixing any defect or regression in any module:
1. **Never write the fix first.**
2. **Step 1 (RED):** Write a targeted test that exposes and reproduces the bug. Confirm failure under `ctest`.
3. **Step 2 (GREEN):** Implement the minimal, MISRA/CERT-compliant fix.
4. **Step 3 (REFACTOR):** Confirm all tests pass with zero compiler warnings and zero compliance errors.

---

## 3. Repository Testing Conventions

1. **Automated CTest Suite:**
   - Execute all test suites via:
     ```bash
     ctest --test-dir build --output-on-failure
     ```
   - 100% of tests must pass with zero failures.
2. **Hardware-Free Simulation Requirement:**
   - Unit tests must **never** require an actual GPU, graphics context, SDL window, or audio device.
   - Use command queue inspections (`RenderCommandQueue`, `AudioCommandQueue`) and synthetic input feeds for deterministic headless verification.
3. **Game Mechanics Conformance:**
   - Verify transformation mechanics, laser auto-targeting formulas, energy pool drain rates, and tile collisions against specified game rules.
4. **Zero Dynamic Allocation Invariant Tests:**
   - Fast paths must be verified to operate without heap allocation during active gameplay frames.
5. **Universal Seam Rule for Any Future Library:**
   - Any new library added under `libs/<NewModule>` must include a companion test directory under `tests/<NewModule>Tests/` with hardware-decoupled mock seams.
