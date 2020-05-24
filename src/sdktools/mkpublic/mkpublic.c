/***
*mkpublic.c - Removes internal definitions in .def files to only export public
*             symbols
*
*   Rewrite of Microsoft mkpublic.exe;  (C) leecher@dose.0wnz.at
*
*Purpose:
*   Rewrites definition files
*
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#define FILES_OPEN	1
#define ERR_PARAM	2
#define ERR_INPFIL	4
#define ERR_OUTFIL	8
#define HAS_PARAM	16

int open_files(int argc, char **argv, FILE **pfpIn, FILE **pfpOut, char *param)
{
	int ret = FILES_OPEN, arg;
	int fOpenIn = 0, fOpenOut = 0, fAllParsed = 0;

	for (arg=1; arg<argc; arg++)
	{
		if (!stricmp(argv[arg], param))
			ret|=HAS_PARAM;
		else
		{
			if (fOpenIn == 0)
			{
				if (!(*pfpIn = fopen(argv[arg], "r")))
				{
					printf("Can't open input file: %s\n", argv[arg]);
					return ERR_INPFIL;
				}
				fOpenIn++;
			}
			else if (fOpenOut == 0)
			{
				if (!(*pfpOut = fopen(argv[arg], "w")))
				{
					printf("Can't open output file: %s\n", argv[arg]);
					return ERR_OUTFIL;
				}
				fOpenOut++;
			}
			else fAllParsed = 1;
		}
	}
	if (!fOpenIn || !fOpenOut || fAllParsed)
	{
		char *pSt, *pEn;

		if (pSt = strrchr(argv[0], '\\')) pSt++; else pSt=argv[0];
		pEn = strchr(pSt, '.');
		if (pEn != NULL) *pEn = 0;
		printf("Command syntax: %s infile outfile", pSt);
		if (*param) printf(" <%s>", param);
		printf("\n");
		return ERR_PARAM;
	}
	return ret;
}

int __cdecl main(int argc, char **argv)
{
	FILE *fpIn, *fpOut;
	char line_in[256], line_out[256];
	int ret;

	if ((ret = open_files(argc, argv, &fpIn, &fpOut, "")) == 1)
	{
		while (fgets(line_in, sizeof(line_in), fpIn))
		{
			strcpy(line_out, line_in);
			_strupr(line_out);
			if (!strstr(line_out, ";INTERNAL"))
				fputs(line_in, fpOut);
		}
		fclose(fpIn);
		fclose(fpOut);
		return 0;
	}
	return ret;
}

