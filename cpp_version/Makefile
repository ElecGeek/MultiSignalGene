CXX?=g++
CXXFLAGS?=-O2
LDFLAGS?=
DEFAULTWAVE?=SINE
CXX_Version?=-std=c++14
SRCDIR?=
BUILDDIR?=

$(BUILDDIR)multisignalgene_DW_$(DEFAULTWAVE)			:	$(BUILDDIR)main.o $(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o $(BUILDDIR)bundle_signals.o $(BUILDDIR)params_input_midi.o $(BUILDDIR)params_input_mnemos.o $(BUILDDIR)params_output_txt.o $(BUILDDIR)params_output_mnemos.o $(BUILDDIR)parameters.o $(BUILDDIR)params_io_handler.o $(BUILDDIR)sample_step.o $(BUILDDIR)sound_file_output.o $(BUILDDIR)amplitude_handler.o $(BUILDDIR)frequency_handler.o $(BUILDDIR)modulation_handler.o
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)multisignalgene_DW_$(DEFAULTWAVE) $(BUILDDIR)main.o $(BUILDDIR)main_loop_DW_$(DEFAULTWAVE).o $(BUILDDIR)bundle_signals.o $(BUILDDIR)params_input_midi.o $(BUILDDIR)params_input_mnemos.o $(BUILDDIR)params_output_txt.o $(BUILDDIR)params_output_mnemos.o $(BUILDDIR)parameters.o $(BUILDDIR)params_io_handler.o $(BUILDDIR)sample_step.o $(BUILDDIR)sound_file_output.o $(BUILDDIR)amplitude_handler.o $(BUILDDIR)frequency_handler.o $(BUILDDIR)modulation_handler.o -l jack

$(BUILDDIR)main.o					:	$(SRCDIR)main.cpp $(SRCDIR)help_message.hxx $(SRCDIR)sound_file_output.hxx $(SRCDIR)main_loop.hxx $(SRCDIR)params_io_handler.hxx
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

$(BUILDDIR)params_io_handler.o			:	$(SRCDIR)parameters.hxx $(SRCDIR)params_io_handler.hxx $(SRCDIR)params_io_handler.cpp
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)params_io_handler.o $(SRCDIR)params_io_handler.cpp

$(BUILDDIR)parameters.o			:	$(SRCDIR)parameters.hxx $(SRCDIR)parameters.cpp $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)parameters.o $(SRCDIR)parameters.cpp

$(BUILDDIR)params_input_midi.o			:	$(SRCDIR)params_input_midi.hxx $(SRCDIR)params_input_midi.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)params_input_midi.o $(SRCDIR)params_input_midi.cpp

$(BUILDDIR)params_input_mnemos.o			:	$(SRCDIR)params_input_mnemos.hxx $(SRCDIR)params_input_mnemos.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)params_input_mnemos.o $(SRCDIR)params_input_mnemos.cpp

$(BUILDDIR)params_output_txt.o			:	$(SRCDIR)params_output_txt.hxx $(SRCDIR)params_output_txt.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)params_output_txt.o $(SRCDIR)params_output_txt.cpp

$(BUILDDIR)params_output_mnemos.o			:	$(SRCDIR)params_output_mnemos.hxx $(SRCDIR)params_output_mnemos.cpp $(SRCDIR)parameters.hxx $(SRCDIR)amplitude_handler.hxx $(SRCDIR)frequency_handler.hxx
	$(CXX) -c $(CXXFLAGS) $(CXX_Version) -o $(BUILDDIR)params_output_mnemos.o $(SRCDIR)params_output_mnemos.cpp

clean			:
	rm -f $(BUILDDIR)*.o $(BUILDDIR)multisignalgene*

