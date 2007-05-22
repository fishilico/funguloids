SUBDIRS = docs music icon
EXTRA_DIST = plugins.cfg.in resources.cfg.in hiscore.lst.in

dist_pixmapinstall_DATA = funguloids.png
dist_gameinstall_DATA = gamesettings.cfg
dist_docinstall_DATA = readme.html
gameinstall_DATA = plugins.cfg resources.cfg

plugins.cfg: plugins.cfg.in
	cat plugins.cfg.in | sed 's,%%ogreplugindir%%,@ogreplugindir@/,' > plugins.cfg

resources.cfg: resources.cfg.in
	cat resources.cfg.in | sed 's,%%gameinstalldir%%,@gameinstalldir@/,' > resources.cfg

# copy the exe into this dir only for developing, real installation is done directly in ../bin/Makefile
all:
	cp -a ../src/funguloids .

install-data-hook:
	if test "x@highscoredir@" != "x"; then \
		mkdir -p "@highscoredir@"; \
		if test ! -e "@highscoredir@/hiscore.lst"; then cp hiscore.lst.in "@highscoredir@/hiscore.lst"; fi; \
		chgrp -R @highscoregroup@ "@highscoredir@"; \
		chmod -R g+w "@highscoredir@"; \
	fi; \
	if test -r bootstrap.mpk; then cp bootstrap.mpk "@gameinstalldir@"; fi; \
	if test -r funguloids.mpk; then cp funguloids.mpk "@gameinstalldir@"; fi

uninstall-hook:
	rm -rf "@docinstalldir@"

distclean: clean

clean:
	-rm -f funguloids funguloids.sh plugins.cfg resources.cfg Ogre.log ogre.cfg *~

