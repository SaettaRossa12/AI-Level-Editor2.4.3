# Script Language

The first MVP accepts one command per line. Commands are case-insensitive.

## Commands

```text
block <x> <y> [length]
spike <x> <y>
orb <yellow|blue|pink|green|red|black> <x> <y>
portal <cube|ship|ball|ufo|wave|robot|spider> <x> <y>
section <x> <y> <cube|ship|ball|ufo|wave|robot|spider> [short|medium|long]
theme <name>
song bpm <number>
start <x> <y>
```

## Example

```text
song bpm 128
theme neon
start 0 0
block 0 0 12
spike 13 0
orb yellow 16 2
portal ship 22 0
section 26 2 ship medium
```
