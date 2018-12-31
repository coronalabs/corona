###################################################
#
# Makefile for car
# Creator [Xcode -> Makefile Ver: Feb 14 2007 09:18:41]
# Created: [Mon Oct 26 18:38:58 2009]
#
###################################################

#
# Macros
#

CC = /usr/bin/gcc
CPP = /usr/bin/g++
CC_OPTIONS = -DRtt_ALLOCATOR_SYSTEM -DRtt_NO_ARCHIVE -DRtt_ARCHIVE_DESERIALIZE
CPP_OPTIONS = $(CC_OPTIONS) -fno-operator-names
LNK_OPTIONS = 


#
# INCLUDE directories for car
#

INCLUDE = -I.\
		-I../../librtt/Core\
		-I../../librtt


#
# Build car
#

car : \
		./Rtt_Assert.o\
		./main.o\
		./Rtt_Archive.o
	$(CPP) $(LNK_OPTIONS) \
		./Rtt_Assert.o\
		./main.o\
		./Rtt_Archive.o\
		-o car

clean : 
		rm \
		./Rtt_Assert.o\
		./main.o\
		./Rtt_Archive.o\
		car

install : car
		cp car car

#
# Build the parts of car
#


# Item # 1 -- Rtt_Assert --
./Rtt_Assert.o : ../../librtt/Core/Rtt_Assert.c
	$(CC) $(CC_OPTIONS) ../../librtt/Core/Rtt_Assert.c -c $(INCLUDE) -o ./Rtt_Assert.o


# Item # 2 -- main --
./main.o : ../../tools/car/main.cpp
	$(CPP) $(CPP_OPTIONS) ../../tools/car/main.cpp -c $(INCLUDE) -o ./main.o


# Item # 3 -- Rtt_Archive --
./Rtt_Archive.o : ../../librtt/Rtt_Archive.cpp
	$(CPP) $(CPP_OPTIONS) ../../librtt/Rtt_Archive.cpp -c $(INCLUDE) -o ./Rtt_Archive.o


##### END RUN ####
