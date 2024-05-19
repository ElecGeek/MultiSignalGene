
#include "modulation_handler.hxx"
#include<iostream>
using namespace std;

extern unsigned char debug_level;

modulation_handler::modulation_handler( const unsigned short& sample_rate_id,
										const unsigned char& division_rate):
  amplitude( sample_rate_id ),
  frequency( sample_rate_id, division_rate),
  step_sine( frequency ),
  modulation_mode( modul_mode_t::normal )
{}

/** \brief Compute the modulation for one sample step
 *
 * The result should always be unsigned. However most of the arithmetic has to be done using signed.\n
 * Some rounding in this computation may generate results sligthly negative. 
 * If is is negative, it is (re)set to 0 and an error is reported. According with
 * the debug level, nothing, small only or all errors are reported.
 * Compute the amplitude modulation in a range of signed 0 +1
 * One should substract the half of the sine from the polarisation (the half of the amplitude)
 * However, the sine comes as a signed as the polarisation comes as an unsigned
 * By this way, it is already divided by 2
 * @param void No parameter as it runs one sample according with the configuration
 * @return The final output value\n
 */
unsigned short modulation_handler::operator()(const unsigned long&val)
{
  unsigned long modulation_amplitude = val * amplitude();
  // return to short
  modulation_amplitude /= 65536;
  if ( modulation_amplitude >= 65536 )
	cerr << "Problems 1bis in signal_channel::operator() MA:" << dec << modulation_amplitude << " PO:" << val << endl;

  // Get the sin of the amplitude modulation and divide by 2 for a range of signed -1/2 to +1/2
  signed short ampl_modul_run = step_sine.Run_Step( (unsigned short)modulation_amplitude );
  if ( ampl_modul_run == -32768 )
	cerr << "Problems 5 in signal_channel::operator()" << endl;
  signed long ampl_modul_run_0_1;
  switch ( modulation_mode )
	{
	case modul_mode_t::normal:
	  ampl_modul_run_0_1 = (signed long)modulation_amplitude / 2 - (signed long)ampl_modul_run;
	  break;
	case modul_mode_t::abs:
	  if ( ampl_modul_run < 0 )
		ampl_modul_run = - ampl_modul_run;
	  ampl_modul_run_0_1 = (signed long)modulation_amplitude - 2 * ((signed long)ampl_modul_run);
	  break;
	case modul_mode_t::neg_abs:
	  if ( ampl_modul_run < 0 )
		ampl_modul_run = - ampl_modul_run;
	  ampl_modul_run_0_1 = 2 * ((signed long)ampl_modul_run);
	  break;
	}
  if ( ampl_modul_run_0_1 < 0 )
	{
	  if ( ampl_modul_run_0_1 < -3 )
	  	cerr << "Problems 6 in signal_channel::operator()" << endl;
	  ampl_modul_run_0_1 = 0;
	}
  if ( (unsigned short)ampl_modul_run_0_1 > val )
	{
	  if ( debug_level >= 3 || ( debug_level >= 1 && (unsigned short)ampl_modul_run_0_1 > ( val + 1 )))
		{
		  cerr << "Problems 7 in signal_channel::operator()" << dec << (unsigned short)ampl_modul_run_0_1 << "  " << val << "\t" << endl;
		}
	  ampl_modul_run_0_1 = val;
	}
  // Last step: apply the amplitude to the base  
  return ( unsigned short )( val - (unsigned short)ampl_modul_run_0_1 );
 
}
