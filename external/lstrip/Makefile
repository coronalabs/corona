# makefile for lstrip

# change these to reflect your Lua installation (Lua 5.1!)
LUA= /tmp/lhf/lua-5.1
LUAINC= $(LUA)/src
LUALIB= $(LUA)/src
LUASRC= $(LUA)/src

# no need to change anything below here
CFLAGS= $(INCS) $(WARN) -O2 $G
WARN= -ansi -pedantic -Wall

INCS= -I$(LUAINC) -I$(LUASRC)
LIBS= -L$(LUALIB) -llua -lm

MYNAME= lstrip
MYLIB= $(MYNAME)
T= $(MYNAME)
OBJS= $(MYNAME).o
TEST= test.lua

all:	test

test:	$T
	-rm -f core core.*
	$T $(TEST)

$T:	$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(OBJS) core core.* a.out $(MYNAME)

# distribution

FTP= $(HOME)/public/ftp/lua/5.1
D= $(MYNAME)
A= $(MYLIB).tar.gz
TOTAR= Makefile,README,$(MYNAME).c,test.lua

tar:	clean
	tar zcvf $A -C .. $D/{$(TOTAR)}

distr:	tar
	touch -r $A .stamp
	mv $A $(FTP)

diff:	clean
	tar zxf $(FTP)/$A
	diff $D .

# eof
