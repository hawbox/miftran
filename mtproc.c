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
/* mtproc.c - top level parse/translate processing routines */

#include "mtutil.h"
#include "mttran.h"

/* Output substitution keys */
MtTypeSubNameTran MtTypeSubNameTranTab[] = {
#define MT_O_STRING 1
	{ MT_O_STRING, "string" },
#define MT_O_PGFNUMSTRING 2
	{ MT_O_PGFNUMSTRING, "pgfnumstring" },
#define MT_O_MARKERTEXT 10
	{ MT_O_MARKERTEXT, "markertext" },
#define MT_O_XREFTEXT 20
	{ MT_O_XREFTEXT, "xreftext" },
#define MT_O_XREFEND 21
	{ MT_O_XREFEND, "xrefend" },
#define MT_O_CHAR 30
	{ MT_O_CHAR, "char" },
#define MT_O_TAB 31
	{ MT_O_TAB, "tab" },
#define MT_O_HARDRETURN 32
	{ MT_O_HARDRETURN, "hardreturn" },
#define MT_O_EMDASH 33
	{ MT_O_EMDASH, "emdash" },
#define MT_O_STARTPGF 100
	{ MT_O_STARTPGF, "startpgf" },
#define MT_O_ENDPGF 101
	{ MT_O_ENDPGF, "endpgf" },
#define MT_O_SWITCHPGF 102
	{ MT_O_SWITCHPGF, "switchpgf" },
#define MT_O_STARTFONT 200
	{ MT_O_STARTFONT, "startfont" },
#define MT_O_ENDFONT 201
	{ MT_O_ENDFONT, "endfont" },
#define MT_O_STARTPSFONT 202
	{ MT_O_STARTPSFONT, "startpsfont" },
#define MT_O_STARTFILE 300
	{ MT_O_STARTFILE, "startfile" },
#define MT_O_ENDFILE 301
	{ MT_O_ENDFILE, "endfile" },
#define MT_O_AFRAMEID 400
	{ MT_O_AFRAMEID, "aframeid" },	/* during definition */
#define MT_O_AFRAMEFILE 401
	{ MT_O_AFRAMEFILE, "aframefile" },	/* during definition */
#define MT_O_AFRAME 402
	{ MT_O_AFRAME, "aframe" },	/* reference to an aframe */
	{ 0 }
	
};

/* Processing functions called from the translation tables below */
/* On error, the function shouls set mti->tranerror=1 and return */

static
void
CheckPgfStart(mti)
MtInfo *mti;
{
	if (mti->needpgfstart && mti->pgftag) {
		MtSubSid(mti,MT_O_STARTPGF,mti->pgftag,"");
		mti->needpgfstart = 0;
		mti->needpgfend = 1;
	}
}

static
void
CheckEndFont(mti)
MtInfo *mti;
{
	if (mti->fonttag) {
		MtSubSid(mti,MT_O_ENDFONT,mti->fonttag,"");
		mti->fonttag = 0;
	}
}

void
MtProcString(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtSubSid(mti,MT_O_STRING,mti->pgftag,s);
}

void
MtProcPgfNumString(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtSubSid(mti,MT_O_PGFNUMSTRING,mti->pgftag,s);
}

void
MtProcMarkerType(mti)
MtInfo *mti;
{
	mti->markertype = mti->args[0].i;
}

void
MtProcMarkerText(mti)
MtInfo *mti;
{
	char *s;
	char mnumstr[10];

	s = mti->args[0].s;
	sprintf(mnumstr,"%d",mti->markertype);
	MtSubStr(mti,MT_O_MARKERTEXT,mnumstr,s);
}

void
MtProcXrefText(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_XREFTEXT,(MtSid)0,s);
}

void
MtProcXrefEnd(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_XREFEND,(MtSid)0,"");
}

void
MtProcFTag(mti)
MtInfo *mti;
{
	char *s;

	CheckEndFont(mti);
	s = mti->args[0].s;
	if (s && *s) {
		CheckPgfStart(mti);
		mti->fonttag = MtStringToSid(s);
		MtSubSid(mti,MT_O_STARTFONT,mti->fonttag,"");
	}
}

void
MtProcFPostScriptName(mti)
MtInfo *mti;
{
	char *s;

	CheckEndFont(mti);
	s = mti->args[0].s;
	if (s && *s) {
		CheckPgfStart(mti);
		mti->fonttag = MtStringToSid(s);
		MtSubSid(mti,MT_O_STARTPSFONT,mti->fonttag,"");
	}
}

void
MtProcChar(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtMakeLower(s);
	if (strcmp(s,"tab")==0)
		MtSubSid(mti,MT_O_TAB,mti->pgftag,"");
	else if (strcmp(s,"hardreturn")==0)
		MtSubSid(mti,MT_O_HARDRETURN,mti->pgftag,"");
	else if (strcmp(s,"emdash")==0)
		MtSubSid(mti,MT_O_EMDASH,mti->pgftag,"");
	else
		MtSubSid(mti,MT_O_CHAR,mti->pgftag,s);
}

void
MtProcParaLinePost(mti)
MtInfo *mti;
{
	CheckEndFont(mti);
}

static
void
SwitchPgf(mti,from,to)
MtInfo *mti;
MtSid from, to;
{
	char *fromstr, *tostr;
	char pbuf[500];
	int t;

	fromstr = MtSidToString(from);
	tostr = MtSidToString(to);
	sprintf(pbuf,"%s.%s",fromstr,tostr);
	t = MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
	if (!t) {
		sprintf(pbuf,"%s.*",fromstr);
		MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
		sprintf(pbuf,"*.%s",tostr);
		MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
	}
}

void
MtProcPgfTag(mti)
MtInfo *mti;
{
	char *type;
	MtSid newpgftag;

	type = mti->args[0].s;
	newpgftag = MtStringToSid(type);
	if (newpgftag != mti->pgftag) {
		SwitchPgf(mti,mti->pgftag,newpgftag);
		mti->pgftag = newpgftag;
	}
	mti->needpgfstart = 1;
}

void
MtProcParaPre(mti)
MtInfo *mti;
{
	mti->needpgfstart = 1;
	mti->needpgfend = 0;
}

void
MtProcParaPost(mti)
MtInfo *mti;
{
	if (mti->needpgfend) {
		MtSubSid(mti,MT_O_ENDPGF,mti->pgftag,"");
		mti->needpgfend = 0;
	}
}

void
MtProcAframeId(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_AFRAMEID,(MtSid)0,buf);
}

void
MtProcImportObFile(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_AFRAMEFILE,(MtSid)0,s);
}

void
MtProcAframe(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_AFRAME,(MtSid)0,buf);
}

/* Translation tables to drive calling the action functions above */

MtSidTran MarkerTranTab[] = {
	{ "MType", 0, MtProcMarkerType, 0, 0 },
	{ "MText", 0, MtProcMarkerText, 0, 0 },
	{ 0 }
};

MtSidTran XrefTranTab[] = {
	{ "XRefSrcText", 0, MtProcXrefText, 0, 0 },
	{ 0 }
};

MtSidTran FontTranTab[] = {
	{ "FTag", 0, MtProcFTag, 0, 0 },
	{ "FPostScriptName", 0, MtProcFPostScriptName, 0, 0 },
	{ 0 }
};

MtSidTran ParaLineTranTab[] = {
	{ "String", 0, MtProcString, 0, 0 },
	{ "Char", 0, MtProcChar, 0, 0 },
	{ "Marker", 0, 0, 0, MarkerTranTab },
	{ "XRef", 0, 0, 0, XrefTranTab },
	{ "XRefEnd", 0, MtProcXrefEnd, 0, 0 },
	{ "Font", 0, 0, 0, FontTranTab },
	{ "AFrame", 0, 0, MtProcAframe, 0 },
	{ 0 }
};

MtSidTran ParaPgfTranTab[] = {
	{ "PgfTag", 0, MtProcPgfTag, 0, 0 },
	{ 0 }
};

MtSidTran ParaTranTab[] = {
	{ "Pgf", 0, 0, 0, ParaPgfTranTab },
	{ "PgfTag", 0, MtProcPgfTag, 0, 0 },
	{ "PgfNumString", 0, MtProcPgfNumString, 0, 0 },
	{ "ParaLine", 0, 0, MtProcParaLinePost, ParaLineTranTab },
	{ 0 }
};

MtSidTran TextFlowTranTab[] = {
	{ "Para", 0, MtProcParaPre, MtProcParaPost, ParaTranTab },
	{ 0 }
};

MtSidTran ImportObjectTranTab[] = {
	{ "ImportObFile", 0, 0, MtProcImportObFile, 0 },
	{ 0 }
};

MtSidTran FrameTranTab[] = {
	{ "ID", 0, 0, MtProcAframeId, 0 },
	{ "ImportObject", 0, 0, 0, ImportObjectTranTab },
	{ 0 }
};

MtSidTran AFramesTranTab[] = {
	{ "Frame", 0, 0, 0, FrameTranTab },
	{ 0 }
};

MtSidTran TopTranTab[] = {
	{ "TextFlow", 0, 0, 0, TextFlowTranTab },
	{ "AFrames", 0, 0, 0, AFramesTranTab },
	{ 0 }
};

int
MtProcTop(mti)
MtInfo *mti;
{
	int t;

	MtPrepareTranTab(TopTranTab);
	MtSubSid(mti,MT_O_STARTFILE,(MtSid)0,mti->ifilename);
	t =  MtTran(mti,TopTranTab);
	if (t) return t;
	if (mti->pgftag) {
		SwitchPgf(mti,mti->pgftag,0);
		mti->pgftag = 0;
	}
	MtSubSid(mti,MT_O_ENDFILE,(MtSid)0,mti->ifilename);
	return 0;
}

/* end */
