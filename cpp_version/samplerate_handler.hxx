#ifndef __SAMPLERATE_HANDLER__
#define __SAMPLERATE_HANDLER__

#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <limits>
#include <algorithm>

using namespace std;



/** \brief Sample rate list
 *
 * Functions to find the sample rate accroding with the restrictions \n
 * of the output modules and the requested sample rate
 */
class sample_rate_list
{
  bool don_t_care;
  set<unsigned short>list_sr;
public:
  sample_rate_list();
  void add_value(const unsigned short&);
  /* \brief Find common values
   *
   * In case of don't care, copy the right list
   * In case not, merges the lists
   */
  sample_rate_list intersect_sr_list( const sample_rate_list&)const;
  /* \brief Get the sample rate to use
   *
   * Accroding with some default choices, return the sample rate to use
   * And tell if the value is low
   */
  pair<unsigned short,bool> get_sample_rate()const;
  friend ostream&operator<<(ostream&,const sample_rate_list&);
};


#endif
