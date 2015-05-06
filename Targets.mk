#############################################################################
# make target definitions

default: $(LIBRARIES) $(PROGRAMS)

libs:	$(LIBRARIES)

bin:	$(PROGRAMS)

# Clean object

xobj:
	rm -f $(OBJ)/*.o 

# Clean MOC files

xmoc:
	rm -f $(BASE)/moc*.cc

############################################################################
