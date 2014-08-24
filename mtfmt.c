/*
 * Copyright 1993,1994 Globetrotter Software, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Globetrotter Software not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Globetrotter Software makes
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * GLOBETROTTER SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL GLOBETROTTER SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Jim McBeath, Globetrotter Software, jimmc@globes.com
 */
/* mtfmt.c - output formatting stage */

#include <ctype.h>
#include "mtutil.h"

extern char *strrchr ARGS((char *s1, char c));

extern char *MtOutFileName;
extern int MtOutFileNum;

#define NUMREGISTERS 100	/* or use -DNUMREGISTER=nnn in Makefile */

int MtDoFmt;
char *MtStringRegisters[NUMREGISTERS] = {0};
int MtIntRegisters[NUMREGISTERS] = {0};

void
MtSubFmt(mti,fmt,data)
MtInfo *mti;
char *fmt;	/* the formatted string, including % escapes */
char *data;	/* data string */
{
	char *s, *p;
	int i, n;
	int num, num2;
	int posflag, negflag, relflag;
	int dotflag;
	int strflag;
	char *d;
	static char *str=0;
	static int stralloc=0;
#define INIT_STR_SIZE 1000

	if (!MtDoFmt) {
		/* no formatting */
		MtPrintf(mti,"%s",fmt);
		return;
	}
	if (stralloc==0) {
		stralloc = INIT_STR_SIZE;
		str = MtMalloc(stralloc);
	}
	for (p=fmt; *p; p++) {
		if (*p!='%') {
			MtPutc(mti,*p);	/* regular char, output it */
			continue;	/*  and look for next char */
		}
		/* Here to process % sequences */
		p++;
		num = 0;
		num2 = 0;
		if (*p=='+') {
			posflag=1;
			p++;
		} else
			posflag=0;
		if (*p=='-') {
			negflag=1;
			p++;
		} else
			negflag=0;
		relflag = posflag||negflag;
		while (isdigit(*p)) {
			num = num*10 + (*p)-'0';
			p++;
		}
		if (*p=='.') {
			dotflag=1;
			p++;
			while (isdigit(*p)) {
				num2 = num2*10 + (*p)-'0';
				p++;
			}
		} else
			dotflag=0;
		if (negflag)
			num = -num;
		if (*p=='"') {
			strflag=1;
			d = str;
			p++;
			while (*p) {
				if (d-str>stralloc-4) {
					stralloc *= 2;
					str = MtRealloc(str,stralloc);
				}
				if (*p=='"') {
					p++;
					break;
				}
				if (*p=='\\') {
					if (!*++p)
						break;
					*d++ = '\\';
					/* continue, to copy char after \ */
				}
				*d++ = *p++;
			}
			*d = 0;
			MtUnBackslash(str);	/* compress in place */
		} else {
			str[0] = 0;
			strflag=0;
		}
		/* Upper case chars have side effects;
		 * lower case chars just print things out. */
		switch(*p) {
		case '\0':	/* % at end of string, ignore it */
			p--;	/* adjust for p++ in for loop */
			break;
		case '%':
			MtPutc(mti,'%');
			break;
		case 'A':	/* set alternate output file */
			if (p[-1]=='%') relflag=1;
				/* make %A same as %+0A */
			MtSetOutputFile(mti,num,num2,relflag,1);
			break;
		case 'F':	/* set main output file */
			if (p[-1]=='%') relflag=1;
				/* make %F same as %+0F */
			MtSetOutputFile(mti,num,num2,relflag,0);
			break;
		case 'L':	/* store literal data into register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				MtIntRegisters[num] = num2;
			} else {
				if (MtStringRegisters[num])
					MtFree(MtStringRegisters[num]);
				MtStringRegisters[num] = MtStrSave(str);
			}
			break;
		case 'O':	/* dual-register operation */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if N1 out of range */
			if (num2<=0 || num2>=NUMREGISTERS)
				break;	/* ignore if N2 out of range */
			if (posflag) {
				n = MtOpInt(str,MtIntRegisters[num],
					MtIntRegisters[num2]);
				MtIntRegisters[num] = n;
			} else {
				extern char *MtOpStr();
				s = MtOpStr(str,MtStringRegisters[num],
					MtStringRegisters[num2]);
				if (s!=MtStringRegisters[num]) {
					if (MtStringRegisters[num])
						MtFree(MtStringRegisters[num]);
					MtStringRegisters[num] = s;
				}
			}
			break;
		case 'R':	/* store data into register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				/* + means use int registers */
				if (num2<0)
					break;	/* reserved for future use */
				s = data;
				while (*s) {
					while (*s && !isdigit(*s)) s++;
					if (!*s) break;
					i = 0;
					while (isdigit(*s)) {
						i = i*10 + *s-'0';
						s++;
					}
					if (num==0) {
						MtIntRegisters[num] = i;
					}
					if (--num<0)
						break;
				}
				break;
			} else {
				if (MtStringRegisters[num])
					MtFree(MtStringRegisters[num]);
				MtStringRegisters[num] = MtStrSave(data);
			}
			break;
		case 'f':	/* print main outfilename */
			if (p[-1]=='%') relflag=1;
				/* make %f same as %+0f */
			/* num arg is used in printed filename */
			if (!MtOutFileName)
				break;
			if (relflag)
				num += MtOutFileNum;
			s = strrchr(MtOutFileName,'/');
			if (s) s++;
			else s=MtOutFileName;
			MtPrintf(mti,s,num);
			break;
		case 'r':	/* retrieve data from register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				/* use int registers */
				MtPrintf(mti,"%d",MtIntRegisters[num]);
			} else {
				if (MtStringRegisters[num])
					MtPuts(mti,MtStringRegisters[num]);
			}
			break;
		case 's':	/* insert the current data string */
			MtPuts(mti,data);
			break;
		default:
			MtPutc(mti,*p);
			break;
		}
	}
}

/* end */