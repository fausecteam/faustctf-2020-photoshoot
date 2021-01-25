SERVICE := photoshoot
DESTDIR ?= dist_root
SERVICEDIR ?= /srv/$(SERVICE)

.PHONY: build install

build:
	$(MAKE) -C src

install: build
	install -d -m 755					$(DESTDIR)/etc/uwsgi/apps-enabled
	install -m 444 conf/photoshoot.ini	$(DESTDIR)/etc/uwsgi/apps-enabled
	install -d -m 755					$(DESTDIR)/etc/nginx/sites-enabled
	install -m 444 conf/nginx/photoshoot.conf	$(DESTDIR)/etc/nginx/sites-enabled
	install -d -m 755					$(DESTDIR)$(SERVICEDIR)/
	install -m 555 src/imager			$(DESTDIR)$(SERVICEDIR)/
	install -m 444 src/app.py			$(DESTDIR)$(SERVICEDIR)/
	install -m 444 src/watermark.ppm	$(DESTDIR)$(SERVICEDIR)/
	install -m 444 conf/postgres/db.sql 	$(DESTDIR)$(SERVICEDIR)/
	install -m 555 conf/setup.sh		$(DESTDIR)$(SERVICEDIR)/
	install -d -m 755					$(DESTDIR)/etc/systemd/system/
	install -m 444 conf/postgres/photoshoot-db-setup.service $(DESTDIR)/etc/systemd/system/
	install -d -m 755					$(DESTDIR)$(SERVICEDIR)/templates
	install -d -m 755					$(DESTDIR)$(SERVICEDIR)/static
	install -m 444 src/static/faustctf*.png		$(DESTDIR)$(SERVICEDIR)/static
	install -m 444 src/static/*.jpg				$(DESTDIR)$(SERVICEDIR)/static
	install -m 444 src/static/*.png				$(DESTDIR)$(SERVICEDIR)/static
	install -m 444 src/static/notsuspicious.js	$(DESTDIR)$(SERVICEDIR)/static
	install -m 444 src/templates/*.html			$(DESTDIR)$(SERVICEDIR)/templates
	
# This (trivial) exploit is NOT working, because the number of arguments does not match
#run-exploit-basic:
#	$(MAKE) -C src
#	$(MAKE) -C exploit
#	rm -f out.ppm
#	src/imager 4 0 0 0 4 exploit/exploit_basic.ppm out.ppm
#	grep -o --binary-file=text "FAUST_[^,]*" out.ppm

test: run-exploit-water run-exploit-blur

run-exploit-water:
	$(MAKE) -C src
	$(MAKE) -C exploit
	rm -f out.ppm
	src/imager 4 0 0 4 exploit/exploit_sql_watermark.ppm out.ppm
	grep -o --binary-file=text "FAUST_[^,]*" out.ppm

# two byte, no modification
# TODO: not functional because we need the old (non-2b) address computation
run-exploit-twobyte:
	$(MAKE) -C src
	$(MAKE) -C exploit
	rm -f out.ppm
	src/imager 4 0 0 4 exploit/exploit_sql_twobyte.ppm out.ppm
	grep -o --binary-file=text "FAUST_[^,]*" out.ppm

# this is still broken
run-exploit-blur:
	$(MAKE) -C src
	$(MAKE) -C exploit
	rm -f out.ppm
	src/imager --config 21 3 0 0 1 exploit/exploit_sql_blur.ppm out.ppm
	grep -o --binary-file=text "FAUST_[^,]*" out.ppm
