CXX?=g++
CXXFLAGS?=-O2
LDFLAGS?=
DEFAULTWAVE?=SINE
CXX_Version?=-std=c++14
SRCDIR?=
BUILDDIR?=

$(BUILDDIR)multisignalgene_DW_$(DEFAULTWAVE)			:	$(BUILDDIR)main.o $(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o $(BUILDDIR)bundle_signals.o $(BUILDDIR)input_midi.o $(BUILDDIR)output_midi.o $(BUILDDIR)parameters.o $(BUILDDIR)sample_step.o $(BUILDDIR)sound_file_output.o $(BUILDDIR)amplitude_handler.o $(BUILDDIR)frequency_handler.o $(BUILDDIR)modulation_handler.o
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)multisignalgene_DW_$(DEFAULTWAVE) $(BUILDDIR)main.o $(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o $(BUILDDIR)bundle_signals.o $(BUILDDIR)input_midi.o $(BUILDDIR)output_midi.o $(BUILDDIR)parameters.o $(BUILDDIR)sample_step.o $(BUILDDIR)sound_file_output.o $(BUILDDIR)amplitude_handler.o $(BUILDDIR)frequency_handler.o $(BUILDDIR)modulation_handler.o -l jack

$(BUILDDIR)main.o					:	$(SRCDIR)main.cpp $(SRCDIR)help_message.hxx $(SRCDIR)sound_file_output.hxx $(SRCDIR)main_loop.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)main.o $(SRCDIR)main.cpp

$(BUILDDIR)amplitude_handler.o		:	$(SRCDIR)amplitude_handler.hxx $(SRCDIR)amplitude_handler.cpp
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)amplitude_handler.o $(SRCDIR)amplitude_handler.cpp

$(BUILDDIR)modulation_handler.o		:	$(SRCDIR)modulation_handler.hxx $(SRCDIR)modulation_handler.cpp
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)modulation_handler.o $(SRCDIR)modulation_handler.cpp

$(BUILDDIR)frequency_handler.o		:	$(SRCDIR)frequency_handler.hxx $(SRCDIR)frequency_handler.cpp
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)frequency_handler.o $(SRCDIR)frequency_handler.cpp

$(BUILDDIR)sample_step.o			:	$(SRCDIR)sample_step.hxx $(SRCDIR)sample_step.cpp $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)sample_step.o $(SRCDIR)sample_step.cpp

$(BUILDDIR)bundle_signals.o		:	$(SRCDIR)bundle_signals.hxx	$(SRCDIR)bundle_signals.cpp $(SRCDIR)sample_step.hxx $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)bundle_signals.o $(SRCDIR)bundle_signals.cpp

$(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o		:	$(SRCDIR)main_loop.hxx	$(SRCDIR)main_loop.cpp $(SRCDIR)sample_step.hxx $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx $(SRCDIR)sound_file_output.hxx $(SRCDIR)sound_file_output_buffer.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -D__OUTPUT_$(DEFAULTWAVE)_MODE__ -o $(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o $(SRCDIR)main_loop.cpp

$(BUILDDIR)sound_file_output.o			:	$(SRCDIR)sound_file_output.hxx	$(SRCDIR)sound_file_output.cpp $(SRCDIR)sound_file_output_buffer.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)sound_file_output.o $(SRCDIR)sound_file_output.cpp

$(BUILDDIR)parameters.o			:	$(SRCDIR)parameters.hxx $(SRCDIR)parameters.cpp $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)parameters.o $(SRCDIR)parameters.cpp

$(BUILDDIR)input_midi.o			:	$(SRCDIR)input_midi.hxx $(SRCDIR)input_midi.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)input_midi.o $(SRCDIR)input_midi.cpp

$(BUILDDIR)output_midi.o			:	$(SRCDIR)output_midi.hxx $(SRCDIR)output_midi.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)output_midi.o $(SRCDIR)output_midi.cpp

clean			:
	rm -f $(BUILDDIR)*.o $(BUILDDIR)multisignalgene*

