AC_PREREQ(2.59)

dnl Standard boilderplate code
AC_INIT([basem], [1.0], [mayer.florian@web.de])
AM_INIT_AUTOMAKE([1.9 gnu])

AC_PROG_CC

dnl That's just playing around
AC_PROG_AWK
if @<:@ x$AWK != xgawk @:>@; then
	AS_BOX([Consider using gawk, because this is known to be the best implementation!])
	AS_ECHO([$AWK])
fi


dnl Check for libs
AC_CHECK_LIB([m], [pow])

dnl Check for headers
AC_CHECK_HEADERS([string.h, unistd.h, stdlib.h])

dnl Check for functions

AC_FUNC_REALLOC
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
