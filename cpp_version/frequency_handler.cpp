#include "frequency_handler.hxx"
#include<iostream>
using namespace std;

frequency_handler::frequency_handler( const unsigned short&sample_rate_id,
									  const unsigned char&division_rate):
  global_rate( sample_rate_id * (unsigned short)division_rate ),
  angle( 0 ), startCycle(false),
  high_hold( 0 ), low_hold( 0 ),
  high_hold_count( 0 ), low_hold_count( 0 ),
  lastQuadrant( 0 )
{
  // put something non null to initialise
  frequency = 1;
}

