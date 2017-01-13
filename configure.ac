AC_PREREQ([2.65])
AC_INIT([webradio], [0.1], [webradio@mikestirling.co.uk], [webradio], [http://mike-stirling.com/webradio/])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/main.cxx])

CXXFLAGS="$CXXFLAGS -std=c++11"

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

# Check for non-pkg-config libs
AC_CHECK_LIB([m], [sinf], [])
AC_CHECK_LIB([mp3lame], [lame_init], [])

# libjsoncpp is pure C++ so try to instantiate an object
#AC_MSG_CHECKING([for libjsoncpp])
#LDFLAGS_SAVE=$LDFLAGS
#LDFLAGS="$LDFLAGS -ljsoncpp"
#AC_LINK_IFELSE(
#	[AC_LANG_PROGRAM([#include <json/json.h>], [Json::Value dummy])],
#	[libjsoncpp_LIBS="-ljsoncpp"] [HAVE_LIBJSONCPP=1],
#	[AC_MSG_ERROR([libjsoncpp is not installed.])])
#LDFLAGS=$LDFLAGS_SAVE
#AC_SUBST([libjsoncpp_LIBS])
#AC_MSG_RESULT([yes])

# Check for pkg-config libs
PKG_CHECK_MODULES([libmicrohttpd], [libmicrohttpd])
PKG_CHECK_MODULES([libfftw3], [fftw3f])
PKG_CHECK_MODULES([librtlsdr], [librtlsdr])
PKG_CHECK_MODULES([libpulsesimple], [libpulse-simple])
PKG_CHECK_MODULES([libjsoncpp], [jsoncpp])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

