#include "params_input_mnemos.hxx"

mnemos_bytes_stream::mnemos_bytes_stream(ostream&os,const bool&with_time_stamp, input_params_base::clearing_t&clearing ):
  info_out_str(os),with_time_stamp( with_time_stamp ),
  timestamp_construct( 0 ),
  track_line( 0 ),
  state( state_end ),
  mbs_clearing( clearing )
{}

bool mnemos_bytes_stream::get_event(istream&i_stm)
{
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

input_params_mnemos_2_action::input_params_mnemos_2_action( const mnemo_event&,
															input_params_base::clearing_t&clearing,
															mnemo_event::status_t&status):
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

  mnemos_list_iter = find_if( mnemos_list.begin(), mnemos_list.end(),
							  [&](multimap<short,string>::const_reference a) -> bool {
								return a.second == the_event.code;
							  });
  
  if ( mnemos_list_iter != mnemos_list.end() )
	{
	  switch( mnemos_list_iter->first )
		{
		case P_base_freq:
		  
		  break;
		case P_main_SR:
		  
		  break;
		case P_main_ampl:
		  
		  break;
		case P_abort:
		  ipm2a_status = mnemo_event::end_track;
		  ipm2a_clearing = input_params_base::c_abort;
		  break;
		case P_ampl_mod_depth:
		  
		  break;
		case P_pulse_depth:
		  
		  break;
		case P_mode_pulse:
		  
		  break;
		case P_mod_ampl_mod:
		  
		  break;
		case P_phase_shift_base:
		  
		  break;
		case P_phase_shift_pulse:
		  
		  break;
		case P_phase_shift_ampl_mod:
		  
		  break;
		case P_phase_overwrite_base:
		  
		  break;
		case P_phase_overwrite_pulse:
		  
		  break;
		case P_phase_overwrite_ampl_mod:
		  
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
	{
	  // Unknown mnemo
	  
	}
}

input_params_mnemos_byte_stream::input_params_mnemos_byte_stream(istream&i_stm, const bool&with_time_stamp):
  i_stm( i_stm ),
  mnemos_bytes_stream( info_out_stream, with_time_stamp, ((input_params_base*)this)->clearing ),
  input_params_mnemos_2_action( *((mnemo_event*)this), ((input_params_base*)this)->clearing, status )
{}


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


ostream&operator<<(ostream&the_out,const input_params_mnemos_byte_stream&)
{
  

  return the_out;
}


input_params_mnemos_file::input_params_mnemos_file( ifstream&input_stream, const unsigned short&loops_counter ):
  input_params_mnemos_2_action( *((mnemo_event*)this), ((input_params_base*)this)->clearing, status ),
  mnemos_bytes_stream( info_out_stream, true, ((input_params_base*)this)->clearing ),
  loops_counter( loops_counter ),
  if_stm( move( input_stream ))
{
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

