# Agent Instructions

- Use the build scripts in `build/` for configure and build checks.
- On Windows/MSVC, prefer `build\build_msvc2026-debug.bat` for a normal debug build verification.
- Do not replace these scripts with direct `cmake -S ...`, `cmake --build ...`, or ad hoc generator commands unless the user explicitly asks for that.
- If a fresh configure is needed, use the matching `build\generate_*.bat` script before the matching `build\build_*.bat` script.
