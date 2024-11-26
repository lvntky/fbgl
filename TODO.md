# TODO List for 2D Framebuffer Graphics Library (`fbgl`)

## Core Rendering
- [x] Implement pixel manipulation: set, get, and clear individual pixels with color data.
- [ ] Implement basic shapes:
  - [x] Draw lines.
  - [x] Draw rectangles.
  - [x] Draw circles.
  - [ ] Draw ellipses.
  - [ ] Draw polygons.
  - [x] Support filled versions of shapes.
  - [x] Support outlined versions of shapes.
- [ ] Implement bitmap rendering:
  - [x] Render 2D images (tga) to the framebuffer.
  - [ ] Support transparency via alpha channels.
  - [ ] Support transparency via a key color.
- [x] Implement text rendering using psf fonts.

---

## Color Handling
- [ ] Support RGB color format.
- [ ] Support grayscale color format.
- [ ] Support indexed color format.
- [ ] Add utility to convert between color formats.
- [ ] Support alpha blending for semi-transparent rendering.

---

## Window and Viewport
- [ ] Add windowing support:
  - [ ] Define sub-framebuffers (windows) within the main framebuffer.
  - [ ] Maintain independent clipping for each window.
- [ ] Implement clipping to respect rendering boundaries.

---

## Utilities and Effects
- [ ] Implement 2D transformations:
  - [ ] Scaling.
  - [ ] Rotation.
  - [ ] Translation.
  - [ ] Transform relative to origin or center.
- [ ] Add optional anti-aliasing for smoother shapes and lines.
- [ ] Implement image manipulation:
  - [ ] Scale tga.
  - [ ] Crop tga.
  - [ ] Rotate tga.
- [ ] Implement gradients and patterns:
  - [ ] Fill shapes with gradients.
  - [ ] Fill shapes with pattern-based textures.

---

## Performance and Optimization
- [ ] Optimize batch rendering to minimize CPU cycles.
- [ ] Add hooks for hardware acceleration (optional).

---

## Integration
- [ ] Provide initialization routines for creating and managing a framebuffer.
- [ ] Ensure platform independence:
  - [ ] Abstract hardware details for Linux framebuffer.
  - [ ] Abstract hardware details for SDL2.
- [ ] Add support for reading framebuffer content to standard image formats (e.g., BMP, PNG).
- [ ] Add support for writing framebuffer content to standard image formats (e.g., BMP, PNG).

---

## Debugging and Diagnostics
- [ ] Develop diagnostics tools to inspect pixel data.
- [ ] Add debugging overlays (e.g., FPS, gridlines).
- [ ] Implement graceful error handling for out-of-bounds operations with informative messages.
