T= lcrypto
V= 0.2.0
CONFIG= ./config

include $(CONFIG)

ifeq "$(LUA_VERSION_NUM)" "500"
COMPAT_O= $(COMPAT_DIR)/compat-5.1.o
endif
OBJS= src/$T.o $(COMPAT_O)
SRCS= src/$T.h src/$T.c

lib: src/$(LIBNAME)

src/$(LIBNAME): $(OBJS)
	export MACOSX_DEPLOYMENT_TARGET="10.3"; $(CC) $(CFLAGS) $(LIB_OPTION) -o src/$(LIBNAME) $(OBJS) $(OPENSSL_LIBS)

$(COMPAT_DIR)/compat-5.1.o: $(COMPAT_DIR)/compat-5.1.c
	$(CC) -c $(CFLAGS) -o $@ $(COMPAT_DIR)/compat-5.1.c

install: src/$(LIBNAME)
	mkdir -p $(LUA_LIBDIR)
	cp src/$(LIBNAME) $(LUA_LIBDIR)
	cd $(LUA_LIBDIR); ln -f -s $(LIBNAME) $T.so

clean:
	rm -f src/$(LIBNAME) $(OBJS) $(COMPAT_O)
