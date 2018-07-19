CXX := g++

CFLAGS := -D_DEBUG -D_REENTRANT -D_THREAD_SAFE -DHAVE_CXX_STDHEADERS -DLINUX -D_FILE_OFFSET_BITS=64 -fPIC -ggdb -Wall -lpthread

INCLUDE = -I$(AD_CODE_ROOT)

LIB = -L$(AD_CODE_ROOT)/Library -lmd5 

SRC := AdSearch.cpp
#SRC := genIndex.cpp
OBJ := $(patsubst %.cpp,%.o, $(SRC))
EXE := $(patsubst %.cpp,exe_%, $(SRC))

all: $(EXE)

$(EXE): exe_% : %.o
	$(CXX) $(LIB) $(CFLAGS) $< -o $@

.cpp.o:
	$(CXX) $(INCLUDE) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXE)
	rm -f $(OBJ)
