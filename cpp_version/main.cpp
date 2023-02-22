#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "bundle_signals.hxx"
#include "output_midi.hxx"
#include "input_midi.hxx"
#include "sound_file_output.hxx"
#include "help_message.hxx"

#include <deque>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <chrono>
#include <thread>


extern char *optarg;

unsigned char debug_level;

int main(int argc,char *argv[] )
{
  int opt;
  deque<string>file_inputs;
  deque<string>file_outputs;
  bool run_dummy( false );
  bool has_input( false );
  bool has_output( false );
  bool has_hv( false );
  bool has_cin;
  bool has_cout;
  bool follow_timebeat=false;
  int n_loops( 0 );
  int wait_for_start_in_sec( 0 );
  sample_rate_list the_sr_list;

#ifdef __OUTPUT_SINE_MODE__
  string output_mode("s");
#elif __OUTPUT_PULSES_MODE__
  string output_mode("p");
#elif __OUTPUT_TRIANGLES_MODE__
  string output_mode("e");
#elif __OUTPUT_CONTINUOUS_MODE__
  string output_mode("c");
#else
  #error A default output mode should be specified
#endif

  string filename;
  string jack_peername;
  char channels_number = 0;

  debug_level = 0;

  while (( opt= getopt( argc, argv, "d:o:i:K:f:c:tj:r:l:w:hv" )) != EOF ) 
	{
	  switch ( opt )
		{
		case 'd':
		  debug_level = atoi( optarg );
		  break;
		case 'o':
		  file_outputs.push_back( string( optarg ));
		  has_output = true;
		  break;
		case 'i':
		  file_inputs.push_back( string( optarg ));
		  has_input = true;
		  break;
		case 'f':
		  filename = string( optarg );
		  has_output = true;
		  break;
		case 'K':
		  output_mode = string( optarg );
		  break;
		case 'c':
		  channels_number = atoi( optarg );
		  break;
		case 'j':
		  has_output = true;
		  jack_peername = string( optarg );
		  break;
		case 'r':
		  switch ( atoi( optarg ) )
			{
			case 48000: the_sr_list.add_value( 1 ); break;
			case 96000: the_sr_list.add_value( 2 ); break;
			case 192000: the_sr_list.add_value( 4 ); break;
			default: cout << "Only 48, 96, 192KHz are allowed sample rates, ignored" << endl; break;
			}		  
		  break;
		case 'l':
		  n_loops = atoi( optarg );
		  break;
		case 'w':
		  wait_for_start_in_sec = atoi( optarg );
		  break;
		case 't':
		  follow_timebeat = true;
		  break;
		case 'h':
		  cout << help();
		  has_hv = true;
		  break;
		case 'v':
		  cout << "estim v0.2" << endl;
		  has_hv = true;
		  break;
		}
	}
  if ( (has_input == false) && (has_output == false) )
	{
	  if ( has_hv )
		exit( EXIT_SUCCESS );
	  else {
		cout << "Both input commands channel and output should be provided" << endl;
		exit( EXIT_FAILURE );
	  }
	}
  if ( has_input == false )
	{
	  cout << "Input commands channel should be provided" << endl;
	  exit( EXIT_FAILURE );
	}
  if ( has_output == false )
	{
	  cout << "Output commands channel, output audio or raw data filename should be provided" << endl;
	  exit( EXIT_FAILURE );
	}

  if( debug_level != 0 )
	{
  	  cout << "Debug set to: " << dec << (unsigned short)debug_level << endl;
	}

  if( filename.empty() == false && jack_peername.empty() == false )
	{
	  cout << "Can NOT open both an audio and a file output " << endl;
	  exit( EXIT_FAILURE );
	}

  cout << output_mode << " = ";
  for( auto om_it= output_mode.begin(); om_it != output_mode.end(); om_it++ )
	{
	  switch ( *om_it )
		{
		case 's':
		  cout << "Sine output";
			break;
		case 'p':
		  cout << "Pulses output";
		  break;
		case 'b':
		  cout << "Both output";
		  break;
		case 't':
		  cout << "Triangle output";
		  break;
		}
	  if ( om_it != output_mode.end() )
		cout << ", ";
	}
  cout << " is/are used" << endl;

  cout << "Opening the sound file output module "<< endl;
  sound_file_output_base * sfob; 
  if ( filename.empty() )
	{
	  if( jack_peername.empty() )
		sfob = (sound_file_output_base*) new sound_file_output_dry( follow_timebeat );
	  else
		sfob = (sound_file_output_base*) new sound_file_output_jackaudio( channels_number, jack_peername );
	}
  else
	{
	  if( jack_peername.empty() )
		sfob = (sound_file_output_base*) new sound_file_output_file(filename, follow_timebeat );
	  else
		{
		  cout << "Internal error" << endl;
		  exit( EXIT_FAILURE );		  
		}
	}
  cout << "Sample rate(s) requested by the parameters: " << the_sr_list << endl;
  the_sr_list = sfob->process_sample_rate( the_sr_list );
  cout << "Sample rate(s) validated by the output module: " << the_sr_list << endl;
  const unsigned char sample_rate_id = the_sr_list.get_sample_rate();

  if ( sample_rate_id == 0 )
	{
	  cout << "Sorry no common value has been found" << endl;
	  exit( EXIT_FAILURE );
	}
  cout << "Sample rate is: " << sample_rate_id*48000 << endl;

  cout << "Opening the input and output modules ";
  cout << "with " << channels_number << "channels" << endl;
 
  main_loop signals(sample_rate_id,output_mode,channels_number);

  for( deque<string>::iterator it= file_inputs.begin(); it != file_inputs.end(); ++it )
	// Check here for pipes and other pckeyboard style files
	if ( (*it).compare( "-" ) != 0 )
	  {
		cout << "Opening midi file " << *it << endl; 
		signals += new input_params_midi_file( *it, n_loops );
	  } else {
	  cout << "Opening midi input keyboard" << endl;
	  signals += new input_params_midi_pckeyboard( cin );
	}
  // signals+=new output_params_txt;
  for( deque<string>::iterator it= file_outputs.begin(); it != file_outputs.end(); ++it )
	if ( (*it).compare( "-" ))
	  {
		cout << "Opening text output parameters file " << *it << endl;
		signals += new output_params_txt_file( *it );
	  } else {
	  cout << "Opening text output parameters console output " << endl;
	  signals += new output_params_txt( cout );
	}
  
  // global_run_file grf( signals, filename.c_str() );
  //  global_run_single ers(signals);
  
  clock_t ticks( clock() );
  do
	{
	  cout << "waiting all input parameters channels to be ready" << endl;
	  this_thread::sleep_for(chrono::milliseconds( 500 ));
	} while( signals.is_all_ready() == false );

  // Today, there is oinly one "physical" ouput at a time
  // Keep in mind there can have more in the future
  sfob->set_signals( &signals );
  if( sfob->pre_run() == true )
	{
	  while( wait_for_start_in_sec > 0 )
		{
		  cout << wait_for_start_in_sec << " ";
		  cout.flush();
		  this_thread::sleep_for(chrono::seconds( 1 ));
		  wait_for_start_in_sec -= 1;
		}
	  cout << "Start" << endl;
	  sfob->run();
	  cout << "End" << endl;
	}else
	cerr << "An error occured while running the audio output" << endl;
  delete sfob;
  cout << endl << signals.get_clearing() << endl;
  
  return 0;
}




