# Makefile pour OpenCV
#
# CC-BY Edouard.Thiel@univ-amu.fr - 07/10/2018

# Pour installer OpenCV sous Linux, taper :
#  sudo apt-get install libopencv-dev

SHELL   = /bin/bash
CC      = g++
RM      = rm -f
CFLAGS  = -Wall --std=c++14 $$(pkg-config opencv --cflags)
LIBS    = $$(pkg-config opencv --libs)
OBJ =  tp7*.o Contour.o
#HDR = contour.h

CFILES  := $(wildcard *.cpp)
# OBJ= $(SRC:.cpp=.o)
#EXECS   := $(tp*.cpp)
EXECS   := tp7

#~ %.o : %.cpp
#~ 	$(CC) $(CFLAGS) -c $*.cpp

#~ all :: $(EXECS)

#~ $(EXECS) : % : %.o
#~ 	$(CC) -o $@ $^ $(LIBS)

#~ clean ::
#~ 	$(RM) *.o *~ $(EXECS) tmp*.*


########################################################################

all: $(EXECS)

tp7: $(OBJ)
	$(CC) $(CFLAGS) -o tp7 $(OBJ) $(LIBS)

tp7*.o: tp7*.cpp Contour.h
	$(CC) $(CCFLAGS) -c tp7*.cpp

Contour.o: Contour.cpp Contour.h
	$(CC) $(CFLAGS) -c Contour.cpp

.PHONY : clean

clean ::
	$(RM) *.o *~ $(EXECS) tmp*.*
