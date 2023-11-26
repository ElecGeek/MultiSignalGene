#include "main_loop.hxx"

//#include <jack/jack.h>

main_loop::main_loop( const unsigned short&sample_rate_id,
					  const string&mode,
					  const unsigned short&n_channels,
					  const unsigned short samples_per_param_check,
					  const unsigned long shutdown_length):
  sample_rate_id( sample_rate_id ),
  samples_count( 0 ),
  samples_per_param_check( samples_per_param_check ),
  send_to_sound_file_output( this ),
  shutdown_start( shutdown_length ),
  shutdown_count( 0 ),
  sample_type_not_found(false),
  debug_once(false)
{
#ifdef __OUTPUT_SINE_MODE__
  unsigned char output_default('s');
#elif __OUTPUT_PULSES_MODE__
  unsigned char output_default('p');
#elif __OUTPUT_TRIANGLES_MODE__
  unsigned char output_default('e');
#elif __OUTPUT_CONTINUOUS_MODE__
  unsigned char output_default('c');
#else
  #error A default output mode should be specified
#endif

  output_waveform_info.reserve( 25 + 2 * n_channels );
  output_waveform_info += "Output waves is(are):";

  // samples_per_TS_unit = sample_rate_id * samples_per_param_check;
  // sample_rate_id is based on multiples of 1KHz,
  //   then to get a refresh of the parameters every mS,
  //   the sample_rate_id is the sample divider 
  samples_per_TS_unit = sample_rate_id * 1;
  //  cout << "SPTU " <<samples_per_TS_unit << endl;
  auto mode_iter = mode.begin();

  signal_channel*sc;
  vector<unsigned char>chan_list( n_channels );
  iota( chan_list.begin(), chan_list.end(), 0 );

  if ( mode.empty() )
	{
	  for ( auto const& ind : chan_list )
		{
		  sc = new signal_channel( ind + 1 , sample_rate_id, output_default );
		  signal_list.insert( pair<unsigned short,signal_channel*>( ind, sc ));
		}
	  output_waveform_info += " all ";
	  output_waveform_info += output_default;
	}
  else
	{
	  for( unsigned short ind= 0 ; ind < n_channels; ind++ )
		{
		  unsigned char actual_mode;
		  if ( *mode_iter == 'b' || *mode_iter == 'B' )
			{
			  if ((( ind / 2 ) * 2) == ind )
				actual_mode = 's';
			  else
				actual_mode = 'p';
			}
		  else if ( *mode_iter == '.' )
			actual_mode = output_default;
		  else
			actual_mode = *mode_iter;

		  sc = new signal_channel( ind + 1, sample_rate_id, actual_mode );
		  signal_list.insert( pair<unsigned short,signal_channel*>( ind, sc ));

		  output_waveform_info += " ";
		  output_waveform_info += actual_mode;

		  if (( mode_iter + 1 ) != mode.end() )
			mode_iter++;
		}
	}
}
main_loop::~main_loop()
{
  signal_channel*sc;
  for( auto it : signal_list)
	delete it.second;
  for( input_params_base* it : params_input_list )
	delete it;
  for( output_params_base* it: params_output_list )
	delete it;
}
main_loop&main_loop::operator+=(input_params_base*const the_input)
{
  params_input_list.push_back( the_input );
  return*this;
};
main_loop&main_loop::operator+=(output_params_base*const the_output)
{
  params_output_list.push_back( the_output );
  return*this;
};
main_loop&main_loop::operator+=(const deque<input_params_base*>& the_input)
{
  for( auto& il : the_input )
	params_input_list.push_back( il );
  return*this;
};
main_loop&main_loop::operator+=(const deque<output_params_base*>& the_output)
{
  for( auto& il : the_output )
	params_output_list.push_back( il );
  return*this;
};

unsigned short main_loop::GetSamplesSize()const
{
  return (short)signal_list.size();
}
ostream&operator<<(ostream&,const sound_file_output_buffer&);

bool main_loop::check_action(void)
{
  // Each "while" loop is one sample, all relevant channels
  // Check if the are parameters to proceed as, for real time rasons,
  // it is done every N samples
  if ( samples_count != 0 )
	samples_count -= 1;
  else
	{
	  // Shutdown is not finished, not tested etc...
	  if( shutdown_count > 0 )
		{
		  if ( shutdown_count != shutdown_start )
			shutdown_count += 1;
		}else{
		// Each exec_action loop is every 1mS, regardles the sample rate
		if ( exec_actions() == false )
		  shutdown_count = 1;
		samples_count = samples_per_TS_unit - 1;
	  }
	}
  if( shutdown_count != shutdown_start )
	return false;
  else
	return true;
}


// Please, keep always this function close to send_to_sound_file_output
bool main_loop::test_sound_format(const size_t&the_type,const bool&is_float,const bool&has_interleave)
{
  return
	send_to_sound_file_output.sample_type_defs_list.find( sound_file_output_buffer::get_super_hash( the_type, is_float, has_interleave ))
	!=
	send_to_sound_file_output.sample_type_defs_list.end();
}

/*
// Please, keep always this function close to test_sound_format
unsigned long main_loop::send_to_sound_file_output(sound_file_output_buffer&buffer)
{
  if ( buffer.channels_bounds.first >= signal_list.size() ||
	   buffer.channels_bounds.first == buffer.channels_bounds.second )
	{
	  //  There is no match between the boundary requested by the audio output
	  //    and the boundary defined in the parameters.
	  //  One reason is: there is no audio output
	  //  Another reason is: in the future a list of buffers might be implemented
	  //    The goal is each buffer takes a set of outputs (with or without overlap)
	  for_each ( buffer.data.begin(), buffer.data.end(), [&](void*buf_ptr){
		  fill( static_cast<char*>(buf_ptr), static_cast<char*>(buf_ptr) + buffer.data_size , 0 );
	  	});
	  if ( exec_actions() )
	   	// ignore all the buffer members, simply tells the time (10mS) supposed to elapsed
		return 1000;
	  else
		// Run is finished, here there is no shutdown to handle, tell to leave now
		return 0;
	}
  else
	{
	  unsigned long sample_action_count = 0;
	  unsigned short chan_begin, chan_end;
	  // Number of elements
	  unsigned long frame_size_exec;
	  unsigned long frame_size_requested;

	  // the intersect is never empty, otherwyse one would have left above
	  // The assume is the parameters channels id always starts at 0
	  // This should be IMPROVED
	  if ( buffer.channels_bounds.second == numeric_limits<unsigned short>::max() )
		{
		  // The buffer is unlimited, use the size of the signals
		  chan_end = signal_list.size();
		  chan_begin = max( buffer.channels_bounds.first, (const unsigned short)0 );
		  frame_size_exec = ( chan_end - chan_begin );
		  frame_size_requested = frame_size_exec;
		}
	  else
		{
		  chan_end = min( buffer.channels_bounds.second,
						  (unsigned short)signal_list.size());
		  // See above
		  chan_begin = max( buffer.channels_bounds.first, (const unsigned short) 0 );
		  frame_size_exec = ( chan_end - chan_begin );
		  frame_size_requested =  ( buffer.channels_bounds.second - buffer.channels_bounds.first );
		}
	  if( debug_once == false )
		{
		  cout << "Excecution begin: "<< chan_begin <<", end: "<<chan_end;
		  cout << ", size exec: " << frame_size_exec << ", size req: " << frame_size_requested<<endl;
		}
	  debug_once = true;

	  bool no_more_input = true;

	  // There are many copy and paste
	  // Since this is a time critical part, I prefer to switch and to loop
	  //   rather than to lopp and to switch

	  if ( buffer.data.empty() )
		{
		  bool action_leave = false;
		  for( unsigned short ind = 0; ind < samples_per_param_check; ind ++ ) 
			if ( check_action() )
			  action_leave = true;
		  if( action_leave == false )
			return 1000;
		  else
			return 0;
		}
   	  unsigned long data_sent = 0;
	  short sample_val;
	  //	  auto sample_type_found = sample_type_defs_list.find( sound_file_output_buffer::get_super_hash( the_type, is_float, has_interleave )) !=
	  //	  if ( sample_type_found != sample_type_defs_list.end()
	  //	{
	  // } else
	  // sample_type_not_found = true;
	  if ( buffer.interleave == true )
		{
		  switch( buffer.sample_size )
			{
			case 1:
			  signed char* frame_iter_1;
			  frame_iter_1 = static_cast<signed char*>(buffer.data[0]);
			  while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
				{
				  no_more_input = check_action();
				  for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
					{
					  sample_val = (*signal_list[ind])();

					  *frame_iter_1++ = sample_val / 256;
					}
				  // now check if there are channels to fill up
				  // This should be done on each buffer refill as outputs
				  // such as jackaudio always sends different buffer
				  // (then one should not considered as cached)
				  if ( frame_size_requested > frame_size_exec )
					for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					  *frame_iter_1++ = 0;
				  
				  data_sent += frame_size_requested;
				  sample_action_count ++;
				}

			  break;
			case 2:
			  signed short* frame_iter_2;
			  frame_iter_2 = static_cast<signed short*>(buffer.data[0]);
			  while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
				{
				  no_more_input = check_action();
				  for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
					{
					  sample_val = (*signal_list[ind])();

					  *frame_iter_2++ = sample_val;
					}
				  // now check if there are channels to fill up
				  // This should be done on each buffer refill as outputs
				  // such as jackaudio always sends different buffer
				  // (then one should not considered as cached)
				  if ( frame_size_requested > frame_size_exec )
					for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					  *frame_iter_2++ = 0;
				  
				  data_sent += frame_size_requested;
				  sample_action_count ++;
				}

			  break;
			case 4:
			  signed long* frame_iter_4;
			  frame_iter_4 = static_cast<signed long*>(buffer.data[0]);
			  while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
				{
				  no_more_input = check_action();
				  long sample_val_long;
				  for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
					{
					  sample_val = (*signal_list[ind])();

					  sample_val_long = sample_val;
					  sample_val_long *= 65536;
					  // keep a rail to rail output
					  *frame_iter_4++ = sample_val_long + sample_val;
					}
				  // now check if there are channels to fill up
				  // This should be done on each buffer refill as outputs
				  // such as jackaudio always sends different buffer
				  // (then one should not considered as cached)
				  if ( frame_size_requested > frame_size_exec )
					for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					  *frame_iter_4++ = 0;
				  data_sent += frame_size_requested;
				  sample_action_count ++;
				}

			  break;
			default:
			  cerr << "NOT SUPPORTED" << endl;
			  break;
			}
		} else {
		  unsigned short buffer_numbers = buffer.data.size();
		  unsigned short buffer_n_exec = buffer_numbers;
		  if ( frame_size_exec < buffer_n_exec )
			// in this case, buffer_n_exec - frame_size_exec has to be blanked, see below
			buffer_n_exec = frame_size_exec;
		  vector<short*>frame_NI_2;
		  vector<jack_default_audio_sample_t*>frame_jack_4;
		  switch( buffer.sample_size )
			{
			case 2:
			  for_each( buffer.data.begin(), buffer.data.begin() + buffer_n_exec,
						[&frame_NI_2](void*buf_ptr){
						  frame_NI_2.push_back( static_cast<short*>(buf_ptr));
						});
			  cerr << "Do be finished" << endl;
			  break;
			case 4:
			  for_each( buffer.data.begin(), buffer.data.begin() + buffer_n_exec,
						[&frame_jack_4](void*buf_ptr){
						  frame_jack_4.push_back( static_cast<jack_default_audio_sample_t*>(buf_ptr));
						});
			  while( ( data_sent + 1 ) <= ( buffer.data_size / buffer.sample_size ))
				{
				  no_more_input = check_action();

				  unsigned short ind = chan_begin;
				  for( vector<jack_default_audio_sample_t*>::iterator iter=frame_jack_4.begin() ;
					   iter != frame_jack_4.end();
					   iter++ )
					{
					  sample_val = (*signal_list[ind++])();
					  
					  *(*iter)++ = ((jack_default_audio_sample_t)sample_val)/32768.0;
					}
				  sample_action_count ++;			  
				  data_sent += 1;
				  }
			  break;
			default:
			  cout << "NOT SUPPORTED" << endl;
			  break;
			}
		}
	  if( no_more_input )
		return 0;
	  else
		{
		  unsigned long elapsed_time = sample_action_count * 1000;
		  return elapsed_time / sample_rate_id ;
		}
	}
}
*/
bool main_loop::exec_actions()
{
  // Regardless to the sample rate, this is called every 1 mS
  bool more_input_params( false );
  bool more_output_params( false );
  // Check if at least one of them has not yet reached the eot. If so true is returned
  for_each( params_input_list.begin(), params_input_list.end(), [&]( input_params_base*const& it )
			{
			  if (it->check_next_event( samples_per_param_check, actions ) )
				more_input_params = true;
			  else
				{
				  if( it->exec_loops() )
					more_input_params = true;
				}
			});
  for( output_params_base* it : params_output_list )
	if (it->check_next_event( samples_per_param_check, actions ) )
	  more_output_params = true;
  for( map<unsigned short,signal_channel*>::iterator it=signal_list.begin(); it!=signal_list.end(); ++it)
	(it->second)->exec_next_event( actions );
  actions.clear();
  return more_input_params;
}
bool main_loop::is_all_ready()const
{
  bool all_ready( true );

  for( input_params_base* it : params_input_list )
	if ( it->is_ready() == false )
	  all_ready = false;

  return all_ready;
}
string main_loop::get_clearing()const
{
  ostringstream oss;
  bool all_ready( true );

  for( input_params_base* it : params_input_list )
	{
	  oss << "Exit input parameters channel: ";
	  switch ( it->clearing )
		{
		case input_params_base::c_unknown: oss << "Unknown reason";  break;
		case input_params_base::c_end_of_data: oss << "Data reached the end";  break;
		case input_params_base::c_abort: oss << "Abort instruction";  break;
		case input_params_base::c_file_err: oss << "File err (may not found)";  break;
		case input_params_base::c_net_err: oss << "Network error";  break;
		case input_params_base::c_data_err: oss << "Data error (maybe header corrupted)";  break;
		  }
	  oss << endl;
	}

  return oss.str();
}

string main_loop::get_output_waveform()const
{
  return output_waveform_info;
}

bool main_loop::HaveSampleTypeNotFound()const
{
  return sample_type_not_found;
}


main_loop::send_to_sound_output::
send_to_sound_output( main_loop*const main_loop_this_ptr ):
  main_loop_this(main_loop_this_ptr),
  sample_type_defs_list( {
	  { sound_file_output_buffer::get_super_hash( typeid(void).hash_code(), false , false ),
		  [&](sound_file_output_buffer&buffer)->tuple< bool, unsigned long>{
		  return make_tuple(false,0);
		}
	  },
		{ sound_file_output_buffer::get_super_hash( typeid(char).hash_code(), false , true ),
			[&](sound_file_output_buffer&buffer)->tuple< bool, unsigned long>{
			signed short sample_val;
			bool no_more_input = true;
			unsigned long sample_action_count = 0;
			unsigned long data_sent = 0;
			
			signed char* frame_iter_1 = static_cast<signed char*>(buffer.data[0]);
			//			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / 1 ))
			  {
				no_more_input = main_loop_this->check_action();
				for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
				  {
					sample_val = (*main_loop_this->signal_list[ind])();
					
					*frame_iter_1++ = sample_val / 256;
				  }
				if ( frame_size_requested > frame_size_exec )
				  for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					*frame_iter_1++ = 0;
				
				data_sent += frame_size_requested;
				sample_action_count ++;
			  }

			return make_tuple(no_more_input,sample_action_count);
		  }
		},
		  { sound_file_output_buffer::get_super_hash( typeid(short).hash_code(), false , true ),
			  [&](sound_file_output_buffer&buffer)->tuple< bool, unsigned long>{
			  signed short sample_val;
			  bool no_more_input = true;
			  unsigned long sample_action_count = 0;
			  unsigned long data_sent = 0;
			  
			signed short* frame_iter_2 = static_cast<signed short*>(buffer.data[0]);
			//			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / 2 ))
			  {
				no_more_input = main_loop_this->check_action();
				for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
				  {
					sample_val = (*main_loop_this->signal_list[ind])();
					
					*frame_iter_2++ = sample_val;
				  }
				if ( frame_size_requested > frame_size_exec )
				  for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					*frame_iter_2++ = 0;
				
				data_sent += frame_size_requested;
				sample_action_count ++;
			  }

			return make_tuple(no_more_input,sample_action_count);
		  }
		},
		  { sound_file_output_buffer::get_super_hash( typeid(long).hash_code(), false , true ),
			  [&](sound_file_output_buffer&buffer)->tuple< bool, unsigned long>{
			signed short sample_val;
			bool no_more_input = true;
			unsigned long sample_action_count = 0;
			unsigned long data_sent = 0;
			
			signed long* frame_iter_4 = static_cast<signed long*>(buffer.data[0]);
			//			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / buffer.sample_size ))
			while( ( data_sent + frame_size_requested ) <= ( buffer.data_size / 4 ))
			  {
				no_more_input = main_loop_this->check_action();
				signed long sample_val_long;
				for( unsigned short ind = chan_begin; ind < chan_end; ind++ )
				  {
					sample_val = (*main_loop_this->signal_list[ind])();
					
					sample_val_long = sample_val;
					sample_val_long *= 65536;
					// keep a rail to rail output
					*frame_iter_4++ = sample_val_long + sample_val;
				  }
				if ( frame_size_requested > frame_size_exec )
				  for( unsigned long ind = 0; ind < ( frame_size_requested - frame_size_exec); ind++ )
					*frame_iter_4++ = 0;
				
				data_sent += frame_size_requested;
				sample_action_count ++;
			  }
			
			return make_tuple(no_more_input,sample_action_count);
			}
		  },
			{ sound_file_output_buffer::get_super_hash( typeid(float).hash_code(), true, false ),
				[&](sound_file_output_buffer&buffer)->tuple< bool, unsigned long>{
				signed short sample_val;
				bool no_more_input = true;
				unsigned long sample_action_count = 0;
				unsigned long data_sent = 0;
				vector<float*>frame_float_4;
				unsigned short buffer_numbers = buffer.data.size();
				unsigned short buffer_n_exec = buffer_numbers;
				for_each( buffer.data.begin(), buffer.data.begin() + buffer_n_exec,
						  [&frame_float_4](void*buf_ptr){
							frame_float_4.push_back( static_cast<float*>(buf_ptr));
						  });
				while( ( data_sent + 1 ) <= ( buffer.data_size / 4 ))
				  {
					no_more_input = main_loop_this->check_action();
					
					unsigned short ind = chan_begin;
					for( vector<float*>::iterator iter=frame_float_4.begin() ;
						 iter != frame_float_4.end();
						 iter++ )
					  {
						sample_val = (*main_loop_this->signal_list[ind++])();
						
						*(*iter)++ = ((float)sample_val)/32768.0;
					  }
					sample_action_count ++;			  
					data_sent += 1;
				  }
			return make_tuple(no_more_input,sample_action_count);
			  }
			}
	})
{}

unsigned long main_loop::send_to_sound_output::operator()(sound_file_output_buffer&buffer)
{
    if ( buffer.channels_bounds.first >= main_loop_this->signal_list.size() ||
	   buffer.channels_bounds.first == buffer.channels_bounds.second )
	{
	  //  There is no match between the boundary requested by the audio output
	  //    and the boundary defined in the parameters.
	  //  One reason is: there is no audio output
	  //  Another reason is: in the future a list of buffers might be implemented
	  //    The goal is each buffer takes a set of outputs (with or without overlap)
	  for_each ( buffer.data.begin(), buffer.data.end(), [&](void*buf_ptr){
		  fill( static_cast<char*>(buf_ptr), static_cast<char*>(buf_ptr) + buffer.data_size , 0 );
	  	});
	  if ( main_loop_this->exec_actions() )
	   	// ignore all the buffer members, simply tells the time (1mS) supposed to elapsed
		return 1000;
	  else
		// Run is finished, here there is no shutdown to handle, tell to leave now
		return 0;
	}
  else
	{
	  // the intersect is never empty, otherwyse one would have left above
	  // The assume is the parameters channels id always starts at 0
	  // This should be IMPROVED
	  if ( buffer.channels_bounds.second == numeric_limits<unsigned short>::max() )
		{
		  // The buffer is unlimited, use the size of the signals
		  chan_end = main_loop_this->signal_list.size();
		  chan_begin = max( buffer.channels_bounds.first, (const unsigned short)0 );
		  frame_size_exec = ( chan_end - chan_begin );
		  frame_size_requested = frame_size_exec;
		}
	  else
		{
		  chan_end = min( buffer.channels_bounds.second,
						  (unsigned short)main_loop_this->signal_list.size());
		  // See above
		  chan_begin = max( buffer.channels_bounds.first, (const unsigned short) 0 );
		  frame_size_exec = ( chan_end - chan_begin );
		  frame_size_requested =  ( buffer.channels_bounds.second - buffer.channels_bounds.first );
		}
	  if( main_loop_this->debug_once == false )
		{
		  cout << "Excecution begin: "<< chan_begin <<", end: "<<chan_end;
		  cout << ", size exec: " << frame_size_exec << ", size req: " << frame_size_requested<<endl;
		}
	  main_loop_this->debug_once = true;

	  bool no_more_input = true;

	  // There are many copy and paste
	  // Since this is a time critical part, I prefer to switch and to loop
	  //   rather than to lopp and to switch

	  if ( buffer.data.empty() )
		{
		  bool action_leave = false;
		  for( unsigned short ind = 0; ind < main_loop_this->samples_per_param_check; ind ++ ) 
			if ( main_loop_this->check_action() )
			  action_leave = true;
		  if( action_leave == false )
			return 1000;
		  else
			return 0;
		}
   	  unsigned long data_sent = 0;
	  short sample_val;
	  auto sample_type_found = sample_type_defs_list.find( buffer.type_float_interleave );
	  tuple<bool,unsigned long>ssfo;
	  if ( sample_type_found != sample_type_defs_list.end() )
		{
		  ssfo = sample_type_found->second(buffer);
		} else
		main_loop_this->sample_type_not_found = true;
	  if( get<0>(ssfo) )
		{
		  return 0;
		}
	  else
		{
		  unsigned long elapsed_time = get<1>(ssfo) * 1000;
		  return elapsed_time / main_loop_this->sample_rate_id ;
		}
	}
}
