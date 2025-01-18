# Chaksu - Image Viewer

Chaksu is a minimal, cross-platform image viewer designed to be fast, lightweight, and dependency-free. It comes as a single binary under 1 MB in size (uncompressed) and under 400 KB when UPX compressed. It supports basic image viewing features such as zooming, panning, and rotation.
Download from [release](https://github.com/jagannathhari/Chaksu/releases) page.

## Features

- **Single Binary**: No installation required. Just download and run.
- **Small Size**: < 1 MB (uncompressed), < 400 KB when UPX compressed.
- **Cross-Platform**: Works on Windows, macOS(not tested), and Linux.
- **Zoom, Pan, and Rotate Support**: Seamlessly zoom in/out, pan across, and rotate images.
- **Minimal**: Simple and clean user interface.
- **Dependency-Free**: No external dependencies, just the binary.

# Controls

- **Rotate:**
  - Press **S** to rotate image clockwise.
  - Press **A** to rotate image counterclockwise.
- **Navigation:**
  - Press **N** to view the next image.
  - Press **P** to view the previous image.
- **Drag & Drop:**
  - Drag and drop an image or a directory containing images to open.
- **Reset View:**
  - Press **0** to reset the image view.
- **Zoom:**
  - Zoom in or out using the mouse wheel.
- **Pan:**
  - Pan the image by clicking and dragging with the mouse.


# Build

## build depenpencies:-
[libwebp 1.5.0](https://github.com/webmproject/libwebp/releases/tag/v1.5.0)<br>
[Raylib 5.5](https://github.com/raysan5/raylib/releases/tag/5.5)<br>

compile both library(statically or dynamically) and link against it.<br>

On Linux:

```
cc main.c -o chaksu -I. -DRELEASE -lraylib -lwebp -lGL -lm -lpthread -ldl -lrt -lX11
```
## Font Use
[Anonymous Pro](https://www.marksimonson.com/fonts/view/anonymous-pro/)
