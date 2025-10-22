CC=g++
#CFLAG= -std=c++17 -O2
# CFLAG= -std=c++17 -g -O0
CFLAG= -std=c++17 -O2 -pthread
LIBS = -lgsl -lgslcblas -lm
#ifeq ($(shell uname), Darwin)
#CFLAG += -w -I/opt/homebrew/opt/gsl/include -L/opt/homebrew/opt/gsl/lib
#endif
ROOTFLAG=`root-config --cflags --glibs`

pLIBS = $(shell pkg-config --libs arrow parquet)
pINCLUDE = $(shell pkg-config --cflags arrow parquet)

all: EventBuilder EventBuilder_S
#all: EventBuilder EventBuilder_MT

EventBuilder: EventBuilder.cpp BinaryReader.h class_Hit.h class_DIG.h class_TDC.h misc.h
	$(CC) $(CFLAG) EventBuilder.cpp -o EventBuilder ${ROOTFLAG}

EventBuilder_S: EventBuilder_S.cpp BinaryReader.h class_Hit.h class_DIG.h class_TDC.h misc.h
	$(CC) $(CFLAG) EventBuilder_S.cpp -o EventBuilder_S ${ROOTFLAG} $(LIBS)

# EventBuilder_MT: EventBuilder_MT.cpp BinaryReader.h class_Hit.h class_DIG.h
# 	$(CC) $(CFLAG) -pthread EventBuilder_MT.cpp -o EventBuilder_MT


clean:
	-rm EventBuilder EventBuilder_MT
