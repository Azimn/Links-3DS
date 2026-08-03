# Emulator Testing

## Recommended emulator

Use Azahar for iterative desktop testing. It is the actively maintained Citra-derived 3DS emulator and provides current Windows, macOS, Linux, and Android releases.

The browser port should still be validated on real 2DS or 3DS hardware before release. Emulator success does not prove correct applet lifecycle, SD-card behavior, Wi-Fi recovery, performance, or CIA installation behavior on hardware.

## Windows workflow

1. Install or extract a current Azahar desktop release.
2. Build the runtime smoke application or full browser through GitHub Actions or a local devkitARM environment.
3. Download the workflow artifact and extract it into the repository build directory.
4. Launch the runtime smoke build first:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run-azahar.ps1 `
  -AzaharPath "C:\Path\To\azahar.exe" `
  -RuntimeSmoke
```

5. Launch the full browser:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run-azahar.ps1 `
  -AzaharPath "C:\Path\To\azahar.exe"
```

The helper passes the `.3dsx` path to Azahar as a positional application argument. If a particular Azahar build does not accept positional launch arguments, open the same `.3dsx` through Azahar's graphical interface.

## Dual-screen alpha checks

The upper screen should contain only the Links page viewport and pointer. The lower screen should show the persistent browser UI, URL status, trackpad instructions, and touch toolbar.

Verify the following behaviors:

- Touching the URL area or pressing SELECT opens the system software keyboard.
- Submitted keyboard text is forwarded to the Links URL dialog.
- The lower touch area moves and drags the upper-screen pointer relatively.
- Back, Forward, Reload, Home, and Menu touch regions produce browser commands.
- Circle Pad, D-pad, A, B, X, Y, L, R, SELECT, and START still work.
- Closing the software keyboard restores the lower-screen UI.
- START exits without leaving the emulator hung.

## Current emulator limits

No maintained browser-hosted 3DS emulator was found that is suitable for automated testing of this application. Desktop Azahar is the practical option. A graphical emulator can validate rendering, touch mapping, software-keyboard applet behavior, and much of Links initialization, but hardware remains required for final socket service, suspend and resume, HOME Menu, CIA, storage, and old-model performance validation.
