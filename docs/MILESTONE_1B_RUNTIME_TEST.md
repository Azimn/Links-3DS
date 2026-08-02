# Milestone 1B Runtime Test

Install `links-3ds-browser.3dsx` at:

```text
/3ds/links-3ds/links-3ds-browser.3dsx
```

Run it from the Homebrew Launcher on a Nintendo 2DS or 3DS with Wi-Fi already configured.

## Required checks

1. The application launches without returning immediately to the Homebrew Launcher.
2. The Links graphics interface appears on the top screen.
3. The Circle Pad moves the pointer.
4. A activates the item under the pointer and also acts as Enter for keyboard navigation.
5. B performs Backspace or browser-back behavior according to the active Links widget.
6. X sends Escape.
7. Y requests reload.
8. L and R send Page Up and Page Down.
9. START requests a normal Links close event.
10. Returning through the HOME menu or closing the application does not hang or crash.
11. A basic HTTP page can be opened while Wi-Fi is connected.

## First network target

Use a simple HTTP endpoint before testing HTTPS or complex pages:

```text
http://example.com/
```

## Failure report

Record:

- whether the top screen changed at launch
- whether the pointer was visible
- whether Circle Pad input moved it
- whether START exited cleanly
- the last visible screen before a crash or hang
- whether Wi-Fi was connected before launch
- whether the failure occurred before or after entering a URL

A phone video or screenshots of the launch sequence are sufficient for the first runtime pass.
