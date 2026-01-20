#include "params_input_json.hxx"
#include <cmath>

json_event::json_event():
  action (nullopt),
  metadata (nullopt)
{}
json_event::json_event(const string&at_key,const string&at_value):
  action (nullopt)
{
  optional<json_event_metadata> metadata;
  metadata->md_key = at_key;
  metadata->md_val = at_value;
  this->metadata = metadata;
}


jsons_bytes_stream::jsons_bytes_stream(ostream&os,
										 const bool&with_time_stamp,
										 input_params_base::clearing_t&clearing ):
  state( state_end ),
  block_state( blk_start ),
  info_out_str(os),
  mbs_clearing( clearing ),
  track_line( 1 ),
  current_channel(0),
  with_time_stamp( with_time_stamp )
{}

bool jsons_bytes_stream::get_event(istream&i_stm)
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
	switch ( val_read )
	  {
	  case '{':

		break;
	  case '}':

		break;
	  case ':':

		break;
	  case ',':

		break;
	  case '"':

		break;
	  default:

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

jsons_bytes_datagram_test::jsons_bytes_datagram_test(ostream&os,
										 const bool&with_time_stamp,
										 input_params_base::clearing_t&clearing ):
  info_out_str(os),
  mbs_clearing( clearing )
{
  if ( with_time_stamp )
	info_out_str << "With time stamps is required, since this is a test module, it is omitted" << endl;
}

bool jsons_bytes_datagram_test::get_event(deque<json_event>::const_iterator&curr,
										   deque<json_event>::const_iterator&end)
{
  if( curr != end )
	{

	  curr++;
	  return true;
	}else
	{
	  mbs_clearing = input_params_base::c_end_of_data;
	  return false;
	}
}



input_params_jsons_2_action::input_params_jsons_2_action( ostream&out_info_str,
															const json_event&the_event,
															input_params_base::clearing_t&clearing,
															input_event::status_t&status):
  info_out_str(info_out_str),
  the_event( the_event ),
  ipm2a_clearing( clearing ),
  ipm2a_status( status ),
  mrf({
	  {"oa",[&](unsigned long&value,signals_param_action::action_list&act)->string{
		  act=signals_param_action::main_ampl_val;
		  // temporary, place the value here
		  return 0;}},
	}),

  jsons_list({
	  {P_abort,"NQ"},{P_abort,"nq"}})
{}



void input_params_jsons_2_action::jsons_2_action_run(vector<signals_param_action>&actions)
{
  signals_param_action action;
  decltype(mrf)::const_iterator jsons_read_funcs_iter;
  multimap<short,string>::const_iterator jsons_list_iter;
  string return_err_str;

  /*  if( the_event.json.compare( "bh" ) == 0 || the_event.json.compare( "bi" ) == 0|| the_event.json.compare( "bf" ) == 0)
 	{
	  cout << "Json to find: " << the_event.json << "\t";
	  cout << the_event.TS_left << "." << the_event.TS_right << "\t" << the_event.channel << "\t";
	  cout << the_event.value_left << "." << the_event.value_right << endl;
	  }*/

  jsons_read_funcs_iter = mrf.end(); //mrf.find(the_event.json);
  if ( jsons_read_funcs_iter != mrf.end() )
	{
	  unsigned long value;
	  signals_param_action::action_list act;
	  return_err_str = jsons_read_funcs_iter->second( value, act );
	  if ( return_err_str.empty() )
		{
		  action.channel_id = 0; // we should decide how to pass the channel number     current_channel;

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
		//		cout << return_err_str << ", channel: " << the_event.channel << ", json: " << the_event.json << ", unknown channel" << endl;
	    cout  << "ERROR" << endl;
	}else
	{
	  // Not found, 3 reasons
	  //   * json handled in the old version
	  //   * json of the extended commands ( store, retrieve etc... NOT YET )
	  //   * really not found
	  // The store retrieve is on hold as a decision has to be taken:
	  //   * does it move to the low level parameters ( recording parameter action structures )
	  //   * should it handled as file pointer. That means it is available in seekable inputs only
	  if ( jsons_list_iter != jsons_list.end() )
		{
		  unsigned long value;
		  unsigned char val_c;
		  jsons_list_iter = find_if( jsons_list.begin(), jsons_list.end(),
									  [&](multimap<short,string>::const_reference a) -> bool {
										return a.second == string() /*the_event.json*/;
									  });
		  /* don't know now how to abort
			 switch( jsons_list_iter->first )
			{
			case P_abort:
			  ipm2a_status = input_event::end_track;
			  ipm2a_clearing = input_params_base::c_abort;
			  break;
			}*/
		  action.channel_id = 0;  //   see above      current_channel;
		  
		  if ( return_err_str.empty() )
			{
			  actions.push_back( action );
			}
		  else
			//	cout << return_err_str << ", channel: " << the_event.channel << ", json: " << the_event.json << endl;
			cout << "ERROR" << endl;
		}
	  else
		return_err_str = "Unknown json. ";
	}
}

input_params_jsons_byte_stream::input_params_jsons_byte_stream(ostream&, istream&i_stm, const bool&with_time_stamp):
  i_stm( i_stm ),
  info_out_str_stream( ioss ),
  jsons_bytes_stream( info_out_stream, with_time_stamp, ((input_params_base*)this)->clearing ),
  input_params_jsons_2_action( info_out_stream, *((json_event*)this), ((input_params_base*)this)->clearing, status )
{
  ioss.reserve( 100 );
}


void input_params_jsons_byte_stream::import_next_event(vector<signals_param_action>&actions)
{
  jsons_2_action_run( actions );
}

unsigned long input_params_jsons_byte_stream::check_next_time_stamp()
{
  if ( get_event( i_stm ) )
	{
	  if ( timestamp_value == true )
		return *timestamp_value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}
	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
bool input_params_jsons_byte_stream::eot(void) const
{
  return false;
}
bool input_params_jsons_byte_stream::is_ready(void)
{
  return true;
}


ostream&operator<<(ostream&the_out,const input_params_jsons_byte_stream&)
{
  

  return the_out;
}


input_params_jsons_file::input_params_jsons_file( ostream&, ifstream&input_stream, const unsigned short&loops_counter ):
  info_out_str_stream( ioss ),
  input_params_jsons_2_action( info_out_str_stream, *((json_event*)this), ((input_params_base*)this)->clearing, status ),
  jsons_bytes_stream( info_out_str_stream, true, ((input_params_base*)this)->clearing ),
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
input_params_jsons_file::~input_params_jsons_file()
{
  // TODO check here if the length is the declared length. if not report a warning
  if_stm.close();
}

void input_params_jsons_file::import_next_event(vector<signals_param_action>&actions)
{
  // cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  // cout << endl;

  jsons_2_action_run( actions );
  // Reset the fields
  // The structure should change. For now initialize one by one
  /*  TS_left.clear();
  TS_right.clear();
  TS_unit.clear();
  channel.clear();
  json.clear();
  value_left.clear();
  value_right.clear();
  value_unit.clear();*/

  if ( info_out_str_stream.str().empty() == false )
	{
	  cout << info_out_str_stream.str();
	  info_out_str_stream.str("");
	}
}

unsigned long input_params_jsons_file::check_next_time_stamp()
{
  if ( get_event( if_stm ) )
	{
	  if ( timestamp_value == true )
		return *timestamp_value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}

	}else
	// Input is "starving" nothing has been received or is not complete
	return 0xffffffff;
}
bool input_params_jsons_file::eot()const
{
  return status == end_track;
}
bool input_params_jsons_file::is_ready()
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
bool input_params_jsons_file::exec_loops(){
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

input_params_jsons_hardcoded::input_params_jsons_hardcoded(ostream&):
  info_out_str_stream( ioss ),
  jsons_bytes_datagram_test( info_out_stream, false, ((input_params_base*)this)->clearing ),
  input_params_jsons_2_action( info_out_stream, *((json_event*)this), ((input_params_base*)this)->clearing, status )
  /*  the_list({{json_event("0","0","s","1","nn","","","/")},
		{json_event("","0","s","1","XX","22","5","/")},
		{json_event("0","","","5","aa","1","20000","%")},
		{json_event("1","2","","6","aa","100","4","%")},
		{json_event("1","2","s","4","ba","100","0","%")},
		{json_event("0","0","s","3","ba","90","0","")},
		{json_event("1","2","s","4","ba","100","0","%")},
		{json_event("1","2","s","7","bm","90","0","")},
		{json_event("1","21","s","8","am","91","0","/")},
		{json_event("1","21","s","9","am","1","1","/")},
		{json_event("1","21","s","1","oo","22","5","/")},
		{json_event("1","21","s","all","nn","466666666666666665","0","/")},
		{json_event("1","21","s","all","bo","45","500","/")},
		{json_event("1","21","s","3","ao","90","0","/")},
	    {json_event("1","2","s","ALL","op","0","5","")},
        {json_event("1","2","s","0","bp","45","500","/")},
	    {json_event("1","2","s","1","ap","90","0","/")},
		{json_event("1","2","s","20","of","0","20000","%")},
		{json_event("1","2","s","21","of","100","4","")},
		{json_event("1","2","s","22","of","1000","0","m")},
		{json_event("1","2","s","23","of","90","0","mhz")},
		{json_event("1","2","s","24","of","1","666","msec")},
		{json_event("1","2","s","25","of","","00166","sec")},
		{json_event("1","2","s","26","of","","600","khz")},
		{json_event("1","2","s","27","of","1","600","hz")},
		{json_event("1","2","s","30","os","0","0166","sec")},
        {json_event("1","2","s","31","os","0","100","khz")},
		{json_event("1","2","s","40","af","1","2000","%")},
		{json_event("1","2","s","41","bf","100","04","")},
		{json_event("1200","0","us","42","af","100","0","m")},
		{json_event("1","2","","43","bf","90","0","mhz")},
		{json_event("1","2","","44","af","1","066","msec")},
		{json_event("1","2","","45","bf","0","0","sec")},
		{json_event("1","2","s","50","bh","0","00166","sec")},
		{json_event("1","2","s","51","bh","0","600","khz")},
		{json_event("1","2","s","52","bi","0","0166","sec")},
        {json_event("1","2","s","53","bi","10","0","s")}}
		)*/
{
  the_list_iter = the_list.begin();
  the_list_end = the_list.end();
  ioss.reserve( 100 );
}
unsigned long input_params_jsons_hardcoded::check_next_time_stamp()
{
  string return_err_str;
  if ( get_event( the_list_iter, the_list_end ) )
	{
	  if ( timestamp_value == true )
		return *timestamp_value;
	  else
		{
		  //info_out_str << err_ret_str;
		  return 0;
		}
	}else
	// Input is "starving", that should not be happened here
	return 0xffffffff;
}
void input_params_jsons_hardcoded::import_next_event(vector<signals_param_action>&actions)
{
  //cout << "TS cumul: " << dec << cumul_time_stamp / 10 << '\t';
  //cout << (unsigned short)key ;

  jsons_2_action_run( actions );
}
bool input_params_jsons_hardcoded::eot(void) const
{
  return status == end_track;
}
bool input_params_jsons_hardcoded::is_ready(void)
{
  if ( samples_per_TS_unity != 5 )
	cout << "Internal ERROR: samples_per_TS_unity not 5" << endl;
  return true;
}
bool input_params_jsons_hardcoded::exec_loops()
{
  return false;
}
