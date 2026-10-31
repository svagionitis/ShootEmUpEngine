# Relative Pathnames in Documentation & Code References

## 1. Core Mandate

All file, directory, header, source, asset, and test references authored in repository documentation (`docs/*.md`, `README.md`, design specs, implementation plans, and architecture notes) must strictly use **relative pathnames**.

Absolute host filesystem paths (`C:\Users\...`, `D:\...`, `/home/...`, `/Users/...`) and absolute URI schemes (`file:///...`) are **strictly prohibited** in all documentation, markdown files, and commit messages.

---

## 2. Rationale & Portability

1. **Cross-Platform Portability**: The codebase is engineered to run seamlessly across Windows (MSVC) and Linux (GCC/Clang). Absolute paths tie documentation to a single developer's host machine and filesystem layout.
2. **Git Hosting & Markdown Viewers**: Relative links resolve natively on GitHub, GitLab, Gitea, and local offline editors (VS Code, Obsidian, Antigravity). Absolute links and `file:///` URIs fail immediately in web browsers and third-party environments.
3. **CI/CD Pipeline Verification**: Automated documentation validation and link-checking scripts fail when encountering host-specific path anchors.
4. **Information Security & Privacy**: Absolute paths leak local user usernames, machine configurations, and private directory structures into version control.

---

## 3. Reference Conventions

### 3.1 Prose & In-Line Code Backticks
When referencing repository files within textual descriptions, tables, or list items, use pathnames relative to the **repository root**:
- **Required**: `` `libs/ShooterCore/WeaponSystem.h` ``
- **Required**: `` `apps/ThexderGame/Main.cpp` ``
- **Required**: `` `tests/ShooterCoreTests/TestWeaponSystem.cpp` ``
- **Banned**: `` `C:\Users\Developer\workspace\shoot-em-up-engine\libs\ShooterCore\WeaponSystem.h` ``
- **Banned**: `` `/home/developer/shoot-em-up-engine/apps/ThexderGame/Main.cpp` ``

### 3.2 Markdown Hyperlinks
When providing clickable Markdown links within files inside `docs/`, use relative paths resolved from the **document's directory**:
- Sibling document link: `[Phase 9 Plan](phase9_plan.md)`
- Target library link: `[`WeaponSystem.h`](../libs/ShooterCore/WeaponSystem.h)`
- Target application link: `[`Main.cpp`](../apps/ThexderGame/Main.cpp)`
- Target test link: `[`TestWeaponSystem.cpp`](../tests/ShooterCoreTests/TestWeaponSystem.cpp)`
- **Banned**: `[`WeaponSystem.h`](file:///c:/Users/Stavros/workspace/shoot-em-up-engine.git/libs/ShooterCore/WeaponSystem.h)`

---

## 4. Architectural Model: Relative vs Absolute Pathing

### ASCII Path Resolution Architecture

```
+-------------------------------------------------------------------------------+
|                       PATH RESOLUTION ARCHITECTURE                            |
|                                                                               |
|   BANNED (ABSOLUTE HOST PATHS / FILE URIs):                                   |
|   file:///c:/Users/Stavros/workspace/shoot-em-up-engine.git/libs/ShooterCore   |
|   * Fragile: Fails on Linux, CI runners, and other developers' workstations   |
|                                                                               |
|   REQUIRED (RELATIVE PATH RESOLUTION):                                        |
|   Repository Root: shoot-em-up-engine.git/                                    |
|   ├── docs/                                                                   |
|   │   └── phase10_plan.md ────(relative link)────┐                            |
|   │         "../libs/ShooterCore/WeaponSystem.h" │                            |
|   └── libs/                                      │                            |
|       └── ShooterCore/                           ▼                            |
|           └── WeaponSystem.h <───────────────────┘                            |
|   * Portable: Resolves correctly on every operating system, IDE, and GitHub   |
+-------------------------------------------------------------------------------+
```

### Mermaid Path Resolution Diagram

```mermaid
graph TD
    subgraph AbsolutePath["Banned: Absolute Host Path"]
        AbsHost["file:///C:/Users/.../libs/ShooterCore/WeaponSystem.h"]
        AbsHost -.->|Breaks on other machines| Fail["Broken Link in CI/Git"]
    end

    subgraph RelativePath["Required: Relative Path"]
        DocFile["docs/phase10_plan.md"]
        TargetFile["libs/ShooterCore/WeaponSystem.h"]
        DocFile -->|Relative link: ../libs/ShooterCore/WeaponSystem.h| TargetFile
        TargetFile -->|Valid across all environments| Pass["Clean Resolution"]
    end
```

---

## 5. Verification Checklist

Before publishing, outputting, or committing any documentation file:
- [ ] No occurrences of `file:///` exist anywhere in the document.
- [ ] No occurrences of drive letters (e.g., `C:`, `D:`) exist in file links or code paths.
- [ ] No occurrences of user home directories (e.g., `/home/`, `C:\Users\`, `/Users/`) exist.
- [ ] Links between documents and source files resolve correctly using relative navigation (`../`).
- [ ] Forward slashes (`/`) are strictly used as the path separator in all Markdown links and prose paths.

