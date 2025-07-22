CC=g++
CFLAG= -std=c++17 -g -O0
# CFLAG= -std=c++17 -O2 -pthread  -lgsl -lgslcblas -lm
#ifeq ($(shell uname), Darwin)
#CFLAG += -w -I/opt/homebrew/opt/gsl/include -L/opt/homebrew/opt/gsl/lib
#endif
ROOTFLAG=`root-config --cflags --glibs`

pLIBS = $(shell pkg-config --libs arrow parquet)
pINCLUDE = $(shell pkg-config --cflags arrow parquet)

all: EventBuilder 

EventBuilder: EventBuilder.cpp BinaryReader.h Hit.h
	$(CC) $(CFLAG) EventBuilder.cpp -o EventBuilder ${ROOTFLAG}


clean:
	-rm EventBuilder
