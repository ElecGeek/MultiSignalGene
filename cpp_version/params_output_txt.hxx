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


#ifndef __PARAMS_OUTPUT_TXT__
#define __PARAMS_OUTPUT_TXT__

#include "parameters.hxx"

class output_params_txt : public output_params_base
{
  void cnv_2_note_velocity( const unsigned char&nbre_bits_expo,const unsigned short&value,
					   unsigned char&note, unsigned char&velocity);
  ostream&o_str;
  bool output_non_nop;
  output_params_txt(void);
 public:
  output_params_txt( ostream&o_str, const bool&output_non_nop );
  virtual ~output_params_txt();
  void export_keep_alive(const unsigned long&absolute_TS);
  void export_next_event(const unsigned long&absolute_TS,
						 const unsigned long&diff_TS,
						 const signals_param_action&);
  friend ostream&operator<<(ostream&,const output_params_txt&);
};
class output_params_txt_file : public output_params_txt
{
  ofstream of_str;
  output_params_txt_file(void);
 public:
  output_params_txt_file( ofstream&of_str, const bool&output_non_nop );
  ~output_params_txt_file(void);
};





#endif
