MKDIR := mkdir
BINDIR := ../build
BIN := hcs-test
SRC := HCS.cpp main.cpp
HEADER := HCS.h
RM := rm
MKDIR := mkdir

LDFLAGS := 
CXXFLAGS = -std=c++17 -I.

#OBJS += $(.cpp=.o)
OBJS += $(SRC:.cpp=.o)

%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

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
	 
