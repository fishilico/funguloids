#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(funguloids,[1.06-4])
AC_CONFIG_SRCDIR([src/mpak.cpp])
AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_CC

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memmove memset])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/param.h wchar.h])

# OGRE ----------------------------------------------------
PKG_CHECK_MODULES(OGRE, [OGRE >= 1.9.0])
AC_SUBST(OGRE_CFLAGS)
AC_SUBST(OGRE_LIBS)

PKG_CHECK_MODULES(OGRE_OVERLAY, [OGRE-Overlay >= 1.9.0])
AC_SUBST([OGRE_OVERLAY_CFLAGS])
AC_SUBST([OGRE_OVERLAY_LIBS])

# Ogre plugin directory for bin/plugins.cfg
ogreplugindir=`pkg-config --variable=plugindir OGRE`
AC_SUBST(ogreplugindir)
AC_MSG_NOTICE([Ogre plugins found in ${ogreplugindir}])

# OIS ----------------------------------------------------
PKG_CHECK_MODULES(OIS, [OIS >= 0.7])
AC_SUBST(OIS_CFLAGS)
AC_SUBST(OIS_LIBS)


# LUA ----------------------------------------------------
PKG_CHECK_MODULES(LUA, [lua >= 5.1, lua < 5.2],[],[
	PKG_CHECK_MODULES(LUA, [lua5.1 >= 5.1],[],[
		PKG_CHECK_MODULES(LUA, [lua51 >= 5.1, lua51 < 5.2],[],[
			AC_MSG_ERROR(Lua 5.1 not found)
		])
	])
])

AC_SUBST(LUA_CFLAGS)
AC_SUBST(LUA_LIBS)

# SOUND ----------------------------------------------------
# 1. FMOD  -------------------------------------------------
AC_LANG_PUSH([C++])
AC_ARG_WITH(fmod,
[  --with-fmod[[=PREFIX]] specify the prefix of your FMOD Ex installation [[autodetect]]],
[
	if test "x$withval" = "xno"; then
		skip_fmod="yes"
	else
		if test "x$withval" != "x"; then
			fmod_pre="$withval"
			force_fmod="yes"
		fi
	fi
],[
	fmod_pre=
])
if test "x$skip_fmod" != "xyes"; then
	FUNGU_SEARCH_HEADER( [fmod.hpp], [$fmod_pre /usr/local /usr], [ include/fmodex include/fmod] )
	if test "x$FUNGU_FOUND_HEADER" = "xno"; then
		AC_MSG_NOTICE(FMOD Ex not found. Don't worry if you have OpenAL installed or specify its prefix with [[--with-fmod=PREFIX]])
	else
		have_fmod="yes"
		fmod_cflags="$FUNGU_FOUND_HEADER"
		fmod_cppflags="$FUNGU_FOUND_HEADER"
		fmod_libs="-lfmodex"
		fmod_ldflags="-L${FUNGU_FOUND_PREFIX}/lib"
	fi
fi
AC_LANG_POP

# 2. OPENAL  -------------------------------------------------
AC_LANG_PUSH(C)
AC_ARG_WITH(openal,
[  --with-openal[[=PREFIX]] specify the prefix of your OpenAL installation [[autodetect]]],
[
	if test "x$withval" != "xno"; then
		if test "x$withval" != "x"
		then
			openal_pre="$withval"
		fi
	else
		skip_openal="yes"
	fi
],[
	openal_pre=""
])
if test "x$skip_openal" != "xyes"; then
	FUNGU_SEARCH_HEADER( [AL/alut.h], [$openal_pre /usr/local /usr], [include] )
	if test "x$FUNGU_FOUND_HEADER" = "xno"; then
		AC_MSG_ERROR(ALUT (OpenAL) headers not found. You can specify their prefix with [[--with-openal=PREFIX]])
	else
		have_openal="yes"
		openal_cflags="$FUNGU_FOUND_HEADER"
		openal_cppflags="$FUNGU_FOUND_HEADER"
		openal_libs="-lopenal -lalut"
		openal_ldflags="-L${FUNGU_FOUND_PREFIX}/lib"
	fi
fi
AC_LANG_POP

# 3. now we decide which sound library to use -------------------------------------------------
if test "x$have_openal" != "x" ; then
	# we have OpenAL but maybe the user wants FMOD
	if test "x$have_fmod" != "x" -a "x$force_fmod" != "x"; then
		use_fmod="yes"
	else
		use_openal="yes"
	fi
else
	# no OpenAL, maybe FMOD
	if test "x$have_fmod" != "x" ; then
		use_fmod="yes"
	fi
fi

if test "x$use_openal" != "x" ; then
	AC_MSG_NOTICE([OpenAL sound renderer selected.])
	LIBS="${openal_libs} $LIBS"
	LDFLAGS="${openal_ldflags} $LDFLAGS"
	CFLAGS="${openal_cflags} $CFLAGS"
	CPPFLAGS="${openal_cppflags} $CPPFLAGS"
	AC_DEFINE_UNQUOTED([SOUND_SYSTEM_OPENAL], [1], [Using OpenAL for sound output])
	AC_CHECK_LIB([openal], [alcGetString])
	AC_CHECK_LIB([alut], [alutInit])
else
	if test "x$use_fmod" != "x" ; then
		AC_MSG_NOTICE([FMOD Ex sound renderer selected.])
		LIBS="${fmod_libs} $LIBS"
		LDFLAGS="${fmod_ldflags} $LDFLAGS"
		CFLAGS="${fmod_cflags} $CFLAGS"
		CPPFLAGS="${fmod_cppflags} $CPPFLAGS"
		AC_DEFINE_UNQUOTED([SOUND_SYSTEM_FMOD], [1], [Using FMOD for sound output])
		AC_LANG_PUSH(C++)
		AC_CHECK_LIB([fmodex], [System_Create])
		AC_LANG_POP
	else
		AC_MSG_ERROR(No sound library found. Please specify the path to your OpenAL or FMOD installation.)
	fi
fi

if test "x$use_openal" != "x" ; then
	# OPTIONAL OGG/VORBIS SUPPORT -------------------------------------------------
	AC_LANG_PUSH(C)
	AC_ARG_WITH(ogg,
	[  --with-ogg[[=PREFIX]] specify the path to the ogg and vorbis include files [[autodetect]]],
	[
		if test "x$withval" != "xno"; then
			if test "x$withval" != "x"; then
				ogg_dir="$withval"
			fi
		else
			skip_oggvorbis="yes"
		fi
	],[
		ogg_dir=""
	])

	if test "x$skip_oggvorbis" != "xyes"; then
		FUNGU_SEARCH_HEADER( [ogg/ogg.h], [$ogg_dir /usr/local /usr], [include] )
		if test "x$FUNGU_FOUND_HEADER" = "xno"; then
			AC_MSG_NOTICE(ogg/ogg.h not found. Support for Ogg music disabled.)
		else
			FUNGU_SEARCH_HEADER( [vorbis/vorbisfile.h], [$ogg_dir /usr/local /usr], [include] )
			if test "x$FUNGU_FOUND_HEADER" = "xno"; then
				AC_MSG_NOTICE(vorbis/vorbisfile.h not found. Support for Ogg music disabled.)
			else
				AC_MSG_NOTICE([Ogg/Vorbis music support available.])
				CFLAGS="$CFLAGS $FUNGU_FOUND_HEADER"
				CPPFLAGS="$CPPFLAGS $FUNGU_FOUND_HEADER"
				LDFLAGS="$LDFLAGS  -L${FUNGU_FOUND_PREFIX}/lib"
				LIBS="$LIBS -logg -lvorbis -lvorbisfile -lvorbisenc"
				AC_DEFINE_UNQUOTED([SOUND_STREAM_VORBIS], [1], [We have Ogg/Vorbis support])
			fi
		fi
	fi
	AC_LANG_POP

	# OPTIONAL MP3 SUPPORT (LIBMAD) ----------------------------------------------
	AC_LANG_PUSH(C)
	AC_ARG_WITH(mad,
	[  --with-mad[[=PREFIX]] specify the path to the libmad (MP3) include files [[autodetect]]],
	[
		if test "x$withval" != "xno"; then
			if test "x$withval" != "x"; then
				mad_dir="$withval"
			fi
		else
			skip_mad="yes"
		fi
	],[
		mad_dir=""
	])

	if test "x$skip_mad" != "xyes"; then
		FUNGU_SEARCH_HEADER( [mad.h], [$mad_dir /usr/local /usr], [include] )
		if test "x$FUNGU_FOUND_HEADER" = "xno"; then
			AC_MSG_NOTICE(mad.h not found. Support for MP3 music disabled.)
		else
			AC_MSG_NOTICE([MP3 music support available.])
			CFLAGS="$CFLAGS $FUNGU_FOUND_HEADER"
			CPPFLAGS="$CPPFLAGS $FUNGU_FOUND_HEADER"
			LDFLAGS="$LDFLAGS -L${FUNGU_FOUND_PREFIX}/lib"
			LIBS="$LIBS -lmad"
			AC_DEFINE_UNQUOTED([SOUND_STREAM_MP3], [1], [We have MP3 support])
		fi
	fi
	AC_LANG_POP
fi

# set INSTALL dirs --------------------------------------------------------
if test "x$prefix" = "xNONE" -o "x$prefix" = "x"; then
	prefix="/usr/local"
fi

bininstalldir="${prefix}/bin"
gameinstalldir="${prefix}/share/funguloids"
iconinstalldir="${prefix}/share/funguloids/icon"
musicinstalldir="${prefix}/share/funguloids/music"
docinstalldir="${prefix}/share/docs/funguloids"
docimageinstalldir="${prefix}/share/docs/funguloids/docs"
pixmapinstalldir="${prefix}/share/pixmaps"

AC_SUBST(prefix)
AC_SUBST(bininstalldir)
AC_SUBST(gameinstalldir)
AC_SUBST(iconinstalldir)
AC_SUBST(musicinstalldir)
AC_SUBST(docinstalldir)
AC_SUBST(docimageinstalldir)
AC_SUBST(pixmapinstalldir)

AC_DEFINE_UNQUOTED([OGRE_PLUGINS_AND_RESOURCES_PATH], ["${gameinstalldir}/"], [That's where plugins.cfg and resources.cfg are installed])
AC_MSG_NOTICE([Installing Funguloids resources in ${gameinstalldir}])
AC_MSG_NOTICE([Installing executable in ${bininstalldir}])


# optional system-wide HISCORE FILE -------------------------------------------
AC_ARG_ENABLE(shared-highscore,
              AC_HELP_STRING([--enable-shared-highscore], [enable system-wide highscore file (see INSTALL!)]),
              [shared_hs="yes"], [shared_hs=""])

if test "x${shared_hs}" != "x"; then
	highscoredir="/var/games/funguloids"
	highscoregroup="games"
	AC_SUBST(highscoregroup)
	AC_SUBST(highscoredir)
	AC_DEFINE_UNQUOTED([FUNGULOIDS_HIGHSCORE_PATH], ["${highscoredir}/"], [Optional system-wide highscore file])
	AC_MSG_NOTICE([Using system-wide highscore file in ${highscoredir}])
fi

# Add boost
AC_CHECK_LIB([boost_system], [main])

AC_CONFIG_FILES([Makefile src/Makefile include/Makefile bin/Makefile bin/docs/Makefile bin/music/Makefile bin/icon/Makefile ])
AC_OUTPUT

