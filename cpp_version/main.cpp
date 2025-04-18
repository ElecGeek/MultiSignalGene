#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "bundle_signals.hxx"
#include "params_output_txt.hxx"
#include "params_output_mnemos.hxx"
#include "params_input_midi.hxx"
#include "sound_file_output.hxx"
#include "sound_jack_output.hxx"
#include "samplerate_handler.hxx"
#include "params_io_handler.hxx"
#include "help_message.hxx"

#include <deque>
#include <string>
#include <string_view>
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
#if defined(__linux__)
  if ( getuid() == 0 || geteuid() == 0 )
	{
	  cout << "ERROR: this software should not run as root" << endl;
	  exit( EXIT_FAILURE );
	}
#elif defined(__CYGWIN__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__APPLE__)
  cout << "This software SHOULD NOT run as root" << endl;
  if ( getuid() != 0 && geteuid() != 0 )
	cout << "It looks like OK, but I have no way to verify the test" << endl;
  else
	cout << "Please leave, until you are sure you are not root" << endl;
#elif _WIN32
  	cout << "This software should not run as root" << endl;
	cout << "You have to ensure manually" << endl;
	cout << "as I have not been planning time to implement a test on WIndows" << endl;
#else
  	cout << "This software should not run as root" << endl;
	cout << "You have to ensure manually, as your plateform has not been detected" << endl;
#endif

  int opt;
  params_io_handler params_io;
  // To be removed when the params handler classes are in full production
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
  const map< string_view, unsigned long > known_sr_list = {
	{ "3000", 3 }, { "3K", 3 },
	{ "6000", 6 }, { "6K", 6 },
	{ "12000", 12 }, { "12K", 12 },
	{ "24000", 24 }, { "24K", 24 },
	{ "48000", 48 }, { "48K", 48 }, { "1", 48 },
	{ "96000", 96 }, { "96K", 96 }, { "2", 96 },
	{ "192000", 192 }, { "192K", 192 }, { "4", 192 }
  };
  map< string_view, unsigned long >::const_iterator known_sr_list_iter;
  string output_mode;

  deque<string> jack_data_list;
  char channels_number = 0;

  debug_level = 0;

  while (( opt= getopt( argc, argv, "d:o:i:F:N:K:f:c:tj:r:l:w:hv" )) != EOF ) 
	{
	  switch ( opt )
		{
		case 'd':
		  debug_level = atoi( optarg );
		  break;
		case 'o':
		case 'i':
		case 'F':
		case 'f':
		case 'N':
		  params_io.SetOption( opt, string( optarg ));
		  break;
		case 'K':
		  output_mode += string( optarg );
		  break;
		case 'c':
		  channels_number = atoi( optarg );
		  break;
		case 'j':
		  has_output = true;
		  jack_data_list.push_back( string( optarg ));
		  break;
		case 'r':
		  known_sr_list_iter = known_sr_list.find( optarg );
		  if ( known_sr_list_iter != known_sr_list.end() )
			the_sr_list.add_value( known_sr_list_iter->second );
		  else
			{
			  cout << "Only 48000, 96000, 192000 are allowed sample rates" << endl;
			  cout << "3000 and 6000 are allowed as well, under the condition -K C, ignored" << endl;
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
		  cout << "multisignalgene v0.4" << endl;
		  has_hv = true;
		  break;
		}
	}
  params_io.FlushOptions();

  params_io.EnvironementNeeds();
  this_thread::sleep_for(chrono::milliseconds( 200 ));
  cout << "Creating input and output parameters channels" << endl;
  params_io.CreateChannels(n_loops);
  cout << params_io.GetInfos();
  if ( params_io.stillHasInputChannels() == false )
	{
	  cout << "There was(were) input channel(s), but they has(have) been rejected" << endl;
	  exit( EXIT_FAILURE ); 
	}
  if ( params_io.hasFileOutput() == true && params_io.stillHasFileOutput() == false )
	{
	  cout << "There was(were) output files, but they has(have) been rejected" << endl;
	  exit( EXIT_FAILURE ); 
	}

  cout << params_io.GetInfos();
  has_input = params_io.hasInputChannels();
  has_output |= params_io.hasOutputChannels();
  has_output |= params_io.hasFileOutput();
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

  if( params_io.hasFileOutput() == true && jack_data_list.empty() == false )
	{
	  cout << "Can NOT open both an audio and a file output " << endl;
	  exit( EXIT_FAILURE );
	}

  cout << "Opening the sound file output module "<< endl;
  sound_file_output_base * sfob; 
  if ( params_io.hasFileOutput() == false )
	{
	  if( jack_data_list.empty() )
		sfob = (sound_file_output_base*) new sound_file_output_dry( follow_timebeat );
	  else
		sfob = (sound_file_output_base*) new sound_file_output_jackaudio( channels_number, jack_data_list );
	}
  else
	{
	  if( jack_data_list.empty() )
		sfob = (sound_file_output_base*) new sound_file_output_file(params_io.GetFileOutput(), follow_timebeat );
	  else
		{
		  cout << "Internal error" << endl;
		  exit( EXIT_FAILURE );		  
		}
	}
  cout << "Sample rate(s) requested by the parameters: " << the_sr_list << endl;
  the_sr_list = sfob->process_sample_rate( the_sr_list );
  cout << "Sample rate(s) validated by the output module: " << the_sr_list << endl;
  const pair<unsigned short,bool> sample_rate_data = the_sr_list.get_sample_rate();

  if ( sample_rate_data.first == 0 )
	{
	  cout << "Sorry no common value has been found" << endl;
	  exit( EXIT_FAILURE );
	}
  cout << "Sample rate is: " << sample_rate_data.first*1000 << endl;

  if ( output_mode.empty() && sample_rate_data.second)
	{
	  cout << "Low sample rate is used, the K option should be set to C (continuous)" <<endl;
	  exit( EXIT_FAILURE );
	}
  auto is_continuous = [](string::value_type i){ return ( i == 'c' ) || ( i =='C' );};
  if ( find_if_not( output_mode.begin(), output_mode.end(), is_continuous ) != output_mode.end() )
	{
	  cout << "Low sample rate is used, the K option should only C (continuous) values" <<endl;
	  exit( EXIT_FAILURE );
	}

  cout << "Opening the input and output modules ";
  cout << "with " << channels_number << "channels" << endl;
 
  main_loop signals(sample_rate_data.first,output_mode,channels_number);
  cout << signals.get_output_waveform() << endl;

  signals += params_io.GetOutputChannels();
  signals += params_io.GetInputChannels();
	
  clock_t ticks( clock() );
  do
	{
	  cout << "waiting all input parameters channels to be ready" << endl;
	  this_thread::sleep_for(chrono::milliseconds( 500 ));
	} while( signals.is_all_ready() == false );

  // Today, there is oinly one "physical" ouput at a time
  // Keep in mind there can have more in the future
  sfob->set_signals( &signals );

  if( sfob->test_sound_format() )
	cout << "Sound data type is known by the engine" << endl;
  else
	{
	  cout << "**** ERROR: The sound output is not possible by this engine ****" << endl;
	  delete sfob;
	  cout << "Opening the dry-run sound engine in 10 seconds" << endl;
	  this_thread::sleep_for(chrono::seconds( 10 ));
	  sfob = (sound_file_output_base*) new sound_file_output_dry( follow_timebeat );
	  sfob->set_signals( &signals );
	}	  

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




