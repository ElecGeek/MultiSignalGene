#include "params_input_mnemos.hxx"

mnemo_event::mnemo_event():
  status( warming_up )
{}
mnemo_event::mnemo_event(const string&TS_left,const string&TS_right,
						 const string&channel,
						 const string&mnemo,
						 const string&value_left,const string&value_right,const string&value_unit):
  TS_left(TS_left),TS_right(TS_right),
  channel(channel),
  mnemo(mnemo),
  value_left(value_left),value_right(value_right),value_unit(value_unit)
{}


mnemos_bytes_stream::mnemos_bytes_stream(ostream&os,
										 const bool&with_time_stamp,
										 input_params_base::clearing_t&clearing ):
  info_out_str(os),
  with_time_stamp( with_time_stamp ),
  timestamp_construct( 0 ),
  track_line( 0 ),
  state( state_end ),
  mbs_clearing( clearing )
{}

bool mnemos_bytes_stream::get_event(istream&i_stm)
{
  // Since it is a stream char iterator, this function checks for the eof before each read
  // The file might be finish or the stream is temporarely starving.
  // In such case, the function does not read anything and returns
  //   until the eof "is fixed"
  unsigned char val_read;
  
  // TODO  TODO  TODO

  // This is only to compile and link

  if( state == state_end )
	{
	  if( with_time_stamp == true )
		{
		  timestamp_construct = 0;
		  state = state_ts;
		}
	  else
		state = state_code;
	  //	  user_str = string();
	}

  while( (i_stm.eof() == false) && (status != end_track) && (state != state_end) )
	{
	  i_stm.read( (char*)(&val_read) , 1 );
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
  timestamp_construct( 0 ),
  mbs_clearing( clearing )
{
  if ( with_time_stamp )
	info_out_str << "With time stamps is required, since this is a test module, it is omitted" << endl;
}

bool mnemos_bytes_datagram_test::get_event(deque<mnemo_event>::const_iterator&curr,
										   deque<mnemo_event>::const_iterator&end)
{
  timestamp_construct = 0;

  if( curr != end )
	{
	  TS_left = (*curr).TS_left;
	  TS_right = (*curr).TS_right;
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

string input_params_mnemos_2_action::Channel_string_2_val(unsigned short&val) const
{
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
string input_params_mnemos_2_action::Mode_strings_2_val(unsigned char&mode) const
{
  if ( the_event.value_unit.compare( "/" ) == 0 || the_event.value_unit.empty() )
	{
	  mode = 0;
	  if ( stoul( the_event.value_right ) == 0 )
		{
		  mode = stoul( the_event.value_left );
		  mode &= 0x01;
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
	  unsigned int the_val = stoul( the_event.value_left );
	  if ( the_val > 100 )
		return "The value should be a positive lower or equal than 100";
	  if ( the_val == 0 )
		{
		  // scale from 0 to (excl) 1 TODO
		}else
		{
		  val_0_255 = the_val * 10 + stoul( the_event.value_right.substr(0,1) );
		  val_0_255 *= 255;
		  val_0_255 /= 1000;
		}
	  if ( val_0_255 >= 256 )
		return "The value slightly exceed 100%";

	  return string();
	}
  else
	return "Unit should be nothing or %";
}
string input_params_mnemos_2_action::Angle_strings_2_val(unsigned long&val_0_15) const
{
  if ( the_event.value_unit.compare( "/" ) == 0 || the_event.value_unit.compare( "degres" ) ||
	   the_event.value_unit.empty() )
	{
	  unsigned int the_val = stoul( the_event.value_left );
	  unsigned int the_dec = stoul( the_event.value_right );
	  if ( the_val == 0 )
		{
		  // 0 to (excluded) 1
		  if ( the_event.value_right.size() < 6 )
			{
			  unsigned long f1 = the_dec * 10 ^ ( 6 - the_event.value_right.size() );
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
  mnemos_list({{P_base_freq,"OF"},{P_base_freq,"of"},
			   {P_main_SR,"OS"},{P_main_SR,"os"},
			   {P_main_ampl,"OA"},{P_main_ampl,"oa"},
			   {P_abort,"NQ"},{P_abort,"nq"},
			   {P_ampl_mod_depth,"AA"},{P_ampl_mod_depth,"aa"},
			   {P_pulse_depth,"BA"},{P_pulse_depth,"ba"},
			   {P_mode_pulse,"BM"},{P_mode_pulse,"bm"},
			   {P_mod_ampl_mod,"AM"},{P_mod_ampl_mod,"am"},
			   {P_phase_shift_base,"OP"},{P_phase_shift_base,"op"},
			   {P_phase_shift_pulse,"BP"},{P_phase_shift_pulse,"bp"},
			   {P_phase_shift_ampl_mod,"AP"},{P_phase_shift_ampl_mod,"ap"},
			   {P_phase_overwrite_base,"OO"},{P_phase_overwrite_base,"oo"},
			   {P_phase_overwrite_pulse,"BO"},{P_phase_overwrite_pulse,"bo"},
			   {P_phase_overwrite_ampl_mod,"AO"},{P_phase_overwrite_ampl_mod,"ao"},
			   {P_nop,"NN"},{P_nop,"nn"},
			   {P_ampl_mod_freq,"AF"},{P_ampl_mod_freq,"af"},
			   {P_pulse_freq,"BF"},{P_pulse_freq,"bf"},
			   {P_pulse_hold_high,"BH"},{P_pulse_hold_high,"bh"},
			   {P_pluse_hold_low,"BI"},{P_pluse_hold_low,"bi"}})
{}



void input_params_mnemos_2_action::mnemos_2_action_run(vector<signals_param_action>&actions)
{
  signals_param_action action;

  multimap<short,string>::const_iterator mnemos_list_iter;
  string return_str;

  mnemos_list_iter = find_if( mnemos_list.begin(), mnemos_list.end(),
							  [&](multimap<short,string>::const_reference a) -> bool {
								return a.second == the_event.mnemo;
							  });

  if ( mnemos_list_iter != mnemos_list.end() )
	{
	  unsigned long value;
	  unsigned char val_c;
	  switch( mnemos_list_iter->first )
		{
		case P_base_freq:
		  
		  break;
		case P_main_SR:
		  
		  break;
		case P_main_ampl:
		  return_str = Depth_strings_2_val( value );
		  if ( return_str.empty() )
			{
			  action.action = signals_param_action::main_ampl_val;
			  action.value = value;
			}		  		  
		  break;
		case P_abort:
		  ipm2a_status = mnemo_event::end_track;
		  ipm2a_clearing = input_params_base::c_abort;
		  break;
		case P_ampl_mod_depth:
		  return_str = Depth_strings_2_val( value );
		  if ( return_str.empty() )
			{
			  action.action = signals_param_action::ampl_modul_depth;
			  action.value = value;
			}
		  break;
		case P_pulse_depth:
		  return_str = Depth_strings_2_val( value );
		  if ( return_str.empty() )
			{
			  action.action = signals_param_action::pulse_depth;
			  action.value = value;
			}		  
		  break;
		case P_mode_pulse:
		  return_str = Mode_strings_2_val( val_c );
		  if ( return_str.empty() )
			{
			  action.action = signals_param_action::pulse_modul_mode;
			  action.value = val_c;
			}
		  break;
		case P_mod_ampl_mod:
		  return_str = Mode_strings_2_val( val_c );
		  if ( return_str.empty() )
			{
			  action.action = signals_param_action::ampl_modul_modul_mode;
			  action.value = val_c;
			}
		  break;
		case P_phase_shift_base:
		case P_phase_shift_pulse:
		case P_phase_shift_ampl_mod:
		case P_phase_overwrite_base:
		case P_phase_overwrite_pulse:
		case P_phase_overwrite_ampl_mod:
   		  return_str = Angle_strings_2_val( value );
		  if ( return_str.empty() )
			{
			  switch( mnemos_list_iter->first )
				{
				case P_phase_shift_base:      action.action = signals_param_action::base_phase_shift;  break;
				case P_phase_shift_pulse:     action.action = signals_param_action::pulse_phase_shift;  break;
				case P_phase_shift_ampl_mod:  action.action = signals_param_action::ampl_modul_phase_shift;  break;
				case P_phase_overwrite_base:  action.action = signals_param_action::base_phase_set;  break;
				case P_phase_overwrite_pulse: action.action = signals_param_action::pulse_phase_set;  break;
				case P_phase_overwrite_ampl_mod:  action.action = signals_param_action::ampl_modul_phase_set;  break;
				}
			  action.value = value;
			}
		  break;
		case P_nop:
		  action.action = signals_param_action::nop;
		  break;
		case P_ampl_mod_freq:
		  
		  break;
		case P_pulse_freq:
		  
		  break;
		case P_pulse_hold_high:
		  
		  break;
		case P_pluse_hold_low:
		  
		  break;
		}
	}
  else
	return_str = "Unknown mnemo. ";

  unsigned short val_s;
  return_str += Channel_string_2_val( val_s );
  action.channel_id = val_s;

  if ( return_str.empty() )
	{
	  actions.push_back( action );
	}
  else
	cout << return_str << ", channel: " << the_event.channel << ", mnemo: " << the_event.mnemo << endl;
}

input_params_mnemos_byte_stream::input_params_mnemos_byte_stream(ostream&, istream&i_stm, const bool&with_time_stamp):
  i_stm( i_stm ),
  info_out_str_stream( ioss ),
  mnemos_bytes_stream( info_out_stream, with_time_stamp, ((input_params_base*)this)->clearing ),
  input_params_mnemos_2_action( info_out_stream, *((mnemo_event*)this), ((input_params_base*)this)->clearing, status )
{
  ioss.reserve( 100 );
}


void input_params_mnemos_byte_stream::exec_next_event(vector<signals_param_action>&actions)
{
  mnemos_2_action_run( actions );
}

unsigned long input_params_mnemos_byte_stream::check_next_time_stamp()
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

void input_params_mnemos_file::exec_next_event(vector<signals_param_action>&actions)
{
  //cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  //cout << (unsigned short)key ;

  mnemos_2_action_run( actions );
}

unsigned long input_params_mnemos_file::check_next_time_stamp()
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
bool input_params_mnemos_file::eot()const
{
  return status == end_track;
}
bool input_params_mnemos_file::is_ready()
{
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
  the_list({{mnemo_event("","","1","nn","","","/")},
		{mnemo_event("","","1","XX","22","5","/")},
		{mnemo_event("","","5","AA","1","20000","%")},
		{mnemo_event("","","6","AA","100","4","%")},
		{mnemo_event("","","4","ba","100","0","%")},
		{mnemo_event("","","3","ba","90","0","")},
		{mnemo_event("","","4","ba","100","0","%")},
		{mnemo_event("","","7","bm","90","0","")},
		{mnemo_event("","","8","am","91","0","/")},
		{mnemo_event("","","9","am","1","1","/")},
		{mnemo_event("","","1","oo","22","5","/")},
		{mnemo_event("","","all","BO","45","500","/")},
		{mnemo_event("","","3","AO","90","0","/")},
	    {mnemo_event("","","ALL","op","0","5","")},
		{mnemo_event("","","0","BP","45","500","/")},
	    {mnemo_event("","","1","AP","90","0","/")}})
{
  the_list_iter = the_list.begin();
  the_list_end = the_list.end();
  ioss.reserve( 100 );
}
unsigned long input_params_mnemos_hardcoded::check_next_time_stamp()
{
  if ( get_event( the_list_iter, the_list_end ) )
	{
	  // Something read
	  //	  info_out_stream<< (*this);
	  // cout << (unsigned short)key ;
	  return timestamp_construct;
	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
void input_params_mnemos_hardcoded::exec_next_event(vector<signals_param_action>&actions)
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
  return true;
}
bool input_params_mnemos_hardcoded::exec_loops()
{
  return false;
}
