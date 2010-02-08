#
# Makefile for building testcms
#

# Don't require all the GNU mandated files
AUTOMAKE_OPTIONS = 1.7 foreign

INCLUDES	=  -I$(top_builddir)/include -I$(top_srcdir)/include -I$(top_srcdir)/src

check_PROGRAMS = testcms

CFLAGS = --pedantic -Wall -std=c99 -O2

testcms_LDADD = $(top_builddir)/src/liblcms2.la 
testcms_LDFLAGS = @LDFLAGS@
testcms_SOURCES = testcms2.c

EXTRA_DIST = sRGBSpac.icm bad.icc toosmall.icc UncoatedFOGRA29.icc \
             USWebCoatedSWOP.icc sRGB_v4_ICC_preference.icc 

check:
	./testcms

dist-hook:
	cp -p $(srcdir)/"sRGB Color Space Profile.icm" $(distdir)
	cp -p $(srcdir)/sRGBSpac.icm $(distdir)
	cp -p $(srcdir)/bad.icc $(distdir)
	cp -p $(srcdir)/toosmall.icc $(distdir)
	cp -p $(srcdir)/UncoatedFOGRA29.icc $(distdir)
	cp -p $(srcdir)/USWebCoatedSWOP.icc $(distdir)
	cp -p $(srcdir)/sRGB_v4_ICC_preference.icc $(distdir)
