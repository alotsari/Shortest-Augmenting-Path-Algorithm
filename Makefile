BOOSTDIR='/usr/include/boost'
LEDALIB='/usr/local/LEDA'
LEDAINCL='/usr/local/LEDA/incl'

compile:
    g++ main.cpp -O3 -o run -I$(BOOSTDIR) -lrt
    g++ leda.cpp -o leda -O2 -I$(LEDAINCL) -L$(LEDALIB) -lleda -lrt
