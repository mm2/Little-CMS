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

EXTRA_DIST = test1.icc bad.icc toosmall.icc test2.icc \
             test3.icc test4.icc \
             test5.icc  

check:
	if [ $(top_srcdir) != $(top_builddir) ]; then \
		cp $(top_srcdir)/testbed/*.ic? $(top_builddir)/testbed; \
	fi
	./testcms
	if [ $(top_srcdir) != $(top_builddir) ]; then \
		rm -f $(top_builddir)/testbed/*.ic?; \
	fi


