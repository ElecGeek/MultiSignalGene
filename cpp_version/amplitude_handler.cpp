#include "amplitude_handler.hxx"
#include <iostream>
using namespace std;

amplitude_handler::amplitude_handler(const unsigned short&sample_rate_id):
  volume( 255 ), requested_ampl( 0 ), amplitude24( 0 ), slewrate( 2048 ), sample_rate_id( sample_rate_id )
{}


