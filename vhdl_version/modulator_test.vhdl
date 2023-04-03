--! Use standard library
library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all,
  ieee.math_real.all,
  work.modulator_pac.all;

--! @brief This module runs a single test of modulator
--!
--! \n
--! This test is performed using a dummy sine generator.
--! It is intended to test, in depth, the arithmetic around.
--! For a global test, including the parameters handling in depth,
--!   use modulator_bundle_test
--! For every value, the size of the counters is set separately 
--! to the size of the vector value.\n
--! The vector value is set by the design.\n
--! The counter is set to an equal or lower size of fast tests. 
--! In such case, a barrel counter populates the lower bits
--! to ensure a rail to rail simulation.\n
--! For long simulation a progress data is sent regularly\n
--! \n
--! It can be used in batch mode or in stand alone.
--! The batch mode provides signal to publish the reports after all the
--! instantiations have terminated. This is to avoid mixing the progress with the
--! reported data.
entity modulator_test is
  generic (
    -- to increase this value, review the size of the subcounter
    iter_depth_size    : integer range 3 to 28 := 10;
    nbre_channels_pwr2 : natural := 2;
    modul_counter_size : integer range 3 to 30 := 5;
    modul_vector_size  : integer range 3 to 30 := 8;
    input_counter_size : integer range 3 to 30 := 5;
    input_vector_size  : integer range 3 to 30 := 8;
    depth_counter_size : integer range 3 to 16 := 5);
    -- no depth_vector_size, it is, by deisgn, 16 
  port (
    --! Tells the simulation is over. It is used (with an and-reduce) in batch mode to start all the reporting
    simul_over : out std_logic;
    --! Controls the report. 0 = wait, 1 = do it, U = do it after the simulation is completed (stand alone)
    display_in :  in std_logic;
    --! Pass the event to the next instantiation after the report is completed (batch mode)
    display_out: out std_logic);
end entity modulator_test;


architecture arch of modulator_test is
  signal CLK : std_logic := '0';
  signal RST : std_logic_vector( 5 downto 0 ) := ( others => '1' );
  procedure update_vector_real_unsigned( signal a_counter :  in unsigned;
                                signal a_vector  : out std_logic_vector;
                                signal a_real    : out real ) is
    variable ind_counter : integer;
    variable a_vector_v  : unsigned( a_vector'range );
    variable a_real_v : real;
  begin
    ind_counter := a_counter'high;
    for ind_a in a_vector_v'high downto a_vector_v'low loop
      a_vector_v( ind_a ) := a_counter( ind_counter );
      if ind_counter > a_counter'low then
        ind_counter := ind_counter - 1;
      else
        ind_counter := a_counter'high;
      end if;
    end loop;
    a_vector <= std_logic_vector( a_vector_v );
    a_real_v := real( to_integer( a_vector_v ));
    a_real <= a_real_v / real( 2 ** a_vector_v'length );
  end procedure update_vector_real_unsigned;
  
  procedure update_vector_real_signed( signal a_counter :  in unsigned;
                                signal a_vector  : out std_logic_vector;
                                signal a_vector_view : out unsigned;
                                signal a_real    : out real;
                                signal has_negative : boolean ) is
    variable ind_counter : integer;
    variable a_vector_v  : signed( a_vector'range );
    variable a_real_v : real;
  begin
    ind_counter := a_counter'high;
    if has_negative = FALSE then
      a_vector_v( a_vector_v'high ) := '0';
      for ind_a in a_vector_v'high - 1 downto a_vector_v'low loop
        a_vector_v( ind_a ) := a_counter( ind_counter );
        if ind_counter > a_counter'low then
          ind_counter := ind_counter - 1;
        else
          ind_counter := a_counter'high;
        end if;
      end loop;
    else
      a_vector_v( a_vector_v'high ) := '1';
      for ind_a in a_vector_v'high - 1 downto a_vector_v'low loop
        a_vector_v( ind_a ) := not a_counter( ind_counter );
        if ind_counter > a_counter'low then
          ind_counter := ind_counter - 1;
        else
          ind_counter := a_counter'high;
        end if;
      end loop;
      a_vector_v := a_vector_v + 1;
    end if;
    a_vector <= std_logic_vector( a_vector_v );
    a_real_v := real( to_integer( a_vector_v ));
    a_real <= a_real_v / real( 2 ** a_vector_v'length - 2 );
    a_vector_v( a_vector_v'high ) := not a_vector_v( a_vector_v'high );
    a_vector_view <= unsigned( a_vector_v );
  end procedure update_vector_real_signed;
                                
  signal EN_ADDR_pre : std_logic_vector( nbre_channels_pwr2 downto 0 ) := ( others => '0' );
  signal EN_ADDR_post : std_logic_vector( EN_ADDR_pre'range ) := ( others => '0' );
  signal cycle_compl_pre : std_logic_vector( EN_ADDR_pre'range ) := ( others => '0' );
  signal cycle_compl_post : std_logic_vector( EN_ADDR_pre'range ) := ( others => '0' );
  signal main_counter : unsigned( 1 downto 0 ) := ( others => '0' );
  constant main_counter_max : unsigned( main_counter'range ) := "10";
  -- More recent VHDL versions allow to create records while passing multiple sizes
  signal modul_is_negative : boolean := false;
  signal modul_counter : unsigned( modul_counter_size - 1 downto 0 ) := ( others => '0' );
  constant modul_counter_max : unsigned( modul_counter'range ) := ( others=> '1' );
  signal modul_vector : std_logic_vector( modul_vector_size - 1 downto 0 );
  signal modul_vector_view : unsigned( modul_vector_size - 1 downto 0 );
  signal modul_real : real;

  signal input_counter : unsigned( input_counter_size - 1 downto 0 ) := ( others => '0' );
  constant input_counter_max : unsigned( input_counter'range ) := ( others=> '1' );
  signal input_vector : std_logic_vector( input_vector_size - 1 downto 0 );
  signal input_real : real;

  signal depth_counter : unsigned( depth_counter_size - 1 downto 0 ) := ( others => '0' );
  constant depth_counter_max : unsigned( depth_counter'range ) := ( others=> '1' );
  signal depth_vector : std_logic_vector( 15 downto 0 );
  signal depth_real : real;

  signal sub_counter : std_logic_vector( 4 downto 0 ) := ( others => '0' );
  constant sub_counter_max : std_logic_vector( sub_counter'range ) := ( others => '1' );

  signal val_param : std_logic_vector( 15 downto 0) := "1000000000000000";
  signal parameter_channel : std_logic_vector( nbre_channels_pwr2 downto 0 ) := ( others => '0' );
  signal which_parameter : std_logic_vector( 3 downto 0 ) := ( others => '0' );
  
  signal error_data : std_logic_vector( 6 downto 0 );
  signal error_data_worst : std_logic_vector( error_data'range ) := ( others => '0' );
  
  signal simul_over_s : std_logic := '0';
  signal display_out_s : std_logic := '0';

  signal output_input_mul_depth : std_logic_vector( input_vector'range );

  signal output_modulated : std_logic_vector( input_vector'range );
  signal output_modulated_u : unsigned( input_vector'range );
  signal output_modulated_open : std_logic_vector( input_vector'range );
  signal output_real : real;

  signal output_pre_theoritical_real, output_pre_actual_real : real;
  signal output_pre_actual_real_min : real := real'high;
  signal output_pre_actual_real_max : real := real'low;
  signal output_post_theoritical_real, output_post_actual_real : real;
  signal output_post_actual_real_min : real := real'high;
  signal output_post_actual_real_max : real := real'low;
  
  signal false_signal : boolean := FALSE;

  signal modul_lt_signal : std_logic;
begin
  assert input_counter_size < input_vector_size report "Size of input counter (" &
    integer'image( input_counter_size ) & ") should not exceed the vector size (" &
    integer'image( input_vector_size ) & ")" severity failure;
  assert modul_counter_size < modul_vector_size report "Size of modul counter (" &
    integer'image( modul_counter_size ) & ") should not exceed the vector size (" &
    integer'image( modul_vector_size ) & ")" severity failure;

  assert modul_counter_size <= input_counter_size report "Size of modul counter (" &
    integer'image( modul_counter_size ) & ") should not exceed the input counter size (" &
    integer'image( input_counter_size ) & ")" severity failure;
  -- no assert as depth_counter_size <= 16 and depth_vector_size is always 16 
  simul_over <= simul_over_s;

  update_moduls : process( modul_counter, modul_is_negative )
  begin
    update_vector_real_signed( modul_counter, modul_vector, modul_vector_view, modul_real, modul_is_negative );
  end process update_moduls;

  update_inputs : process( input_counter )
  begin
    update_vector_real_unsigned( input_counter, input_vector, input_real );
  end process update_inputs;
  
  update_depths : process( depth_counter )
  begin
    update_vector_real_unsigned( depth_counter, depth_vector, depth_real );
  end process update_depths;

  update_output : process( output_modulated )
  begin
    output_modulated_u <= unsigned( output_modulated );
    update_vector_real_unsigned( output_modulated_u, output_modulated_open, output_real );
  end process update_output;
  
  main_proc : process
    variable ind_counter: integer;
    variable output_pre_theoritical_real_v, output_pre_actual_real_v : real;
    variable output_post_theoritical_real_v, output_post_actual_real_v : real;
    variable delta_result : real;
  begin
      if main_counter /= main_counter_max then
        CLK_IF : if CLK = '1' then
          RST( RST'high - 1 downto RST'low ) <= RST( RST'high downto RST'low + 1 );
          RST( RST'high ) <= '0';
          RST_IF : if RST = std_logic_vector( to_unsigned( 0 , RST'length )) then

            SUB_MAX : if sub_counter = sub_counter_max then
              if unsigned( error_data ) > unsigned( error_data_worst ) then
                error_data_worst <= error_data;
              end if;
              -- Now testing the test software
              if modul_real <= input_real then
                modul_lt_signal <= '1';
              else
                modul_lt_signal <= '0';
              end if;
              sub_counter <= ( others => '0' );
              -- Compute the result of the pre

              output_pre_theoritical_real_v := input_real * depth_real;
              output_pre_theoritical_real <= output_pre_theoritical_real_v;
              output_pre_actual_real_v := real( to_integer( unsigned( output_input_mul_depth ))) /
                                        real( 2 ** output_input_mul_depth'length );
              output_pre_actual_real <= output_pre_actual_real_v;
              pre_delta_if : if EN_ADDR_post( EN_ADDR_post'low ) = '1' then
                if ( real( output_input_mul_depth'length ) * output_pre_theoritical_real_v ) > 3.0 then
                  -- Regular precision
                  delta_result := ( output_pre_actual_real_v - output_pre_theoritical_real_v ) /
                                  output_pre_theoritical_real_v;
                else
                  -- Value is low, we only check it should.
                  -- The normalisation is against the smallest possible value
                  -- of the output
                  delta_result := ( output_pre_actual_real_v - output_pre_theoritical_real_v ) /
                                  real( 2 ** ( output_input_mul_depth'length + 1 ));
                end if;
                if delta_result > output_pre_actual_real_max then
                  output_pre_actual_real_max <= delta_result;
                end if;
                if delta_result < output_pre_actual_real_min then
                  output_pre_actual_real_min <= delta_result;
                end if;
              end if pre_delta_if;
              output_post_theoritical_real_v := input_real -
                                         ( output_pre_theoritical_real_v / 2.0 - modul_real );
              output_post_theoritical_real <= output_post_theoritical_real_v;
              output_post_actual_real_v := real( to_integer( unsigned( output_modulated ))) /
                                        real( 2 ** output_modulated'length );
              output_post_actual_real <= output_post_actual_real_v;
              post_delta_if : if cycle_compl_post /= "000" then
                if ( real( output_modulated'length ) * output_post_theoritical_real_v ) > 3.0 then
                  -- Regular precision
                  delta_result := ( output_post_actual_real_v - output_post_theoritical_real_v ) /
                                  output_post_theoritical_real_v;
                else
                  -- Value is low, we only check it should.
                  -- The normalisation is against the smallest possible value
                  -- of the output
                  delta_result := ( output_post_actual_real_v - output_post_theoritical_real_v ) /
                                  real( 2 ** ( output_modulated'length + 1 ));
                end if;
                if delta_result > output_post_actual_real_max then
                  output_post_actual_real_max <= delta_result;
                end if;
                if delta_result < output_post_actual_real_min then
                  output_post_actual_real_min <= delta_result;
                end if;
              end if post_delta_if;
              -- In real life, the cordic algo, computes the sine and the
              -- product with the amplitude
              -- Here, we are testing with all possible values of
              -- sine( output_pre_theoritical_real ) positive or negatives
              -- A loop with variable modul_real generates all the possible values between
              --   +output_pre_theoritical_real end - output_pre_theoritical_real 
              -- that slighly increases the simulation time as 0 is treated twice
              -- Otherwyse, one would have to test the test!
              MODUL_NEG : if modul_is_negative then
                modul_is_negative <= false;
                  -- This is a triangle signal to speed up the simulation
                  -- The real sine is simulated in the bundle simulation
                if modul_counter < unsigned( output_input_mul_depth(
                  output_input_mul_depth'high downto output_input_mul_depth'high - modul_counter'length + 1 )) then
                  modul_counter <= modul_counter + 1;
                else
                  modul_counter <= ( others => '0' );
                  if input_counter /= input_counter_max then
                    input_counter <= input_counter + 1;
                  else
                    report "L " &
                      integer'image( to_integer( unsigned( depth_counter )) + 1 ) & "/" &
                      integer'image( 2**depth_counter'length ) & ", " &
                      integer'image( to_integer( unsigned( main_counter )) + 1 ) & "/2 done";
                    input_counter <= ( others => '0' );
                    if depth_counter /= depth_counter_max then
                      depth_counter <= depth_counter + 1;
                    else
                      depth_counter <= ( others => '0' );
                      main_counter <= main_counter + 1;
                    end if;
                  end if;
                end if;
              else
                modul_is_negative <= TRUE;
              end if MODUL_NEG;
            else
              sub_counter <= std_logic_vector( unsigned( sub_counter ) + 1 );
              SUB_COUNTER_CASE : case sub_counter is
                when "00001" =>
                  val_param <= depth_vector;
                  which_parameter <= "0000";
                when "00010" =>
                  parameter_channel( parameter_channel'low ) <= '1';
                when "00011" =>
                  parameter_channel( parameter_channel'low ) <= '0';
                when "00100" =>
                  val_param( val_param'low ) <= main_counter( main_counter'low );
                  which_parameter <= "0001";
                when "00101" =>
                  parameter_channel( parameter_channel'low ) <= '1';
                when "00110" =>
                  parameter_channel( parameter_channel'low ) <= '0';
                when "00111" =>
                  EN_ADDR_pre( EN_ADDR_pre'low ) <= '1';
                  EN_ADDR_post( EN_ADDR_post'low ) <= cycle_compl_pre( cycle_compl_pre'low );
                when "01000" =>
                  EN_ADDR_pre( EN_ADDR_pre'low ) <= '0';
                  EN_ADDR_post( EN_ADDR_post'low ) <= '0';
                when others =>
                  null;
              end case SUB_COUNTER_CASE;
            end if SUB_MAX;
          end if RST_IF;
        end if CLK_IF;
        CLK <= not CLK;
        wait for 20 nS;
      else
        report "Simulation is over" severity note;
        report "Pre module: output error between " &
          real'image( output_pre_actual_real_min ) & " and " &
          real'image( output_pre_actual_real_max )
          severity note;
        report "Post module: output error between " &
          real'image( output_post_actual_real_min ) & " and " &
          real'image( output_post_actual_real_max )
          severity note;        
        report "Worst error: " & integer'image( to_integer( unsigned( error_data_worst ))); 
        report "See in the test vhdl code for details about how it is calculated" severity note;
        report "This test does not simulate the bundle pre+sine+post" severity note;
        simul_over_s <= '1';
        wait;
      end if;
    end process main_proc;

    display : process
    begin
      wait until ( display_in = '1' or ( display_in = 'U' and simul_over_s = '1' ));
      assert false report "Simulation is over" severity note;
      display_out_s <= '1';
    end process display;

modulator_pre_instanc : modulator_pre generic map(
  iter_depth_size => iter_depth_size,
  write_prefix => "00001111")
  port map (
    CLK => CLK,
    RST => RST( RST'low ),
    EN_ADDR => EN_ADDR_pre,
    cycle_completed => cycle_compl_pre,
    Ready_for_new_data => open,
    input_amplitude => input_vector,
    parameter_data =>    val_param,
    parameter_write_prefix => "00001111",
    parameter_channel => parameter_channel,
    which_parameter => which_parameter,
    output_input_mul_depth => output_input_mul_depth);

modulator_post_instanc : modulator_post generic map(
  write_prefix => "00001111")
  port map(
    CLK => CLK,
    RST => RST( RST'low ),
    EN_ADDR => EN_ADDR_post,
    cycle_completed => cycle_compl_post,
    input_val => input_vector,
    modul_after_sine => modul_vector,
    modul_before_sine => output_input_mul_depth,
    parameter_data =>    val_param,
    parameter_write_prefix => "00001111",
    parameter_channel => parameter_channel,
    which_parameter => which_parameter,
    output_modulated => output_modulated,
    error_data => error_data
);

  
end architecture arch;



library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all,
  ieee.math_real.all,
  work.modulator_pac.all,
  work.ampl_freq_pacs.all;

--! @brief This module runs a single test of modulator
--!
--! \n
--! This test is performed using the real sine generator.
--! It is intended to test, in depth, the arithmetic around.
--! For an in-depth test of the arithmetic,
--!   use modulator_test
entity modulator_bundle_test is
  port (
    --! Tells the simulation is over. It is used (with an and-reduce) in batch mode to start all the reporting
    simul_over : out std_logic;
    --! Controls the report. 0 = wait, 1 = do it, U = do it after the simulation is completed (stand alone)
    display_in :  in std_logic;
    --! Pass the event to the next instantiation after the report is completed (batch mode)
    display_out: out std_logic);

end entity modulator_bundle_test;
