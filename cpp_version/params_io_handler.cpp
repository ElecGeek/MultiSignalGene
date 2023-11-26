#include "params_io_handler.hxx"


params_fio_handler_base::params_fio_handler_base(unsigned char related_param,
												 string err_msg_prefix,
												 multimap< short, string > supported_formats,
												 multimap< short, string > unsupported_formats,
												 multimap< short, string > planned_formats):
  related_param(related_param),
  err_msg_prefix(err_msg_prefix),
  supported_formats(supported_formats),
  unsupported_formats(supported_formats),
  planned_formats(planned_formats),
  is_bad( false )
{}

string params_fio_handler_base::GetInfos()
{
  string the_return=info_fio_params.str();
  info_fio_params.str("");
  info_fio_params.clear();
  return the_return;
}


params_input_handler::params_input_handler():
  params_fio_handler_base(
						  'i',
						  "Input command option",
						  {{params_io_midi_file, "mid"},{params_io_midi_file,"MID"},
						   {params_io_mnemos, "mnemo"},{params_io_mnemos,"MNEMO"},
						   {params_io_test, "test"},{params_io_test,"TEST"}},
                          {{params_io_text, "txt"},{params_io_text,"TXT"},
						   {params_io_text,"text"},{params_io_text,"TEXT"},
						   {params_io_vhdl_test,"vhdl"},{params_io_vhdl_test,"VHDL"}},
                          {{params_io_midi_connec,"jmidi"},{params_io_midi_connec,"JMIDI"}})
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
			//			fio_channels_with_opts.push_back( make_pair( it.first, opts_list ));
			fio_channels_with_opts.push_back( make_pair( supported_formats_iter->first, opts_list ));
			info_fio_params << "Input commands option "<<in_format->second << " OK"<< endl;;
			return;
		  }
	  for( auto it : unsupported_formats )
		if( it.second == in_format->second )
		  {
			info_fio_params << "The input commands format "<<in_format->second;
			info_fio_params << " is not or is irrelevant to be supported"<<endl;
			return;
		  }
	  for( auto it : planned_formats )
		if( it.second == in_format->second )
		  {
			info_fio_params << "The input commands format "<<in_format->second;
			info_fio_params << " is not yet supported, comming soon"<<endl;
			return;
		  }
	  info_fio_params << "The input commands format "<<in_format->second;
	  info_fio_params << " is unknown"<<endl;
	  return;
	}
  // no format info, add it and check later
  fio_channels_with_opts.push_back( make_pair( params_io_unknown, opts_list ));
}
void params_input_handler::CreateInputChannels(const unsigned short&loop_counter)
{
  // For the input channels there is a sanity check.
  // If the type is explicitely defined, the check verifies the input. If false, the input is omitted.
  // If the type is not defined, the software tries to identify with the sanity checks.
  for( auto& chan : fio_channels_with_opts )
	{
	  options_list_t::const_iterator in_opt_name;
	  in_opt_name = chan.second.find( 'i' );
	  switch( chan.first )
		{
		  // We have open the input and to check which one is a good candidate
		case params_io_unknown:
		  if( (*in_opt_name).second.compare( "-" ))
			info_fio_params << "Format auto detection to be done" << endl;
		  else
			info_fio_params << "Format auto detection not allowed for standard input" << endl;
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
							  info_fio_params << "Opening midi file mode input parameters file ";
							  info_fio_params << (*in_opt_name).second << " Ok" << endl;
							}
						  else
							{
							  info_fio_params << "Error the file " << (*in_opt_name).second;
							  info_fio_params << " does not looks like a mid file" << endl;
							}
						  break;
						case params_io_mnemos:
						  if ( CheckMnemos( input_stream ) )
							{
					   		  IPB_list.push_back( new input_params_mnemos_file( cout, input_stream, loop_counter ));
							  info_fio_params << "Opening mnemos mode input parameters file ";
							  info_fio_params << (*in_opt_name).second << " Ok" << endl;
							}
						  else
							{
							  info_fio_params << "Error the file " << (*in_opt_name).second;
							  info_fio_params << " does not looks like a mnemos file" << endl;
							}
						  break;
						}
					}
				  else
					{
					  info_fio_params << "Problem opening file " << (*in_opt_name).second;
					  info_fio_params << ", check directory and permissions" << endl;
					}
				}
			  else
				switch( chan.first )
				  {
				  case params_io_midi_file:
					info_fio_params << "Opening midi file mode input parameters on standard input NOT allowed.";
					info_fio_params << " The channel should be binary" << endl;
					break;
				  case params_io_mnemos:
					IPB_list.push_back( new input_params_mnemos_byte_stream( cout, cin, false ));
					info_fio_params << "Opening mnemo mode input parameters on standard input Ok" << endl;
					break;
				  }
			  break;
			}
		  else
			cout << "INTERNAL ERROR Input params name" << endl;
		  break;
		case params_io_test:
		  IPB_list.push_back( new input_params_mnemos_hardcoded( cout ));
		  info_fio_params << "Opening test mode input parameters file ";
		  info_fio_params << "Filename " << (*in_opt_name).second << " Omitted" << endl;
		  break;
		case params_io_midi_connec:

		  break;
		}
	}
}
void params_input_handler::EnvironementNeeds(void)
{}

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
  params_fio_handler_base( 
						  'o',
						  "Output command option",
						  {{params_io_text, "txt"},{params_io_text,"TXT"},
							{params_io_text,"text"},{params_io_text,"TEXT"},
							{params_io_mnemos, "mnemo"},{params_io_mnemos,"MNEMO"}},
	                       {},
	                       {{params_io_midi_file, "mid"},{params_io_midi_file,"MID"},
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
			fio_channels_with_opts.push_back( make_pair( it.first, opts_list ));
			info_fio_params << "Output commands option F "<<out_format->second << " OK"<< endl;
			return;
		  }
	  for( auto it : unsupported_formats )
		if( it.second == out_format->second )
		  {
			info_fio_params << "The output commands format "<<out_format->second;
			info_fio_params << " is not or is irrelevant to be supported"<<endl;
			return;
		  }
	  for( auto it : planned_formats )
		if( it.second == out_format->second )
		  {
			info_fio_params << "The output commands format "<<out_format->second;
			info_fio_params << " is not yet supported, comming soon"<<endl;
			return;
		  }
	  info_fio_params << "The output commands format "<<out_format->second;
	  info_fio_params << " is unknown"<<endl;
	  return;
	}
  // no format info, use a defult
  else
	fio_channels_with_opts.push_back( make_pair( params_io_text, opts_list ));
}
void params_output_handler::CreateOutputChannels(void)
{
  // For the output channels there is no sanity check nor type to find
  // This function do what is defined
  for( auto& chan : fio_channels_with_opts )
	{
	  bool openingOk = true;
	  options_list_t::const_iterator out_opt_name;
	  switch( chan.first )
		{
		case params_io_unknown:
		  info_fio_params << "INTERNAL ERROR output has a default value" << endl;		  
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
						  info_fio_params << "Opening text mode output parameters file " << (*out_opt_name).second;
						  info_fio_params << " Ok" << endl;
						  break;
						case params_io_mnemos:
						  OPB_list.push_back( new output_params_mnemos_file( output_stream ));
						  info_fio_params << "Opening mnemos mode output parameters file " << (*out_opt_name).second;
						  info_fio_params << " Ok" << endl;
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
					  info_fio_params << "Problem opening file " << (*out_opt_name).second;
					  info_fio_params << ", check directory and permissions" << endl;
					}
				}
			  else
				switch( chan.first )
				  {
				  case params_io_text:
					OPB_list.push_back( new output_params_txt( cout ));
					info_fio_params << "Opening text mode output parameters on standard out Ok" << endl;
					break;
				  case params_io_mnemos:
					OPB_list.push_back( new output_params_mnemos( cout ));
					info_fio_params << "Opening mnemo mode output parameters on standard out Ok" << endl;
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


params_file_handler::params_file_handler():
  params_fio_handler_base( 
						  'f',
						  "File output option",
						  {{params_file_8, "8"},{params_io_text,"char"},{params_io_text,"CHAR"},
							{params_file_16BE,"16"},{params_file_16BE,"short"},{params_file_16BE,"SHORT"},
							{params_file_32BE, "32"},{params_file_32BE, "long"},{params_file_32BE, "LONG"}},
                          {{params_file_32BE, "int"},{params_file_32BE, "INT"}},
	                      {})
{}

params_file_handler::~params_file_handler()
{
  if( output_file_stream.is_open() )
	{
	  output_file_stream.flush();
	  output_file_stream.close();
	}
}

void params_file_handler::AddChannelOptions( const options_list_t & opts_list )
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
			fio_channels_with_opts.push_back( make_pair( it.first, opts_list ));
			info_fio_params << "File format option F "<<out_format->second << " OK"<< endl;
			return;
			}
	  for( auto it : unsupported_formats )
		if( it.second == out_format->second )
		  {
			info_fio_params << "The file format format "<<out_format->second;
			info_fio_params << " is not or is irrelevant to be supported"<<endl;
			return;
		  }
	  for( auto it : planned_formats )
		if( it.second == out_format->second )
		  {
			info_fio_params << "The file format format "<<out_format->second;
			info_fio_params << " is not yet supported, comming soon"<<endl;
			return;
		  }
	  info_fio_params << "The file format "<<out_format->second;
	  info_fio_params << " is unknown"<<endl;
	  return;
	}
  // no format info, use a defult
  else
	fio_channels_with_opts.push_back( make_pair( params_file_16BE, opts_list ));
}
void params_file_handler::EnvironementNeeds(void)
{}
void params_file_handler::CreateOutputFile()
{
  // This may be temporary as today, there is only one possiblie file at a time
  if ( fio_channels_with_opts.empty() == false )
	{
	  auto chan = fio_channels_with_opts.rbegin();
	  options_list_t::const_iterator file_opt_name;
	  switch( chan->first )
		{
		case params_file_8:
		  sfo_buffer.type_float_interleave =
			sound_file_output_buffer::get_super_hash( typeid(char).hash_code(), false, true);
		  break;
		case params_file_16BE:
		  sfo_buffer.type_float_interleave =
			sound_file_output_buffer::get_super_hash( typeid(short).hash_code(), false, true);
		  break;
		case params_file_32BE:
		  sfo_buffer.type_float_interleave =
			sound_file_output_buffer::get_super_hash( typeid(long).hash_code(), false, true);
		  break;
		default:
		  return;
		  break;
		}
	  file_opt_name = chan->second.find( 'f' );
	  if ( file_opt_name != chan->second.end() )
		{
		  if( (*file_opt_name).second.compare( "-" ))
			{
			  output_file_stream.open( (*file_opt_name).second, ostream::binary | ostream::trunc );
			  info_fio_params << "Opening the output file" << (*file_opt_name).second << endl; 
			}else{
			info_fio_params << "Using the standard output for the file output is discourageous" << endl;
		  }
		}else
		cout << "Internal ERROR output file" << endl;
	}
}

params_io_handler::params_io_handler():
  option_type_current ( option_list_by_type.end() ),
  option_list_by_type( {{&p_out_h},{&p_in_h},{&p_file_h}})
{
  option_type_current = option_list_by_type.end();
}

void params_io_handler::SetOption( const char&opt, const string&optarg )
{
  decltype( option_list_by_type )::const_iterator new_params =
	option_list_by_type.end();
  // search in case the option is to switch parameter type 
  for ( decltype( option_list_by_type )::const_iterator iter = option_list_by_type.begin();
		iter != option_list_by_type.end(); iter++)
	if( opt == (*iter)->related_param )
	  new_params = iter;

  if ( new_params != option_list_by_type.end() )
	{
	  // Yes it is a switch to the new param.
	  // Is there a previous one running?
	  if ( option_type_current != option_list_by_type.end() )
		{
		  // Yes, flush the previous params set if so
		  (*option_type_current)->AddChannelOptions( options_list );
		  options_list.clear();
		}
	  // In all cases, set the new param set
	  option_type_current = new_params;
	  // Put its first option
	  options_list.insert( make_pair( opt, optarg ));
	}else{
	// No. check if there is a current params set
	if ( option_type_current != option_list_by_type.end() )
	  // set into it
	  options_list.insert( make_pair( opt, optarg ));
	else
	  {	
		// I don't know what to do
		info_io_params << "ERROR: option " << opt << " with argument " << optarg;
		info_io_params << " can not be attached to any parameter. Use one of this option ";
		for ( auto iter : option_list_by_type )
		  info_io_params << "-" << iter->related_param << " ";	
		info_io_params << "before" << endl;
	  }
  }
}

void params_io_handler::FlushOptions(void)
{
  if ( option_type_current != option_list_by_type.end() )
	(*option_type_current)->AddChannelOptions( options_list );
  else
	{
	  if ( options_list.empty() == false )
		{
		  info_io_params << "Error: in or out parameters options has not been set before" << endl;
		}
	}
  options_list.clear();
}
//#include <jack/jack.h>
//#include <jack/midiport.h>

void params_io_handler::EnvironementNeeds(void)
{
  for( auto iter : option_list_by_type )
	iter->EnvironementNeeds();
}


  // For testing if one application can have more than one jackaudio root

  // const char *server_name = nullptr;
  //jack_options_t options = JackNullOption;
  //jack_status_t status;
  // jack_client_open( "MSG_parameters", options, &status, server_name );


void params_io_handler::CreateChannels(const unsigned short&n_loops)
{
  p_in_h.CreateInputChannels(n_loops);
  p_out_h.CreateOutputChannels();
  p_file_h.CreateOutputFile();
}
bool params_io_handler::hasInputChannels()const
{
  return !p_in_h.fio_channels_with_opts.empty();
}
bool params_io_handler::stillHasInputChannels()const
{
  return !p_in_h.IPB_list.empty();
}
bool params_io_handler::stillHasFileOutput()const
{
  cout << "----------------- " << p_file_h.output_file_stream.is_open() << endl;
  return p_file_h.output_file_stream.is_open();
}
bool params_io_handler::hasOutputChannels()const
{
  return !p_out_h.fio_channels_with_opts.empty();
}
bool params_io_handler::hasFileOutput()const
{
  return !p_file_h.fio_channels_with_opts.empty();
}
const deque<input_params_base*>&params_io_handler::GetInputChannels()const
{
  return p_in_h.IPB_list;
}
const deque<output_params_base*>&params_io_handler::GetOutputChannels()const
{
  return p_out_h.OPB_list;
}
pair< sound_file_output_buffer, ostream* > params_io_handler::GetFileOutput()
{
  return make_pair(p_file_h.sfo_buffer, &p_file_h.output_file_stream);
}

string params_io_handler::GetInfos()
{
  for( auto iter : option_list_by_type )
	{
	  info_io_params << iter->info_fio_params.str();
	  iter->info_fio_params.str("");
	  iter->info_fio_params.clear();
	}
  string the_return=info_io_params.str();
  info_io_params.str("");
  info_io_params.clear();
  return the_return;
}


