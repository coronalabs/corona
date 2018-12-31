###################################################
#
# Makefile for app_sign
#
###################################################

#
# Macros
#

CPP = /usr/bin/gcc
CPP = /usr/bin/g++
CC_OPTIONS = -O -DRtt_NO_GUI -DRtt_ALLOCATOR_SYSTEM -DAPP_SIGN_UBUNTU -fpermissive
CPP_OPTIONS = $(CC_OPTIONS) -fno-operator-names
LNK_OPTIONS = -lssl

#
# INCLUDE directories for app_sign
#

INCLUDE = -I.\
		-I../../librtt\
		-I../../librtt/Core\
		-I../../external/rsa\
		-I../../platform/shared


#
# Build app_sign
#

app_sign : \
		./main.o\
		./Rtt_AppSign.o\
		./Rtt_Message.o\
		./Rtt_MD4.o\
		./Rtt_MessageDigest.o\
		./Rtt_Assert.o\
		./Rtt_AuthorizationTicket.o\
		./Rtt_String.o\
		./Rtt_TargetDevice.o\
		./Rtt_VersionTimestamp.o\
		./vlong.o\
		./vlong_io.o\
		./rsa.o\
		./prime.o
	$(CPP) $(LNK_OPTIONS) \
		./main.o\
		./Rtt_AppSign.o\
		./Rtt_Message.o\
		./Rtt_MD4.o\
		./Rtt_MessageDigest.o\
		./Rtt_Assert.o\
		./Rtt_AuthorizationTicket.o\
		./Rtt_String.o\
		./Rtt_TargetDevice.o\
		./Rtt_VersionTimestamp.o\
		./vlong.o\
		./vlong_io.o\
		./rsa.o\
		./prime.o\
		-o app_sign

clean : 
		rm -f \
		./main.o\
		./Rtt_AppSign.o\
		./Rtt_Message.o\
		./Rtt_MD4.o\
		./Rtt_MessageDigest.o\
		./Rtt_Assert.o\
		./Rtt_AuthorizationTicket.o\
		./Rtt_String.o\
		./Rtt_TargetDevice.o\
		./Rtt_VersionTimestamp.o\
		./vlong.o\
		./vlong_io.o\
		./rsa.o\
		./prime.o\
		app_sign

install : app_sign
		cp app_sign app_sign

#
# Build the parts of app_sign
#


# Item # 1 -- main --
./main.o : main.cpp
	$(CPP) $(CPP_OPTIONS) main.cpp -c $(INCLUDE) -o ./main.o


# Item # 2 -- Rtt_Message --
./Rtt_Message.o : ../../librtt/Rtt_Message.cpp
	$(CPP) $(CPP_OPTIONS) ../../librtt/Rtt_Message.cpp -c $(INCLUDE) -o ./Rtt_Message.o


# Item # 3 -- Rtt_MD4 --
./Rtt_MD4.o : ../../librtt/Core/Rtt_MD4.c
	$(CC) $(CC_OPTIONS) ../../librtt/Core/Rtt_MD4.c -c $(INCLUDE) -o ./Rtt_MD4.o


# Item # 4 -- Rtt_MessageDigest --
./Rtt_MessageDigest.o : ../../librtt/Core/Rtt_MessageDigest.cpp
	$(CPP) $(CPP_OPTIONS) ../../librtt/Core/Rtt_MessageDigest.cpp -c $(INCLUDE) -o ./Rtt_MessageDigest.o


# Item # 5 -- Rtt_Assert --
./Rtt_Assert.o : ../../librtt/Core/Rtt_Assert.c
	$(CC) $(CC_OPTIONS) ../../librtt/Core/Rtt_Assert.c -c $(INCLUDE) -o ./Rtt_Assert.o

./Rtt_String.o : ../../librtt/Core/Rtt_String.cpp
	$(CPP) $(CPP_OPTIONS) ../../librtt/Core/Rtt_String.cpp -c $(INCLUDE) -o ./Rtt_String.o

./Rtt_AuthorizationTicket.o : ../../platform/shared/Rtt_AuthorizationTicket.cpp
	$(CPP) $(CPP_OPTIONS) ../../platform/shared/Rtt_AuthorizationTicket.cpp -c $(INCLUDE) -o ./Rtt_AuthorizationTicket.o

./Rtt_TargetDevice.o : ../../platform/shared/Rtt_TargetDevice.cpp
	$(CPP) $(CPP_OPTIONS) ../../platform/shared/Rtt_TargetDevice.cpp -c $(INCLUDE) -o ./Rtt_TargetDevice.o

./Rtt_VersionTimestamp.o : ../../librtt/Core/Rtt_VersionTimestamp.c
	$(CC) $(CC_OPTIONS) ../../librtt/Core/Rtt_VersionTimestamp.c -c $(INCLUDE) -o ./Rtt_VersionTimestamp.o

# Item # 6 -- vlong --
./vlong.o : ../../external/rsa/vlong.cpp
	$(CPP) $(CPP_OPTIONS) ../../external/rsa/vlong.cpp -c $(INCLUDE) -o ./vlong.o

./vlong_io.o : ../../external/rsa/vlong_io.cpp
	$(CPP) $(CPP_OPTIONS) ../../external/rsa/vlong_io.cpp -c $(INCLUDE) -o ./vlong_io.o


# Item # 7 -- rsa --
./rsa.o : ../../external/rsa/rsa.cpp
	$(CPP) $(CPP_OPTIONS) ../../external/rsa/rsa.cpp -c $(INCLUDE) -o ./rsa.o


# Item # 8 -- prime --
./prime.o : ../../external/rsa/prime.cpp
	$(CPP) $(CPP_OPTIONS) ../../external/rsa/prime.cpp -c $(INCLUDE) -o ./prime.o

# Item # 9 -- Rtt_AppSign --
./Rtt_AppSign.o : Rtt_AppSign.cpp
	$(CPP) $(CPP_OPTIONS) Rtt_AppSign.cpp -c $(INCLUDE) -o ./Rtt_AppSign.o
