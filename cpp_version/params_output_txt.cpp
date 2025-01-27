#include "params_output_txt.hxx"


output_params_txt::output_params_txt( ostream& o_str ):
  o_str( o_str )
{
  samples_per_TS_unity = 960;
}
output_params_txt::~output_params_txt()
{}
void output_params_txt::cnv_2_note_velocity( const unsigned char&nbre_bits_expo,
											 const unsigned short&value,
											 unsigned char&note,
											 unsigned char&velocity)
{

}

void output_params_txt::export_keep_alive(const unsigned long&absolute_TS)
{
  string out_string;
  out_string.reserve( 120 );
  ostringstream out_line( out_string );

  out_line << right << setw(6) << "/";
  out_line << "\t";
  out_line << right << setw(6) << dec << float( absolute_TS * samples_per_TS_unity ) / 48000.0;
  out_line << "\tChannel: all\tNo operation (keep alive)" << endl;

  o_str << out_line.str();
}
void output_params_txt::export_next_event(const unsigned long&absolute_TS,
										  const unsigned long&diff_TS,
										  const signals_param_action&action)
{
  unsigned char note;
  unsigned char velocity;

  string out_string;
  out_string.reserve( 120 );
  ostringstream out_line( out_string );

  out_line << right << setw(6) << dec << float( diff_TS * samples_per_TS_unity ) / 48000.0;
  out_line << "\t";
  out_line << right << setw(6) << dec << float( absolute_TS * samples_per_TS_unity ) / 48000.0;
  if ( action.channel_id == 0 )
	{
	  out_line << "\tChannel: all\t";
	}
  else
	{
	  out_line << "\tChannel: " << right << setw( 3 ) << action.channel_id << "\t";
	}
  float val_float;
  switch( action.action )
	{
	case signals_param_action::track_name:
	  out_line << "Set track name to " << endl;
	  break;
	case signals_param_action::base_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 * 4.0 / 16777216.0;
	  out_line << "Set base frequency " << hex << action.value << ", means: " << val_float;
	  out_line << "Hz (step: " << (2.0 * 48000.0 * 4.0 * 4.0 / 16777216.0) << ")" << endl;
	  break;
	case signals_param_action::main_ampl_val:
	  out_line << "Sets the amplitude " << hex << action.value << ", dec: " << dec << action.value << endl;
	  break;
	case signals_param_action::main_ampl_slewrate:
	  val_float = 16777216.0 * 2.0 / ( (float)action.value * 48000 * 4.0 );
	  out_line << "Set global amplitude slew-rate " << hex << action.value;
	  out_line << ", means " << val_float;
	  out_line << "s 0-255 (next: ";
	  out_line << (16777216.0 * 2.0 / ( (float)( action.value + 1 ) * 48000 * 4.0 ));
	  if ( action.value > 0 )
		{
		  out_line << ", prev: ";
		  out_line << (16777216.0 * 2.0 / ( (float)( action.value - 1 ) * 48000 * 4.0 ));
		}
	  out_line << ")" << endl;
	  break;
	case signals_param_action::ampl_modul_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 / 16777216.0;
	  out_line << "Sets the amplitude modulation frequency " << hex << action.value;
	  out_line << ", means: " << val_float;
	  out_line << "Hz (step: " << (48000.0 * 4.0 / 16777216.0) << ")" << endl;
	  break;
	case signals_param_action::ampl_modul_depth:
	  out_line << "Sets the amplitude modulation depth " << hex << action.value;
	  out_line << ", means: " << dec << action.value << " (0-255)"<<endl;
	  break;
	case signals_param_action::pulse_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 * 4.0 / 16777216.0;
	  out_line << "Set pulse frequency " << hex << action.value << ", means: " << val_float;
	  out_line << "Hz (step: " << (2.0 * 48000.0 * 4.0 * 4.0 / 16777216.0) << ")" << endl;
	  break;
	case signals_param_action::pulse_high_hold:
	  val_float = (float)action.value * 16.0 * 2.0 / 48000.0;
	  out_line << "Set high hold time " << hex << action.value << ", means: " << val_float;
	  out_line << "S (step: " << (16.0 * 2.0 / 48000.0) << ")" << endl;
	  break;
	case signals_param_action::pulse_low_hold:
	  val_float = (float)action.value * 16.0 * 2.0 / 48000.0;
	  out_line << "Set low hold time " << hex << action.value << ", means: " << val_float;
	  out_line << "S (step: " << (16.0 * 2.0 / 48000.0) << ")" << endl;
	  break;
	case signals_param_action::pulse_depth:
	  out_line << "Sets the pulse depth " << hex << action.value << ", dec: " << dec << action.value << endl;
	  break;
	case signals_param_action::base_phase_shift:
	  out_line << "Shift the phase of the base " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::ampl_modul_phase_shift:
	  out_line << "Shift the phase of the amplitude modulation " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::pulse_phase_shift:
	  out_line << "Shift the phase of the pulse " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::ampl_modul_phase_set:
	  out_line << "Set the phase of the amplitude modulation " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::base_phase_set:
	  out_line << "Set the phase of the base " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::pulse_phase_set:
	  out_line << "Set the phase of the pulse " << dec << action.value << " PI/16 = " << ((float)action.value) * 22.5 << " degree" << endl;
	  break;
	case signals_param_action::ampl_modul_modul_mode:
	  out_line << "Set the modulation mode of the amplitude modulation " << dec << action.value << " 0=normal 1=abs 2=-abs" << endl;
	  break;
	case signals_param_action::pulse_modul_mode:
	  out_line << "Set the modulation mode of the pulse " << dec << action.value << " 0=normal 1=abs 2=-abs" << endl;
	  break;
	case signals_param_action::user_volume:
	  out_line << "Sets the user_volume " << hex << action.value << ", dec: " << dec << action.value << endl;
	  break;
	case signals_param_action::nop:
	  out_line << "No operation " << endl;
	  break;
	default:
	  out_line << endl;
	  break;
	}
  o_str << out_line.str();
}


output_params_txt_file::output_params_txt_file( ofstream& o ):
  of_str( move( o )),output_params_txt( of_str )
{}
output_params_txt_file::~output_params_txt_file()
{
  of_str.close();
}

