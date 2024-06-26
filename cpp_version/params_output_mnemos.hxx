// char, short, long24 are used because this is a PoC of the VHDL version

#include <deque>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;


#ifndef __PARAMS_OUTPUT_MNEMOS__
#define __PARAMS_OUTPUT_MNEMOS__

#include "params_codes.hxx"
#include "parameters.hxx"

class output_params_mnemos : public output_params_base, private midi_event
{
  void cnv_2_note_velocity( const unsigned char&nbre_bits_expo,const unsigned short&value,
					   unsigned char&note, unsigned char&velocity);
  ostream&o_str;
 public:
  output_params_mnemos()=delete;
  explicit output_params_mnemos( ostream&o_str );
  virtual ~output_params_mnemos();
  void export_next_event(const unsigned long&absolute_TS,
						 const unsigned long&diff_TS,
						 const signals_param_action&);
  friend ostream&operator<<(ostream&,const output_params_mnemos&);
};
class output_params_mnemos_file : public output_params_mnemos
{
  ofstream of_str;
  output_params_mnemos_file(void);
 public:
  explicit output_params_mnemos_file( ofstream& );
  ~output_params_mnemos_file(void);
};





#endif
