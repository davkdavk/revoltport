# RGH/JTAG Test

Build the current development image:

```bash
bash rv360/build_xenon.sh
```

The script prints a fresh `build/xenon.<id>/` directory. Copy its
also created at `build/rgh-current/`.

## Launch

Launch `revolt360-dev.xex` through XeXMenu, Aurora, or another homebrew
launcher. Do not overwrite a retail game executable.

## Expected Scope

- Offline development image only.
- Network, Xbox Live, DLC, voice chat, and music streaming are disabled or
  stubbed.
- The first useful result is whether the image boots, reaches the title/menu,
  and begins loading without crashing.

## Report Back

Record the first failing stage:

1. Does the XEX launch at all?
2. Does it reach a blank screen, title screen, or menu?
3. Does it load a track?
4. Does it reach a race?
5. Does the controller respond?
6. Does it crash or return to the dashboard?

Include the console dashboard/XeLL error code if available and any serial,
debug monitor, or launch-log output. A photo of the screen is useful for
rendering failures.
