##############################################################################
## Makefile.am -- Process this file with automake to produce Makefile.in
## Revision: $Id: Makefile.am 439 2013-12-02 13:30:22Z kulshres $
##
## Copyright (C)  Andrea Walther, Andreas Kowarz, Kshitij Kulshreshtha
##
## This file is part of ADOL-C. This software is provided as open source.
## Any use, reproduction, or distribution of the software constitutes 
## recipient's acceptance of the terms of the accompanying license file.
## 
##############################################################################

AM_CFLAGS                 = @ac_adolc_cflags@ -DADOLC_INTERNAL=1
AM_CXXFLAGS               = @ac_adolc_cxxflags@ -DADOLC_INTERNAL=1
AM_LDFLAGS                = -no-undefined
AM_CPPFLAGS               = -I$(top_builddir)/ADOL-C/include -I$(top_srcdir)/ADOL-C/include -I$(top_srcdir)/ADOL-C/src

MAINTAINERCLEANFILES      = Makefile.in *~ *.orig

noinst_LTLIBRARIES        = libdrivers.la

libdrivers_la_SOURCES     = drivers.c driversf.c odedrivers.c odedriversf.c \
                            taylor.c
