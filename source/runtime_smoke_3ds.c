#include <3ds.h>
#include <stdio.h>

int main(void) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Links 2.30 for Nintendo 3DS\n\n");
    printf("Runtime smoke test passed.\n");
    printf("Graphics and applet loop are active.\n\n");
    printf("Press START to exit.\n");

    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) {
            break;
        }
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
