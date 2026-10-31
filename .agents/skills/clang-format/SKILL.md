---
name: clang-format
description: >-
  Use this skill to format C++ code in-place and verify code style compliance against .clang-format across all codebase libraries.
---

# Code Formatting & `clang-format` Verification

This skill guides the formatting and style verification of C++ source and header files, ensuring strict compliance with [`.clang-format`](../../../.clang-format) across the Shoot-Em-Up Game Engine repository whenever completing an implementation task.

---

## 1. Core Principles & Safety Requirements

1. **Mandatory Post-Implementation Formatting:**
   Every time an implementation, bug fix, or refactoring task is completed, modified files must be formatted using `clang-format -i` before presenting code or creating a commit.

2. **Zero Formatting Violations:**
   The verification check `clang-format --dry-run --Werror` or `python3 scripts/verify_clang_format.py` must return exit code 0.

3. **Deterministic Style:**
   Code style is anchored to WebKit with project customizations: 4-space indentation, 120 column limit, left pointer alignment, WebKit braces, and strict prevention of single-line control statements.

---

## 2. Command Reference

### Format Specific Files In-Place:
```bash
clang-format -i libs/ShooterCore/TransformSystem.h libs/ShooterCore/TransformSystem.cpp
```
Or with helper script:
```bash
python3 scripts/verify_clang_format.py --fix --files libs/ShooterCore/TransformSystem.h libs/ShooterCore/TransformSystem.cpp
```

### Dry-Run Verification (Strict / CI Mode):
```bash
clang-format --dry-run --Werror libs/ShooterCore/TransformSystem.h libs/ShooterCore/TransformSystem.cpp
```
Or with helper script:
```bash
python3 scripts/verify_clang_format.py --files libs/ShooterCore/TransformSystem.h libs/ShooterCore/TransformSystem.cpp
```

### Full Repository Format via CMake:
```bash
# In-place format
cmake --build build --target format

# Check only
cmake --build build --target check-format
```

---

## 3. Workflow Diagrams

### ASCII Diagram: Completion Verification Gate

```
+--------------------------------------------------------------------------+
|                     CODE COMPLETION FORMATTING GATE                      |
|                                                                          |
|   [Modified Files: *.h, *.cpp]                                           |
|        │                                                                 |
|        ▼                                                                 |
|   Step 1: In-Place Formatting                                            |
|   clang-format -i <files>                                                |
|        │                                                                 |
|        ▼                                                                 |
|   Step 2: Verification                                                   |
|   python3 scripts/verify_clang_format.py --files <files>                 |
|        │                                                                 |
|        ├── (Fail: Violations Found) ──> Re-run clang-format -i           |
|        │                                                                 |
|        ▼ (Pass: Code 0)                                                  |
|   Step 3: Staging & Git Commit Verification                              |
+--------------------------------------------------------------------------+
```

### Mermaid Diagram: Completion Verification Gate

```mermaid
flowchart TD
    A["Completed Implementation Diff"] --> B["clang-format -i &lt;files&gt;"]
    B --> C{"verify_clang_format.py --files &lt;files&gt;"}
    C -->|Violations Found| B
    C -->|Pass (Code 0)| D["Pre-Commit Style Gate Passed"]
    D --> E["Ready to Present / Commit"]
```

---

## 4. Verification Checklist

Before finalizing any C++ implementation:
- [ ] 1. Are all modified `.h` and `.cpp` files formatted with `clang-format -i`?
- [ ] 2. Does `python3 scripts/verify_clang_format.py --files <files>` pass with code 0?
- [ ] 3. Are braces, indentations, and column limits aligned with [`.clang-format`](../../../.clang-format)?
- [ ] 4. Has IWYU verification run alongside clang-format?

