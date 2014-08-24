#Makefile for miftran
#Copyright 1993,1994 Globetrotter Software; see source files for full copyright.
#Jim McBeath (jimmc@globes.com)

PROGRAM = miftran

#name of the directory we are in, for use by 'make kit'
DIR = miftran

CDEFINES =

CCOPTS = -g

CFLAGS = $(INCLUDES) $(CDEFINES) $(CCOPTS)

LIBS =

KITFILES = README Makefile $(SRCS) *.h TODO miftran.mif fixindex fixref \
	html/Makefile html/miftran.rc htmlref/*.html

SRCS =	main.c \
	mtcmd.c \
	mtfmt.c \
	mtlex.c \
	mtop.c \
	mtout.c \
	mtproc.c \
	mtrc.c \
	mtsub.c \
	mttran.c \
	mtutil.c

OBJS =	main.o  \
	mtcmd.o \
	mtfmt.o \
	mtlex.o \
	mtop.o \
	mtout.o \
	mtproc.o \
	mtrc.o \
	mtsub.o \
	mttran.o \
	mtutil.o

default:	all

all:	prog

prog: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LIBS)

depend:;	makedepend $(SRCS)

refhtml:	htmlref/chap1.html

htmlref/chap1.html:	miftran.mif
	cd html; make
	cp -p html/*.html htmlref/

kit:	$(DIR).tgz

$(DIR).tgz:	$(KITFILES) refhtml
	rm -f kit.list
	for f in $(KITFILES); do echo $(DIR)/$$f >> kit.list; done
	cd ..; tar cvf $(DIR)/$(DIR).tar `cat $(DIR)/kit.list`
	ls -l $(DIR).tar
	gzip -9 $(DIR).tar
	mv $(DIR).tar.gz $(DIR).tgz
	ls -l $(DIR).tgz

clean:;	rm -f $(PROGRAM) $(OBJS) core $(DIR).tar $(DIR).tar.gz $(DIR).tgz

###
# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: mtutil.h mtinfo.h /usr/include/stdio.h mtlex.h mtcmd.h version.h
mtcmd.o: mtutil.h mtinfo.h /usr/include/stdio.h mtlex.h mtcmd.h
mtfmt.o: /usr/include/ctype.h mtutil.h mtinfo.h /usr/include/stdio.h
mtlex.o: mtutil.h mtinfo.h /usr/include/stdio.h mtlex.h
mtout.o: /usr/include/varargs.h mtutil.h mtinfo.h /usr/include/stdio.h
mtproc.o: mtutil.h mtinfo.h /usr/include/stdio.h mttran.h
mtrc.o: mtutil.h mtinfo.h /usr/include/stdio.h mttran.h
mtsub.o: /usr/include/ctype.h mtutil.h mtinfo.h /usr/include/stdio.h
mttran.o: mtutil.h mtinfo.h /usr/include/stdio.h mtcmd.h mttran.h
mtutil.o: /usr/include/stdio.h /usr/include/varargs.h /usr/include/ctype.h
mtutil.o: mtinfo.h