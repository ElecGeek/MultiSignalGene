#include "params_output_mnemos.hxx"


output_params_mnemos::output_params_mnemos( ostream& o_str ):
  o_str( o_str )
{
  samples_per_TS_unity = 2400;
}
output_params_mnemos::~output_params_mnemos()
{}
void output_params_mnemos::cnv_2_note_velocity( const unsigned char&nbre_bits_expo,
											 const unsigned short&value,
											 unsigned char&note,
											 unsigned char&velocity)
{

}

void output_params_mnemos::export_next_event(const unsigned long&absolute_TS,
										  const unsigned long&diff_TS,
										  const signals_param_action&action)
{
  unsigned char note;
  unsigned char velocity;

  string out_string;
  out_string.reserve( 120 );
  ostringstream out_line( out_string );

  out_line << right << dec << float( diff_TS * samples_per_TS_unity ) / 48000.0;
  if ( action.channel_id == 0 )
	{
	  out_line << "\tall\t";
	}
  else
	{
	  out_line << "\t" << right << setw( 3 ) << action.channel_id << "\t";
	}
  float val_float;
  switch( action.action )
	{
	case signals_param_action::track_name:
	  out_line << "Set track name to " << endl;
	  break;
	case signals_param_action::base_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 * 8.0 / 16777216.0;
	  out_line << "OF " << val_float;
	  out_line << "Hz " << "\t% base frequency" << endl;
	  break;
	case signals_param_action::main_ampl_val:
	  out_line << "OA " << dec << ( action.value * 100 ) / 255;
	  out_line << "% " << "\t% main amplitude" << endl;
	  break;
	case signals_param_action::main_ampl_slewrate:
	  val_float = 16777216.0 / ( (float)action.value * 48000 * 4.0 );
	  out_line << "OS " << val_float;
	  out_line << "S" << "\t% slew-rate" << endl;
	  break;
	case signals_param_action::ampl_modul_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 / 16777216.0;
	  out_line << "AF " << val_float;
	  out_line << "Hz " << "\t% amplitude modulation frequency" << endl;
	  break;
	case signals_param_action::ampl_modul_depth:
	  out_line << "AA " << dec << ( action.value * 100 ) / 255;
	  out_line << "% " << "\t% amplitude modulation depth" << endl;
	  break;
	case signals_param_action::pulse_freq:
	  val_float = (float)action.value * 48000.0 * 4.0 * 4.0 / 16777216.0;
	  out_line << "BF " << val_float;
	  out_line << "Hz " << "\t% pulse modulation frequency" << endl;
	  break;
	case signals_param_action::pulse_high_hold:
	  val_float = (float)action.value * 16.0 * 2.0 / 48000.0;
	  out_line << "BH " << val_float;
	  out_line << "S " << "\t% pulse modulation high hold" << endl;
	  break;
	case signals_param_action::pulse_low_hold:
	  val_float = (float)action.value * 16.0 * 2.0 / 48000.0;
	  out_line << "BI " << val_float;
	  out_line << "S " << "\t% pulse modulation low hold" << endl;
	  break;
	case signals_param_action::pulse_depth:
	  out_line << "BA " << dec << ( action.value * 100 ) / 255;
	  out_line << "% " << "\t% pulse modulation depth" << endl;
	  break;
	case signals_param_action::base_phase_shift:
	  out_line << "OP " << dec << ((float)action.value ) * 22.5;
	  out_line << "\t% base phase shift" << endl;
	  break;
	case signals_param_action::ampl_modul_phase_shift:
	  out_line << "AP " << dec << ((float)action.value ) * 22.5;
 	  out_line << "\t% amplitude modulation phase shift" << endl;
	  break;
	case signals_param_action::pulse_phase_shift:
	  out_line << "BP " << dec << ((float)action.value ) * 22.5;
	  out_line << "\t% pulse modulation phase shift" << endl;
	  break;
	case signals_param_action::base_phase_set:
	  out_line << "OO " << dec << ((float)action.value ) * 22.5;
	  out_line << "\t% base phase (re)set" << endl;
	  break;
	case signals_param_action::ampl_modul_phase_set:
	  out_line << "AO " << dec << ((float)action.value ) * 22.5;
	  out_line << "\t% amplitude modulation phase (re)set" << endl;
	  break;
	case signals_param_action::pulse_phase_set:
	  out_line << "BO " << dec << ((float)action.value ) * 22.5;
	  out_line << "\t% pulse modulation phase (re)set" << endl;
	  break;
	case signals_param_action::ampl_modul_modul_mode:
	  out_line << "AM " << dec << action.value;
	  out_line << "\t% amplitude modulation mode" << endl;
	  break;
	case signals_param_action::pulse_modul_mode:
	  out_line << "BM " << dec << action.value;
	  out_line << "\t% pulse modulation mode" << endl;
	  break;
	case signals_param_action::user_volume:
	  out_line << "Sets the user_volume " << ( action.value * 100 )/255;
	  out_line << "%" << endl;
	  break;
	case signals_param_action::nop:
	  out_line << "NN 0  %no operation" << endl;
	  break;
	default:
	  out_line << endl;
	  break;
	}
  o_str << out_line.str();
  //  o_str.flush();
}


output_params_mnemos_file::output_params_mnemos_file( ofstream& o ):
  output_params_mnemos( of_str )
{
  of_str = move( o );
}
output_params_mnemos_file::~output_params_mnemos_file()
{
  of_str.close();
}

