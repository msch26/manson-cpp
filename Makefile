MAKE := make
SRCDIR := src



all:
	$(MAKE) -C $(SRCDIR)
	
clean:
	$(MAKE) -C $(SRCDIR) clean
	