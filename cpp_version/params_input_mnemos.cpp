#include "params_input_mnemos.hxx"
#include <cmath>

mnemo_event::mnemo_event():
  status( warming_up )
{
  TS_left.reserve(15);
  TS_right.reserve(15);
  TS_unit.reserve(15);
  channel.reserve(10);
  mnemo.reserve(10);
  value_left.reserve(15);
  value_right.reserve(15);
  value_unit.reserve(15);
}
mnemo_event::mnemo_event(const string&TS_left,const string&TS_right,const string&TS_unit,
						 const string&channel,
						 const string&mnemo,
						 const string&value_left,const string&value_right,const string&value_unit):
  TS_left(TS_left),TS_right(TS_right),TS_unit(TS_unit),
  channel(channel),
  mnemo(mnemo),
  value_left(value_left),value_right(value_right),value_unit(value_unit)
{}


mnemos_bytes_stream::mnemos_bytes_stream(ostream&os,
										 const bool&with_time_stamp,
										 input_params_base::clearing_t&clearing ):
  info_out_str(os),
  with_time_stamp( with_time_stamp ),
  track_line( 1 ),
  line_state( ls_start ),
  state( state_end ),
  mbs_clearing( clearing )
{}

bool mnemos_bytes_stream::get_event(istream&i_stm)
{
  // Since it is a stream char iterator, this function checks for the eof before each read
  // The file might be finish or the stream is temporarily starving.
  // In such case, the function does not read anything and returns
  //   until the eof "is fixed"
  unsigned char val_read;
  
  enum { shoot_line, clear_line, idle_line } action_line;
		
  if( state == state_end )
	{
	  if( with_time_stamp == true )
		{
		  state = state_ts;
		}
	  else
		state = state_code;
	  //	  user_str = string();
	}

  while( (i_stm.eof() == false) && (status != end_track) && (state != state_end) )
	{
	  action_line = idle_line;
	  i_stm.read( (char*)(&val_read) , 1 );
	  if ( ( val_read >= 'a' && val_read <= 'z' ) || ( val_read >= 'A' && val_read <= 'Z' ) || val_read == '/' )
	    {
	      if ( val_read >= 'A' && val_read <= 'Z' )
			val_read += 'a' - 'A';
	      switch ( line_state )
			{
			case ls_in_ts_left:
			case ls_in_ts_right:
			  line_state = ls_in_ts_unit;
			case ls_in_ts_unit:
			  TS_unit += val_read;
			  break;
			case ls_spctab_ts:
			  line_state = ls_in_channel;
			case ls_in_channel:
			  channel += val_read;
			  break;
			case ls_spctab_channel:
			  line_state = ls_in_mnemo;
			case ls_in_mnemo:
			  mnemo += val_read;
			  break;
			case ls_spctab_mnemo:
			  info_out_str << "Line " << track_line << ": numerical digits are expected for the value" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_in_val_left:
			case ls_in_val_right:
			  line_state = ls_in_val_unit;
			case ls_in_val_unit:
			  value_unit += val_read;
			  break;
			case ls_spctab_val:
			  info_out_str << "Line " << track_line << ": warning, please start comments with a comment character" << endl;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_in_crlf:
			case ls_start:
			  info_out_str << "Line " << track_line << ": numerical digits are expected for the time-stamp" << endl;
			  // may be irrelevant is ls_start state
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_wait_eol_comment:
			  break;
			}
		  
	    }
	  else if ( val_read >= '0' && val_read <= '9' )
	    {
	      switch ( line_state )
			{
			case ls_in_crlf:
			case ls_start:
			  line_state = ls_in_ts_left;
			case ls_in_ts_left:
			  TS_left += val_read;
			  break;
			case ls_in_ts_right:
			  TS_right += val_read;
			  break;
			case ls_in_ts_unit:
			  info_out_str << "Line " << track_line << ": no numerical digit is allowed in the TS unit" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_ts:
			  line_state = ls_in_channel;
			case ls_in_channel:
			  channel += val_read;
			  break;
			case ls_spctab_channel:
			  line_state = ls_in_mnemo;
			case ls_in_mnemo:
			  mnemo += val_read;
			  break;
			case ls_spctab_mnemo:
			  line_state = ls_in_val_left;
			case ls_in_val_left:
			  value_left += val_read;
			  break;
			case ls_in_val_right:
			  value_right += val_read;
			  break;
			case ls_in_val_unit:
			  info_out_str << "Line " << track_line << ": no numerical digit is allowed in the value unit" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_val:
			  info_out_str << "Line " << track_line << ": warning, please start comments with a comment character" << endl;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_wait_eol_comment:
			  break;
			}
	    }
	  else if ( val_read == '.' || val_read == ',' )
	    {
	      switch ( line_state )
			{
			case ls_in_ts_left:
			case ls_start:
			  line_state = ls_in_ts_right;
			  break;
			case ls_in_ts_right:
			  info_out_str << "Line " << track_line << ": no second decimal separator is allowed in the TS" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_in_ts_unit:
			  info_out_str << "Line " << track_line << ": no decimal separator is allowed in the TS unit" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_ts:
			case ls_in_channel:
			  info_out_str << "Line " << track_line << ": no decimal separator is allowed in the channel" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_channel:
			case ls_in_mnemo:
			  info_out_str << "Line " << track_line << ": no decimal separator is allowed in the mnemonic" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_mnemo:
			case ls_in_val_left:
			  line_state = ls_in_val_right;
			  break;
			case ls_in_val_right:
			  info_out_str << "Line " << track_line << ": no second decimal separator is allowed in the value" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_in_val_unit:
			  info_out_str << "Line " << track_line << ": no second decimal separator is allowed in the value unit" << endl;
			  action_line = clear_line;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_spctab_val:
			  info_out_str << "Line " << track_line << ": warning, please start comments with a comment character" << endl;
			  line_state = ls_wait_eol_comment;
			  break;
			case ls_wait_eol_comment:
			  break;
			case ls_in_crlf:
			  line_state = ls_in_ts_right;
			  break;
			}
	    }
	  else if ( val_read == '\n' || val_read == '\r' )
	    {
	      switch ( line_state )
			{
			case ls_start:
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_in_ts_left:
			case ls_in_ts_right:
			case ls_in_ts_unit:
			case ls_spctab_ts:
			  info_out_str << "Line" << track_line << ": the line format is wrong, channel, mnemonic and value fields are missing" << endl;
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_in_channel:
			case ls_spctab_channel:
			  info_out_str << "Line" << track_line << ": the line format is wrong, mnemonic and value fields are missing" << endl;
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_in_mnemo:
			case ls_spctab_mnemo:
			  info_out_str << "Line" << track_line << ": the line format is wrong, value field are missing" << endl;
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_in_val_left:
			case ls_in_val_right:
			case ls_in_val_unit:
			  action_line = shoot_line;
			case ls_spctab_val:
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_wait_eol_comment:
			  crlf_first_used = val_read;
			  track_line += 1;
			  break;
			case ls_in_crlf:
			  if ( crlf_first_used == val_read )
				track_line += 1;
			  break;
			}
	      line_state = ls_in_crlf;
	    }
	  else if ( val_read == ' ' || val_read == '\t' )
	    {
	      switch ( line_state )
			{
			case ls_in_ts_left:
			  line_state = ls_spctab_ts;
			  break;
			case ls_in_ts_right:
			case ls_in_ts_unit:
			  line_state = ls_spctab_ts;
			  break;
			case ls_spctab_ts:
			  break;
			case ls_in_channel:
			  line_state = ls_spctab_channel;
			  break;
			case ls_spctab_channel:
			  break;
			case ls_in_mnemo:
			  line_state = ls_spctab_mnemo;
			  break;
			case ls_spctab_mnemo:
			  break;
			case ls_in_val_left:
			  line_state = ls_spctab_val;
			  action_line = shoot_line;
			  break;
			case ls_in_val_right:
			case ls_in_val_unit:
			  line_state = ls_spctab_val;
			  action_line = shoot_line;
			  break;
			case ls_spctab_val:
			  break;
			case ls_wait_eol_comment:
			  break;
			case ls_start:
			case ls_in_crlf:
			  info_out_str << "Line " << track_line << ": comments or instructions should start at column 1" << endl;
			  line_state = ls_wait_eol_comment;
			  break;
			}
	    }
	  else if ( val_read == '#' || val_read == ';' || val_read == '%' )
	    {
		  if ( line_state == ls_in_val_left || line_state == ls_in_val_right || line_state == ls_in_val_unit )
			{
			  line_state = ls_in_val_unit;
			  value_unit += val_read;
			}
		  else
			{
			  // Comment starts here
			  // If the line is complete or empty, no error message is sent
  			  if ( line_state != ls_start && line_state != ls_wait_eol_comment && line_state != ls_spctab_val && line_state != ls_in_crlf  )
				info_out_str << "Line " << track_line << ": the line format is wrong " << val_read << "  " << (short)line_state << endl;
			  line_state = ls_wait_eol_comment;
			}
	    }
	  else
	    {
	      // If we are already in a comment it is OK,
	      // Otherwise the line is discarded
	      if ( line_state != ls_wait_eol_comment )
			info_out_str << "Line " << track_line << ": exotic characters ( " << val_read << " ) allowed only in comments" << endl;
	    }
	  switch ( action_line )
	    {
		case shoot_line:
		  state = state_end;
		  break;
		case clear_line:
  // The structure should change. For now initialize one by one
		  TS_left.clear();
		  TS_right.clear();
		  TS_unit.clear();
		  channel.clear();
		  mnemo.clear();
		  value_left.clear();
		  value_right.clear();
		  value_unit.clear();
		  break;
		case idle_line:
		  break;
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

mnemos_bytes_datagram_test::mnemos_bytes_datagram_test(ostream&os,
										 const bool&with_time_stamp,
										 input_params_base::clearing_t&clearing ):
  info_out_str(os),
  mbs_clearing( clearing )
{
  if ( with_time_stamp )
	info_out_str << "With time stamps is required, since this is a test module, it is omitted" << endl;
}

bool mnemos_bytes_datagram_test::get_event(deque<mnemo_event>::const_iterator&curr,
										   deque<mnemo_event>::const_iterator&end)
{
  if( curr != end )
	{
	  TS_left = (*curr).TS_left;
	  TS_right = (*curr).TS_right;
	  TS_unit = (*curr).TS_unit;
	  channel = (*curr).channel;
	  mnemo = (*curr).mnemo;
	  value_left = (*curr).value_left;
	  value_right = (*curr).value_right;
	  value_unit = (*curr).value_unit;

	  curr++;
	  return true;
	}else
	{
	  mbs_clearing = input_params_base::c_end_of_data;
	  return false;
	}
}

string input_params_mnemos_2_action::FreqDelay_strings_2_val(unsigned long&value,
															 bool default_freq_not_seconds,
															 const char&post_proc) const
{
  // This does not perform any to-lower.
  // It should be done at the general preprocessing
  char power10 = 0;
  bool freq_not_seconds;
  string unit_without_scale;
  string current_unit;
  if ( post_proc != 20 )
	current_unit = the_event.value_unit;
  else
	current_unit = the_event.TS_unit;

  if ( current_unit.empty() == false )
	{
	  string::value_type scale = current_unit[ 0 ];
	  switch( scale )
		{
		case 'm':
		  // Check here if it is meg
		  if ( current_unit.size() >= 3 )
			{
			  if ( current_unit[ 1 ] == 'e' && current_unit[ 2 ] == 'g' )
				return string( "Sorry, mega (" ) + current_unit + string( ") is not supported" );
			  else
				{
				  power10 = -3;
				  unit_without_scale = current_unit.substr( 1 );
				}
			}
		  else
			{
			  power10 = -3;
			  unit_without_scale = current_unit.substr( 1 );
			}
		  break;
		case 'g':
		  return string( "Sorry, giga (" ) + current_unit + string( ") is not supported" );
		case 'n':
		  return string( "Sorry, nano (" ) + current_unit + string( ") is not supported" );
		case 'p':
		  return string( "Sorry, pico (" ) + current_unit + string( ") is not supported" );
		case 'k':
		  power10 = 3;
		  unit_without_scale = current_unit.substr( 1 );
		  break;
		case 'u':
		  power10 = -6;
		  unit_without_scale = current_unit.substr( 1 );
		  break;
		case 'h':
		case 's':
		  // It can be seconds or hertz, let's the code below doing the job
		  // power10 = 0; irrelevant
		  unit_without_scale = current_unit;
		  break;
		default:
		  return string( "Sorry, the unit " ) + current_unit + string( " is unknown" );
		}
	}
  // irrelevant else here to set unit_without_scale as it is initialized to empty
  // step 2:
  if ( unit_without_scale.compare( "hz" ) == 0 || unit_without_scale.compare( "hertz" ) == 0 )
	{
	  freq_not_seconds = true;
	}
  else if ( unit_without_scale.compare( "s" ) == 0 || unit_without_scale.compare( "sec" ) == 0 )
	{
	  freq_not_seconds = false;
	}
  else if ( unit_without_scale.empty() )
	{
	  freq_not_seconds = default_freq_not_seconds;
	}
  else
	return string("The unit (" ) + unit_without_scale + string( ") is unknown" );

  // step 3: parse the value
  float the_val;
  if ( post_proc != 20 )
    {
      if ( the_event.value_left.empty() == false )
		the_val = float( stoul( the_event.value_left ));
      else
		the_val = 0;
      
	  if ( the_event.value_right.empty() == false )
		{
		  if ( the_event.value_right.size() < 6 )
			{
			  unsigned int the_dec = stoul( the_event.value_right );
			  float f1 = float( the_dec )/ float( pow( 10, the_event.value_right.size() ));
			  the_val += f1;
			}
		  else
			return string( "The value (" ) +
			  the_event.value_left + string( "." ) + the_event.value_right +
			  string( ") should not have more than 5 digits after the decimal separator.");
		}
	  if ( the_val == 0.0 )
		return "The value should never be null, use NOP instead";
    }else{
    if ( the_event.TS_left.empty() == false )
      the_val = float( stoul( the_event.TS_left ));
    else
      the_val = 0;
    
	if ( the_event.TS_right.empty() == false )
	  {
		if ( the_event.TS_right.size() < 6 )
		  {
			unsigned int the_dec = stoul( the_event.TS_right );
			float f1 = float( the_dec )/ float( pow( 10, the_event.TS_right.size() ));
			the_val += f1;
		  }
		else
		  return string( "The time-stamp (" ) +
			the_event.TS_left + string( "." ) + the_event.TS_right +
			string( ") should not have more than 5 digits after the decimal separator.");
	  }
  }
  // step 4: computes u, m or K
  // avoid irrelevant calculation
  switch ( power10 ) {
  case -3:
	the_val /= 1000;
	break;
  case -6:
	the_val /= 1000000;
	break;
  case 3:
	the_val *= 1000;
	break;
  }
  // step 5: check if S or Hz
  if ( default_freq_not_seconds != freq_not_seconds )
	{
	  if ( the_val < 0.0000015259022 )
		the_val = 1.0 / the_val;
	  else
		// Take the maximum valus
		the_val = 65535.0;
	}
  // step 6:
  switch ( post_proc )
	{
	case 1:
	  // Used by amplitude modulation frequency
	  the_val *= 16777216.0 / ( 48000.0 * 4.0 );
	  break;
	case 4:
	  // Used by pulse modulation frequency
	  // Used by base frequency
	  the_val *= 16777216.0 / ( 48000.0 * 4.0 * 4.0 );
	  break;
	case 8:
	  // Not used
	  the_val *= 16777216.0 / ( 48000.0 * 4.0 * 8.0 );
	  break;
	case -1:
	  // Used by slew rate
	  the_val = 16777216.0 * 2.0 / ( 48000.0 * 4.0 * the_val );
	  break;
	case -2:
	  // Used by high or low hold
	  the_val *= 48000.0 / ( 16.0 * 2.0 );
	  break;
	case 20:
	  //Used by the timestamps
	  // The default value of the samples_per_TS_unity is 5
	  // This precision is enough as it is 104.17uS
	  the_val *= 9600.0;
	  //	  cout << "TS :" << the_val << endl;
	  break;
	default:
	  return "Internal error";
	}
  if( the_val >= 16777215.0 )
	return "Value too big";
  the_val = ceil( the_val );
  value = (long)the_val;

  return string();
}
string input_params_mnemos_2_action::Channel_string_2_val(unsigned short&val) const
{
  // This supports both uppercase and lowercase
  unsigned short value;
  if ( the_event.channel.compare( "all" ) == 0 || the_event.channel.compare( "ALL" ) == 0 ||
	   the_event.channel.compare( "/" ) == 0 || the_event.channel.compare( "0" ) == 0 )
	{
	  val = 0;
	  return string();
	}
  else if ( ( value = stoul( the_event.channel )) != 0 )
	{
	  val = value;
	  return string();	  
	}
  else
	return "Channel id should be a integer or the keyword 'all'.";
}
string input_params_mnemos_2_action::Mode_strings_2_val(unsigned long&mode) const
{
  if ( the_event.value_unit.compare( "/" ) == 0 || the_event.value_unit.empty() )
	{
	  mode = 0;
	  if ( the_event.value_right.size() == 0 )
		{
		  if ( the_event.value_left.empty() == false )
			mode = stoul( the_event.value_left );
		  mode &= 0x03;
		  return string();
		}
	  else
		return "Mode should be an integer 0 or 1.";
	}
  else
	return "There is no unit for the mode.";
}
string input_params_mnemos_2_action::Depth_strings_2_val(unsigned long&val_0_255) const
{
  val_0_255 = 0;
  if ( the_event.value_unit.compare( "%" ) == 0 || the_event.value_unit.empty())
	{
	  unsigned int the_val = 0;
	  if ( the_event.value_left.empty() == false )
		the_val = stoul( the_event.value_left );
	  if ( the_val > 100 )
		return "The depth/volume value should be a positive lower or equal than 100";
	  if ( the_val == 0 )
		{
		  // scale from 0 to (excl) 1

		  if ( the_event.value_right.empty() == false )
			{
			  if ( the_event.value_right.size() < 6 )
				{
				  unsigned int the_dec = stoul( the_event.value_right );
				  unsigned long f1 = the_dec * pow( 10, ( 6 - the_event.value_right.size() ));
				  val_0_255 = f1 / 3921;
				  if ( val_0_255 > 255 )
					val_0_255 = 255;
				  return string();
				}
			  else
				{
				  val_0_255 = 0;
				  return "The depth/volume value should not have more than 5 digits after the decimal separator.";
				}
			}
		  else
			{
			  val_0_255 = 0;
			  return string();
			}
		}else
		{
		  if ( the_event.value_right.empty() == false )
			{
			  val_0_255 = the_val * 10 + stoul( the_event.value_right.substr(0,1) );
			  val_0_255 *= 255;
			  val_0_255 /= 1000;
			}else{
			val_0_255 = 2550 * the_val;
			val_0_255 /= 1000;
		  } 
		}
	  if ( val_0_255 >= 256 )
		return "The depth/volume value slightly exceed 100%";

	  return string();
	}
  else
	return "Unit should be nothing or %";
}
string input_params_mnemos_2_action::Angle_strings_2_val(unsigned long&val_0_15) const
{
  if ( the_event.value_unit.compare( "/" ) == 0 || the_event.value_unit.compare( "degrees" ) ||
	   the_event.value_unit.empty() )
	{
	  unsigned int the_val = 0;
	  if ( the_event.value_left.empty() == false )
		the_val = stoul( the_event.value_left );
	  if ( the_val == 0 )
		{
		  // 0 to (excluded) 1
		  if ( the_event.value_right.empty() == false )
			{
			  if ( the_event.value_right.size() < 6 )
				{
				  unsigned int the_dec = stoul( the_event.value_right );
				  unsigned long f1 = the_dec * pow( 10 , ( 6 - the_event.value_right.size() ));
				  val_0_15 = ( f1 + 1953 )/ 62500;
				  if ( val_0_15 == 16 )
					val_0_15 = 0;
				  return string();
				}
			  else
				{
				  val_0_15 = 0;
				  return "The angle should not have more than 5 digits after the decimal separator.";
				}
			}
		  else
			{
			  val_0_15 = 0;
			  return string();
			}
		}
	  else if ( the_val < 360 )
		{
		  // This allow to divide by 22.5 and to make a proper rounding
		  val_0_15 = ( 2 * the_val + 22 ) / 45;
		  if ( val_0_15 == 16 )
			val_0_15 = 0;
		  return string();
		}
	  else
		{
		  val_0_15 = 0;
		  return "The angle should be lower than 360.";
		}
	}
  else
	return "Unit does not mean an angle";
}


input_params_mnemos_2_action::input_params_mnemos_2_action( ostream&out_info_str,
															const mnemo_event&the_event,
															input_params_base::clearing_t&clearing,
															mnemo_event::status_t&status):
  info_out_str(info_out_str),
  the_event( the_event ),
  ipm2a_clearing( clearing ),
  ipm2a_status( status ),
  mrf({
	  {"of",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::base_freq;
		  return FreqDelay_strings_2_val(value,true,4);}},
	  {"os",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::main_ampl_slewrate;
		  return FreqDelay_strings_2_val(value,false,-1);}},
	  {"oa",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::main_ampl_val;
		  return Depth_strings_2_val( value );}},
	  {"aa",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::ampl_modul_depth;
		  return Depth_strings_2_val( value );}},
	  {"ba",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_depth;
		  return Depth_strings_2_val( value );}},
	  {"bm",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_modul_mode;
		  return Mode_strings_2_val( value );}},
	  {"am",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::ampl_modul_modul_mode;
		  return Mode_strings_2_val( value );}},
	  {"op",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::base_phase_shift;
		  return Angle_strings_2_val(value);}},
	  {"bp",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_phase_shift;
		  return Angle_strings_2_val(value);}},
	  {"ap",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::ampl_modul_phase_shift;
		  return Angle_strings_2_val(value);}},
	  {"oo",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::base_phase_set;
		  return Angle_strings_2_val(value);}},
	  {"bo",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_phase_set;
		  return Angle_strings_2_val(value);}},
	  {"ao",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::ampl_modul_phase_set;
		  return Angle_strings_2_val(value);}},
	  {"nn",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::nop;
		  value=0; return string();}},
	  {"af",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::ampl_modul_freq;
		  return FreqDelay_strings_2_val(value,true,1);}},
	  {"bf",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_freq;
		  return FreqDelay_strings_2_val(value,true,4);}},
	  {"bh",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_high_hold;
		  return FreqDelay_strings_2_val(value,false,-2);}},
	  {"bi",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::pulse_low_hold;
		  return FreqDelay_strings_2_val(value,false,-2);}}
	}),

  mnemos_list({
	  {P_abort,"NQ"},{P_abort,"nq"}})
{}



void input_params_mnemos_2_action::mnemos_2_action_run(vector<signals_param_action>&actions)
{
  signals_param_action action;
  decltype(mrf)::const_iterator mnemos_read_funcs_iter;
  multimap<short,string>::const_iterator mnemos_list_iter;
  string return_err_str;

  /*  if( the_event.mnemo.compare( "bh" ) == 0 || the_event.mnemo.compare( "bi" ) == 0|| the_event.mnemo.compare( "bf" ) == 0)
 	{
	  cout << "Mnemo to find: " << the_event.mnemo << "\t";
	  cout << the_event.TS_left << "." << the_event.TS_right << "\t" << the_event.channel << "\t";
	  cout << the_event.value_left << "." << the_event.value_right << endl;
	  }*/

  mnemos_read_funcs_iter = mrf.find(the_event.mnemo);
  if ( mnemos_read_funcs_iter != mrf.end() )
	{
	  unsigned long value;
	  signals_param_action::action_list act;
	  return_err_str = mnemos_read_funcs_iter->second( value, act );
	  if ( return_err_str.empty() )
		{
		  unsigned short val_s;
		  return_err_str = Channel_string_2_val( val_s );
		  action.channel_id = val_s;

		  action.action = act;

		  if ( value >= 65536 )
			{
			  cout << "**** Value " << value << " of mneno is too big *****" << endl; 
			  value = 65535;
			}
		  action.value = value;

		  actions.push_back( action );
		}
	  else
		cout << return_err_str << ", channel: " << the_event.channel << ", mnemo: " << the_event.mnemo << ", unknown channel" << endl;

	}else
	{
	  // Not found, 3 reasons
	  //   * mnemo handled in the old version
	  //   * mnemo of the extended commands ( store, retrieve etc... NOT YET )
	  //   * really not found
	  // The store retrieve is on hold as a decision has to be taken:
	  //   * does it move to the low level parameters ( recording parameter action structures )
	  //   * should it handled as file pointer. That means it is available in seekable inputs only
	  if ( mnemos_list_iter != mnemos_list.end() )
		{
		  unsigned long value;
		  unsigned char val_c;
		  mnemos_list_iter = find_if( mnemos_list.begin(), mnemos_list.end(),
									  [&](multimap<short,string>::const_reference a) -> bool {
										return a.second == the_event.mnemo;
									  });
		  switch( mnemos_list_iter->first )
			{
			case P_abort:
			  ipm2a_status = mnemo_event::end_track;
			  ipm2a_clearing = input_params_base::c_abort;
			  break;
			}
		  unsigned short val_s;
		  return_err_str += Channel_string_2_val( val_s );
		  action.channel_id = val_s;
		  
		  if ( return_err_str.empty() )
			{
			  actions.push_back( action );
			}
		  else
			cout << return_err_str << ", channel: " << the_event.channel << ", mnemo: " << the_event.mnemo << endl;

		}
	  else
		return_err_str = "Unknown mnemo. ";
	}
}

input_params_mnemos_byte_stream::input_params_mnemos_byte_stream(ostream&, istream&i_stm, const bool&with_time_stamp):
  i_stm( i_stm ),
  info_out_str_stream( ioss ),
  mnemos_bytes_stream( info_out_stream, with_time_stamp, ((input_params_base*)this)->clearing ),
  input_params_mnemos_2_action( info_out_stream, *((mnemo_event*)this), ((input_params_base*)this)->clearing, status )
{
  ioss.reserve( 100 );
}


void input_params_mnemos_byte_stream::import_next_event(vector<signals_param_action>&actions)
{
  mnemos_2_action_run( actions );
}

unsigned long input_params_mnemos_byte_stream::check_next_time_stamp()
{
  if ( get_event( i_stm ) )
	{
	  unsigned long value;
	  string err_ret_str = FreqDelay_strings_2_val( value, false, 20 );
	  if ( err_ret_str.empty() )
		return value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}
	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
bool input_params_mnemos_byte_stream::eot(void) const
{
  return false;
}
bool input_params_mnemos_byte_stream::is_ready(void)
{
  return true;
}


ostream&operator<<(ostream&the_out,const input_params_mnemos_byte_stream&)
{
  

  return the_out;
}


input_params_mnemos_file::input_params_mnemos_file( ostream&, ifstream&input_stream, const unsigned short&loops_counter ):
  info_out_str_stream( ioss ),
  input_params_mnemos_2_action( info_out_str_stream, *((mnemo_event*)this), ((input_params_base*)this)->clearing, status ),
  mnemos_bytes_stream( info_out_str_stream, true, ((input_params_base*)this)->clearing ),
  loops_counter( loops_counter ),
  if_stm( move( input_stream ))
{
  ioss.reserve( 100 );
  if ( if_stm.is_open() == false )
	{
	  status = end_track;
	  clearing = c_file_err;
	}
}
input_params_mnemos_file::~input_params_mnemos_file()
{
  // TODO check here if the length is the declared length. if not report a warning
  if_stm.close();
}

void input_params_mnemos_file::import_next_event(vector<signals_param_action>&actions)
{
  // cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  // cout << endl;

  mnemos_2_action_run( actions );
  // Reset the fields
  // The structure should change. For now initialize one by one
  TS_left.clear();
  TS_right.clear();
  TS_unit.clear();
  channel.clear();
  mnemo.clear();
  value_left.clear();
  value_right.clear();
  value_unit.clear();

  if ( info_out_str_stream.str().empty() == false )
	{
	  cout << info_out_str_stream.str();
	  info_out_str_stream.str("");
	}
}

unsigned long input_params_mnemos_file::check_next_time_stamp()
{
  if ( get_event( if_stm ) )
	{
	  unsigned long value;
	  string err_ret_str = FreqDelay_strings_2_val( value, false, 20 );
	  if ( err_ret_str.empty() )
		return value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}

	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
bool input_params_mnemos_file::eot()const
{
  return status == end_track;
}
bool input_params_mnemos_file::is_ready()
{
  if ( samples_per_TS_unity != 5 )
	cout << "Internal ERROR: samples_per_TS_unity not 5" << endl;
  return true;
}

/** \brief re-execute handler
 *
 * It computes the loop counter and tell if it is over
 * \return true after the last run
 */
bool input_params_mnemos_file::exec_loops(){
  cout << "Checking loop" << endl;
  if( loops_counter > 0 )
	{
	  if_stm.seekg( 0 );
	  status = running;
	  loops_counter -= 1;
	  return true;
	}else
	return false;
}

input_params_mnemos_hardcoded::input_params_mnemos_hardcoded(ostream&):
  info_out_str_stream( ioss ),
  mnemos_bytes_datagram_test( info_out_stream, false, ((input_params_base*)this)->clearing ),
  input_params_mnemos_2_action( info_out_stream, *((mnemo_event*)this), ((input_params_base*)this)->clearing, status ),
  the_list({{mnemo_event("0","0","s","1","nn","","","/")},
		{mnemo_event("","0","s","1","XX","22","5","/")},
		{mnemo_event("0","","","5","aa","1","20000","%")},
		{mnemo_event("1","2","","6","aa","100","4","%")},
		{mnemo_event("1","2","s","4","ba","100","0","%")},
		{mnemo_event("0","0","s","3","ba","90","0","")},
		{mnemo_event("1","2","s","4","ba","100","0","%")},
		{mnemo_event("1","2","s","7","bm","90","0","")},
		{mnemo_event("1","21","s","8","am","91","0","/")},
		{mnemo_event("1","21","s","9","am","1","1","/")},
		{mnemo_event("1","21","s","1","oo","22","5","/")},
		{mnemo_event("1","21","s","all","nn","466666666666666665","0","/")},
		{mnemo_event("1","21","s","all","bo","45","500","/")},
		{mnemo_event("1","21","s","3","ao","90","0","/")},
	    {mnemo_event("1","2","s","ALL","op","0","5","")},
        {mnemo_event("1","2","s","0","bp","45","500","/")},
	    {mnemo_event("1","2","s","1","ap","90","0","/")},
		{mnemo_event("1","2","s","20","of","0","20000","%")},
		{mnemo_event("1","2","s","21","of","100","4","")},
		{mnemo_event("1","2","s","22","of","1000","0","m")},
		{mnemo_event("1","2","s","23","of","90","0","mhz")},
		{mnemo_event("1","2","s","24","of","1","666","msec")},
		{mnemo_event("1","2","s","25","of","","00166","sec")},
		{mnemo_event("1","2","s","26","of","","600","khz")},
		{mnemo_event("1","2","s","27","of","1","600","hz")},
		{mnemo_event("1","2","s","30","os","0","0166","sec")},
        {mnemo_event("1","2","s","31","os","0","100","khz")},
		{mnemo_event("1","2","s","40","af","1","2000","%")},
		{mnemo_event("1","2","s","41","bf","100","04","")},
		{mnemo_event("1200","0","us","42","af","100","0","m")},
		{mnemo_event("1","2","","43","bf","90","0","mhz")},
		{mnemo_event("1","2","","44","af","1","066","msec")},
		{mnemo_event("1","2","","45","bf","0","0","sec")},
		{mnemo_event("1","2","s","50","bh","0","00166","sec")},
		{mnemo_event("1","2","s","51","bh","0","600","khz")},
		{mnemo_event("1","2","s","52","bi","0","0166","sec")},
        {mnemo_event("1","2","s","53","bi","10","0","s")}}
)
{
  the_list_iter = the_list.begin();
  the_list_end = the_list.end();
  ioss.reserve( 100 );
}
unsigned long input_params_mnemos_hardcoded::check_next_time_stamp()
{
  string return_err_str;
  if ( get_event( the_list_iter, the_list_end ) )
	{
	  unsigned long value;
	  string err_ret_str = FreqDelay_strings_2_val( value, false, 20 );
	  if ( err_ret_str.empty() )
		return value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}
	}else
	// Input is "starving", that should not be happened here
	return 0xffffffff;
}
void input_params_mnemos_hardcoded::import_next_event(vector<signals_param_action>&actions)
{
  //cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  //cout << (unsigned short)key ;

  mnemos_2_action_run( actions );
}
bool input_params_mnemos_hardcoded::eot(void) const
{
  return status == end_track;
}
bool input_params_mnemos_hardcoded::is_ready(void)
{
  if ( samples_per_TS_unity != 5 )
	cout << "Internal ERROR: samples_per_TS_unity not 5" << endl;
  return true;
}
bool input_params_mnemos_hardcoded::exec_loops()
{
  return false;
}
