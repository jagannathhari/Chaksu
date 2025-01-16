# Chaksu - Image Viewer

Chaksu is a minimal, cross-platform image viewer designed to be fast, lightweight, and dependency-free. It comes as a single binary under 1 MB in size (uncompressed) and under 400 KB when UPX compressed. It supports basic image viewing features such as zooming, panning, and rotation.

## Features

- **Single Binary**: No installation required. Just download and run.
- **Small Size**: < 1 MB (uncompressed), < 400 KB when UPX compressed.
- **Cross-Platform**: Works on Windows, macOS(not tested), and Linux.
- **Zoom, Pan, and Rotate Support**: Seamlessly zoom in/out, pan across, and rotate images.
- **Minimal**: Simple and clean user interface.
- **Dependency-Free**: No external dependencies, just the binary.

# Build

On Linux:

```
cc main.c -o chaksu -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```
