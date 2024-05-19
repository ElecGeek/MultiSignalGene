// char, short, long24 are used because this is a PoC of the VHDL version

#ifndef __MODULATION_HANDLER__
#define __MODULATION_HANDLER__

#include "amplitude_handler.hxx"
#include "frequency_handler.hxx"
#include "sample_step.hxx"

/** \brief Stores the parameters and computes a modulation
 *
 * Stores the modulation frequency.\n
 * The instanciation is public, all the parameters can be accessed.\n
 * Stroes the modulation amplitude. This is the depth of the modulation.
 * If the depth = 0, the output is equal to the input.
 * If the depth is max, the modulation is maximum.\n
 * The instanciation is public, all the parameters can be accessed.
 */
class modulation_handler
{
  enum modul_mode_t { normal=0 , abs=1 , neg_abs=2 };
  modul_mode_t modulation_mode;
public:
  modulation_handler()=delete;
  // Holds the depth of
  amplitude_handler amplitude;
  frequency_handler frequency;
  sample_step_sine step_sine;

  modulation_handler( const unsigned short& sample_rate_id,
					 const unsigned char& division_rate);
  /** Run operator
   *
   * Runs the modulation at each sample.
   */
  unsigned short operator()(const unsigned long&val);
  constexpr void modul_mode(const unsigned char&a)
  {
	switch( a )
	  {
	  case 0:   modulation_mode = modul_mode_t::normal;break;
	  case 1:   modulation_mode = modul_mode_t::abs;break;
	  case 2:   modulation_mode = modul_mode_t::neg_abs;break;
	  }
  }

};

#endif
