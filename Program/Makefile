all : icavrp

CCC = g++
CCFLAGS = -O3 -Wall -std=c++11
# for parallelization
# CCFLAGS = -O3 -Wall -std=c++11 -fopenmp
TARGETDIR=.

OBJS2 = \
        $(TARGETDIR)/ICA.o \
        $(TARGETDIR)/Country.o \
        $(TARGETDIR)/LocalSearch.o \
        $(TARGETDIR)/main.o \
        $(TARGETDIR)/Params.o \
        $(TARGETDIR)/Split.o \
        $(TARGETDIR)/Population.o

$(TARGETDIR)/icavrp: $(OBJS2)
	$(CCC) $(CCFLAGS) -o $(TARGETDIR)/icavrp $(OBJS2)

$(TARGETDIR)/ICA.o: ICA.h ICA.cpp
	$(CCC) $(CCFLAGS) -c ICA.cpp -o $(TARGETDIR)/ICA.o

$(TARGETDIR)/Country.o: Country.h Country.cpp
	$(CCC) $(CCFLAGS) -c Country.cpp -o $(TARGETDIR)/Country.o

$(TARGETDIR)/LocalSearch.o: LocalSearch.h LocalSearch.cpp
	$(CCC) $(CCFLAGS) -c LocalSearch.cpp -o $(TARGETDIR)/LocalSearch.o
	
$(TARGETDIR)/main.o: main.cpp
	$(CCC) $(CCFLAGS) -c main.cpp -o $(TARGETDIR)/main.o

$(TARGETDIR)/Params.o: Params.h Params.cpp
	$(CCC) $(CCFLAGS) -c Params.cpp -o $(TARGETDIR)/Params.o

$(TARGETDIR)/Split.o: Split.h Split.cpp
	$(CCC) $(CCFLAGS) -c Split.cpp -o $(TARGETDIR)/Split.o

$(TARGETDIR)/Population.o: Population.h Population.cpp
	$(CCC) $(CCFLAGS) -c Population.cpp -o $(TARGETDIR)/Population.o

test: icavrp
	./icavrp -i X-n101-k25 -s 1 -cmp 10 -t 20 -p 100 -ii 3 -rr 0.5

clean:
	$(RM) \
    $(TARGETDIR)/main.o \
    $(TARGETDIR)/ICA.o \
    $(TARGETDIR)/Country.o \
    $(TARGETDIR)/LocalSearch.o \
    $(TARGETDIR)/Params.o \
    $(TARGETDIR)/Split.o \
    $(TARGETDIR)/Population.o
     