#include "sound_jack_links.hxx"


void sound_jack_connections_data::insert_append( const string&peer, const string&dest )
{
  connections_list.insert( make_pair( peer, dest ));
}
void sound_jack_connections_data::insert_append( const unsigned char&N,
												const string_view& sce_peer,
												const string_view&dest_app, const string_view&dest_peer,
												const bool& stereo )
{
  string src_str;
  src_str.reserve( 15 );
  string dest_str;
  dest_str.reserve( 15 );
  list<unsigned char>peers_list_inds( N );
  iota( peers_list_inds.begin(), peers_list_inds.end(), 1 );
  for( auto&list_inds_iter : peers_list_inds )
	{
	  src_str = string( sce_peer ) + to_string( list_inds_iter );
	  dest_str = string( dest_app ) + string ( ":" ) + string( dest_peer ) + to_string( list_inds_iter );
	  connections_list.insert( make_pair( src_str, dest_str ));
	}
	  // For the stereo mode, the last one is duplicated
  if ( stereo )
	{
	  src_str = string( sce_peer ) + to_string( N );
	  dest_str = string( dest_app ) + string ( ":" ) + string( dest_peer ) + to_string( N + 1 );
	  connections_list.insert( make_pair( src_str, dest_str ));		  
	}
}

const multimap<string,string> sound_jack_connections_data::links_list()const
{
  return connections_list;
}
const deque<string> sound_jack_connections_data::peers_list()const
{
  deque< pair<string,string> >key_list;

  unique_copy( connections_list.begin(), connections_list.end(),
			   back_inserter(key_list),
			   [](const pair<string, string>&sp1,
				  const pair<string, string>&sp2)->bool
			   {return sp1.first.compare(sp2.first) == 0;});

  deque<string> the_return;
  for( auto &m: key_list )
	the_return.push_back( m.first );

  return the_return;
}
deque<string>::size_type sound_jack_connections_data::peers_size()const
{
  deque< pair<string,string> >key_list;

  unique_copy( connections_list.begin(), connections_list.end(),
			   back_inserter(key_list),
			   [](const pair<string, string>&sp1,
				  const pair<string, string>&sp2)->bool
			   {return sp1.first.compare(sp2.first) == 0;});

  return key_list.size();
}


sound_jack_connections_lists::sound_jack_connections_lists( const unsigned char&nbre_channels,
															const deque<string>&jack_connections )
{
  bool has_default_system = false;
  bool has_default_x42_scope = false;

  string_view prefix_gene( "gene_" );
  string_view prefix_system( "system" );
  string_view postfix_playback( "playback_" );
  string_view prefix_x42( "Simple Scope (4 channel)" );
  string_view postfix_x42_in( "in" );

  for ( const auto& connections_iter : jack_connections )
	{
	  // Get the file containing the jackaudio parameters
	  // if not "." then open the file and process it
	  //   else take the default values
	  if( connections_iter.compare(".") == 0 )
		  has_default_system = true;

	  else if ( connections_iter.compare("-") == 0 )
		  has_default_x42_scope = true;

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
			}
		}
	}
  if ( has_default_x42_scope )
	{
	  connections_list_audio.insert_append( nbre_channels,
											prefix_gene,
											prefix_x42, postfix_x42_in,
											false );
	}
  if ( has_default_system )
	{
	  connections_list_audio.insert_append( nbre_channels,
											prefix_gene,
											prefix_system, postfix_playback,
											true );
	  connections_list_midi_in.insert_append( 1,
											string_view( "commands_in_" ),
											prefix_system,postfix_playback,
											false );
	  connections_list_midi_out.insert_append( 1,
											string_view( "commands_out_" ),
											prefix_system, postfix_playback,
											false );
	}
}
deque<string>::size_type sound_jack_connections_lists::audio_peers_size()const
{
  return connections_list_audio.peers_size();
}
bool sound_jack_connections_lists::has_atleast_one_audio()const
{
  return !connections_list_audio.links_list().empty();
}
const sound_jack_connections_data&sound_jack_connections_lists::audio()const{
  return connections_list_audio;
}
const sound_jack_connections_data&sound_jack_connections_lists::midi_in()const{
  return connections_list_midi_in;
}
const sound_jack_connections_data&sound_jack_connections_lists::midi_out()const{
  return connections_list_midi_out;
}
