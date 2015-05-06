#############################################################################
# source code 'make' pattern matching rules
 
$(OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# generate object files
$(OBJ)/%.o: %.cc
	$(CPP) $(CFLAGS) -c $< -o $@
 
# generate assembly object files
$(OBJ)/%.o: %.a
	$(ASCPP) $< -o $@

#############################################################################
