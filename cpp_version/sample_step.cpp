#include "sample_step.hxx"
extern unsigned char debug_level;

sample_step::sample_step( frequency_handler&frequency):
  frequency( frequency )
{}
sample_step_sine :: sample_step_sine( frequency_handler&frequency ):
  sample_step( frequency )
{}
signed short sample_step_sine :: Run_Step(const unsigned short&amplitude)
{
  // Only 20 iterations. Since 110dB of THD are not required, this is OK
  // C++03
  //const signed long cal[] = { 2097152, 1238021, 654136, 332050, 166669, 83416, 41718, 20860, 10430, 5215, 2608, 1304, 652, 326, 163, 81, 41, 20, 10, 5 };
  //const vector<signed long> cor_angle_list( cal, cal + sizeof( cal ) / sizeof( cal[ 0 ] ));
  // C++11
  // array<signed long,20> cor_angle_list{ 2097152, 1238021, 654136, 332050, 166669, 83416, 41718, 20860, 10430, 5215, 2608, 1304, 652, 326, 163, 81, 41, 20, 10, 5};
  // C++14
  // see the .hxx file
  constexpr static array< signed long, 20 > cor_angle_list = {{ 2097152, 1238021, 654136, 332050, 166669, 83416, 41718, 20860, 10430, 5215, 2608, 1304, 652, 326, 163, 81, 41, 20, 10, 5 }};

  signed long cor_c, cor_s;
  signed long cor_z;

  signed long amplitude_r( amplitude );

  // Shift would have been 7 to convert unsigned short into signed long24
  // However cordic algho introduces a small gain of about 1.6468
  // then the starting amplitude is divided by two
  amplitude_r <<= 6;
  // To avoid the rounding does not keep a regular spacing
  // between the consecutive angles, the value is multiplied by 1.0..01
  amplitude_r |= amplitude_r >> 16;

  frequency();
  unsigned long angle = frequency.GetAngle();

  // Pereform "by hand the 2 first steps"
  switch( angle & 0x00c00000 )
    {
    case 0x00000000:
      cor_c = amplitude_r;
      cor_s = 0;
      break;
    case 0x00400000:
      cor_s = amplitude_r;
      cor_c = 0;
      break;
    case 0x00800000:
      cor_c = - amplitude_r;
      cor_s = 0;
      break;
    case 0x00c00000:
      cor_s = - amplitude_r;
      cor_c = 0;
      break;
    }
  cor_z = angle & 0x003fffff;
  unsigned short shifts = 0;
  // Now run the alghorythm
  // for( vector<signed long>::const_iterator z_diff = cor_angle_list.begin();
  for_each( cor_angle_list.begin(), cor_angle_list.end(), [&](const signed long&z_diff)
			{
			  signed long cor_c_temp, cor_s_temp;
			  if ( cor_z >= 0 )
				{
				  cor_s_temp = cor_s + ( cor_c >> shifts );
				  cor_c_temp = cor_c - ( cor_s >> shifts );
				  cor_z -= z_diff;
				  //  cout <<'+';
				} else {
				cor_s_temp = cor_s - ( cor_c >> shifts );
				cor_c_temp = cor_c + ( cor_s >> shifts );
				cor_z += z_diff;
				//cout << '-';	  
			  }
			  cor_s = cor_s_temp;
			  cor_c = cor_c_temp;
			  shifts += 1;
			  if( true )
				{
				  if ( (cor_s >= 16777214) || (cor_s < -16777216) )
					{
					  cerr << "********** Problem in the cordic algo (sin) **********" << endl;
					}
				  if ( (cor_c >= 16777214) || (cor_c < -16777216) )
					{
					  cerr << "********** Problem in the cordic algo (cos) **********" << endl;
					}
				}
			}
			);
	// For debug chek the sum of the squares of the sine and cosine is 1 ... in fact the amplitude
  //  return (signed short)(((cor_s >> 8 )*(cor_s >> 8 ) + (cor_c >> 8 )*(cor_c >> 8 ))>>16);
	const signed long the_return = cor_s + ( cor_s >> 3 ) + ( cor_s >> 4 ) + ( cor_s >> 6 ) + ( cor_s >> 7 );
  if ( the_return <= -8388608 )
	{
	  if ( debug_level >= 2 )
		{
		  cout << hex << "-" << the_return;
		}
	  return 0x8001;
	}
  else
	{
	  if ( the_return >= 8388608 )
		{
		  if ( debug_level >= 2 )
			{
			  cout << "+";
			}
		  return 0x7fff;
		}
	  else
		{
		  return (signed short)( the_return >> 8 );
		}
	}
}
sample_step_triangle :: sample_step_triangle( frequency_handler&frequency ):
  sample_step( frequency )
{}
signed short sample_step_triangle :: Run_Step(const unsigned short&amplitude)
{
  // amplitude is a 16 bits unsigned
  signed long amplitude_r( amplitude );

  frequency();
  unsigned long angle = frequency.GetAngle();
  signed long the_return;

  // We are in the quadrant PI/2 -> PI or 3.PI/2 -> 2.PI (excluded)
  //   then get the symetric
  if ( ( angle & 0x00400000 ) == 0x00400000 )
	{
	  angle &= 0x00bfffff;
	  angle ^= 0x003fffff;
	}
  if( ( angle & 0x00200000 ) == 0x00200000 )
    {
	  //We are at the saturation level
	  // the_return is a signed short and subject to be negated
	  the_return = amplitude_r >> 1;
    } else {
	// We are in the triangle are, multiply the angle by the amplitude
	// with an angle limited to 8 bits
	  // amplitude_r is unisgned short, slice of angle is unsigned char
	  the_return = amplitude_r * (( angle & 0x001fe000) >> 13);
	  // the_return is 24 bits unsigned
	  the_return >>= 9;
	  // the_return is now signed short
  }
  if ( ( angle & 0x00800000 ) == 0x00800000 )
	{
	  the_return = - the_return;
	}
  return (signed short)the_return;
}
sample_step_continuous :: sample_step_continuous( frequency_handler&frequency ):
  sample_step( frequency )
{}
signed short sample_step_continuous :: Run_Step(const unsigned short&amplitude)
{
  // does not look like nice. But it is a light version of the triangle
  // then the debug is more easy
  signed long amplitude_r( amplitude );

  frequency();
  unsigned long angle = frequency.GetAngle();
  signed long the_return;

  the_return = amplitude_r >> 1;

  return (signed short)the_return;
}
sample_step_pulse :: sample_step_pulse( frequency_handler&frequency, const unsigned short&length ):
	sample_step( frequency ), state( 0 )
{
  if ( length > 6 )
	{
	  this->length = length - 6;
	}
  else
	{
	  this->length = 1;
	}
}
signed short sample_step_pulse ::Run_Step(const unsigned short&amplitude)
{
  signed long the_return;
  frequency();
  signed long amplitude_r( amplitude );
  // Shift is 7 to convert unsigned short into signed long24
  amplitude_r <<= 7;
  // To avoid the rounding does not keep a regular spacing
  // between the consecutive angles, the value is multiplied by 1.0..01
  amplitude_r |= amplitude_r >> 16;
  // With a few additional calculation, the EMC problems are reduced
  // The goal is not to implement a full filter but to make simple
  // arithmetics to avoid sharp corners
  switch( state )
	{
	case 0:
	  // The counter just make a roll out
	  if ( frequency.GetStartCycle() )
		{
		  state = 1;
		}
	  the_return = 0;
	  break;
	case 1:
	case 15:
	case 17:
	case 31:
	  state += 1;
	  the_return = amplitude_r >> 4;
	  break;
	case 2:
	case 14:
	case 18:
	case 30:
	  state += 1;
	  the_return = amplitude_r >> 3;
	  break;
	case 3:
	case 13:
	case 19:
	case 29:
	  state += 1;
	  the_return = amplitude_r >> 2;
	  break;
	case 4:
	case 12:
	case 20:
	case 28:
	  state += 1;
	  the_return = amplitude_r >> 1;
	  break;
	case 5:
	case 11:
	case 21:
	case 27:
	  state += 1;
	  the_return = amplitude_r - (amplitude_r >> 2);
	  break;
	case 6:
	case 10:
	case 22:
	case 26:
	  state += 1;
	  the_return = amplitude_r - (amplitude_r >> 3 );
	  break;
	case 7:
	case 9:
	case 23:
	case 25:
	  state += 1;
	  the_return = amplitude_r - (amplitude_r >> 4 );
	  length_count = 0;
	  break;
	case 8:
	case 24:
	  length_count += 1;
	  if ( length_count > length )
		{
		  state += 1;
		}
	  the_return = amplitude_r;
	  break;
	case 16:
	  state += 1;
	  the_return = 0;
	  break;
	case 32:
	  state = 0;
	  the_return = 0;
	  break;
	}
  if ( state > 16 )
	{
	  the_return = -the_return;
	}

    return (signed short)(the_return >> 8 );
}

sample_step_txt :: sample_step_txt( frequency_handler&frequency ):
  sample_step( frequency ), out_str( cout )
{}
signed short sample_step_txt ::Run_Step(const unsigned short&amplitude)
{
  signed long the_return;
  const signed long amplitude_r( amplitude );

  frequency();
  // The counter just make a roll out
  if ( frequency.GetStartCycle() )
	{
	  out_str << "Pulse, amplitude_r: " << hex << "  ";
	  return amplitude_r / 2;
	}
  else
	return 0;
  return 0;
}
