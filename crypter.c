#include <stdio.h>
#include <crypter.h>

int main(int argc, char **argv) {
	if (argc < 3)
	{
		printf("Usage: %s [PAYLOAD FILE] [OUTPUT EXE]\n", argv[0]);

		return -1;
	}

	char *payload_file = argv[1];
	char *crypto_exe = argv[2];

	CryptoPackageCtx ctx;
	CryptoPackageInit(&ctx, payload_file);
	CryptoPackageInvoke(&ctx, crypto_exe);

	char source[strlen(crypto_exe) + 3];

	snprintf(source, sizeof(source), "%s.c", crypto_exe);

	char command[strlen(source) * 2 + 256];

	snprintf(command, sizeof(command), "x86_64-w64-mingw32-gcc %s -o %s -fno-stack-protector", source, crypto_exe);

	printf("Now run: %s\n", command);

	printf("Finished\n");

	return 0;
}
