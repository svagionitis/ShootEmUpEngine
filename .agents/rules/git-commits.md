# Git Commit Instructions

When writing a commit message, follow these strict repository rules:

## 1. Subject Line Format
- **Format:** `<Scope>: <verb in imperative> <summary>`
  - Scope is PascalCase/CamelCase identifying the affected component:
    - `Core`: `libs/ShooterCore` general updates
    - `Physics`: Kinematics, gravity, friction, swept AABB collision
    - `Transform`: Robot and Jet mode transformation state machine
    - `Weapon`: Homing laser auto-targeting and beam tracing
    - `Shield`: Energy barrier activation and damage absorption
    - `TileMap`: Tile grid collision, destructible tiles, level parsing
    - `Entity`: Entity pool, enemy behaviors, spawner mechanics
    - `Commands`: RenderCommandQueue and AudioCommandQueue protocols
    - `SdlAdapter`: `libs/SdlAdapter` SDL2/3 renderer, audio, and input
    - `ThexderGame`: `apps/ThexderGame` game application and levels
    - `Tests`: Unit tests and verification suites
    - `Docs`: Architecture, game design, or engine specs
    - `CMake`: Build system, hardening flags, and sanitizers
  - Use `/` for multi-component changes (e.g., `Core/Weapon:`, `Transform/Physics:`).
  - Use brace expansion for sibling units (e.g., `TransformSystem{,.cpp}:`).
- **Verb:** Use lowercase imperative mood immediately after the colon (`fix`, `add`, `implement`, `refactor`, `align`, `remove`, `configure`).
- **Punctuation:** Do not end the subject line with a period.
- **Length:** Target <= 50 characters when possible; 72 characters is the absolute hard limit (especially with multi-component scopes).

## 2. Body Structure
- **Blank Line:** Separate subject and body with exactly one blank line.
- **Wrapping:** Manually hard-wrap all body lines at 72 characters.
- **Summary Paragraph:** Start with a brief overview explaining the problem, context, and rationale (*what* and *why*, not raw code diffs).
- **Component Breakdown:** Group technical details by component bullet points:
  ```text
  - libs/ShooterCore/ComponentName:
    - Detail specific design shifts or API adjustments.
    - Explain non-obvious fixes or invariants maintained.
  ```
