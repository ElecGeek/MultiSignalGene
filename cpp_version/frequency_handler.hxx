// char, short, long24 are used because this is a PoC of the VHDL version

#ifndef __FREQUENCY_HANDLER__
#define __FREQUENCY_HANDLER__

/** \brief Manages the frequency and the period for one generator
 * 
 * Computes the data of a periodic output except the shape itself
 * It computes the period as an angle between 0 and 16777216 (2 power 24).
 * After the value passed the high limit, it returns to 0 and set a starting pulse
 * The actual sampling rate is computed to be transparent for the input and output values
 *
 * It holds the angle at 1/4 and 3/4 of the period for a certain time (2 different parameters)
 *   The delay can be set from 0 (no hold) to 21.85s with a step of 0.33mS
 *
 * The signal class uses this class to generate the output.
 * The run operator computes the next angle and the period start data
 * Other functions fetch them
 *
 * The maximum frequency is 5999.91Hz, the step is 0.091Hz
 * There is a division rate in order to get lower frequencies with a lower step, with longer holds
 *   The base signal should use 1 as the range is around Hz to a few KHz.
 *   The modulations should use higher values as their range are around centiHz to a couple of Hz
 *     Second, in this project, this a non sense to modulate at rates higher than the carrier.
 *     For instance a division rate for 8 the maximum is 750Hz and the step is a period of 87s
 *
 * CAUTION: The parameters described in this class defined the maximum and steps.
 * The can be narrow if an input module does not support the full range.
 */
class frequency_handler
{
  unsigned long frequency;
  const unsigned short global_rate;
  unsigned long angle;
  bool startCycle;
  unsigned long high_hold, low_hold;
  unsigned long high_hold_count, low_hold_count;
  unsigned char lastQuadrant;
/** \brief Computes when the sine reached the min or the max
 *
 * The sine is checked if it passes the min or the max.
 * If 24th and 23rd bits are 01 as it was 00, the max is just passed
 * if they are 11 as they was 10, the min is just passed
 *
 * If none, the angle is checked for the overflow against 24 bits in order
 * to clear the 25th bit (and higher)
 *
 * If just passed, the relevant counter is loaded
 */
  constexpr void CheckHold(void)
  {
	unsigned char newQuadrant = angle >> 22; 
	if ( ( newQuadrant & 0x04 ) == 0x04 )
	  {
		angle &= 0x00ffffff;
		newQuadrant = 0;
		startCycle = true;
	  }
	else if (( newQuadrant == 0x01) && (lastQuadrant == 0x00) )
	  {
		high_hold_count = high_hold;
	  }
	else if (( newQuadrant == 0x03) && (lastQuadrant == 0x02) )
	  {
		low_hold_count = low_hold;
	  }
	lastQuadrant = newQuadrant;
}

 public:
  frequency_handler()=delete;
  /** \brief Constructor
   *
   *  \param sample_rate_id 1 = 48KHz 2 = 96KHz 4 = 192KHz
   * set at construction time as machines can not set it on the fly
   * \param division_rate division value to get lower frequencies at lower step (see the class documentation)
   * The division rate should not be greater than 8 to guarantee the rest of the project
   * It might work for higher values but it is a non sense in this project
   */
 frequency_handler( const unsigned short& samle_rate_id,
					 const unsigned char& division_rate);
  /** \brief set the frequency
   *
   * Set the frequency.
   * \param frequency Enter the frequency as a value to be added to the phase at each (48KHz) sample
   * It is cast as an unsigned long 24.
   *   If the division rate is 1, the value is multiplied by 32 at 48KHz, 16 at 96KHz and 8 at 192KHz.
   *     In this case, the maximum frequency is a period in about 8 samples
   *   If the division rate is higher,
   *  A full period is reached after the angle reached 16777216 by the successive value additions
   */
  constexpr void set_frequency( const unsigned short&frequency )
  {
	if ( frequency < 1 )
	  throw;
	this->frequency = frequency; 
	this->frequency *= 16 * 48;
	this->frequency /= global_rate;
	//cout << "Frequency settings " << this->frequency << endl;
  }
  /** \brief Set the high hold time
   *
   * \param high_hold Enter the hold time. The unit is the number of 16 samples at 48KHz
   * The minimum is 0.333mS, the maximum is 21.85S
   *
   * It gets the requested value and apply the coeficient
   * according with the division rate and the sample rate used
   * The result is stored to initialize the counter if so.
   * Since the hold time terminates when the counter reaches 0
   * the DR and the SR are multiplied to the requested value
   */
  constexpr void set_high_hold(const unsigned short&high_hold)
  {
	this->high_hold = high_hold;
	this->high_hold *= global_rate;
	this->high_hold /= 3;
	//cout << "val high_hold " << 16 * high_hold * global_rate << endl;
  }
  /** \brief Set the low hold time
   *
   * \param low_hold Enter the hold time. The unit is the number of 16 samples at 48KHz
   * The minimum is 0.333mS, the maximum is 21.85S
   *
   * It gets the requested value and apply the coefficient
   * according with the division rate and the sample rate used
   * The result is stored to initialize the counter if so.
   * Since the hold time terminates when the counter reaches 0
   * the DR and the SR are multiplied to the requested value
   */
  constexpr void set_low_hold(const unsigned short&low_hold)
  {
	this->low_hold = low_hold;
	this->low_hold *= global_rate;
	this->low_hold /= 3;
	//  cout << "val low_hold " << 16 * low_hold * global_rate << endl;
  }
  /** \brief Shift the phase
   *
   * shift definitively the phase
   * This should be used with care, perhaps only if the volume is 0
   * \param phsh 4 bits value, the unit is PI/8
   *
   * Since the value is a shift between 0 and (excluded) 2.PI
   * and it is coded using 4 bits unsigned,
   * A shift is performed to move into a 24 bits value
   */ 
  constexpr void shift_phase(const unsigned char&shift)
  {
	unsigned long shift_shifted( ((unsigned long)( 0x0f & shift )) << 20 );
	// cout << hex << angle << "  ";
	angle += shift_shifted;
	// cout << "shift of " << dec << (unsigned short)shift << ", means: " << shift_shifted << "  ";
	CheckHold();
	// cout << hex << angle << endl;
  }
  /** \brief Set the phase
   *
   * set definitively the phase
   * This should be used with care, perhaps only if the volume is 0
   * \param phsh 4 bits value, the unit is PI/8
   *
   * Since the value is a shift between 0 and (excluded) 2.PI
   * and it is coded using 4 bits unsigned,
   * A shift is performed to move into a 24 bits value
   */
  constexpr void set_phase( const unsigned char&new_phase )
  {
	unsigned long shift_shifted( ((unsigned long)( 0x0f & new_phase )) << 20 );
	// cout << hex << angle << "  ";
	angle = shift_shifted;
	// cout << "shift of " << dec << (unsigned short)shift << ", means: " << shift_shifted << "  ";
	// cout << hex << angle << endl;
  }
  /** Run operator
   *
   * Runs the phase at each sample. To get the result, use the relevant function according with the mode
   */
  constexpr void operator()(void)
  {
	startCycle = false;
	if ( high_hold_count != 0 )
	  {
		high_hold_count -= 1;
	  }
	else if ( low_hold_count != 0 )
	  {
		low_hold_count -= 1;
	  }
	else
	  {
		angle += frequency;
		CheckHold();
	  }
  };
  /** Get angle
   *
   * Get the angle for the primitives as sinusoid.
   * The operator run should be used before
   * \return The result is a 24 bits unsigned
   */
  constexpr unsigned long GetAngle(void)const
  {
	return angle;
  }
  /** Get start cycle
   *
   * Get an event each time the angle rolls out the maximum value
   * The operator run should be used before
   * \return Returns a boolean true one time when the cycle starts
   */ 
  constexpr bool GetStartCycle(void)const
  {
	return startCycle;
  }
};

#endif
