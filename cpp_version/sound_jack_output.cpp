#include "sound_jack_output.hxx"
#include "sound_output_buffer.hxx"

#include <chrono>
#include <thread>



void sound_jack_connections_data::insert_append( const string&peer, const string&dest )
{
  bool not_found = true;
  for( auto&existing_peer : connections_list )
	if( existing_peer.first == peer )
	  {
		existing_peer.second.push_back( dest );
		not_found = false;
		break;
	  }
  if ( not_found )
	{
	  // Add the peer
	  connections_list.push_back( make_pair( peer, deque<string>{ dest }));
	}
}
void sound_jack_connections_data::insert_append( const unsigned char&N,
												const string& sce_peer,
												const string&dest_app, const string&dest_peer,
												const bool& stereo )
{
  short ind_peer = 0;
  // exisitng sce peers: use them
  for ( auto&existing_peer : connections_list )
	{
	// Idel peers could have been declared before
	  if ( ind_peer < N )
		{ 
		  existing_peer.second.push_back( dest_app + string( ":" ) + dest_peer + to_string( ind_peer + 1 ));
		  if ( ((ind_peer + 1 ) == N ) && stereo )
			existing_peer.second.push_back( dest_app + string( ":" ) + dest_peer + to_string( ind_peer++ + 2 ));
		  else
			ind_peer += 1;
		}
	}
  // now add peers if not all the connections has been done
  for ( unsigned char ind_add = ind_peer; ind_add < N; ind_add++ )
	{
	  connections_list.push_back(make_pair(sce_peer + to_string( ind_add + 1 ),
										   deque< string >{dest_app + string( ":" ) +
											   dest_peer + to_string( ind_add + 1 )}));
	  if ( ((ind_add + 1 ) == N ) && stereo )
		connections_list.back().second.push_back( dest_app + string( ":" ) +
			  dest_peer + to_string( ind_add + 2 ));
	}
}

// C++17 is going to siplify the writing
const deque< pair< string, deque< string> > >&sound_jack_connections_data::operator()()const
{
  return connections_list;
}

#include <cstring>

int sound_file_output_jackaudio::call_back_audio( jack_nframes_t nframes, void * arg )
{
  auto& my_this = *static_cast<sound_file_output_jackaudio*>(arg);

  jack_default_audio_sample_t *out;

  if( my_this.sound_started && ( my_this.shutdown_requested == false ))
	{
	  transform( my_this.output_ports.begin(), my_this.output_ports.end(), my_this.sfo_buffer.data.begin(),
				 [&](jack_port_t*the_port){
				   return jack_port_get_buffer( the_port, nframes );
				 });
	  my_this.sfo_buffer.data_size = sizeof(jack_default_audio_sample_t ) * nframes;

	  unsigned long microseconds_elapsed;
	  	  microseconds_elapsed = my_this.signals->send_to_sound_file_output( my_this.sfo_buffer );
	  if ( microseconds_elapsed == 0 )
	  	my_this.shutdown_requested = true;
	}else{
	for_each( my_this.output_ports.begin(), my_this.output_ports.end(), [&](jack_port_t*the_port){
		out = (jack_default_audio_sample_t*)jack_port_get_buffer( the_port, nframes );  
		memset( out, 0, sizeof(jack_default_audio_sample_t ) * nframes );
	  });
  }
  return 0;
};



sound_file_output_jackaudio::sound_file_output_jackaudio(const unsigned char&nbre_channels,
														 const deque<string>&jack_connections):
  sound_file_output_base(  sound_file_output_buffer( typeid(jack_default_audio_sample_t).hash_code(),
													 true,
													 false,
													 0,vector<void*>())),
  is_started(false),sound_started(false),shutdown_requested(false)
{
  // In the entire constructor, one does not have to take care about the real time
  //   as jack is not activated
  const char *server_name = nullptr;
  jack_options_t options = JackNullOption;
  jack_status_t status;
  bool has_default_system = false;
  bool has_default_x42_scope = false;
  is_open_b = false;

  for ( const auto& connections_iter : jack_connections )
	{
	  // Get the file containing the jackaudio parameters
	  // if not "." then open the file and process it
	  //   else take the default values
	  if( connections_iter.compare(".") == 0 )
		{
		  has_default_system = true;
		  is_open_b = true;
		}
	  else if ( connections_iter.compare("-") == 0 )
		{
		  has_default_x42_scope = true;
		  is_open_b = true;
		}
	  else
		{
		  // Read the connection file
		  // fill up the connections and peers
		  ifstream inputfile_stream( connections_iter, istream::binary );
		  if ( inputfile_stream.is_open() == true )
			{
			  cerr << "Sorry not yet implemented" << endl;
			  // TODO
			  inputfile_stream.close();
			  //			  is_open_b  = true;
			  is_open_b  = false;
			}
		}
	}
  if ( has_default_x42_scope )
	{
	  connections_list_audio.insert_append( nbre_channels,
											string( "gene_" ),
											string( "Simple Scope (4 channel)" ), string( "in" ),
											false );
	}
  if ( has_default_system )
	{
	  connections_list_audio.insert_append( nbre_channels,
											string( "gene_" ),
											string( "system" ), string( "playback_" ),
											true );
	  connections_list_midi_in.insert_append( 1,
											string( "commands_in_" ),
											string( "system" ), string( "playback_" ),
											false );
	  connections_list_midi_out.insert_append( 1,
											string( "commands_out_" ),
											string( "system" ), string( "playback_" ),
											false );
	}

	jack_peer = string( "MultiSignalsGene" );

	sfo_buffer.channels_bounds.first = numeric_limits<unsigned short>::min();
	sfo_buffer.channels_bounds.second = numeric_limits<unsigned short>::min() +
	  connections_list_audio().size();


  cout << sizeof( jack_default_audio_sample_t ) << endl;
  // Everything is ready
  if( is_open_b == true )
	{
	  client = jack_client_open( jack_peer.c_str(), options, &status, server_name );
	  if ( client != nullptr )
		{
		  if( status & JackServerStarted )
			cout <<"JACK server started ";
		  if( status & JackNameNotUnique )
			cout << "Unique name: " << jack_get_client_name( client ) << "assigned";
		}else
		is_open_b = false;
	}
  if ( is_open_b )
	{
   	  jack_set_process_callback( client, call_back_audio, this);
	  
	  // stop and throw an error message
	  // jack_on_shutdown( client, call_back_jack_shutdown, this );

	  // this is not handeled by the software then
	  //   stop and throw an error message
	  // jack_get_sample_rate_callback( client, call_back_samplerate_change, this ); 
	  
	  unsigned long jack_sr = jack_get_sample_rate( client );
	  cout << jack_sr << endl;

	  if ( jack_sr >= 3000 && jack_sr <= 192000 )
		{
		  sr_list.add_value( jack_sr / 1000 );
		  cout << "jack sr: " << sr_list << endl;
		}else{
		is_open_b = false;
		cout << "Sample rate of " << jack_sr << " is not supported" << endl;
	  }

	}else{
	is_open_b = false;
  }
  if ( is_open_b )
	{
	  for( auto &m : connections_list_audio() )
		{
		  output_ports.push_back( jack_port_register( client,
													  m.first.c_str(),
													  JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	  for( auto &m : connections_list_midi_in() )
		{
		  output_ports.push_back( jack_port_register( client,
													  m.first.c_str(),
													  JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	  for( auto &m : connections_list_midi_out() )
		{
		  output_ports.push_back( jack_port_register( client,
													  m.first.c_str(),
													  JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	}

}
sound_file_output_jackaudio::~sound_file_output_jackaudio()
{
  if ( is_open_b )
	jack_client_close( client );
}
bool sound_file_output_jackaudio::test_sound_format()const
{
  return signals->test_sound_format( typeid(jack_default_audio_sample_t).hash_code(),true,false);
}
bool sound_file_output_jackaudio::is_ready()const
{
  return is_started && ( ~shutdown_requested );
}
//sound_file_output_jackaudio::~sound_file_output_jackaudio()
//{}
// the code inside should go into the callback function
bool sound_file_output_jackaudio::pre_run()
{
  if ( is_open_b )
	{
	  if ( jack_activate( client ) != 0 )
		is_open_b = false;
	}
  // From here, special care should be taken to not break the real time

  if ( is_open_b )
	{
	  const char*port;
	  for( auto &m : connections_list_audio() )
		for( auto &n : m.second )
		  {
			cout << "Connecting " << jack_peer << ":" << m.first.c_str() << " to " << n.c_str();
			if ( jack_connect( client, ( jack_peer + ":" + m.first ).c_str(), n.c_str() ) != 0 )
			cout << " Probem with the connection" << endl;
		  else
			cout << " Ok" << endl;
		  }
	}
  return is_open_b;
}

#include <unistd.h>
void sound_file_output_jackaudio::run()
{
  bool the_loop( true );	  
  if ( is_open_b == true )
	{
	  sound_started = true;
	  while( the_loop )
		{
		  this_thread::sleep_for(chrono::seconds(2));
		  if( shutdown_requested )
			{
			  the_loop = false;
			  cout << "main loop" << endl;
			}
	  }
	}
}


