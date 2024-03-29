--! Use standard library
library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

package signal_gene is
component sample_step_sine is
  generic (
        --! Send (others=>'1') for full computation, or an unsigned limit for fast one
    limit_calc : integer range 4 to 31 := 31 );
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! Unsigned 0 to 2.PI value
    angle      : in  std_logic_vector(23 downto 0);
    completed  : out std_logic;
    --! Signed truncated z (mostly used to test the residual value is close to 0)
    out_z      : out std_logic_vector(23 downto 0);
    --! Signed sine output;
    out_s      : out std_logic_vector;
    --! Signed cosine output (mostly used for test purposes)
    out_c      : out std_logic_vector(23 downto 0));
end component sample_step_sine;

component sample_step_triangle is
  generic (
        --! Send (others=>'1') for full computation, or an unsigned limit for fast one
    limit_calc : integer range 4 to 14 := 10 );
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! Unsigned 0 to 2.PI value
    angle      : in  std_logic_vector(23 downto 0);
    completed  : out std_logic;
    --! Signed triangle output;
    out_t      : out std_logic_vector);
end component sample_step_triangle;

component sample_step_pulse is
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! No angle, only a starting pulse signal (not the start_calc)
    start_pulse : in std_logic;
    width      : in  std_logic_vector(15 downto 0);
    --! Completed signal is always 1 as the computation is only one subtraction,
    --! and sometimes a negation
    completed  : out std_logic;
    --! Signed pulse output
    out_p      : out std_logic_vector);
end component sample_step_pulse;

end package signal_gene;




library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! @brief This module computes the sine (and cosine)\n
--!
--! It does not know anything about the frequency\n
--! It takes an unsigned 24 bits as the angle,
--! low $0 = angle 0, high $ffffff = angle 2.PI - epsilon\n
--! It returns the sine as an unconstrainst signed\n
--! In run mode, the 16 bits is the standard.
--! In debug mode, 24 bits is nice for some verifications\n
--! The cosine and the residual z values as signed 24 bits are returned\n
--! In run mode, they should be left open
--! In debug mode, they provide a high precision for validation\n
--! More information is in the files located in the cpp_version folder
entity sample_step_sine is
  generic (
        --! Send (others=>'1') for full computation, or an unsigned limit for fast one
    limit_calc : integer range 4 to 31 := 31 );
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! Unsigned 0 to 2.PI value
    angle      : in  std_logic_vector(23 downto 0);
    completed  : out std_logic;
    --! Signed truncated z (mostly used to test the residual value is close to 0)
    out_z      : out std_logic_vector(23 downto 0);
    --! Signed sine output;
    out_s      : out std_logic_vector;
    --! Signed cosine output (mostly used for test purposes)
    out_c      : out std_logic_vector(23 downto 0));
end entity sample_step_sine;

--! This architecture computes the algo for the run and debug mode\n
--! The precision can be limited to a certain number of iterations for fast calculations
architecture arch of sample_step_sine is
  subtype z_range is integer range 23 downto 0;
  subtype amplitude_range is integer range 23 downto 0;
  subtype a_range_input is integer range amplitude_range'high - 2 downto amplitude_range'low;  
  type cor_angle_list_t is array(integer range<>) of std_logic_vector(z_range);
  constant cor_angle_list : cor_angle_list_t( 0 to 19 ) := (x"200000", x"12E405", x"09FB38", x"051112", x"028B0D", x"0145D8", x"00A2F6", x"00517C", x"0028BE", x"00145F", x"000A30", x"000518", x"00028C", x"000146", x"0000A3", x"000051", x"000029", x"000014", x"00000A", x"000005");
-- List of the angles for which their tangent is the inverse of a power of 2 
  signal the_sin : std_logic_vector( amplitude_range );
  signal the_cos : std_logic_vector( amplitude_range );
  signal shifted_sin : std_logic_vector( amplitude_range );
  signal shifted_cos : std_logic_vector( amplitude_range );
  signal the_z   : std_logic_vector( z_range );
  signal cordic_iter : std_logic_vector( 4 downto 0 );
  signal cordic_iter_last : std_logic_vector( cordic_iter'range );
begin
  assert out_s'length > 2 report "Size of the output should be at least 3, " & integer'image( out_s'length ) & " is a non sense" severity error;
  assert cor_angle_list'length < 30 report "Size of cordic_iter is too small" severity failure;
  assert amplitude'length <= the_z'length report "Size of the amplitude (" & integer'image( amplitude'length ) & ") should be lower of equal of the size of z " & integer'image( the_z'length ) & ")" severity failure;
  
  out_z <= the_z;
  out_s <= the_sin(the_sin'high downto the_sin'high + 1 - out_s'length );
  out_c <= the_cos(the_cos'high downto the_cos'high + 1 - out_c'length );
      
main_proc : process ( CLK )
  variable v_cos : std_logic_vector( amplitude_range );
  variable shifted_cos_v : std_logic_vector( amplitude_range );
  variable finalcalc_op_1, finalcalc_op_2 : std_logic_vector( amplitude_range );
  variable shifted_sincos_padding_5 : std_logic_vector( 4 downto 0 );
  variable shifted_sincos_padding_2 : std_logic_vector( 1 downto 0 );
  variable shifted_sincos_padding_1 : std_logic_vector( 0 downto 0 );
  variable v_sin : std_logic_vector( amplitude_range );
  variable shifted_sin_v : std_logic_vector( amplitude_range );
  variable v_z   : std_logic_vector( z_range );
  variable delta_z : std_logic_vector( z_range );
  variable shifted_sin_padding : std_logic_vector( amplitude_range );
  variable shifted_cos_padding : std_logic_vector( amplitude_range );
  variable input_amplitude : std_logic_vector( a_range_input );
  variable ind_a : integer;
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        -- Index is up to N - 1, process the cordic algo
        RUN_IF : if to_integer( unsigned( cordic_iter )) < to_integer( unsigned( cordic_iter_last )) then
          if to_integer( unsigned( cordic_iter )) < cor_angle_list'length then
            delta_z := cor_angle_list( to_integer( unsigned( cordic_iter )));
          else
            delta_z := ( others => '0' );
          end if;
          shifted_sin_v := the_sin;
          shifted_cos_v := the_cos;
          -- Computes the sine and cosine multiplied by the tangent
          if cordic_iter /= std_logic_vector( to_unsigned ( 0, cordic_iter'length )) then
				shifted_sin_padding := ( others => the_sin( the_sin'high ) );
				shifted_sin_v( shifted_sin_v'high downto shifted_sin_v'high + 1 - to_integer ( unsigned( cordic_iter )) - 1 ) :=
					shifted_sin_padding( shifted_sin_padding'high downto shifted_sin_padding'high + 1 - to_integer ( unsigned( cordic_iter )) - 1 );
                shifted_sin_v( shifted_sin_v'high - to_integer ( unsigned( cordic_iter )) - 1 downto shifted_sin_v'low ) :=
					the_sin( the_sin'high - 1 downto the_sin'low + to_integer ( unsigned( cordic_iter )) );

			    shifted_cos_padding := ( others => the_cos( the_cos'high ) );
                shifted_cos_v( shifted_cos_v'high downto shifted_cos_v'high + 1 - to_integer ( unsigned( cordic_iter )) - 1 ) :=
                  shifted_cos_padding( shifted_cos_padding'high downto shifted_cos_padding'high + 1 - to_integer ( unsigned( cordic_iter )) - 1 );
                shifted_cos_v( shifted_cos_v'high - to_integer ( unsigned( cordic_iter )) - 1 downto shifted_cos_v'low ) :=
					the_cos( the_cos'high - 1 downto the_cos'low + to_integer ( unsigned( cordic_iter )) );
          end if;
          -- Run the algo a, y anhd z
          if signed( the_z ) > to_signed( 0 , the_z'length ) then
            the_sin <= std_logic_vector( signed( the_sin ) + signed( shifted_cos_v )); 
            the_cos <= std_logic_vector( signed( the_cos ) - signed( shifted_sin_v )); 
            the_z <= std_logic_vector( signed( the_z ) - signed( delta_z )); 
          else
            the_sin <= std_logic_vector( signed( the_sin ) - signed( shifted_cos_v )); 
            the_cos <= std_logic_vector( signed( the_cos ) + signed( shifted_sin_v )); 
            the_z <= std_logic_vector( signed( the_z ) + signed( delta_z ));
          end if;
          cordic_iter <= std_logic_vector( unsigned( cordic_iter ) + 1 );
        elsif cordic_iter = cordic_iter_last then
          -- Index is N then final processing is processed
          -- The algo lefts a cosine( h ) at every step
          -- The multiplication is done at the end with the product of all of them
          -- See in the tek_and_gene folder for more information
          --
          -- In our case ( 2 first iterations done by hand ) the coeff is 1.0011011
          -- at this point we added the sin and cos shifted by 1 and 2
          -- Next point, we added the shift shifted by 2 and by 5
          finalcalc_op_1 := ( others => the_sin( the_sin'high ));
          finalcalc_op_1( finalcalc_op_1'high - 1 downto finalcalc_op_1'low ) := the_sin( the_sin'high downto the_sin'low + 1 );
          finalcalc_op_2 := ( others => the_sin( the_sin'high ));
          finalcalc_op_2( finalcalc_op_2'high - 2 downto finalcalc_op_2'low ) := the_sin( the_sin'high downto the_sin'low + 2 );
          shifted_sin <= std_logic_vector( signed( finalcalc_op_1 ) + signed( finalcalc_op_2 ));
          finalcalc_op_1 := ( others => the_cos( the_cos'high ));
          finalcalc_op_1( finalcalc_op_1'high - 1 downto finalcalc_op_1'low ) := the_cos( the_cos'high downto the_cos'low + 1 );
          finalcalc_op_2 := ( others => the_cos( the_cos'high ));
          finalcalc_op_2( finalcalc_op_2'high - 2 downto finalcalc_op_2'low ) := the_cos( the_cos'high downto the_cos'low + 2 );
          shifted_cos <= std_logic_vector( signed( finalcalc_op_1 ) + signed( finalcalc_op_2 ));
          cordic_iter <= std_logic_vector( unsigned( cordic_iter_last ) + 1 );
        elsif cordic_iter = std_logic_vector( unsigned( cordic_iter_last ) + 1 ) then
          finalcalc_op_1 := ( others => shifted_sin( shifted_sin'high ));
          finalcalc_op_1( finalcalc_op_1'high - 2 downto finalcalc_op_1'low ) := shifted_sin( shifted_sin'high downto shifted_sin'low + 2 );
          finalcalc_op_2 := ( others => shifted_sin( shifted_sin'high ));
          finalcalc_op_2( finalcalc_op_2'high - 5 downto finalcalc_op_2'low ) := shifted_sin( shifted_sin'high downto shifted_sin'low + 5 );
          the_sin <= std_logic_vector( signed( the_sin ) + signed( finalcalc_op_1 ) + signed( finalcalc_op_2 ));
          finalcalc_op_1 := ( others => shifted_cos( shifted_cos'high ));
          finalcalc_op_1( finalcalc_op_1'high - 2 downto finalcalc_op_1'low ) := shifted_cos( shifted_cos'high downto shifted_cos'low + 2 );
          finalcalc_op_2 := ( others => shifted_cos( shifted_cos'high ));
          finalcalc_op_2( finalcalc_op_2'high - 5 downto finalcalc_op_2'low ) := shifted_cos( shifted_cos'high downto shifted_cos'low + 5 );
          the_cos <= std_logic_vector( signed( the_cos ) + signed( finalcalc_op_1 ) + signed( finalcalc_op_2 ));
          completed <= '1';
          cordic_iter <= ( others => '1' );
        -- If on hold, starting now
        elsif start_calc = '1' then
          cordic_iter <= ( others => '0' );

          -- Fill up the amplitude with a barrel shifting of the given amplitude
          -- This improves the rail to rail in case the precision of the input
          -- is limited
          ind_a := amplitude'length - 1;
          build_input_amplitude: for ind_ia in input_amplitude'high downto input_amplitude'low loop
            input_amplitude( ind_ia ) := amplitude( amplitude'low + ind_a );
            if ind_a /= 0 then
              ind_a := ind_a - 1;
            else
              ind_a := amplitude'length - 1;
            end if;
          end loop build_input_amplitude;
			           
          v_cos := ( others => '0' );
          v_sin := ( others => '0' );
          -- Execute the 2 first step by hand while placing the value into 
          case angle( angle'high downto angle'high - 1 ) is
            when "00" =>
              v_cos( the_cos'high - 2 downto the_cos'low ) := input_amplitude;				  
            when "01" =>
              v_sin( the_sin'high - 2 downto the_sin'low ) := input_amplitude;
            when "10" =>
              v_cos( the_cos'high - 2 downto the_cos'low ) := input_amplitude;
              v_cos := std_logic_vector( - signed( v_cos ));
            when "11" =>
              v_sin( the_sin'high - 2 downto the_sin'low ) := input_amplitude;
              v_sin := std_logic_vector( - signed( v_sin ));
            when others =>
              NULL;
          end case;
          v_z := ( others => '0' );
          v_z( v_z'high - 2 downto v_z'high + 1 - angle'length ) := angle( angle'high - 2 downto angle'low );
          the_z <= v_z;
          the_sin <= v_sin;
          the_cos <= v_cos;
          completed <= '0';
          -- Initiate the termination if all the list was computed or if an
          -- abort is reached
          if limit_calc < to_integer( unsigned( cordic_iter )) then
            cordic_iter_last <= std_logic_vector( to_unsigned( limit_calc, cordic_iter'length ));
          else
            cordic_iter_last <= std_logic_vector( to_unsigned( cor_angle_list'length, cordic_iter'length ));
          end if;
          -- no else as there is nothing to do, waiting for the start
        end if RUN_IF;
      else
        -- This is always greater than the limit calc
        -- cordic_iter is a 5 bits vector as the list contains about 20 elements
        cordic_iter <= ( others => '1' );
        completed <= '0';
      end if RST_IF;
    end if;
  end process main_proc;  
      
end architecture arch;

--! This architecture computes a very fast calculation 
--! to get a very fast behaviour while verifying at a higher level.
--! The 4 high bits of the angle are considered, the others are voided.\n
--! PI/8 is the step of the calculation
architecture fast of sample_step_sine is
  subtype z_range is integer range 23 downto 0;
  subtype amplitude_range is integer range 23 downto 0;
  subtype a_range_input is integer range amplitude_range'high - 1 downto amplitude_range'low;  
  type cor_angle_list_t is array(integer range<>) of std_logic_vector(z_range);
  constant cor_angle_list_length : integer := 20;
-- List of the angles for which their tangent is the inverse of a power of 2 
  signal the_sin : std_logic_vector( amplitude_range );
  signal the_cos : std_logic_vector( amplitude_range );
  signal the_z   : std_logic_vector( z_range );
  signal cordic_iter : std_logic_vector( 4 downto 0 );
  signal cordic_iter_last : std_logic_vector( cordic_iter'range );

  signal d1 : std_logic_vector( a_range_input );
  signal d2, d3, d4 : std_logic_vector( amplitude_range );
begin
  assert out_s'length > 2 report "Size of the output should be at least 3, " & integer'image( out_s'length ) & " is a non sense" severity error;
  assert cor_angle_list_length < 30 report "Size of cordic_iter is too small" severity failure;
  assert amplitude'length <= the_z'length report "Size of the amplitude (" & integer'image( amplitude'length ) & ") should be lower of equal of the size of z " & integer'image( the_z'length ) & ")" severity failure;
  
  out_z <= the_z;
  out_s <= the_sin(the_sin'high downto the_sin'high + 1 - out_s'length );
  out_c <= the_cos(the_cos'high downto the_cos'high + 1 - out_c'length );
      
main_proc : process ( CLK )
  variable v_cos : std_logic_vector( amplitude_range );
  variable v_sin : std_logic_vector( amplitude_range );
  variable input_amplitude : std_logic_vector( a_range_input );
  variable input_full : std_logic_vector( amplitude_range );
  variable input_half : std_logic_vector( amplitude_range );
  variable input_fourth : std_logic_vector( amplitude_range );
  variable input_eighth : std_logic_vector( amplitude_range );
  variable input_sixteenth : std_logic_vector( amplitude_range );
  variable ind_a : integer;
--  variable dumy_var : std_logic;
begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        -- Index is up to N - 1, process the cordic algo
        RUN_IF : if to_integer( unsigned( cordic_iter )) < to_integer( unsigned( cordic_iter_last )) then

          cordic_iter <= std_logic_vector( unsigned( cordic_iter ) + 1 );
        elsif cordic_iter = cordic_iter_last then
          cordic_iter <= std_logic_vector( unsigned( cordic_iter ) + 1 );          
        elsif cordic_iter = std_logic_vector( unsigned( cordic_iter_last ) + 1 ) then
          completed <= '1';
          cordic_iter <= ( others => '1' );
        -- If on hold, starting now
        elsif start_calc = '1' then
          cordic_iter <= ( others => '0' );

          ind_a := amplitude'length - 1;
          build_input_amplitude: for ind_ia in input_amplitude'high downto input_amplitude'low loop
            input_amplitude( ind_ia ) := amplitude( amplitude'low + ind_a );
            if ind_a /= 0 then
              ind_a := ind_a - 1;
            else
              ind_a := amplitude'length - 1;
            end if;
          end loop build_input_amplitude;
          input_full( input_half'high ) :=                                           '0';
          input_half( input_half'high downto input_half'high - 1 ) :=                ( others => '0' );
          input_fourth( input_fourth'high downto input_fourth'high - 2 ) :=          ( others => '0' );
          input_eighth( input_eighth'high downto input_eighth'high - 3 ) :=          ( others => '0' );
          input_sixteenth( input_sixteenth'high downto input_sixteenth'high - 4 ) := ( others => '0' );
          input_full( input_full'high - 1 downto input_full'low ) :=                input_amplitude;
          input_half( input_half'high - 2 downto input_half'low ) :=                input_amplitude( input_amplitude'high downto input_amplitude'low + 1 );
          input_fourth( input_fourth'high - 3 downto input_fourth'low ) :=          input_amplitude( input_amplitude'high downto input_amplitude'low + 2 );
          input_eighth( input_eighth'high - 4 downto input_eighth'low ) :=          input_amplitude( input_amplitude'high downto input_amplitude'low + 3 );
          input_sixteenth( input_sixteenth'high - 5 downto input_sixteenth'low ) := input_amplitude( input_amplitude'high downto input_amplitude'low + 4 );

          d1 <= input_amplitude;
          d2 <= input_fourth;
          d3 <= input_eighth;
          d4 <= input_sixteenth;
          
          v_cos := ( others => '0' );
          v_sin := ( others => '0' );
          case angle( angle'high downto angle'high - 3 ) is
            when "0000" | "1000" =>
              v_cos := input_full; 
            when "0100" | "1100" =>
              v_sin := input_full;
            when "0010" | "0110" | "1010" | "1110" =>
              v_cos := std_logic_vector( signed( input_half ) + signed( input_eighth ) + signed( input_sixteenth ));
              v_sin := std_logic_vector( signed( input_half ) + signed( input_eighth ) + signed( input_sixteenth ));
            when "0001" | "0111" | "1001" | "1111" =>
              v_cos := std_logic_vector( signed( input_half ) + signed( input_fourth ) + signed( input_sixteenth ));
              v_sin := input_half;
            when "0011" | "0101" | "1011" | "1101" =>
              v_cos := input_half;
              v_sin := std_logic_vector( signed( input_half ) + signed( input_fourth ) + signed( input_sixteenth ));
            when others =>
              NULL;
          end case;
          if angle( angle'high ) = '1' then
            the_sin <= std_logic_vector( - signed( v_sin ));
          else
            the_sin <= v_sin;
          end if;
          if angle( angle'high ) = '1' xor angle( angle'high - 1 ) = '1' then
            the_cos <= std_logic_vector( - signed( v_cos ));
          else
            the_cos <= v_cos;
          end if;
          the_z <= ( others => '0' );
          completed <= '0';
          -- Initiate the termination if all the list was computed or if an
          -- abort is reached
          if limit_calc  < to_integer( unsigned( cordic_iter )) then
            cordic_iter_last <= std_logic_vector( to_unsigned( limit_calc, cordic_iter_last'length ));
          else
            cordic_iter_last <= std_logic_vector( to_unsigned( cor_angle_list_length, cordic_iter'length ));
          end if;
          -- no else as there is nothing to do, waiting for the start
        end if RUN_IF;
      else
        -- This is always greater than the limit calc
        -- cordic_iter is a 5 bits vector as the list contains about 20 elements
        cordic_iter <= ( others => '1' );
        completed <= '0';
      end if RST_IF;
    end if;
  end process main_proc;  
      
end architecture fast;
    

library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! @brief This module computes the sine (and cosine)\n
--!
--! It does not know anything about the frequency\n
--! It takes an unsigned 24 bits as the angle,
--! low $0 = angle 0, high $ffffff = angle 2.PI - epsilon\n
--! It returns the triangle as an unconstrained signed\n
--! In run mode, the 16 bits is the standard.
--! In debug mode, 24 bits is nice for some verifications\n
--! Since the precision is limited and symmetry properties applied,\n
--! the zero crossing is done with 2 samples at 0\n

entity sample_step_triangle is
  generic (
        --! Send (others=>'1') for full computation, or an unsigned limit for fast one
    limit_calc : integer range 4 to 14 := 10 );
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! Unsigned 0 to 2.PI value
    angle      : in  std_logic_vector(23 downto 0);
    completed  : out std_logic;
    --! Signed triangle output;
    out_t      : out std_logic_vector);
end entity sample_step_triangle;

architecture arch of sample_step_triangle is
  signal angle_h : std_logic;
  signal angle_l : std_logic_vector( angle'high - 3 downto angle'low );
  signal the_out : std_logic_vector( out_t'range );
  signal amplitude_iter : std_logic_vector( amplitude'high - 2 downto amplitude'low );
  signal triangle_iter, triangle_iter_last : std_logic_vector( 3 downto 0 );
begin

  assert amplitude'length <= the_out'length
       report "Size of amplitude should be less than size of the_out"
             severity failure;

  out_t <= the_out;

  main_proc: process ( CLK )
  variable angle_m : std_logic;
  variable padding2 : std_logic_vector( 1 downto 0 );
  begin
  if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        -- Index is up to N - 1, process the triangle product algo
        RUN_IF : if to_integer( unsigned( triangle_iter )) < to_integer( unsigned( triangle_iter_last ) - 1 ) then
          if angle_l( angle_l'high ) = '1' then
            padding2 := "00";
            the_out <= std_logic_vector( unsigned( the_out ) + unsigned( padding2 & amplitude_iter ));
          end if;
          amplitude_iter( amplitude_iter'high - 1 downto amplitude_iter'low ) <=
            amplitude_iter( amplitude_iter'high downto amplitude_iter'low + 1 );
          amplitude_iter( amplitude_iter'high ) <= '0';
          angle_l( angle_l'high downto angle_l'low + 1 ) <= angle_l(angle_l'high - 1 downto angle_l'low );
          angle_l( angle_l'low ) <= angle_l( angle_l'high );
          triangle_iter <= std_logic_vector( unsigned( triangle_iter ) + 1 );
        elsif triangle_iter < std_logic_vector( unsigned( triangle_iter_last )) then
          -- Check if negative, eexcetute if so
          if angle_h = '1' then
            the_out <= std_logic_vector( - signed( the_out ));
          end if;
          triangle_iter <= std_logic_vector( unsigned( triangle_iter ) + 1 );
        elsif triangle_iter = triangle_iter_last then
          completed <= '1';
          triangle_iter <= ( others => '1' );
        -- If on hold, starting now
        elsif start_calc = '1' then
          triangle_iter <= ( others => '0' );
          angle_h <= angle( angle'high );
          if angle( angle'high - 1 ) = '0' then
            angle_m := angle( angle'high - 2 );
            angle_l <= angle( angle'high - 3 downto angle'low );
          else
            angle_m := not angle( angle'high - 2 );
            angle_l <= not angle( angle'high - 3 downto angle'low );
          end if;
          if angle_m = '1' then
            -- saturation of the triangle
            -- load the final value only and prevent any multiplication
            the_out( the_out'high ) <= '0';
            the_out( the_out'high - 1 downto the_out'low ) <= amplitude( amplitude'high downto amplitude'high - the_out'length + 1 + 1 );
            amplitude_iter <= ( others => '0' );
          else
            the_out <= ( others => '0' );
            amplitude_iter <= amplitude( amplitude'high downto amplitude'high - the_out'length + 1 + 2 );
          end if;
          triangle_iter_last <= std_logic_vector( to_unsigned( limit_calc, triangle_iter_last'length ));
          completed <= '0';
        end if;
      else
        completed <= '0';
      end if RST_IF;
    end if;
  end process main_proc;

end architecture arch;


library ieee;
use ieee.std_logic_1164.all,
  ieee.numeric_std.all;

--! @brief This module computes the sine (and cosine)\n
--!
--! It does not know anything about the frequency\n
--! It takes a start_pulse signal, not an angle,
--! It runs every start_calc until the end of the pulse.
--! It waits, sending 0, until the next start
--! It returns the pulse as an unconstrained signed\n
--! In run mode, the 16 bits is the standard.
--! In debug mode, 24 bits is nice for some verifications\n
--! More information is in the files located in the cpp_version folder

entity sample_step_pulse is
  port (
    --! Component runs on each clock cycle (if it is in the running state)
    CLK        : in  std_logic;
    RST         : in  std_logic;
    start_calc : in  std_logic;
    --! Unsigned amplitude value
    amplitude  : in  std_logic_vector;
    --! No angle, only a starting pulse signal (not the start_calc)
    start_pulse : in std_logic;
    width      : in  std_logic_vector(15 downto 0);
    --! Completed signal is always 1 as the computation is only one subtraction,
    --! and sometimes a negation
    completed  : out std_logic;
    --! Signed pulse output
    out_p      : out std_logic_vector);
end entity sample_step_pulse;


architecture arch of sample_step_pulse is
  signal amplitude_s : std_logic_vector( amplitude'length downto 0 );
  signal state : std_logic_vector( 5 downto 0 );
  signal length_count : std_logic_vector( width'range );
begin
  assert out_p'length > 1 report "Size of the output should be at least 2, " & integer'image( out_p'length ) & " is a non sense" severity error;
  assert amplitude'length >= ( out_p'length - 1 )
    report "Size of the output (" & integer'image( amplitude'length ) &
    ") should not be larger than the size of the amplitude ( " & integer'image( amplitude'length ) &
    ") + 1"
    severity failure;

  out_p <= amplitude_s(amplitude_s'high downto amplitude_s'high + 1 - out_p'length );
  
  completed <= '1';
  main_proc : process( CLK ) is
    variable amplitude_v : std_logic_vector( amplitude_s'range );
  begin
    if rising_edge( CLK ) then
      RST_IF : if RST = '0' then
        START_PULSE_IF : if start_pulse = '1' and state = "000000" then
          state <= "000001";
        elsif start_calc = '1' then
          amplitude_v := ( others => '0' );
          case state is
            -- when "000000" is already computed
            when "000001" | "001111" | "010001" | "011111" =>
              amplitude_v( amplitude_v'high - 5 downto amplitude_v'low ) :=
                amplitude( amplitude'high downto amplitude'low + 4 );
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000010" | "001110" | "010010" | "011110" =>
              amplitude_v( amplitude_v'high - 4 downto amplitude_v'low ) :=
                amplitude( amplitude'high downto amplitude'low + 3 ); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000011" | "001101" | "010011" | "011101" =>
              amplitude_v( amplitude_v'high - 3 downto amplitude_v'low ) :=
                amplitude( amplitude'high downto amplitude'low + 2 ); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000100" | "001100" | "010100" | "011100" =>
              amplitude_v( amplitude_v'high - 2 downto amplitude_v'low ) :=
                amplitude( amplitude'high downto amplitude'low + 1 ); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000101" | "001011" | "010101" | "011011" =>
              amplitude_v( amplitude_v'high - 1 downto amplitude_v'low ) :=
                std_logic_vector( unsigned( amplitude ) -
                                  unsigned( "00" & amplitude( amplitude'high downto amplitude'low + 2 ))); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000110" | "001010" | "010110" | "011010" =>
              amplitude_v( amplitude_v'high - 1 downto amplitude_v'low ) :=
                std_logic_vector( unsigned( amplitude ) -
                                  unsigned( "000" & amplitude( amplitude'high downto amplitude'low + 3 ))); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when "000111" | "001001" | "010111" | "011001" =>
              amplitude_v( amplitude_v'high - 1 downto amplitude_v'low ) :=
                std_logic_vector( unsigned( amplitude ) -
                                  unsigned( "0000" & amplitude( amplitude'high downto amplitude'low + 4 ))); 
              state <= std_logic_vector( Unsigned( state ) + 1 );
              length_count <= width;
            when "001000" | "011000" =>
              amplitude_v( amplitude_v'high - 1 downto amplitude_v'low ) := amplitude;
              if length_count = std_logic_vector( to_unsigned( 0, length_count'length )) then
                state <= std_logic_vector( Unsigned( state ) + 1 );
              else
                length_count <= std_logic_vector( unsigned( length_count ) - 1 );
              end if;
            when "010000" =>
              state <= std_logic_vector( Unsigned( state ) + 1 );
            when others =>
              state <= "000000";
          end case;
          if state( state'high downto state'high - 1 ) /= "00" then
            amplitude_v := std_logic_vector( - signed( amplitude_v ));
          end if;
          amplitude_s <= amplitude_v;
        end if START_PULSE_IF;
      else
        state <= "000000";
      end if RST_IF;
    end if;
  end process main_proc;
end architecture arch;
