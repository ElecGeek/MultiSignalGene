#include "sample_step.hxx"
#include "main_loop.hxx"

extern unsigned char debug_level;

signal_channel::signal_channel( const unsigned short&channel_id,
								const unsigned char&sample_rate_id,
								const unsigned char&mode):
  channel_id( channel_id ),
  frequency(sample_rate_id,1), amplitude(1),
  ampl_modul(sample_rate_id, 8),
  pulse_modul(sample_rate_id, 2)
  /*  ampl_modul_depth( 1 ), ampl_modul_freq(sample_rate_id,8),
  pulse_depth( 1 ), pulse_freq(sample_rate_id,2),
  ampl_modul_step( ampl_modul_freq ), 
  pulse_step( pulse_freq )*/
{
  switch( mode )
	{
	case 's':
	case 'S':
	  the_step = new sample_step_sine( frequency );
	  break;
	case 'p':
	case 'P':
	  the_step = new sample_step_pulse( frequency, 10 );
	  break;
	case 't':
	case 'T':
	  the_step = new sample_step_txt( frequency );
	  break;
	case 'e':
	case 'E':
	  the_step = new sample_step_triangle( frequency );
	  break;
	case 'c':
	case 'C':
	  the_step = new sample_step_continuous( frequency );
	  break;
	case 'd':
	case 'D':
	  return;
	default:
	  cerr<< "Bad mode"<<endl;
	  throw;
	  break;
	}
}
signal_channel::~signal_channel()
{
    delete the_step;
}
/** \brief Computes the final output value
 *
 * It considers the 2 modulations and the base to produce the final sample\n
 * The documentation graph is drawn for clarity.
 * However the "stream" is done while computing the base last.\n
 * The advantage of the cordic algo is to perform, in one set of iterations,
 * the sine (and cosine) and the multiplication by the amplitude\n
 * Then the global amplitude followed by the 2 modulatiors follwed by the base signal
 * are proceed.
 */
signed short signal_channel::operator()()
{
  // Set the global amplitude the amplitude
  unsigned long base_amplitude( amplitude() );
  // Modulation 1
  unsigned short pulse_out = pulse_modul( base_amplitude );
  // Modulation 2
  unsigned short ampl_modul_output = ampl_modul(pulse_out);
  // Run the base waveform
  return the_step->Run_Step(ampl_modul_output);
}


void signal_channel::exec_next_event( const vector<signals_param_action>&spa )
{
  for( const signals_param_action it : spa )
	if( (it.channel_id == 0) || (it.channel_id == channel_id) )
	  switch( it.action )
		{
		case signals_param_action::base_freq:
		  frequency.set_frequency( (unsigned short)it.value );
		  break;
 		case signals_param_action::base_phase_shift:
		  frequency.shift_phase( (unsigned char)it.value );
		  break;
 		case signals_param_action::base_phase_set:
		  frequency.set_phase( (unsigned char)it.value );
		  break;
		case signals_param_action::main_ampl_val:
		  amplitude.set_amplitude( (unsigned char)it.value);
		  break;
		case signals_param_action::main_ampl_slewrate:
		  amplitude.set_slewrate( it.value );
		  break;
		case signals_param_action::ampl_modul_freq:
		  ampl_modul.frequency.set_frequency( (unsigned short)it.value );
		  break;
		case signals_param_action::ampl_modul_depth:
		  ampl_modul.amplitude.set_amplitude( (unsigned char)it.value );
		  break;
 		case signals_param_action::ampl_modul_phase_shift:
		  ampl_modul.frequency.shift_phase( (unsigned char)it.value );
		  break;
 		case signals_param_action::ampl_modul_phase_set:
		  ampl_modul.frequency.set_phase( (unsigned char)it.value );
		  break;
		case signals_param_action::ampl_modul_modul_mode:
		  ampl_modul.modul_mode( (unsigned char)it.value );
		  break;
		case signals_param_action::pulse_freq:
		  pulse_modul.frequency.set_frequency( (unsigned short)it.value );
		  break;
		case signals_param_action::pulse_depth:
		  pulse_modul.amplitude.set_amplitude( (unsigned char)it.value );
		  break;
 		case signals_param_action::pulse_high_hold:
		  pulse_modul.frequency.set_high_hold( (unsigned short)it.value );
		  break;
 		case signals_param_action::pulse_low_hold:
		  pulse_modul.frequency.set_low_hold( (unsigned short)it.value );
		  break;
 		case signals_param_action::pulse_phase_shift:
		  pulse_modul.frequency.shift_phase( (unsigned char)it.value );
		  break;
 		case signals_param_action::pulse_phase_set:
		  pulse_modul.frequency.set_phase( (unsigned char)it.value );
		  break;
		case signals_param_action::pulse_modul_mode:
		  pulse_modul.modul_mode( (unsigned char)it.value );
		  break;

		}
}

ostream&operator<<(ostream&os,const signals_param_action&a){
  switch( a.action )
	{
	case signals_param_action::base_freq:  os<<"Base frequency";  break;
	case signals_param_action::main_ampl_val:  os<<"Main amplitude";  break;
	case signals_param_action::main_ampl_slewrate:  os<<"Amplitude slewrate";  break;
	case signals_param_action::ampl_modul_freq:  os<<"Ampl_Modul frequency";  break;
	case signals_param_action::ampl_modul_depth:  os<<"Ampl_Modul depth";  break;
	}
  return os;
}


