musicinstall_DATA = playlist.lua

EXTRA_DIST = playlist.lua.in

playlist.lua: playlist.lua.in
	cat playlist.lua.in | sed 's,%%musicinstalldir%%,@musicinstalldir@,' > playlist.lua

install-data-hook:
	if test -r MarylandInMay.ogg; then cp MarylandInMay.ogg "@musicinstalldir@"; fi;


distclean: clean

clean:
	-rm -f playlist.lua *~
