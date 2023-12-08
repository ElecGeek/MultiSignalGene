DOT?=dot
DOTLANG?=pdf
DOTFLAGS?=
SRCDIR?=
BUILDDIR?=

doc			:
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Overview.gv -o $(BUILDDIR)Overview.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Wave_gene.gv -o $(BUILDDIR)Wave_gene.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Modulation.gv -o $(BUILDDIR)Modulation.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Channels.gv -o $(BUILDDIR)Channels.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Sound_output_software.gv -o $(BUILDDIR)Sound_output_software.$(DOTLANG) 

clean		:
	rm -f $(BUILDDIR)Wave_gene.$(DOTLANG)
	rm -f $(BUILDDIR)Modulation.$(DOTLANG)
	rm -f $(BUILDDIR)Channels.$(DOTLANG)
	rm -f $(BUILDDIR)Modulation.$(DOTLANG)
	rm -f $(BUILDDIR)Sound_output_software.$(DOTLANG)
