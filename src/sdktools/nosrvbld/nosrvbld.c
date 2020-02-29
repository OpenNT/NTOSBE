/***
*nosrvbld.c - Creates message class files from SKL and MSG files
*
*   Rewrite of Microsoft nosrvbld.exe;  (C) leecher@dose.0wnz.at
*
*Purpose:
*   Creates message class files
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LNMAX 160                      /* Max length of a line. */
#define LVMAX 4                        /* Max length of the message level. */
#define FNMAX 64                       /* Max length of the file spec. */

void trimln(char *line);

int __cdecl main(int argc, char **argv)
{
	char skl_file[FNMAX], skl_name[FNMAX], msg_file[FNMAX], idx_file[FNMAX], cl_file[FNMAX]; 
	char skl_basen[8+3+1];
	char sklline[LNMAX], idxline[LNMAX], msgline[LNMAX], clline[LNMAX];
	char defline[LNMAX], msgstatus[LNMAX], idxcompnm[16], msgbuf[LNMAX];
	char rectype[8], num[2], *p;
	long idxlvl, msglvl, msgoffs;
	int def_msgnr, msg_msgnr, i, use=0;
	FILE *fpSkl, *fpMsg, *fpIdx, *fpCl = NULL;

	if (argc<2)
	{
		printf ("\nUsage:  NOSRVBLD <SKL File> [MSG file]\n");
		printf ("        <SKL File> = Skeleton file (contains message #'s & Label names)\n");
		printf ("        [MSG file] = (optional) message file;\n");
		printf ("                     if omitted G:\\MESSAGES\\USA.MSG  will be used.\n");
		return 1;
	}

	strcpy(skl_file, argv[1]);
	strcpy(skl_name, skl_file);
	for (i=0; skl_name[i]; i++)
		if (skl_name[i]=='.') break;
	skl_name[i]=0;
	if (argc>2)
		strcpy(msg_file, argv[2]);
	else
		strcpy(msg_file, "G:\\messages\\usa.msg");
	
	printf (";  Creating .CT? files for %s ", skl_file);
	printf ("using %s file of messages. \n", msg_file);
	strcpy (idx_file, msg_file);
	strcpy (idx_file+strlen(idx_file)-3, "IDX");
	if (!(fpSkl = fopen(skl_file, "r")))
	{
		printf ("\nCouldn't open %s . \n", skl_file);
		return 1;
	}
	if (!(fpMsg = fopen(msg_file, "r")))
	{
		printf ("\nCouldn't open %s . \n", msg_file);
		return 1;
	}
	if (!(fpIdx = fopen(idx_file, "r")))
	{
		printf ("\nCouldn't open %s . \n", idx_file);
		return 1;
	}
	printf (";  SKL, MSG and IDX files have been opened for reading.\n");

	while (!feof(fpSkl))
	{
		fgets(sklline, sizeof(sklline), fpSkl);
		trimln(sklline);
		sscanf(sklline, "%s %1s", rectype,  num);

		if (!stricmp(rectype, ":end")) break;
		if (sklline[0] == ':')
		{
			if (!stricmp(rectype, ":class")) 
			{
				if (fpCl) fclose(fpCl);
				strcpy(cl_file, skl_file);
				strcpy (cl_file+strlen(cl_file)-3, "cl");
				strcat (cl_file, num);
				fgets(sklline, sizeof(sklline), fpSkl);
				trimln(sklline);
				fgets(idxline, sizeof(idxline), fpIdx);
				sscanf(idxline, "%ld", &idxlvl);
				fgets(msgline, sizeof(msgline), fpMsg);
				sscanf(msgline, "%ld", &msglvl);
				if (idxlvl != msglvl)
				{
					printf("Header levels in message and Index files do not match.\n");
					printf("Operation aborted!\n");
					return 1;
				}
				if (!(fpCl = fopen(cl_file, "w")))
				{
					printf ("\nCouldn't open %s . \n", skl_file);
					return 1;
				}
				printf (";  Writing to %s\n", cl_file);
				fprintf (fpCl, "%c %s \n\n", ';', cl_file);
				continue;
			} else 
			if (!stricmp(rectype, ":def")) 
			{
				fputs("\n;_______________________\n\n", fpCl);
				use = 1;
				strcpy(defline, sklline+4);
				sscanf(defline, "%d %s", &def_msgnr, msgstatus);
				strcpy(skl_basen, skl_file);
				if (p = strrchr(skl_basen, '\\'))
					strcpy(skl_basen, ++p);
				skl_basen[strlen(skl_basen)-4]=0;
			} else 
			if (!stricmp(rectype, ":use")) 
			{
				fputs("\n;_______________________\n\n", fpCl);
				use = 1;
				strcpy(defline, sklline+4);
				sscanf(defline, "%d %s %s", &def_msgnr, skl_basen, msgstatus);
			} else {
				printf ("Error - unknown command: %s\n", rectype);
				printf ("Output file incomplete!!\n");
				return 1;
			}

			rewind(fpIdx);
			fgets(idxline, sizeof(idxline), fpIdx);
			while (!feof(fpIdx))
			{
				fgets(idxline, sizeof(idxline), fpIdx);
				sscanf(idxline, "%s %lX", idxcompnm, &msgoffs);
				if (!stricmp(idxcompnm, skl_basen)) break;
			}
			if (!stricmp(idxcompnm, skl_basen))
				fseek(fpMsg, msgoffs, SEEK_SET);
			else
			{
				printf ("Can not find %s in index file", skl_basen);
				return 1;
			}
			fgets(msgline, sizeof(msgline), fpMsg);
			while (use && !feof(fpMsg) && !feof(fpSkl))
			{
				fgets(msgline, sizeof(msgline), fpMsg);
				if (!feof(fpMsg)) sscanf(msgline, "%d",  &msg_msgnr);
				if (def_msgnr == msg_msgnr)
				{
					use = 0;
					strcpy(clline, msgstatus);
					strcat(clline, " DB   ");
					strncpy(&msgbuf[11], &msgline[11], LNMAX);
					strcat(clline, &msgbuf[11]);
				}
				else use=1;
			}
			fputs(clline, fpCl);
			if (feof(fpMsg))
			{
				strcpy(clline, ";_________\n");
				fputs(clline, fpCl);
			}
			else
			{
				fgets(clline, sizeof(clline), fpMsg);
				while (!isdigit(clline[0]) && !isalpha(clline[0]) && !feof(fpMsg))
				{
					char buf[LNMAX];

					strcpy(buf, "\tDB   ");
					strcat(buf, clline);
					fputs(buf, fpCl);
					fgets(clline, sizeof(clline), fpMsg);
				}
				rewind(fpMsg);
				if (feof(fpSkl))
					printf ("End of the %s file\n", skl_file);

			}
		}
	}

	clearerr(fpSkl);
	if (fpCl) fclose(fpCl);
	fclose(fpMsg);
	fclose(fpIdx);
	fclose(fpSkl);
	printf (";  Message files for %s completed.\n", skl_name);
	return 0;
}

void trimln(char *line)
{
	while (*line)
	{
		if (*line=='\n')
		{
			*line=0;
			return;
		}
		line++;
	}
}