#include "sound_jack_output.hxx"
#include "sound_jack_links.hxx"
#include "sound_output_buffer.hxx"

#include <chrono>
#include <thread>
#include <cstring>

#ifndef __NO_JACK__
int sound_file_output_jackaudio::call_back_audio( jack_nframes_t nframes, void * arg )
{
  auto& my_this = *static_cast<sound_file_output_jackaudio*>(arg);

  jack_default_audio_sample_t *out;

  if( my_this.sound_started && ( my_this.shutdown_requested == false ))
	{
	  // Put the pointers into the relevant structures
	  transform( my_this.output_sound_ports.begin(), my_this.output_sound_ports.end(), my_this.sfo_buffer.data.begin(),
	  	 [&](jack_port_t*the_port){
	  		   return jack_port_get_buffer( the_port, nframes );
	  		 });
	  my_this.sfo_buffer.data_size = sizeof(jack_default_audio_sample_t ) * nframes;

	  unsigned long microseconds_elapsed;
	  // ... and call the run of 
	  microseconds_elapsed = my_this.signals->send_to_sound_file_output( my_this.sfo_buffer );
	  if ( microseconds_elapsed == 0 )
		my_this.shutdown_requested = true;
	  // Put the midid strutures
	  // my_this.output_midi_ports

	  // ... and run the decoding
	  // my_this.signals(my_this.mdio_buffer);
	}else{
	for_each( my_this.output_sound_ports.begin(), my_this.output_sound_ports.end(), [&](jack_port_t*the_port){
		out = (jack_default_audio_sample_t*)jack_port_get_buffer( the_port, nframes );  
		memset( out, 0, sizeof(jack_default_audio_sample_t ) * nframes );
	  });
  }
  return 0;
};



sound_file_output_jackaudio::sound_file_output_jackaudio(const unsigned char&nbre_channels,
														 const deque<string>&jack_connections):
  sound_file_output_base(  sound_data_output_buffer( make_tuple( typeid(jack_default_audio_sample_t).hash_code(),
													 true, false),
													 0,vector<void*>())),
  is_started(false),sound_started(false),shutdown_requested(false),
  connections_list( nbre_channels, jack_connections )
{
  // In the entire constructor, one does not have to take care about the real time
  //   as jack is not activated
  const char *server_name = nullptr;
  jack_options_t options = JackNullOption;
  jack_status_t status;
  bool has_default_system = false;
  bool has_default_x42_scope = false;

  is_open_b = connections_list.has_atleast_one_audio();

	jack_peer = "MultiSignalsGene";

	sfo_buffer.channels_bounds.first = numeric_limits<unsigned short>::min();
	sfo_buffer.channels_bounds.second = numeric_limits<unsigned short>::min() +
	  connections_list.audio_peers_size();

	cout << "audio peer size " <<	  connections_list.audio_peers_size() << endl;


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
	  for( auto &m : connections_list.audio().peers_list() )
		{
		  output_sound_ports.push_back( jack_port_register( client,
													  m.c_str(),
													  JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	  	  for( auto &m : connections_list.midi_in().peers_list() )
		{
		  output_midi_ports.push_back( jack_port_register( client,
													  m.c_str(),
													  JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 ));
		  // TODO midi connetions
		  //		  mdio_buffer.data.push_back( nullptr );
		}
	  for( auto &m : connections_list.midi_out().peers_list() )
		{
		  output_midi_ports.push_back( jack_port_register( client,
													  m.c_str(),
													  JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0 ));
		  // TODO midsi connection
		  // mdio_buffer.data.push_back( nullptr );
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
	  for( auto &m : connections_list.audio().links_list() )
		  {
			cout << "Connecting " << jack_peer << ":" << m.first.c_str() << " to " << m.second.c_str();
			if ( jack_connect( client, ( jack_peer + ":" + m.first ).c_str(), m.second.c_str() ) != 0 )
			cout << " Probem with the connection" << endl;
		  else
			cout << " Ok" << endl;
		  }
	}
  return is_open_b;
}

#else
sound_file_output_jackaudio::sound_file_output_jackaudio(const unsigned char&nbre_channels,
														 const deque<string>&jack_connections):
  sound_file_output_base(  sound_data_output_buffer( make_tuple( 0,true,false),
													 0,vector<void*>())),
  is_started(false),sound_started(false),shutdown_requested(false),
  connections_list( nbre_channels, jack_connections ),
  is_open_b(false)
{
  cout << "This software has not been compiled with jackaudio enable" << endl << endl;
  cout << "However, it can be used to test the syntax of the connections" << endl << endl;
  
  for( auto &m : connections_list.audio().links_list() )
	cout << m.first.c_str() << " would have been connected to " << m.second.c_str() << endl;
  cout << endl;
}
sound_file_output_jackaudio::~sound_file_output_jackaudio()
{}
bool sound_file_output_jackaudio::test_sound_format()const
{
  return false;
}

#endif
bool sound_file_output_jackaudio::is_ready()const
{
  return is_started && ( ~shutdown_requested );
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


