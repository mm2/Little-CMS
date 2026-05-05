USER_PLUGINS =
if COND_FASTFLOAT
    USER_PLUGINS += fast_float
endif

if COND_THREADED
    USER_PLUGINS += threaded
endif

DIST_SUBDIRS = fast_float threaded
SUBDIRS = $(USER_PLUGINS)
