# Here be dragons

## Building

I don't know what I'm doing. If you want to participate in my not knowing what I'm doing, you can build this mess the usual Meson way:

```sh
$ meson setup build-debug
$ cd build-debug
$ meson compile
$ ./reactor
```

This requires SDL2 and GLM (runtime dependencies), and Meson and Ninja (build dependencies) to be installed.

## Usage

Not much to use right now, but here you go (QWERTY):

 - WASD: movement along the camera's forward-rightward plane (i.e. horizontal movement)
 - Space/Lshift or FX: move along the camera's upward axis (i.e. vertical movement)
 - EQ: rotation along the camera's forward plane (i.e. lean, kinda)
 - mouse (hold and drag LMB) or RZVC: rotate camera (i.e. look around)
 - P: toggle pause state (initial state is paused)

FPS is locked at 60, TPS at 30. If your system could do more, it won't. **If your system can't do this much, most likely your entire desktop will lag to oblivion, or potentially break due to bad coding on whosever part who coded your desktop.** You've been warned.
