# AI-Level-Editor

AI-Level-Editor lets AI create your Geometry Dash level. Learn the script system with the simple in-editor tutorial included with the mod, or use a regular English prompt three times per month. Attempts to modify the mod code may result in a two-week ban.

## Scope

- In-game Geode panel for writing a structured level script.
- Intro tutorial shown on first launch and available again from the panel.
- Script parser that turns commands into level object placements.
- Natural-language prompt panel, limited to 3 uses per calendar month.
- Client integrity signals and server-side entitlement checks for a paid 5 EUR product.

## Important Security Note

Do not enforce payment, prompt limits, or bans only inside the mod. A user can patch local code or saved values. The client should report integrity signals, but the backend must own:

- purchase entitlement,
- monthly natural-prompt usage,
- suspicious tamper events,
- 2-week temporary bans,
- AI provider API keys.

The included C++ code is an MVP client scaffold. The backend contract is documented in `docs/backend-contract.md`.

## Script Example

```text
song bpm 128
theme factory
start 0 0
block 0 0 8
spike 9 0
orb yellow 12 2
portal ship 18 0
section 24 0 wave short
```

## Build

Install the Geode SDK, then run:

```powershell
geode build
```

The official Geode docs describe the project structure and build command:

- https://docs.geode-sdk.org/getting-started/create-mod
- https://docs.geode-sdk.org/mods/configuring/
