/***
*reloc.c - Relocate MZ executables
*
*   Rewrite of Microsoft reloc.exe;  (C) leecher@dose.0wnz.at
*
*Purpose:
*   Relocates an executable
*
*******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tools.h>

unsigned char buf[0x400];

char *error()
{
	return strerror(errno);
}

void relocate(char *src, char *dst, char *baseseg)
{
	FILE *fpIn, *fpOut;
	struct exe_hdr hdr;
	long org = 0, pos;
	unsigned short e_crlc, posout, seg;
	size_t rd;

	if (!(fpIn = fopen(src, "rb")))
	{
		fprintf (stderr, "reloc: cannot open %s - %s\n", src, error());
		exit(1);
	}
	if (fread(&hdr, sizeof(hdr), 1, fpIn) != 1)
	{
		fprintf (stderr, "reloc not enough data in header\n");
		exit(1);
	}
	if (hdr.e_ss || hdr.e_sp)
	{
		fprintf (stderr, "reloc: conversion not possible, stack present\n");
		exit(1);
	}
	if (hdr.e_cs)
	{
		fprintf (stderr, "reloc: conversion not possible, entry segment not 0\n");
		exit(1);
	}
	if (hdr.e_ip == 0x100)
	{
		if (hdr.e_crlc)
		{
			fprintf(stderr, "reloc: conversion not possible, CS:IP = 0:100 with relocations\n");
			exit(1);
		}
		fseek(fpIn, 0, SEEK_END);
		if (ftell(fpIn) - (long)hdr.e_cparhdr * 16 > 0xFFFF)
		{
			fprintf(stderr, "reloc: conversion not possible, COM files are < 64K\n");
			exit(1);
		}
		org = 0x100;
	}
	else
	{
		if (hdr.e_ip)
		{
			fprintf(stderr, "reloc: conversion not possible, invalid initial IP\n");
			exit(1);
		}
	}

	if (!(fpOut = fopen(dst, "w+b")))
	{
		fprintf (stderr, "reloc: cannot create %s - %s\n", src, error());
		exit(1);
	}
	org += (long)hdr.e_cparhdr * 16;
	fseek(fpIn, org, SEEK_SET);
	while (!feof(fpIn))
	{
		rd = fread(buf, 1, sizeof(buf), fpIn);
		if (fwrite(buf, 1, rd, fpOut) != rd)
		{
			fprintf(stderr, "reloc: write error on %s\n", dst);
			exit(1);
		}
	}
	if (e_crlc = hdr.e_crlc)
	{
		if (!*baseseg)
		{
			printf("Fix-ups needed - base segment (hex): ");
			scanf("%x", &seg);
		}
		else
		{
			seg = (unsigned short)ntoi(baseseg, 16);
		}
		fseek(fpIn, hdr.e_lfarlc, SEEK_SET);
		while (e_crlc--)
		{
			if (fread(&pos, sizeof(pos), 1, fpIn) != 1)
			{
				fprintf(stderr, "reloc: read error during relocation\n");
				exit(1);
			}
			pos = POINTER(DHIGH(pos),DLOW(pos));
			fseek(fpOut, pos, SEEK_SET);
			if (fread(&posout, sizeof(posout), 1, fpOut) != 1)
			{
				fprintf(stderr, "reloc: read error during relocation\n");
				exit(1);
			}
			posout += seg;
			fseek(fpOut, pos, SEEK_SET);
			if (fwrite(&posout, sizeof(posout), 1, fpOut) != 1)
			{
				fprintf(stderr, "reloc: write error during relocation\n");
				exit(1);
			}
		}
	}
	fclose(fpIn);
	fclose(fpOut);
}

int __cdecl main(int argc, char **argv)
{
	char src[128], dst[128];

	if (argc<2 || argc>4)
	{
		fprintf(stderr, "Usage: reloc source [dest]\n");
		exit(1);
	}
	upd(argv[1], ".exe", src);
	upd(src, argc>2?argv[2]:".bin", dst);
	relocate(src, dst, argc == 4?strbskip(argv[3], "-"):"");
	return 0;
}
