#include "sound_file_output.hxx"
#include "sound_file_output_buffer.hxx"

#include <chrono>
#include <thread>

sample_rate_list::sample_rate_list():
  don_t_care( true)
{}
void sample_rate_list::add_value(const unsigned char&a)
{
  don_t_care = false;
  if( find( list_sr.begin(), list_sr.end(), a ) == list_sr.end())
	list_sr.insert( a );
}
sample_rate_list sample_rate_list::intersect_sr_list(const sample_rate_list&a)const
{
  sample_rate_list the_return;
  if ( don_t_care )
	{
	  // copy form a
	  the_return.don_t_care = a.don_t_care;
	  the_return.list_sr = a.list_sr;
	}else{
	the_return.don_t_care = false;
	if ( a.don_t_care )
	  the_return.list_sr = list_sr;
	else
	  // search for the intersection
	  // no set_intersection as it came with C++17
	  for_each( list_sr.begin(), list_sr.end(), [&](const unsigned char& b){
		  auto iter = find( a.list_sr.begin(), a.list_sr.end(), b );
		  if ( iter != a.list_sr.end() )
			the_return.list_sr.insert( b );
		});
  }
	return the_return;
}
unsigned char sample_rate_list::get_sample_rate()const
{
  // default strategy is to return the lowest one
  if( don_t_care )
	// return 4;
	  return 1;
  else
	if ( list_sr.begin() == list_sr.end() )
	  // list is empty, return 0
	  return 0;
	else
	  // return list_sr.rbegin();
	  return *list_sr.begin();
}
ostream&operator<<(ostream&a,const sample_rate_list&b)
{
  if ( b.don_t_care )
	a<<"[all]";
  else
	if ( b.list_sr.begin() == b.list_sr.end() )
	  a<<"[none]";
	else
	  for_each( b.list_sr.begin(), b.list_sr.end(), [&a](const unsigned char&c)
				{
				  a<<" "<<c*48000<<" ";
				});
  return a;
}

sound_file_output_buffer::sound_file_output_buffer( const unsigned short&sample_size,
													const bool&interleave,
													const size_t&data_size,
													const vector<void*>& data):
  channels_bounds(pair<unsigned short,unsigned short>
													(numeric_limits<unsigned short>::min(),
													 numeric_limits<unsigned short>::max())),
  sample_size(sample_size),
  interleave(interleave),
  data_size(data_size),
  data(data)
{}
ostream&operator<<(ostream&the_out,const sound_file_output_buffer&a)
{
  the_out<<"Channels: ["<<a.channels_bounds.first<<":"<<a.channels_bounds.second<<"]"<<endl;
  switch( a.sample_size )
	{
	case 2: the_out << "Signed short, ";break;
	case 4: the_out << "Signed long, ";break;
	default: the_out << "Unsupported format" << a.sample_size << ", ";break;
	}
  if( a.interleave )
	the_out << "frames one after the other containing all the channels" << endl;
  else
	the_out << "one sub buffer per channel" << endl;

  return the_out;
}


sound_file_output_base::sound_file_output_base(const sound_file_output_buffer&sfo_buffer):
  sfo_buffer(sfo_buffer)
{}

void sound_file_output_base::set_signals(main_loop*const&signals)
{
  this->signals = signals;
}
bool sound_file_output_base::is_ready()const
{
  return true;
}
bool sound_file_output_base::pre_run()
{
  return true;
}
sample_rate_list sound_file_output_base::process_sample_rate(const sample_rate_list&a)const
{
  return sr_list.intersect_sr_list( a );
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
														 const string&jack_connections):
  sound_file_output_base(  sound_file_output_buffer( 4,false,
													 0,vector<void*>())),
  is_open_b( true ),is_started(false),sound_started(false),shutdown_requested(false)
{
  // In the entire constructor, one does not have to take care about the real time
  //   as jack is not activated
  const char *server_name = nullptr;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  // Get the file containing the jackaudio parameters
  // if not "." then open the file and process it
  //   else take the default values
  if( jack_connections.compare(".") != 0 )
	{
	  // Read the connection file
	  ifstream inputfile_stream( jack_connections, istream::binary );
	  if ( inputfile_stream.is_open() == true )
		{
		  cerr << "Sorry not yet implemented" << endl;
		  // TODO
		  inputfile_stream.close();
		  is_open_b  = false;
		}else
		  is_open_b  = false;
	} else {
	// Or use the hardcoded default values
	jack_peer = string( "estim" );

	sfo_buffer.channels_bounds.first = numeric_limits<unsigned short>::min();
	sfo_buffer.channels_bounds.second = numeric_limits<unsigned short>::min();
	array< unsigned short, 4 > hardcoded_connections_audio = { 0, 1, 2, 3 };
	for( auto &m : hardcoded_connections_audio )
	  {
		if ( m != 3 )
		  connections_list_audio.insert( pair< string, deque< string> >
										(string( "gene_" ) + to_string( m ),
										 deque< string >{string( "system:playback_" ) + to_string( m + 1 )}));
		else
		  connections_list_audio[ string( "gene_2" ) ].push_back( "system:playback_" + to_string( m + 1 )); 
		sfo_buffer.channels_bounds.second += 1;
	  }
	cout << "-------------" << sfo_buffer.channels_bounds.second << "-----------------------------" << endl;
	array< unsigned short, 1 > hardcoded_connections_midi = { 0 };
	for( auto &m : hardcoded_connections_midi )
	  {
		  connections_list_midi_in.insert( pair< string, deque< string> >
										(string( "commands_in_" ) + to_string( m ),
										 deque< string >{string( "system:playback_" ) + to_string( m + 1 )}));
	  }
	for( auto &m : hardcoded_connections_midi )
	  {
		  connections_list_midi_out.insert( pair< string, deque< string> >
										(string( "commands_out_" ) + to_string( m ),
										 deque< string >{string( "system:playback_" ) + to_string( m + 1 )}));
	  }
  }

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
	  switch ( jack_sr )
		{
		case 48000:  sr_list.add_value( 1 );  cout << "jack sr: " << sr_list << endl; break;
		case 96000:  sr_list.add_value( 2 );  cout << "jack sr: " << sr_list << endl; break;
		case 192000:  sr_list.add_value( 4 );  cout << "jack sr: " << sr_list << endl; break;
		default: is_open_b = false; cout << "Sample rate of " << jack_sr << " is not supported" << endl;
		}
	}else{
	is_open_b = false;
  }
  if ( is_open_b )
	{
	  for( auto &m : connections_list_audio )
		{
		  output_ports.push_back( jack_port_register( client,
													  m.first.c_str(),
													  JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	  for( auto &m : connections_list_midi_in )
		{
		  output_ports.push_back( jack_port_register( client,
													  m.first.c_str(),
													  JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0 ));
		  sfo_buffer.data.push_back( nullptr );
		}
	  for( auto &m : connections_list_midi_out )
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
	  for( auto &m : connections_list_audio )
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


sound_file_output_dry::sound_file_output_dry(const bool&follow_timebeat):
  sound_file_output_base(  sound_file_output_buffer( 2,true,
													 0,vector<void*>())),
  follow_timebeat(follow_timebeat),
  cumul_us_elapsed( 0 )
{}
void sound_file_output_dry::run()
{
  // for debug purpose, limit the run
  //  unsigned short limit=0;
  unsigned long microseconds_elapsed;
  while( true )
	{
	  microseconds_elapsed = signals->send_to_sound_file_output( sfo_buffer );
	  if ( microseconds_elapsed == 0 )
		break;
	  if ( follow_timebeat )
		{
		  cumul_us_elapsed += microseconds_elapsed;
		  if( ( cumul_us_elapsed / 1000 ) != 0 )
			{ 
			  this_thread::sleep_for(chrono::milliseconds(cumul_us_elapsed / 1000));
			  cumul_us_elapsed -= 1000 * ( cumul_us_elapsed / 1000);
			  }
		}
	  // limit++;
	  // if( limit > 200000 )
	  //   break;
	}
}



sound_file_output_file::sound_file_output_file(const string& filename,
											   const bool&follow_timebeat):
  sound_file_output_base( sound_file_output_buffer( 2,true,
													19200,vector<void*>())),
  follow_timebeat(follow_timebeat),
  outputfile_stream( filename, ostream::binary | ostream::trunc ),
  cumul_us_elapsed( 0 )
{
  sfo_buffer.data.push_back( (void*)values );
}
sound_file_output_file::~sound_file_output_file()
{
  outputfile_stream.flush();
  outputfile_stream.close();
}
void sound_file_output_file::run()
{
  //unsigned short limit=0;
  unsigned long microseconds_elapsed;
  while( true )
	{
	  microseconds_elapsed = signals->send_to_sound_file_output( sfo_buffer );
	  outputfile_stream.write( (const char*)sfo_buffer.data[0], sfo_buffer.data_size );
	  outputfile_stream.flush();
	  if ( microseconds_elapsed == 0 )
		break;
	  if ( follow_timebeat )
		{
		  cumul_us_elapsed += microseconds_elapsed;
		  if( ( cumul_us_elapsed / 1000 ) != 0 )
			{
			  this_thread::sleep_for(chrono::milliseconds(cumul_us_elapsed / 1000));
			  cumul_us_elapsed -= 1000 * ( cumul_us_elapsed / 1000);
			  }
		}
	  // limit++;
	  // if( limit > 200000 )
	  //   break;
	}
}

