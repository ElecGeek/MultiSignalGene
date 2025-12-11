#include "parameters.hxx"

input_event::input_event():
  status(warming_up)
{}

signals_param_action::signals_param_action( const unsigned short&channel_id,
											const action_list&action,
											const unsigned short&value):
  channel_id(channel_id),action(action),value(value)
{}

signals_param_action::signals_param_action():
  action(nop)
{}


input_params_base::input_params_base():
  wait_for_next_TS( false ), cumul_time_stamp( 0 ),
  current_samples( 0 ),
  samples_per_TS_unity( 5 ),
  clearing( c_unknown )
{}
bool input_params_base::check_next_event( const unsigned short&elapsed_samples, vector<signals_param_action>&actions)
{
  // Somewhere, the elapsed_samples is always 48 as the functions is called
  // accordingly with the sample rate and 1mS is nice to avoid to break the real time
  bool do_leave( false );
  current_samples += elapsed_samples;
  while( do_leave == false )
	{
	  // Think to respawn the function immediately in case of a ts = 0
	  // In order to avoid awaiting for some output
	  // The function is left only if 1) it is too early against the next event
	  // 2) There are actually no event
	  if( wait_for_next_TS )
		{
		  if( current_samples >= requested_samples )
			{
			  // Something to do now
			  import_next_event(actions);
			  current_samples -= requested_samples;
			  wait_for_next_TS = false;
			  cout << info_out_stream.str();
			  info_out_stream.str("");
			} else
			// Too early
			do_leave = true;
		} else {
		unsigned long requested_time_stamp = check_next_time_stamp();
		if( requested_time_stamp != 0xffffffff )
		  {
			// An event has been received. Computer the waiting time
			requested_samples = samples_per_TS_unity * requested_time_stamp; 
			cumul_time_stamp += requested_time_stamp;
			wait_for_next_TS = true;
		  }
		else
		  {
			// No event or event not fully received
			do_leave = true;
			requested_samples = 0xffffffff;
			if ( eot() )
			  return false;
		  }
	  }
	}
  return true;
}
/** \brief re-execute handler
 *
 * The base (vitual) function is the NON seekable inputs
 * \return always false
 */
bool input_params_base::exec_loops(){
  return false;
}


output_params_base::output_params_base():
  cumul_time_stamp( 0 ), current_samples( 0 ), samples_per_TS_unity( 5 ),
  keep_alive_max( 60000 ), keep_alive_count( 0 )
{}
void output_params_base::export_keep_alive(const unsigned long&absolute_TS)
{}
bool output_params_base::check_next_event( const unsigned short&elapsed_samples,
										   const vector<signals_param_action>&actions)
{
  current_samples += elapsed_samples;
  if ( actions.empty() == false )
	{
	  unsigned long diff_TS;
	  diff_TS = current_samples / samples_per_TS_unity;
	  cumul_time_stamp += diff_TS;
	  current_samples -= diff_TS * samples_per_TS_unity;
	  for( const signals_param_action& it : actions )
		{
		  export_next_event( cumul_time_stamp, diff_TS, it );
		  diff_TS = 0;
		}
	  keep_alive_count = 0;
	} else {
	keep_alive_count += 1;
	if ( keep_alive_count == keep_alive_max )
	  {
		keep_alive_count = 0;
		export_keep_alive( cumul_time_stamp + current_samples / samples_per_TS_unity );
	  }
  }
  return true;
}

