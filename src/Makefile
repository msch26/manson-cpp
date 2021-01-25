MKDIR := mkdir
BINDIR := ../build
BIN := manson-example
SRC := HCS.cpp 
SRC_MAIN := main.cpp
HEADER := HCS.h
RM := rm
MKDIR := mkdir

LIB_VERSION := 1.0.0

LDFLAGS := 
CXXFLAGS = -std=c++17 -I.

OBJS += $(SRC:.cpp=.o)
OBJS += $(SRC_MAIN:.cpp=.o)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<


#all: binary builddir
all: builddir binary
	echo $^
	@echo 'Finished building: $<'
	@echo ' '	
	

builddir:
	$(MKDIR) -p $(BINDIR)
	
binary: $(OBJS)   
	echo "linking"  $^
	$(CXX) -o $(BINDIR)/$(BIN) $^ $(CXXFLAGS) $(LDFLAGS)


	
clean: 
	$(RM) -f *.o
	$(RM) -f $(BINDIR)/$(BIN)
	$(RM) -rf $(BINDIR)/
	$(RM) -f libmanson.a
	 
#lib-dynamic:
#	$(CXX) -fPIC -c HCS.cpp
#	$(CXX) -shared HCS.o -Wl,--soname,libmanson.so -o libmanson.so
	
lib-static:
	$(CXX) -fPIC -c HCS.cpp
	ar rcs libmanson.a HCS.o
	
	

	
	
	