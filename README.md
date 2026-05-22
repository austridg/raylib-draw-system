# Animation

A simple sprite animation test built with [raylib](https://www.raylib.com/) in C++.

## Building

Make sure raylib is installed, then compile with:

```bash
g++ main.cpp animation.cpp -o animation -lraylib
```

## Running

```bash
./animation
```

A window will open displaying a cropped region of the sprite sheet.

## Files

- `animation.h` / `animation.cpp` — animation system (frames, rules, animation registry)
