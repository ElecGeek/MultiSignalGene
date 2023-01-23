DOT?=dot
DOTLANG?=pdf
DOTFLAGS?=

doc			:
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) Wave_gene.gv -o Wave_gene.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) Modulation.gv -o Modulation.$(DOTLANG) 
	$(DOT) $(DOTFLAGS) -T$(DOTLANG) Channels.gv -o Channels.$(DOTLANG) 

clean		:
	rm -f Wave_gene.$(DOTLANG)
	rm -f Modulation.$(DOTLANG)
	rm -f Channels.$(DOTLANG)
