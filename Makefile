MAKE := make
SRCDIR := src
RM := rm


all: move-lib example
	

move-lib:
	$(MAKE) -C $(SRCDIR) lib-static
	mkdir -p lib
	mv $(SRCDIR)/libmanson.a lib/
	cp $(SRCDIR)/HCS.h lib/

example:
	$(MAKE) -C $(SRCDIR) all

clean:
	$(MAKE) -C $(SRCDIR) clean
	$(RM) -rf lib/
	