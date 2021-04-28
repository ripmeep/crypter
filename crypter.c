#include <stdio.h>
#include <crypter.h>

int main(int argc, char **argv) {
    if (argc < 3)
    {
        printf("Usage: %s [PAYLOAD FILE] [OUTPUT EXE]\n", argv[0]);

        return -1;
    }

    char *payload_file = argv[1];
    char *crypter_exe = argv[2];

    CrypterPackageCtx ctx;
    CrypterPackageInit(&ctx, payload_file);
    CrypterPackageInvoke(&ctx, crypter_exe);

    char source[strlen(crypter_exe) + 3];

    snprintf(source, sizeof(source), "%s.c", crypter_exe);

    char command[strlen(source) * 2 + 256];

    snprintf(command, sizeof(command), "x86_64-w64-mingw32-gcc %s -o %s -fno-stack-protector", source, crypter_exe);

    printf("Now run: %s\n", command);

    printf("Finished\n");

    return 0;
}
