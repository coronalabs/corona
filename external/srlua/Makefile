# makefile for srlua

# change these to reflect your Lua installation
LUA= ../lua-5.1.3
LUAINC= $(LUA)/src
LUALIB= $(LUA)/src

# no need to change anything below here
CFLAGS= $(INCS) $(WARN) -O2 $G
WARN= -ansi -pedantic -Wall

INCS= -I$(LUAINC)
LIBS= -L$(LUALIB) -llua -lm -ldl

OBJS= srlua.o
TEST= test.lua

T= a.out
S= srlua

all:	test

test:	$T
	./$T *

$T:	$S $(TEST) glue
	./glue $S $(TEST) $T
	chmod +x $T

$S:	$(OBJS)
	$(CC) -o $@ -Wl $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $T $S core core.* a.out *.o glue

# distribution

FTP= $(HOME)/public/ftp/lua/5.1
D= srlua
A= $D.tar.gz
TOTAR= Makefile,README,glue.c,glue.h,srlua.c,$(TEST)

tar:	clean
	tar zcvf $A -C .. $D/{$(TOTAR)}

distr:	tar
	touch -r $A .stamp
	mv $A $(FTP)

diff:	clean
	tar zxf $(FTP)/$A
	diff $D .

# eof
