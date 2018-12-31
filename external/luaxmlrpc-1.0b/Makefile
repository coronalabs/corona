LUA_DIR= /usr/local/share/lua/5.0
VERSION= 1.0b
PKG= luaxmlrpc-$(VERSION)
DIST_DIR= $(PKG)
TAR_FILE= $(PKG).tar.gz
ZIP_FILE= $(PKG).zip
SRCS= README Makefile \
	xmlrpc.lua http.lua cgi.lua test.lua \
	index.html manual.html license.html luaxmlrpc.png

dist: dist_dir
	tar -czf $(TAR_FILE) $(DIST_DIR)
	zip -rq $(ZIP_FILE) $(DIST_DIR)/*
	rm -rf $(DIST_DIR)

dist_dir:
	mkdir $(DIST_DIR)
	cp $(SRCS) $(DIST_DIR)

install:
	mkdir -p $(LUA_DIR)/xmlrpc
	cp xmlrpc.lua http.lua cgi.lua $(LUA_DIR)/xmlrpc

clean:
	rm -f $(TAR_FILE) $(ZIP_FILE)
