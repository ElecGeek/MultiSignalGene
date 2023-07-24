#include "params_io_handler.hxx"

params_input_handler::params_input_handler():
  supported_formats({{params_io_midi_file, "mid"},{params_io_midi_file,"MID"},
					 {params_io_mnemos, "mnemo"},{params_io_mnemos,"MNEMO"},
                     {params_io_test, "test"},{params_io_test,"TEST"}}),
  unsupported_formats({{params_io_text, "txt"},{params_io_text,"TXT"},
												 {params_io_text,"text"},{params_io_text,"TEXT"},
												 {params_io_vhdl_test,"vhdl"},{params_io_vhdl_test,"VHDL"}}),
  planned_formats({{params_io_midi_connec,"jmidi"},{params_io_midi_connec,"JMIDI"}})
{}
/* @breif Validate options in the current channel
 *
 * Each time the software receives a new input (-i), a new output (-o)
 * or a flush (calling parameters are over), the pending input options are tested and
 * if everythingare is correct, a structure is added to the input channels list.
 * If something is wrong, the options of this input are discarded.
 * That does NOT prevent the usage of other input channels
 */
void params_input_handler::AddChannelOptions( const options_list_t  & opts_list )
{
  bool valid = false;
  // The quick sanity check is to check if there is a 'F' option.
  // If so, check against valid formats
  options_list_t::const_iterator in_format;
  in_format = opts_list.find( 'F' );
  if ( in_format != opts_list.end() )
	{
	  decltype( supported_formats )::const_iterator supported_formats_iter;
	  supported_formats_iter = find_if( supported_formats.begin(),supported_formats.end(),
										[&]( decltype(supported_formats)::const_reference a ) -> bool {
										  return a.second == in_format->second;
										});
	  if ( supported_formats_iter != supported_formats.end() )
		//	  for( auto it : supported_formats )
		//if( it.second == in_format->second )
		  {
			//			channels_list.push_back( make_pair( it.first, opts_list ));
			channels_list.push_back( make_pair( supported_formats_iter->first, opts_list ));
			info_in_params << "Input commands option "<<in_format->second << " OK"<< endl;;
			return;
		  }
	  for( auto it : unsupported_formats )
		if( it.second == in_format->second )
		  {
			info_in_params << "The input commands format "<<in_format->second;
			info_in_params << " is not or is irrelevant to be supported"<<endl;
			return;
		  }
	  for( auto it : planned_formats )
		if( it.second == in_format->second )
		  {
			info_in_params << "The input commands format "<<in_format->second;
			info_in_params << " is not yet supported, comming soon"<<endl;
			return;
		  }
	  info_in_params << "The input commands format "<<in_format->second;
	  info_in_params << " is unknown"<<endl;
	  return;
	}
  // no format info, add it and check later
  channels_list.push_back( make_pair( params_io_unknown, opts_list ));
}
void params_input_handler::CreateInputChannels(const unsigned short&loop_counter)
{
  // For the input channels there is a sanity check.
  // If the type is explicitely defined, the check verifies the input. If false, the input is omitted.
  // If the type is not defined, the software tries to identify with the sanity checks.
  for( auto& chan : channels_list )
	{
	  options_list_t::const_iterator in_opt_name;
	  in_opt_name = chan.second.find( 'i' );
	  switch( chan.first )
		{
		  // We have open the input and to check which one is a good candidate
		case params_io_unknown:
		  if( (*in_opt_name).second.compare( "-" ))
			info_in_params << "Format auto detection to be done" << endl;
		  else
			info_in_params << "Format auto detection not allowed for standard input" << endl;
		  break; 
		case params_io_mnemos:
		case params_io_midi_file:
		  if ( in_opt_name != chan.second.end() )
			{
			  ifstream input_stream;
			  if( (*in_opt_name).second.compare( "-" ))
				{
		   		  input_stream.open( (*in_opt_name).second, ios_base::in );
				  if( input_stream.is_open() )
					{
					  switch( chan.first )
						{
						case params_io_midi_file:
						  if ( CheckMidiFile( input_stream ) )
							{
							  IPB_list.push_back( new input_params_midi_file( cout, input_stream, loop_counter ));
							  info_in_params << "Opening midi file mode input parameters file ";
							  info_in_params << (*in_opt_name).second << " Ok" << endl;
							}
						  else
							{
							  info_in_params << "Error the file " << (*in_opt_name).second;
							  info_in_params << " does not looks like a mid file" << endl;
							}
						  break;
						case params_io_mnemos:
						  if ( CheckMnemos( input_stream ) )
							{
					   		  IPB_list.push_back( new input_params_mnemos_file( cout, input_stream, loop_counter ));
							  info_in_params << "Opening mnemos mode input parameters file ";
							  info_in_params << (*in_opt_name).second << " Ok" << endl;
							}
						  else
							{
							  info_in_params << "Error the file " << (*in_opt_name).second;
							  info_in_params << " does not looks like a mnemos file" << endl;
							}
						  break;
						}
					}
				  else
					{
					  info_in_params << "Problem opening file " << (*in_opt_name).second;
					  info_in_params << ", check directory and permissions" << endl;
					}
				}
			  else
				switch( chan.first )
				  {
				  case params_io_midi_file:
					info_in_params << "Opening midi file mode input parameters on standard input NOT allowed.";
					info_in_params << " The channel should be binary" << endl;
					break;
				  case params_io_mnemos:
					IPB_list.push_back( new input_params_mnemos_byte_stream( cout, cin, false ));
					info_in_params << "Opening mnemo mode input parameters on standard input Ok" << endl;
					break;
				  }
			  break;
			}
		  else
			cout << "INTERNAL ERROR Input params name" << endl;
		  break;
		case params_io_test:
		  IPB_list.push_back( new input_params_mnemos_hardcoded( cout ));
		  info_in_params << "Opening test mode input parameters file ";
		  info_in_params << "Filename " << (*in_opt_name).second << " Omitted" << endl;
		  break;
		case params_io_midi_connec:

		  break;
		}
	}
}
void params_input_handler::EnvironementNeeds(void)
{}

string params_input_handler::GetInfos()
{
  string the_return=info_in_params.str();
  info_in_params.str("");
  info_in_params.clear();
  return the_return;
}
bool params_input_handler::CheckMidiFile(ifstream&)const
{
  // TODO
  return true;
}
bool params_input_handler::CheckMnemos(ifstream&)const
{
  // TODO
  return true;
}

params_output_handler::params_output_handler():
  supported_formats({{params_io_text, "txt"},{params_io_text,"TXT"},
								  {params_io_text,"text"},{params_io_text,"TEXT"},
											   {params_io_mnemos, "mnemo"},{params_io_mnemos,"MNEMO"}}),
  unsupported_formats(),
  planned_formats({{params_io_midi_file, "mid"},{params_io_midi_file,"MID"},
												  {params_io_midi_connec,"jmidi"},{params_io_midi_connec,"JMIDI"},
								{params_io_vhdl_test,"vhdl"},{params_io_vhdl_test,"VHDL"}})
{}
/* @breif Validate options in the current channel
 *
 * Each time the software receives a new input (-i), a new output (-o)
 * or a flush (calling parameters are over), the pending output options are tested and
 * if everythingare is correct, a structure is added to the output channels list.
 * If something is wrong, the options of this output are discarded.
 * That does NOT prevent the usage of other output channels
 */
void params_output_handler::AddChannelOptions( const options_list_t  & opts_list )
{
  bool valid = false;
  // The quick sanity check is to check if there is a 'F' option.
  // If so, check against valid formats
  options_list_t::const_iterator out_format;
  out_format = opts_list.find( 'F' );
  if ( out_format != opts_list.end() )
	{
	  for( auto it : supported_formats )
		if( it.second == out_format->second )
		  {
			channels_list.push_back( make_pair( it.first, opts_list ));
			info_out_params << "Output commands option F "<<out_format->second << " OK"<< endl;
			return;
		  }
	  for( auto it : unsupported_formats )
		if( it.second == out_format->second )
		  {
			info_out_params << "The output commands format "<<out_format->second;
			info_out_params << " is not or is irrelevant to be supported"<<endl;
			return;
		  }
	  for( auto it : planned_formats )
		if( it.second == out_format->second )
		  {
			info_out_params << "The output commands format "<<out_format->second;
			info_out_params << " is not yet supported, comming soon"<<endl;
			return;
		  }
	  info_out_params << "The output commands format "<<out_format->second;
	  info_out_params << " is unknown"<<endl;
	  return;
	}
  // no format info, use a defult
  else
	channels_list.push_back( make_pair( params_io_text, opts_list ));
}
void params_output_handler::CreateOutputChannels(void)
{
  // For the output channels there is no sanity check nor type to find
  // This function do what is defined
  for( auto& chan : channels_list )
	{
	  bool openingOk = true;
	  options_list_t::const_iterator out_opt_name;
	  switch( chan.first )
		{
		case params_io_unknown:
		  info_out_params << "INTERNAL ERROR output has a default value" << endl;		  
		  break;
		case params_io_text:
		case params_io_mnemos:
		case params_io_vhdl_test:
		case params_io_midi_file:
		  out_opt_name = chan.second.find( 'o' );
		  if ( out_opt_name != chan.second.end() )
			{
			  ofstream output_stream;

			  if( (*out_opt_name).second.compare( "-" ))
				{
				  output_stream.open( (*out_opt_name).second, ios_base::out );
				  if( output_stream.is_open() )
					{
					  switch( chan.first )
						{
						case params_io_text:
						  OPB_list.push_back( new output_params_txt_file( output_stream ));
						  info_out_params << "Opening text mode output parameters file " << (*out_opt_name).second;
						  info_out_params << " Ok" << endl;
						  break;
						case params_io_mnemos:
						  OPB_list.push_back( new output_params_mnemos_file( output_stream ));
						  info_out_params << "Opening mnemos mode output parameters file " << (*out_opt_name).second;
						  info_out_params << " Ok" << endl;
						  break;
						case params_io_midi_file:
						  // TODO
						  break;
						case params_io_vhdl_test:
						  // TODO
						  break;
						}
					}
				  else
					{
					  info_out_params << "Problem opening file " << (*out_opt_name).second;
					  info_out_params << ", check directory and permissions" << endl;
					}
				}
			  else
				switch( chan.first )
				  {
				  case params_io_text:
					OPB_list.push_back( new output_params_txt( cout ));
					info_out_params << "Opening text mode output parameters on standard out Ok" << endl;
					break;
				  case params_io_mnemos:
					OPB_list.push_back( new output_params_mnemos( cout ));
					info_out_params << "Opening mnemo mode output parameters on standard out Ok" << endl;
					break;
				  case params_io_midi_file:
					// TODO
					break;
				  case params_io_vhdl_test:
					// TODO
					break;
				  }
			  break;
			}
		  else
			cout << "INTERNAL ERROR Output params name" << endl;
		  break;
		case params_io_midi_connec:
		  out_opt_name = chan.second.find( 'o' );
		  if ( out_opt_name != chan.second.end() )
			{
			  // TODO
			}
		  else
			cout << "INTERNAL ERROR Output params name" << endl;
		  break;
		}
	}
}
void params_output_handler::EnvironementNeeds(void)
{}
string params_output_handler::GetInfos()
{
  string the_return=info_out_params.str();
  info_out_params.str("");
  info_out_params.clear();
  return the_return;
}


params_io_handler::params_io_handler():
  last_option( no_option )
{}

void params_io_handler::SetOption( const char&opt, const string&optarg )
{
  // Does the option a switch to input or output
  which_option new_option;
  if ( opt == 'i' )
	new_option = input_option;
  else if ( opt == 'o' )
	new_option = output_option;
  else
	// otherwise, the option is unchanged
	new_option = last_option;

  // Before anything else, flush the previous input or output paramaters channel
  if ( last_option != new_option )
	FlushOptions();

  // Noz place the option in the list
  decltype(options_list)::iterator it;
  it = options_list.find( opt );
  if ( it != options_list.end() )
	it->second = optarg;
  else
	options_list.insert( make_pair( opt, optarg ));

	last_option = new_option;
}

void params_io_handler::FlushOptions(void)
{
  switch( last_option )
	{
	case no_option:
	  if ( options_list.empty() == false )
		{
		  info_io_params << "Error: in or out parameters options has not been set before" << endl;
		}
	  break;
	case ( input_option ):
	  p_in_h.AddChannelOptions( options_list );
	  break;
	case ( output_option ):
	  p_out_h.AddChannelOptions( options_list );
	  break;
	}
  options_list.clear();
}
#include <jack/jack.h>
#include <jack/midiport.h>

void params_io_handler::EnvironementNeeds(void)
{
  p_in_h.EnvironementNeeds();
  p_out_h.EnvironementNeeds();
  // For testing if one application can have more than one jackaudio root

  const char *server_name = nullptr;
  jack_options_t options = JackNullOption;
  jack_status_t status;
  // jack_client_open( "MSG_parameters", options, &status, server_name );
}
void params_io_handler::CreateChannels(const unsigned short&n_loops)
{
  p_in_h.CreateInputChannels(n_loops);
  p_out_h.CreateOutputChannels();
}
bool params_io_handler::hasInputChannels()const
{
  return !p_in_h.channels_list.empty();
}
bool params_io_handler::stillHasInputChannels()const
{
  return !p_in_h.IPB_list.empty();
}
bool params_io_handler::hasOutputChannels()const
{
  return !p_out_h.channels_list.empty();
}
const deque<input_params_base*>&params_io_handler::GetInputChannels()const
{
  return p_in_h.IPB_list;
}
const deque<output_params_base*>&params_io_handler::GetOutputChannels()const
{
  return p_out_h.OPB_list;
}

string params_io_handler::GetInfos()
{
  info_io_params << p_in_h.GetInfos();
  info_io_params << p_out_h.GetInfos();
  string the_return=info_io_params.str();
  info_io_params.str("");
  info_io_params.clear();
  return the_return;
}


