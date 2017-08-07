#ifndef HX711_h
#define HX711_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class HX711
{
	private:
		byte PD_SCK;	// Power Down and Serial Clock Input Pin
		byte DOUT;		// Serial Data Output Pin
		bool HIGAIN_A;	// maximum amplification factor (128) for channel A
		bool SHIFT_B;	// shift readings from channel B to match channel A scale
		bool SELECT_A;		// channel A will be selected (for next read)
		bool SELECTED_A;	// channel A was selected
		long OFFSET_A = 0;	// used for zero adjustment / tare weight
		long OFFSET_B = 0;	// used for zero adjustment / tare weight
		float SCALE_A = 1;	// used to return weight in grams, kg, ounces, whatever
		float SCALE_B = 1;	// used to return weight in grams, kg, ounces, whatever

	public:
		// define clock and data pin, gain factor and whether channel B readings will be shifted to match channel A.
		// Channel A can have gain of 128 or 64 (channel B is fixed at 32). Initially channel A is selected.
		
		HX711(byte dout, byte pd_sck, bool channel_a_hi_gain = true, bool channel_b_shift = true);

		HX711();

		virtual ~HX711();

		// Allows to set the pins and gain later than in the constructor
		void begin(byte dout, byte pd_sck, bool channel_a_hi_gain = true, bool channel_b_shift = true);

		// check if HX711 is ready
		// from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
		// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
		bool is_ready();

		// set the gain factor; takes effect only after a call to read();
		// this method is now deprecated and included for backward compatability only;
		// channel A gain is set either by the constructor or begin method.;
		// channel selection is achieved by the set_channel method.
		void set_gain(byte gain = 128);

		// set the channel from which to read; takes effect only after a call to read();
		// if argument is true, first channel will be read, if false, second channel.
		void set_channel(bool channel_a);
		
		// waits for the chip to be ready and returns a reading
		long read();

		// returns an average reading; times = how many times to read
		// operates on last selected channel
		long read_average(byte times = 10);

		// returns (read_average() - OFFSET), that is the current value without the tare weight; times = how many readings to do
		// operates on last selected channel
		long get_value(byte times = 1);

		// returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration
		// times = how many readings to do
		// operates on last selected channel
		float get_units(byte times = 1);

		// set the OFFSET value for tare weight; times = how many times to read the tare value
		// operates on last selected channel
		void tare(byte times = 10);

		// set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
		void set_scale(float scale = 1.f, bool channel_a = true);

		// get the current SCALE
		float get_scale(bool channel_a = true);

		// set OFFSET, the value that's subtracted from the actual reading (tare weight)
		void set_offset(long offset = 0, bool channel_a = true);

		// get the current OFFSET
		long get_offset(bool channel_a = true);

		// puts the chip into power down mode
		void power_down();

		// wakes up the chip after power down mode
		void power_up();
};

#endif /* HX711_h */
