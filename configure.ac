AC_PREREQ([2.65])
AC_INIT([webradio], [0.1], [webradio@mikestirling.co.uk], [webradio], [http://mike-stirling.com/webradio/])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/main.cxx])

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

# Check for non-pkg-config libs
AC_CHECK_LIB([m], [sinf], [])
AC_CHECK_LIB([mp3lame], [lame_init], [])

# Check for pkg-config libs
PKG_CHECK_MODULES([libmicrohttpd], [libmicrohttpd])
PKG_CHECK_MODULES([libfftw3], [fftw3f])
PKG_CHECK_MODULES([librtlsdr], [librtlsdr])
PKG_CHECK_MODULES([libpulsesimple], [libpulse-simple])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

