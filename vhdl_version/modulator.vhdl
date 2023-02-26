library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

package modulator_pac is
  component modulator_pre is
    generic (
      --! Possible size of the depth\n
      --! Should not be greater than the parameter_data size.
      --! Should be at least 3
      iter_depth_size : integer range 3 to 16 := 16;
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      write_prefix : in std_logic_vector);
    port (
      --! master clock
      CLK     :  in std_logic;
      RST     :  in std_logic;
      --! Low bit:\n
      --! Steps forward when is '1'. This is to control the amplitude
      --! The result of the previous run should be latched no later
      --! than one clock cycle after the EN\n
      --! Other bits:\n
      --! Since it is a state machine that acts "sometimes"
      --!   it can handle more than 1 machine.
      --! This tells which machine.
      --! The number of machines should be a power of 2.
      --! The size of the vector should be the same as parameter machine
      --! Since the data is latched at the begining of a cycle,
      --!   the data should be stable up to the next clk cycle only
      EN_ADDR :  in std_logic_vector;
      --! Similar as the EN_ADDR but tells the chanel is available
      --! Since this module is able to handle more than one (pipeline),
      --! this tells a particular channel is available.\n
      --! The data is still valid until at least
      --!   a new cycle is required
      cycle_completed : out std_logic_vector;
      --! Ready to accept a new input data
      Ready_for_new_data :  out std_logic;
      --! Input amplitude unsigned from others=>'0' to others=>'1'
      input_amplitude :  in std_logic_vector;
      parameter_data :  in std_logic_vector( 15 downto 0 );
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      parameter_write_prefix : in std_logic_vector;
      --! Low bit:\n
      --! Write enable of a pramameter\n
      --! Other bits:\n
      --! Tells which chanel
      parameter_chanel : in std_logic_vector;
      --! 0000 = amplitude which is the modulation depth
      --! Depth of modulation unsigned.\n
      --! From others=>'0' for always 100% of the input\n
      --! To others=>'1' for maximum modulation of the input
      which_parameter :  in std_logic_vector( 3 downto 0 );
      --! Output amplitude multiplied by the depth
      output_input_mul_depth : out std_logic_vector
      );
  end component modulator_pre;
  component modulator_post is
    generic (
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      write_prefix : in std_logic_vector);
    port (
      --! master clock
      CLK     :  in std_logic;
      RST     :  in std_logic;
      --! Low bit:\n
      --! Steps forward when is '1'. This is to control the amplitude
      --! The result of the previous run should be latched no later
      --! than one clock cycle after the EN\n
      --! Other bits:\n
      --! Since it is a state machine that acts "sometimes"
      --!   it can handle more than 1 machine.
      --! This tells which machine.
      --! The number of machines should be a power of 2.
      --! The size of the vector should be the same as parameter machine
      --! Since the data is latched at the begining of a cycle,
      --!   the data should be stable up to the next clk cycle only
      EN_ADDR :  in std_logic_vector;
      --! Similar as the EN_ADDR but tells the chanel is available
      --! Since this module is able to handle more than one (pipeline),
      --! this tells a particular channel is available.\n
      --! The data is still valid until at least
      --!   a new cycle is required
      cycle_completed : out std_logic_vector;
      --! There is no ready to accept a new input data\n
      --!   because it is always faster then the pre in test mode \n
      --!   and always faster than the sine modulation
      --! Input value, the same than the one supplied to pre, unsigned
      input_val :  in std_logic_vector;
      --! Input amplitude multiplied by the depth unsigned from others=>'0' to others=>'1'
      modul_before_sine :  in std_logic_vector;
      --! Input from the modulation sine value signed
      modul_after_sine :  in std_logic_vector;
      parameter_data :  in std_logic_vector( 15 downto 0 );
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      parameter_write_prefix : in std_logic_vector;
      --! Low bit:\n
      --! Write enable of a pramameter\n
      --! Other bits:\n
      --! Tells which chanel
      parameter_chanel : in std_logic_vector;
      --! 0001 = amplitude which is the modulation depth
      which_parameter :  in std_logic_vector( 3 downto 0 );
      --! Unsigned
      output_modulated : out std_logic_vector;
      --! Returns an error if a result exceed the boundary\n
      --! others=>'0' = no error, others=>'1' = error greater than the max,
      --! accroding with the size of the vector, some details about
      --! how much the value exceed the boudary is returned.
      error_data       : out std_logic_vector
      );
  end component modulator_post;

  component modulator_bundle is

  end component modulator_bundle;
end package modulator_pac;

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

entity modulator_pre is
  --! Pamameter  code $0000 is used by modulator_pre: modulation depth
  --! Pamameter  code $0001 is used by modulator_post: modulation mode
  --! Pamameter  code $0010 is used by sample_step_sine
  --! Pamameter  code $0011 is reserved for future use

  generic (
    --! Possible size of the depth\n
    --! Should not be greater than the parameter_data size.
    --! Should be at least 3
    iter_depth_size : integer range 3 to 16 := 16;
    --! Since the frequency handler can be instanciated more than one time
    --! for a given channel, this is a predecode
    write_prefix : in std_logic_vector);
  port (
     --! master clock
    CLK     :  in std_logic;
    RST     :  in std_logic;
    --! Start serial computation.\n
    --! Since the data is latched at the begining of a cycle,
    --!   the data should be stable up to the next clk cycle only
    EN_ADDR : in std_logic_vector;
    --! Similar as the EN_ADDR but tells the chanel is available
    --! Since this module is able to handle more than one (pipeline),
    --! this tells a particular channel is available.\n
    --! The data is still valid until at least
    --!   a new cycle is required
    cycle_completed : out std_logic_vector;
    --! Ready to accept a new input data
    Ready_for_new_data :  out std_logic;
    --! Input amplitude unsigned from others=>'0' to others=>'1'
    input_amplitude :  in std_logic_vector;
    parameter_data :  in std_logic_vector( 15 downto 0 );
    --! Since the frequency handler can be instanciated more than one time
    --! for a given channel, this is a predecode
    parameter_write_prefix : in std_logic_vector;
    --! Low bit:\n
    --! Write enable of a pramameter\n
    --! Other bits:\n
    --! Tells which chanel
    parameter_chanel : in std_logic_vector;
    --! 0000 = amplitude which is the modulation depth
    --! Depth of modulation unsigned.\n
    --! From others=>'0' for always 100% of the input\n
    --! To others=>'1' for maximum modulation of the input
    --! 0010 to 1000 are passed to the frequency handler of the modulation
    which_parameter :  in std_logic_vector( 3 downto 0 );
    --! Output amplitude multiplied by the depth
    output_input_mul_depth : out std_logic_vector
    );
end entity modulator_pre;

architecture arch of modulator_pre is
  signal depth_value : std_logic_vector( iter_depth_size - 1 downto 0 );
  signal depth_value_shift : std_logic_vector( iter_depth_size - 1 downto 0 );
  -- Only one additional bit is enough
  -- 1/2 + 1/4 + 1/8 etc... does never generate overflow,
  --   regardles the number of iterations
  -- Since the number added are slightly lower than the 1/2^^N,
  --   (0).011..111 < (0).100..000  ( 1/2 )
  --   (0).001..111 < (0).010..000  ( 1/4 )
  --   it is Ok
  signal product_result : std_logic_vector( input_amplitude'length + depth_value'length + 1 - 1 downto 0 );
  signal input_amplitude_0_2 : std_logic_vector( product_result'range );
  signal state : std_logic_vector( 4 downto 0 );
begin
  -- Step 1,        pre: multiply the input amplitude by the depth, result is unsigned
  -- Step 2, in between: compute the result divided by 2 minus the input modulation
  -- The result should be unsigned
  -- The input modulation should be also divided by 2,
  -- but it comes as a signed integer,
  -- then its absolute value is already divided by 2
  -- Step 3        post: substract the result from the input amplitude
  -- The result should be unsigned
  --
  -- Some rounding in this computation or the sine may generate
  -- results sligthly negative. Then the arithmetic is done using signed
  -- sometimes the result is set to 0 and and error is reported

  assert EN_ADDR'length = parameter_chanel'length
    report "EN_ADDR and parameter_chanel should have the same size"
      severity failure;
  assert EN_ADDR'length = cycle_completed'length
    report "EN_ADDR (" & integer'image(EN_ADDR'length) &
    ") and parameter_chanel (" & integer'image(cycle_completed'length) & ") should have the same size"
      severity failure;
  assert iter_depth_size < ( parameter_data'length + 1 )
    report "Size of depth should not be greater than the size of parameter_data"
    severity failure;
  assert input_amplitude'length = output_input_mul_depth'length
    report "Input (" & integer'image(input_amplitude'length) &
    ") and output (" & integer'image(output_input_mul_depth'length) & ") amplitudes should have the same size"
    severity failure;
    
  output_input_mul_depth <= product_result( product_result'high downto
                                            product_result'high - output_input_mul_depth'length + 1 );
  main_proc : process( CLK )
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        param_compo : if parameter_chanel( parameter_chanel'low ) = '1' and
                        parameter_write_prefix = write_prefix then
          if which_parameter = "0000" then
            depth_value <=
              parameter_data( parameter_data'high downto parameter_data'high + 1 - depth_value'length );
          end if;
        end if param_compo;
        STATE_IF : if state = "11111" then
          if EN_ADDR( EN_ADDR'low ) = '1' then
            state <= "00000";
            cycle_completed( cycle_completed'high downto cycle_completed'low + 1 ) <=
              EN_ADDR( EN_ADDR'high downto EN_ADDR'low + 1 );
            cycle_completed(cycle_completed'low) <= '0';
            input_amplitude_0_2( input_amplitude_0_2'high ) <= '0';
            input_amplitude_0_2( input_amplitude_0_2'high - 1 downto input_amplitude_0_2'high - 1 - input_amplitude'length + 1 ) <= input_amplitude; 
            input_amplitude_0_2( input_amplitude_0_2'high - 1 - input_amplitude'length + 1 - 1 downto input_amplitude_0_2'low ) <= ( others=> '0' );
            product_result <= ( others => '0' );
            depth_value_shift <= depth_value;
          end if;
        elsif unsigned( state ) /= to_unsigned( depth_value'length - 1 - 1 , state'length ) then
          -- Step two: multiply the modulation by the depth
          if depth_value_shift( depth_value_shift'high ) = '1' then
            product_result <= std_logic_vector( unsigned( product_result ) + unsigned( input_amplitude_0_2 ));
          end if;
          input_amplitude_0_2( input_amplitude_0_2'high ) <= '0';
          input_amplitude_0_2( input_amplitude_0_2'high - 1 downto input_amplitude_0_2'low ) <=
            input_amplitude_0_2( input_amplitude_0_2'high downto input_amplitude_0_2'low + 1 );
          depth_value_shift( depth_value_shift'high downto depth_value_shift'low + 1 ) <=
            depth_value_shift( depth_value_shift'high - 1 downto depth_value_shift'low );
          depth_value_shift( depth_value_shift'low ) <= '-';
          state <= std_logic_vector( unsigned( state ) + 1 );
        else
          cycle_completed(cycle_completed'low) <= '1';
          state <= std_logic_vector( unsigned( state ) + 1 );          
        end if STATE_IF;
      else
        state <= "11111";
        cycle_completed(cycle_completed'low) <= '1';
      end if RST_IF;
    end if;
  end process main_proc;

end architecture arch;

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

entity modulator_post is
  generic (
    --! Since the frequency handler can be instanciated more than one time
    --! for a given channel, this is a predecode
    write_prefix : in std_logic_vector);
  port (
    --! master clock
    CLK     :  in std_logic;
    RST     :  in std_logic;
    --! Low bit:\n
    --! Steps forward when is '1'. This is to control the amplitude
    --! The result of the previous run should be latched no later
    --! than one clock cycle after the EN\n
    --! Other bits:\n
    --! Since it is a state machine that acts "sometimes"
    --!   it can handle more than 1 machine.
    --! This tells which machine.
    --! The number of machines should be a power of 2.
    --! The size of the vector should be the same as parameter machine
    --! Since the data is latched at the begining of a cycle,
    --!   the data should be stable up to the next clk cycle only
    EN_ADDR :  in std_logic_vector;
    --! Similar as the EN_ADDR but tells the chanel is available
    --! Since this module is able to handle more than one (pipeline),
    --! this tells a particular channel is available.\n
    --! The data is still valid until at least
    --!   a new cycle is required
    cycle_completed : out std_logic_vector;
    --! There is no ready to accept a new input data\n
    --!   because it is always faster then the pre in test mode \n
    --!   and always faster than the sine modulation
    --! Input value, the same than the one supplied to pre, unsigned
    input_val :  in std_logic_vector;
    --! Input amplitude multiplied by the depth unsigned from others=>'0' to others=>'1'
    modul_before_sine :  in std_logic_vector;
    --! Input from the modulation sine value signed
    modul_after_sine :  in std_logic_vector;
    parameter_data :  in std_logic_vector( 15 downto 0 );
    --! Since the frequency handler can be instanciated more than one time
    --! for a given channel, this is a predecode
    parameter_write_prefix : in std_logic_vector;
    --! Low bit:\n
    --! Write enable of a pramameter\n
    --! Other bits:\n
    --! Tells which chanel
    parameter_chanel : in std_logic_vector;
    --! 0001 = amplitude which is the modulation depth
    which_parameter :  in std_logic_vector( 3 downto 0 );
    --! Unsigned
    output_modulated : out std_logic_vector;
    --! Returns an error if a result exceed the boundary\n
    --! others=>'0' = no error, others=>'1' = error greater than the max,
    --! accroding with the size of the vector, some details about
    --! how much the value exceed the boudary is returned.
    error_data       : out std_logic_vector
    );
end entity modulator_post;


architecture arch of modulator_post is
  signal state : std_logic_vector( 1 downto 0 );
  signal diff_signal_modulated : std_logic_vector( output_modulated'length downto 0 );
  signal input_val_s : std_logic_vector( input_val'length downto 0 );
  signal abs_not_normal : std_logic := '0';
begin
  assert EN_ADDR'length = cycle_completed'length
    report "EN_ADDR (" & integer'image(EN_ADDR'length) &
    ") and parameter_chanel (" & integer'image(cycle_completed'length) & ") should have the same size"
      severity failure;
  assert modul_after_sine'length = modul_before_sine'length
    report "modul_after_sine (" & integer'image(modul_after_sine'length) &
    ") and modul_before_sine (" & integer'image(modul_before_sine'length) &
    ") should have the same size"
    severity failure;
  assert input_val'length = modul_before_sine'length
    report "Input (" & integer'image(input_val'length) &
    ") and output (" & integer'image(modul_before_sine'length) & ") amplitudes should have the same size"
    severity failure;

  main_proc : process( CLK )
    variable substraction_padding_0 : std_logic_vector( 1 downto 0 );
    variable substraction_abs_padding_0 : std_logic_vector( 0 downto 0 );
    variable substraction_padding_s : std_logic_vector( 0 downto 0 );
    variable padding_1_dsm_ed : std_logic_vector( diff_signal_modulated'length + error_data'length - 1 downto 0 );
    variable padding_0_dsm_ed : std_logic_vector( error_data'range );
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        param_compo : if parameter_chanel( parameter_chanel'low ) = '1' and
                        parameter_write_prefix = write_prefix then
          if which_parameter = "0001" then
            abs_not_normal <=
              parameter_data( parameter_data'low );
          end if;
        end if param_compo;
        STATE_CASE : case state is
          when "11" =>
            state_wait_if : if EN_ADDR( EN_ADDR'low ) = '1' then
              state <= "00";
              input_val_s( input_val_s'high ) <= '0';
              input_val_s( input_val_s'high - 1 downto 0 ) <= input_val;
              if abs_not_normal = '1' then
                substraction_abs_padding_0 := ( others => '0' );
                if modul_after_sine( modul_after_sine'high ) = '0' then
                  diff_signal_modulated <= std_logic_vector(
                    signed( substraction_abs_padding_0 & modul_before_sine(
                      modul_before_sine'high downto modul_before_sine'high - modul_after_sine'length + 1 )) -
                    signed( modul_after_sine & modul_after_sine(modul_after_sine'high - 1 )));
                else
                  diff_signal_modulated <= std_logic_vector(
                    signed( substraction_abs_padding_0 & modul_before_sine(
                      modul_before_sine'high downto modul_before_sine'high - modul_after_sine'length + 1 )) +
                    signed( modul_after_sine & modul_after_sine(modul_after_sine'high - 1 )));
                end if;
              else
                substraction_padding_0 := ( others => '0' );
                substraction_padding_s := ( others => modul_after_sine( modul_after_sine'high ));
                diff_signal_modulated <= std_logic_vector(
                  signed( substraction_padding_0 & modul_before_sine(
                    modul_before_sine'high downto modul_before_sine'high - modul_after_sine'length + 2 )) -
                  signed( substraction_padding_s & modul_after_sine ));
              end if;
              cycle_completed( cycle_completed'high downto cycle_completed'low + 1 ) <=
                EN_ADDR( EN_ADDR'high downto EN_ADDR'low + 1 );
              cycle_completed(cycle_completed'low) <= '0';
            end if state_wait_if;
          when "00" =>
            -- Step Four: the modulation does not increase the amplitude
            -- Then if it is negative, it is set to 0 and nothing is done
            -- Step Five: substract the modulation from the input
            if diff_signal_modulated( diff_signal_modulated'high ) = '0' then
              input_val_s <= std_logic_vector(
                signed( input_val_s ) - signed( diff_signal_modulated ));
              -- else do nothing, until the input and output variables are separated
            end if;
            state <= std_logic_vector( unsigned( state ) + 1 );
          when "01" =>
            if input_val_s( input_val_s'high ) = '0' then
          --   Step six: if positive, Ok 
              output_modulated <= input_val_s( input_val_s'high - 1 downto input_val_s'low );
              error_data <= ( others => '0' );
            else
          --   Step four: is negative, NOk and report the error
              output_modulated <= ( others => '0' );
              padding_1_dsm_ed := ( others => '1' );
              padding_1_dsm_ed := ( others => '0' );
              if diff_signal_modulated(
                diff_signal_modulated'high downto diff_signal_modulated'low + error_data'length ) /=
                padding_1_dsm_ed then
                error_data <= ( others => '1' );
              elsif diff_signal_modulated(
                diff_signal_modulated'low + error_data'length - 1 downto diff_signal_modulated'low ) /=
                padding_0_dsm_ed then
                error_data <= ( others => '1' );
              else
                error_data <= std_logic_vector( unsigned ( not diff_signal_modulated( diff_signal_modulated'low + error_data'length - 1 downto diff_signal_modulated'low )));
              end if;
            end if;
            state <= std_logic_vector( unsigned( state ) + 1 );          
          when "10" =>
            cycle_completed(cycle_completed'low) <= '1';
            state <= std_logic_vector( unsigned( state ) + 1 );          
          when others => null;
        end case STATE_CASE;
      else
        state <= "11";      
      end if RST_IF;
    end if;
  end process main_proc;
end architecture arch;

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all,
  work.modulator_pac.all,
  work.ampl_freq_pacs.all,
  work.signal_gene.sample_step_sine;

entity modulator_bundle is
    generic (
      --! Possible size of the depth\n
      --! Should not be greater than the parameter_data size.
      --! Should be at least 3
      iter_depth_size : integer range 3 to 16 := 16;
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      write_prefix : in std_logic_vector);
    port (
      --! master clock
      CLK     :  in std_logic;
      RST     :  in std_logic;
      --! Low bit:\n
      --! Steps forward when is '1'. This is to control the amplitude
      --! The result of the previous run should be latched no later
      --! than one clock cycle after the EN\n
      --! Other bits:\n
      --! Since it is a state machine that acts "sometimes"
      --!   it can handle more than 1 machine.
      --! This tells which machine.
      --! The number of machines should be a power of 2.
      --! The size of the vector should be the same as parameter machine
      --! Since the data is latched at the begining of a cycle,
      --!   the data should be stable up to the next clk cycle only
      EN_ADDR_mod :  in std_logic_vector;
      --! Similar as the EN_ADDR but tells the chanel is available
      --! Since this module is able to handle more than one (pipeline),
      --! this tells a particular channel is available.\n
      --! The data is still valid until at least
      --!   a new cycle is required
      cycle_completed_mod : out std_logic_vector;
      --! Ready to accept a new input data
      Ready_for_new_data :  out std_logic;
      --! Input amplitude unsigned from others=>'0' to others=>'1'
      input_amplitude :  in std_logic_vector;
      parameter_data :  in std_logic_vector( 15 downto 0 );
      --! Since the frequency handler can be instanciated more than one time
      --! for a given channel, this is a predecode
      parameter_write_prefix : in std_logic_vector;
      --! Low bit:\n
      --! Write enable of a pramameter\n
      --! Other bits:\n
      --! Tells which chanel
      parameter_chanel : in std_logic_vector;
      --! 0000 = amplitude which is the modulation depth
      --! Depth of modulation unsigned.\n
      --! From others=>'0' for always 100% of the input\n
      --! To others=>'1' for maximum modulation of the input
      which_parameter :  in std_logic_vector( 3 downto 0 );
      --! Output amplitude multiplied by the depth
      output_input_mul_depth : out std_logic_vector );
end entity modulator_bundle;

architecture arch of modulator_bundle is
  signal EN_ADDR_pre : std_logic_vector( EN_ADDR_mod'range );
  signal EN_ADDR_post : std_logic_vector( EN_ADDR_mod'range );
  signal cycle_completed_pre : std_logic_vector( cycle_completed_mod'range );
  signal cycle_completed_post : std_logic_vector( cycle_completed_mod'range );
begin

--modulator_pre_instanc : modulator_pre generic map(
--  iter_depth_size => iter_depth_size,
--  write_prefix => write_prefix)
--  port map (
--    CLK => CLK,
--    RST => RST,
--    EN_ADDR => EN_ADDR_pre,
--    cycle_completed => cycle_completed_pre,
--    Ready_for_new_data => open,
--    input_amplitude => input_vector,
--    parameter_data => parameter_data,
--    parameter_write_prefix => parameter_write_prefix,
--    parameter_chanel => parameter_chanel,
--    which_parameter => which_parameter,
--    output_input_mul_depth => output_input_mul_depth);

--modulation_sine_ampl_instanc : amplitude_handler generic map (
--  sample_rate_id_pwr2 => closed,
--  write_prefix => write_prefix)
--  port map (
--    CLK => CLK,
--    RST => RST,
--    EN_ADDR => EN_ADDR_pre,
--    cycle_completed => cycle_completed_pre,
--    master_volume => "11111111",
--    amplitude => close,
--    parameter_data => parameter_data,
--    parameter_write_prefix => parameter_write_prefix,
--    parameter_chanel => parameter_chanel,
--    which_parameter => which_parameter,
--    amplitude_out => closed
--    );

--modulation_sine_freq_instanc : frequency_handler generic map (
--  sample_rate_id_pwr2 => closed,
--  division_rate_pwr2 => closed,
--  hi_low_hold_always_0 => closed,
--  write_prefix => write_prefix)
--  port map (
--    CLK => CLK,
--    RST => RST,
--    EN_ADDR => EN_ADDR_pre,
--    parameter_data => parameter_data,
--    parameter_write_prefix => parameter_write_prefix,
--    parameter_chanel => parameter_chanel,
--    which_parameter => which_parameter,
--    start_cycle => open,
--    angle_out => closed
--    );

--signal_sine_instanc : sample_step_sine generic map (
--  limit_calc => iter_depth_size - 1 )
--  port map (
--    CLK => CLK,
--    RST => RST,
--    start_calc => closed,
--    amplitude => closed,
--    angle => closed,
--    completed => closed,
--    out_z => open,
--    out_s => closed,
--    out_c => open
--    );

--modulator_post_instanc : modulator_post generic map(
--  write_prefix => write_prefix)
--  port map(
--    CLK => CLK,
--    RST => RST,
--    EN_ADDR => EN_ADDR_post,
--    cycle_completed => cycle_completed_post,
--    input_val => input_vector,
--    modul_after_sine => modul_vector,
--    modul_before_sine => output_input_mul_depth,
--    parameter_data => parameter_data,
--    parameter_write_prefix => parameter_write_prefix,
--    parameter_chanel => parameter_chanel,
--    which_parameter => which_parameter,
--    output_modulated => output_modulated,
--    error_data => error_data
--);

end architecture arch;
