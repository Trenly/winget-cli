---
applyTo: "**"
---

# Copilot Memory

Preferences and conventions learned from working in this repository. Any conventions that are not explicitly mentioned here should be inferred from the codebase.

## How to Handle Corrections

When a user corrects you on terminology, naming conventions, workflow preferences, or any other practice that applies repository-wide (not just to the current task), follow these steps:

1. **Recognize the correction.** A correction has occurred when the user tells you that something you said, named, or did is wrong or should be done differently — especially if it involves consistent patterns such as naming conventions, terminology, architectural practices, or tool usage.

2. **Assess the scope.** Ask yourself: *Would other contributors to this repository benefit from knowing this? Would a future agent session make the same mistake without this guidance?* If yes, it is a candidate for shared repository memory (this file). If the correction is personal to this user (e.g., a workflow preference or output style), it is a candidate for user memory instead.

3. **Ask the user before storing.** Do not silently write to either location. Instead, ask:

   > "Should I remember this correction for the future? If so, should I add it to the **repository memory** (`copilot-memory.md`) so all contributors' agents benefit, or as a **user memory** (via the memory skill) that applies only to your sessions?"

   Offer both options clearly and let the user decide.

4. **Act on the answer.**
   - If **repository memory**: add a concise, unambiguous entry under the appropriate section of this file (`copilot-memory.md`) and commit it.
   - If **user memory**: call the `store_memory` tool with a clear `fact`, a `reason` explaining when it applies, and a `citations` reference to this conversation or the relevant file.
   - If **neither**: acknowledge and move on without storing.

## Terminology

- Use **Packaged** and **Unpackaged** (not "Package context" / "Non-package context") when referring to the two WinGet execution contexts.
