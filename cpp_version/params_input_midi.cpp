#include "params_input_midi.hxx"

midi_event::midi_event():
  code( 0 ), key( 0 ), value( 0 ),
  status( warming_up )
{}

midi_bytes_stream::midi_bytes_stream(ostream&os,const bool&with_time_stamp, input_params_base::clearing_t&clearing ):
  info_out_str(os),with_time_stamp( with_time_stamp ),
  timestamp_construct( 0 ),
  track_tellg( 0 ),
  state( state_end ), header( 0 ),
  mbs_clearing( clearing )
{}
bool midi_bytes_stream::get_event(istream&i_stm)
{
  unsigned char val_read;
  
  if( state == state_end )
	{
	  if( with_time_stamp == true )
		{
		  timestamp_construct = 0;
		  state = state_ts;
		}
	  else
		state = state_code;
	  user_str = string();
	}

  while( (i_stm.eof() == false) && (status != end_track) && (state != state_end) )
	{
	  i_stm.read( (char*)(&val_read) , 1 );
	  // For future error plain text display
	  track_tellg += 1;
	  // info_out_str << hex << (unsigned short)val_read << '\t';
	  switch( state ){
		// Totally done here as the TS is always provided
	  case state_ts:
		if ( val_read > 127 )
		  {
			timestamp_construct += val_read;
			timestamp_construct -= 128;
			timestamp_construct *= 128;
		  } else {
		  timestamp_construct += val_read;
		  state = state_code;
		}
		break;
		// Totally done here as the midi code is always provided
	  case state_code:
		code = val_read;
		state = state_key;
		if ( code < 128 )
		  {
			cerr << "Error reading midi smf/mid file: position " << track_tellg << ": code "<< hex << key << " shoud have its high bit set to one" << endl;
		  }
		break;
		// Some codes may not expect parameters, they should not be in the file
		// TODO fix that
	  case state_key:
		key = val_read;
		state = state_val;
		break;
	  case state_val:
		value = val_read;
		if ( code != 0xff )
		  {
			state = state_end;
		  }
		else {
		  switch ( key )
			{
			case 03:
			  if ( value > 0 )
				{
				  state = state_string;
				  str_length = value - 1;
				}
			  else
				{
				  state = state_end;
				}
			  // end of track
			  break;
			case 0x2f:
			  status = end_track;
			  mbs_clearing = input_params_base::c_end_of_data;
			  state = state_end;
			  break;
			default:
			  cerr << "Error reading midi smf/mid file: position " << track_tellg << ": unknown FF "<< hex << key << endl;
			  break;
			}
		}
		break;
	  case state_string:
		  user_str += (string::value_type)val_read;
		if ( str_length > 0 )
		  {
			str_length -= 1;
		  }
		else
		  {
			state = state_end;
		  }
	  }	
	}
  if ((state == state_end) && (status != end_track))
	{
	  return true;
	}
  else
	{
	  return false;
	}
}

ostream& operator<<( ostream&the_out , const midi_bytes_stream&me )
{
  the_out << "Timestamp: " << hex << me.timestamp_construct << ", code: " << (unsigned short)me.code;
  the_out << ", key/note: " << (unsigned short)me.key;
  the_out << ", value: " << (unsigned short)me.value << ", optional str: " << me.user_str;
  return the_out;
}

input_params_midi_2_action::input_params_midi_2_action(const midi_event&the_event,
													   input_params_base::clearing_t&clearing,
													   midi_event::status_t&status):
  the_event( the_event ),
  ipm2a_clearing( clearing ),
  ipm2a_status( status )
{}


unsigned long input_params_midi_2_action::get_value( const unsigned char&exponent_size,
													 const unsigned char&exponent_const )const{
  unsigned long val( the_event.value );

  if ( exponent_const > 0 )
	{
	  val <<= exponent_const;
	}
  unsigned char expo_ind( exponent_size );
  unsigned short expo_mask( 1 );
  while( expo_ind > 0 )
	{
	  expo_ind -= 1;
	  expo_mask *= 2;
	}
  expo_mask -= 1;
  
  // info_out_str << "  ___ " << (unsigned short)exponent_size << " _ " << (unsigned short)expo_mask << " _ ";
  // info_out_str << (unsigned short)exponent_const << " ___  ";
  val <<= (expo_mask & the_event.key); 
 
  return val;
}


void input_params_midi_2_action::midi_2_action_run(vector<signals_param_action>&actions)
{
  // float val_float;
  unsigned long long_value;
  signals_param_action action;
  
  action.channel_id = the_event.code & 0x0f;
  switch( the_event.code & 0xf0 )
	{
	case 0x80:
	  // info_out_stream << "Note off, only for handling the time stamp" << endl;
	  break;
	case 0x90:
	  switch( the_event.key & 0xf8 )
		{
		case 0x00:
		  long_value = get_value( 3, 1 );
		  // val_float = (float)long_value * 48000.0 * 4.0 * 8.0 / 16777216.0;
		  // info_out_stream << "Set base frequency " << hex << long_value << ", means: " << val_float;
		  // info_out_stream << "Hz (step: " << 2.0 * 48000.0 * 4.0 * 8.0 / 16777216.0 << ')' << endl;
		  action.action = signals_param_action::base_freq;
		  action.value = long_value;
		  break;
		case 0x08:
		  long_value = get_value( 3, 0 );
		  // val_float = 16777216.0 / ( (float)long_value * 48000 * 4.0 );
		  // info_out_stream << "Set global amplitude slewrate " << hex << long_value;
		  // info_out_stream << ", means " << val_float;
		  // info_out_stream << "s 0-255 (step: 1/" << 16777216.0 /( 48000.0 * 4.0 ) << ')' << endl;
		  action.action = signals_param_action::main_ampl_slewrate;
		  action.value = long_value;
		  break;
		case 0x10:
		  switch( the_event.key & 0x7 )
			{
			case 0x00:
			case 0x01:
			  long_value = get_value( 1, 0 );
			  // info_out_stream << "Sets the amplitude " << hex << long_value << ", dec: " << dec << long_value << endl;
			  action.action = signals_param_action::main_ampl_val;
			  action.value = long_value;
			  break;
			case 0x02:
			  // Abort
			  ipm2a_status = midi_event::end_track;
			  ipm2a_clearing = input_params_base::c_abort;
			  break;
			case 0x04:
			  long_value = get_value( 0, 1 );
			  // info_out_stream << "Sets the amplitude modulation depth " << hex << long_value;
			  // info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			  action.action = signals_param_action::ampl_modul_depth;
			  action.value = long_value;
			  break;
			case 0x05:
			  long_value = get_value( 0, 1 );
			  // info_out_stream << "Sets the pulse depth " << hex << long_value;
			  // info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			  action.action = signals_param_action::pulse_depth;
			  action.value = long_value;
			  break;
			case 0x06:
			  // info_out_stream << "Sets phase shift " << hex << long_value;
			  // 
			  action.value = get_value( 0, 0 );
			  switch( action.value & 0x70 )
				{
				case 0x00:
				  switch( action.value & 0x0c )
					{
					case 0x00:
					  action.action = signals_param_action::pulse_modul_mode;
					  // irrelevant action.value &= 0x03;
					  break;
					case 0x04:
					  action.action = signals_param_action::ampl_modul_modul_mode;
					  action.value &= 0x03;
					  break;
					}
				  break;
				case 0x10:  action.action = signals_param_action::base_phase_shift;  break;
				case 0x20:  action.action = signals_param_action::pulse_phase_shift;  break;
				case 0x30:  action.action = signals_param_action::ampl_modul_phase_shift;  break;
				case 0x50:  action.action = signals_param_action::base_phase_set;  break;
				case 0x60:  action.action = signals_param_action::pulse_phase_set;  break;
				case 0x70:  action.action = signals_param_action::ampl_modul_phase_set;  break;
				}
			  action.value &= 0x0f;
			  break;
			case 0x03:
			  action.action = signals_param_action::nop;
			  break;
			}
		  break;
		case 0x18:
		  long_value = get_value( 3, 0 );
		  // val_float = (float)long_value * 48000.0 * 4.0 * 4.0 / 16777216.0;
		  // info_out_stream << "Sets the amplitude modulation frequency " << hex << long_value;
		  // info_out_stream << ", means: " << val_float;
		  // info_out_stream << "Hz (step: " << 48000.0 * 4.0 * 4.0 / 16777216.0 << ')' << endl;
		  action.action = signals_param_action::ampl_modul_freq;
		  action.value = long_value;
		  break;
		case 0x20:
		  if ( ( the_event.key & 0x04 ) == 0x0 )
			{
			  long_value = get_value( 2, 0 );
			  // info_out_stream << "Sets the pulse data " << hex << long_value;
			  // info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			  action.action = signals_param_action::pulse_freq;
			  action.value = long_value;
			} else {  // key == 0x24
			long_value = get_value( 2, 6 );
			// info_out_stream << "Sets the pulse data " << hex << long_value;
			// info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			action.action = signals_param_action::pulse_high_hold;
			action.value = long_value;
		  }
		  break;		  
		case 0x28:
		  if ( ( the_event.key & 0x04 ) == 0x0 )
			{
			  long_value = get_value( 2, 6 );
			  // info_out_stream << "Sets the pulse data " << hex << long_value;
			  // info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			  action.action = signals_param_action::pulse_low_hold;
			  action.value = long_value;
			} else { // key == 0x2c
			long_value = get_value( 2, 6 );
			// info_out_stream << "Sets the pulse data " << hex << long_value;
			// info_out_stream << ", means: " << dec << long_value << " (0-255)"<<endl;
			action.action = signals_param_action::nop;
			action.value = long_value;
		  }
		  break;		  
		default:
		  // info_out_stream << "Unknown action" << endl;
		  break;
		}
	  actions.push_back( action );
	  break;
	case 0xf0:
	  if ( the_event.code == 0xff )
		{
		  switch ( the_event.key )
			{
			case 0x03:
			  // info_out_stream << "Name of track: " << user_str << endl;
			  action.action = signals_param_action::track_name;
			  actions.push_back( action );
			  break;
			case 0x2f:
			  // info_out_stream << "Explicit end of track" << endl;
			  break;
			default:
			  // info_out_stream << "Unknown control code" << endl;
			  break;
			}
		}// else
		// info_out_stream << "Unknown midi code" << endl;
	}			  
}


input_params_midi_byte_stream::input_params_midi_byte_stream(istream&i_stm, const bool&with_time_stamp):
  i_stm( i_stm ),
  midi_bytes_stream( info_out_stream, with_time_stamp, ((input_params_base*)this)->clearing ),
  input_params_midi_2_action( *((midi_event*)this), ((input_params_base*)this)->clearing, status )
{}


void input_params_midi_byte_stream::exec_next_event(vector<signals_param_action>&actions)
{
  midi_2_action_run( actions );
}

unsigned long input_params_midi_byte_stream::check_next_time_stamp()
{
  if ( get_event( i_stm ) )
	{
	  // Something read
	  //	  info_out_stream<< (*this);
	  return timestamp_construct;
	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}


ostream&operator<<(ostream&the_out,const input_params_midi_byte_stream&)
{
  

  return the_out;
}


input_params_midi_file::input_params_midi_file( ifstream&input_stream, const unsigned short&loops_counter ):
  input_params_midi_2_action( *((midi_event*)this), ((input_params_base*)this)->clearing, status ),
  midi_bytes_stream( info_out_stream, true, ((input_params_base*)this)->clearing ),
  loops_counter( loops_counter ),
  if_stm( move( input_stream ))
{
  if ( if_stm.is_open() == false )
	{
	  status = end_track;
	  clearing = c_file_err;
	}
}
input_params_midi_file::~input_params_midi_file()
{
  // TODO check here if the length is the declared length. if not report a warning
  if_stm.close();
}

void input_params_midi_file::exec_next_event(vector<signals_param_action>&actions)
{
  //cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  //cout << (unsigned short)key ;

  midi_2_action_run( actions );
}

unsigned long input_params_midi_file::check_next_time_stamp()
{
  if ( get_event( if_stm ) )
	{
	  // Something read
	  //	  info_out_stream<< (*this);
	  // cout << (unsigned short)key ;
	  return timestamp_construct;
	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
bool input_params_midi_file::eot()const
{
  return status == end_track;
}
bool input_params_midi_file::is_ready()
{
  unsigned char val_read;
  bool QuaterNote_not_SMPTE;
  while( (if_stm.eof() == false) && (status == warming_up) )
	{
	  if_stm.read( (char*)(&val_read) , 1 );
	 
	  switch( header )
		{
		case 0 : if ( val_read != 'M' ) { status = end_track;  clearing = c_data_err;  } break;
		case 1 : if ( val_read != 'T' ) { status = end_track;  clearing = c_data_err;  } break;
		case 2 : if ( val_read != 'h' ) { status = end_track;  clearing = c_data_err;  } break;
		case 3 : if ( val_read != 'd' ) { status = end_track;  clearing = c_data_err;  } break;
		case 12 :
		  if ( val_read < 128 )
			{
			  QuaterNote_not_SMPTE = true;
			  // Since we are here, the high bit is always 0
			  samples_per_TS_unity = static_cast<unsigned short>( val_read );			  
			}
		  else
			{
			  QuaterNote_not_SMPTE = false;
			  samples_per_TS_unity = static_cast<unsigned short>( - static_cast<char>( val_read ));
			  if ( samples_per_TS_unity != 10 &&
				   samples_per_TS_unity != 24 &&
				   samples_per_TS_unity != 25 &&
				   samples_per_TS_unity != 29 &&
				   samples_per_TS_unity != 30 )
				{
				  cerr << "Sorry the images per second should be 10(compatibility mode), 24, 25, 29 or 30" << endl;
				  status = end_track;
				  clearing = c_data_err;			  
				}
			}
		  break;
		case 13 :
		  if ( QuaterNote_not_SMPTE )
			{
			  samples_per_TS_unity *= 128;
			  samples_per_TS_unity += val_read;
			  // Before execution of the next line,
			  //   samples_per_TS_unity contains the number of ticks per quarter note
			  // We assume a metronome at 60 (per minute)
			  //   then the number of samples per tick
			  //   is the samples per second divided by the ticks per second
			  if ( samples_per_TS_unity > 0 )
				{
				  samples_per_TS_unity =
					static_cast<unsigned short>( samples_per_second_base /
												 samples_per_TS_unity );
				  cout << "Sorry, the ticks per quarter note has not been tested" << endl;
				}
			  else
				{
				  samples_per_TS_unity = 30000;
				  status = end_track;
				  clearing = c_data_err;
				}
			}
		  else
			{
			  // Before execution of the next line,
			  //   sample_per_TS_unity holds the number of images per seconds 
			  samples_per_TS_unity *= val_read;
			  // Before execution of the next line,
			  //   samples_per_TS_unity hold the number of ticks per second
			  // The cast and the division are Ok as:
			  //   * the number of images per second is at least 10 in compatibility mode
			  //       or at least 24
			  //   * the number of ticks per image is an integer at least 1
			  //   * the samples_per_second_base should not exceed 655350
			  //       even for 384KHz, use a lower base and the sample_rate multiplier
			  samples_per_TS_unity =
				static_cast<unsigned short>( samples_per_second_base /
											 samples_per_TS_unity );
			  // Now, samples_per_TS_unity holds the number of samples per tick
			}
		  cout << "Samples_per TS unity: " << samples_per_TS_unity << endl;
		  break;
		case 14 : if ( val_read != 'M' ) { status = end_track;  clearing = c_data_err;  } break;
		case 15 : if ( val_read != 'T' ) { status = end_track;  clearing = c_data_err;  } break;
		case 16 : if ( val_read != 'r' ) { status = end_track;  clearing = c_data_err;  } break;
		case 17 : if ( val_read != 'k' ) { status = end_track;  clearing = c_data_err;  } break;
		  // TODO complete the header checking
		  // In order to run with beats others than code 6
		}
	  header += 1;
	  if ( header == 22 )
		{
		  status = running;
		}
	}

  return status != warming_up;
}
/** \brief re-execute handler
 *
 * It computes the loop counter and tell if it is over
 * \return true after the last run
 */
bool input_params_midi_file::exec_loops(){
  cout << "Checking loop" << endl;
  if( loops_counter > 0 )
	{
	  if_stm.seekg( 22 );
	  status = running;
	  loops_counter -= 1;
	  return true;
	}else
	return false;
}



	
input_params_midi_connec::input_params_midi_connec()
{
}
unsigned long input_params_midi_connec::check_next_time_stamp()
{
  return 0xffffffff;
}
bool input_params_midi_connec::eot()const
{
  return true;
}

