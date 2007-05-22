
AC_DEFUN([FUNGU_SEARCH_HEADER],
[
 FUNGU_FOUND_HEADER=""
 FUNGU_FOUND_PREFIX=""
 AC_CHECK_HEADER($1, [$4 got="yes"], [$5 got="no"], $6)
 hdr=`echo $1 | $as_tr_sh`
 if test "x$3" = "x" ; then
   dirs="/"
 else
   dirs="$3"
 fi
 for pre in $2; do
   for post in $dirs; do
     dir="${pre}/${post}"
     if test "x${got}" = "xno"; then
       FUNGU_FOUND_HEADER="no"
       ext_hashdr_cvdir=`echo $dir | $as_tr_sh`
       AC_CACHE_CHECK(
         [for $1 library with -I$dir],
         [ext_cv${ext_hashdr_cvdir}_hashdr_${hdr}],
         [ext_have_hdr_save_cflags=${CFLAGS}
         ext_have_hdr_save_cppflags=${CPPFLAGS}
         CFLAGS="${CFLAGS} -I${dir}"
         CPPFLAGS="${CPPFLAGS} -I${dir}"
         AC_COMPILE_IFELSE(
           [AC_LANG_PROGRAM([#include <$1>])],
           [got="yes"; eval "ext_cv${ext_hashdr_cvdir}_hashdr_${hdr}"="yes"],
           [got="no"; eval "ext_cv${ext_hashdr_cvdir}_hashdr_${hdr}"="no"]
         )
         CFLAGS=$ext_have_hdr_save_cflags
         CPPFLAGS=$ext_have_hdr_save_cppflags]
       )
       if eval `echo 'test x${'ext_cv${ext_hashdr_cvdir}_hashdr_${hdr}'}' = "xyes"`; then
         FUNGU_FOUND_HEADER="-I${dir}"
         FUNGU_FOUND_PREFIX="${pre}"
         got="yes";
         hdr=`echo $1 | $as_tr_cpp`
         AC_DEFINE_UNQUOTED(HAVE_${hdr}, 1,
          [Define this if you have the $1 header]
         )
       fi
     fi
   done
 done
])
