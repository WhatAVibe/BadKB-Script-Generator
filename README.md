# BadDK — Script Generator

A visual DuckyScript builder for Flipper Zero BadUSB and Hak5 Rubber Ducky. Build scripts by clicking and dragging blocks from a categorized palette onto a canvas, reorder them, tweak values, and export a working payload. Includes a built-in executor to test scripts directly on your machine before deploying to a device.

![BadKB Screenshot](src/duck.png)

## Features

**Visual Script Builder**
- 50+ action types organized into collapsible categories — delays, keystrokes, modifiers, function keys, and more
- Click any action in the palette to add it to your script, or drag it directly onto the canvas
- Drag canvas blocks to reorder them with a visual insertion indicator
- Double-click any block to edit its value, or right-click for duplicate/move/delete

**Scratch Theme**
- Click the duck icon in the bottom-right corner to switch to a Scratch-inspired block view
- Blocks are color-coded by type (orange for control/timing, purple for text input, yellow for navigation, blue for modifiers)
- Notched blocks with value bubbles that visually snap together like Scratch code blocks
- Resets to default on application restart

**Live Script Execution**
- File → Execute (or F5) to test your script with real keyboard input
- 3-second countdown lets you switch to your target window
- Press F5 at any time to force-stop execution
- Supports STRING typing, modifier combos (GUI r, CTRL v, ALT F4), delays, repeats, and all standard DuckyScript commands

**Script Preview**
- Live preview panel shows the generated DuckyScript output as you build
- Syntax-colored output with copy-to-clipboard (Ctrl+Shift+C)

**Templates**
- 6 built-in templates: WiFi Grabber, Download & Execute, Open Website, Spawn Shell, Lock Machine, Hello World
- Templates → click any template to load it

**Productivity**
- Full undo/redo (Ctrl+Z / Ctrl+Y)
- Save/Open standard `.txt` DuckyScript files
- Import existing scripts — the parser reads DuckyScript back into the visual editor
- Keyboard shortcuts for every common action

**Documentation**
- Help → Documentation opens a reference window explaining every action type
- Hover tooltips on all palette items
- Detailed REPEAT examples showing correct vs incorrect usage

## Building

### Prerequisites
- **CMake** 3.20 or later
- **Visual Studio 2022 or 2026** with C++ desktop workload
- **Git** (for FetchContent to download dependencies)

### Build
```
build.bat
```

That's it. CMake automatically downloads ImGui (docking branch), GLFW, and stb_image via FetchContent. The first build takes a minute or two to fetch dependencies; subsequent builds are near-instant.

The output lands at `build\Release\BadKB.exe`. The build script also copies it to the project root.

### Build flags
- `/O2 /GS-` for release optimization
- `/SUBSYSTEM:WINDOWS` to suppress the console window
- OpenGL 3.2 core profile — no external GL loader required (functions loaded inline via wglGetProcAddress)

## Project Structure
```
BadDK Script Generator/
  src/
    main.cpp       — Entire application (~950 lines)
    duck.ico       — Application icon
    duck.png       — Duck texture for the status bar
  CMakeLists.txt   — Build configuration
  build.bat        — One-click build script
  resources.rc     — Windows resource file (icon embedding)
```

## Usage

1. **Build your script** — click or drag actions from the palette (left panel) onto the canvas (center)
2. **Edit values** — double-click any block to change its delay, text content, or key combination
3. **Reorder** — drag blocks up and down to rearrange
4. **Preview** — the right panel shows the generated DuckyScript
5. **Test** — File → Execute to run the script on your machine
6. **Export** — File → Save to write a `.txt` file ready for your Flipper Zero or Rubber Ducky

## Action Types

| Category | Action | Description |
|----------|--------|-------------|
| Comments | REM | Comment line (not executed) |
| Timing | DELAY, DEFAULT_DELAY | Pause in milliseconds |
| Text | STRING, STRINGLN | Type text, optionally with Enter |
| Navigation | ENTER, TAB, ESC, SPACE, BACKSPACE, DELETE, HOME, END | Key presses |
| Arrows | UP, DOWN, LEFT, RIGHT | Arrow keys |
| Modifiers | GUI, CTRL, ALT, SHIFT combinations | Keyboard shortcuts |
| Function | F1-F12 | Function keys |
| Advanced | REPEAT, LED, HOLD, RELEASE | Loops, LED toggles, key holds |

## Why "BadDK"?

BadUSB + DuckyScript Keyboard = BadDK. Runs on anything that speaks DuckyScript — Flipper Zero, Hak5 Rubber Ducky, USB Rubber Ducky, Malduino, and custom BadUSB implementations.

## License

MIT — see [LICENSE](LICENSE)
