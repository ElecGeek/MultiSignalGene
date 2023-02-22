DOT?=dot
DOTLANG?=pdf
DOTFLAGS?=
SRCDIR?=
BUILDDIR?=

doc			:
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Wave_gene.gv -o $(BUILDDIR)Wave_gene.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Modulation.gv -o $(BUILDDIR)Modulation.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) $(SRCDIR)Channels.gv -o $(BUILDDIR)Channels.$(DOTLANG) 

clean		:
	rm -f $(BUILDDIR)Wave_gene.$(DOTLANG)
	rm -f $(BUILDDIR)Modulation.$(DOTLANG)
	rm -f $(BUILDDIR)Channels.$(DOTLANG)
