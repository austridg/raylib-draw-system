# Changelog

All notable changes to RAD-2D are documented here. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and the project aims
to follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2026-06-12

### Changed
- RAD-2D is now distributed purely as a library. The bundled example program
  (`main.cpp`) and its demo art assets have been removed.
- The Makefile now builds the static library (`librad2d.a`) instead of the
  example; CMake no longer builds an example target.
- Reworked the README into a focused landing page and moved the full usage guide
  into a new **`DOCUMENTATION.md`**.

### Removed
- The "Status & roadmap" section from the README.

## [0.2.0] - 2026-06-11

### Added
- **`Background` drawable** — fills an area with a tileable texture that scrolls
  via `TEXTURE_WRAP_REPEAT` for a seamless loop, or draws an animation frame.
- **`Text` drawable** — glyph-by-glyph rendering with a typewriter reveal,
  per-glyph effects (`SHAKE` / `WAVE` / `FADE_IN` / `FADE_OUT`), and a
  `setOnReveal` hook for per-letter events (e.g. blip sounds).
- **`UI` drawable** — composite HUD element with an (optionally animated)
  background texture, an optional icon, and an optional attached `Text`.
- **`Tilemap` / `TileSet` / `Tile`** (`tiles/`) — id-keyed tile definitions
  (static or animated) and a z-layered, row-major tile grid for drawing maps.
- **`AnimatedDrawable` base** — shared animation plumbing (registry pointer,
  usable-animation set, `AnimationState`) behind `Sprite`, `UI`, and `Background`.
- `find_package(rad2d)` support: install now generates `rad2dConfig.cmake` and a
  version file, and pulls in raylib via `find_dependency`.
- `RAD2D_VERSION_MAJOR` / `MINOR` / `PATCH` macros in the umbrella header.

### Changed
- `Drawable::draw()` is now pure virtual, so drawables can be stored and drawn
  polymorphically (`std::vector<std::unique_ptr<Drawable>>`).

### Fixed
- `TextureRegistry::get` / `FontRegistry::get` now log and return `nullptr` on an
  unknown id instead of throwing `std::out_of_range`.
- `AnimationState::getSource` guards against an animation built with no frames.
- The example sprite sheet (`test_anim.png`) is no longer excluded by the
  blanket `*.png` ignore rule.

## [0.1.0]

- Initial beta: asset registries, central animation registry, and the
  `Drawable` / `Sprite` rendering pipeline.
