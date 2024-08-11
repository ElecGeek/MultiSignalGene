#include "amplitude_handler.hxx"
#include <iostream>
using namespace std;

amplitude_handler::amplitude_handler(const unsigned short&sample_rate_K):
  volume( 255 ), requested_ampl( 0 ), amplitude24( 0 ), slewrate( 98304 / sample_rate_K ), sample_rate_K( sample_rate_K )
{}


