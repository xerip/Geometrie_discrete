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

CFILES  := $(wildcard *.cpp)
EXECS   := $(CFILES:%.cpp=%)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $*.cpp

all :: $(EXECS)

$(EXECS) : % : %.o
	$(CC) -o $@ $^ $(LIBS)

clean ::
	$(RM) *.o *~ $(EXECS) tmp*.*


