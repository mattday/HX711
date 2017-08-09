#ifndef HX711_h
#define HX711_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


// Template parameters define clock and data pin, whether channel A uses highest gain (128)
// and whether channel B readings will be shifted to match channel A.
template <byte PD_SCK, byte DOUT, bool HIGAIN_A = true, bool SHIFT_B = true>
class HX711
{	

//	private:
//		bool SELECT_A;		// channel A will be selected (for next read)
//		bool SELECTED_A;	// channel A was selected
//		long OFFSET_A = 0;	// used for zero adjustment / tare weight
//		long OFFSET_B = 0;	// used for zero adjustment / tare weight
//		float SCALE_A = 1;	// used to return weight in grams, kg, ounces, whatever
//		float SCALE_B = 1;	// used to return weight in grams, kg, ounces, whatever
//		ReadCallback CALLBACK_A; // function to call when new value read from channel A
//		ReadCallback CALLBACK_B; // function to call when new value read from channel B
		
	public:

		typedef void (*ReadCallback)(long);
	
		HX711();

		virtual ~HX711();

		// Initialise the HX711.
		// If alternateChannels is true, readings will be taken alternately from channel A and channel B.
		void begin(bool alternateChannels = false);
		
		// Check if HX711 is ready.
		// From the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
		// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
		bool isReady();

		// Set the gain factor; takes effect only after a call to read();
		// this method is now deprecated and included for backward compatability only;
		// channel A gain is set either by the constructor or begin method;
		// channel selection is achieved by the set_channel method.
//		void set_gain(byte gain = 128);

		// Set the channel from which to read; takes effect only after a call to read();
		// if argument is true, first channel will be read, if false, second channel.
//		void set_channel(bool channel_a);
		
		// Set a handler to process value when it becomes available.
		// Setting a handler for channel B will cause values to be read alternately from channel A and channel B.
		// Handler is called by an interrupt, so usual precautions for ISRs apply.
//		void set_read_handler(ReadCallback &callback, bool channel_a = true);
		
		// Set a handler to process value when it becomes available from channel A
		// Handler is called by an interrupt, so usual precautions for ISRs apply.
		void setReadHandlerA(ReadCallback &callback);
		
		// Set a handler to process value when it becomes available from channel B;
		// Setting a handler for channel B will cause values to be read alternately from channel A and channel B.
		// Handler is called by an interrupt, so usual precautions for ISRs apply.
		void setReadHandlerB(ReadCallback &callback);
		
		// Waits for the chip to be ready and returns a reading
		long read();
		
		// Waits for a reading to become available and stores it with associated channel flag
		void read(long &value, bool &channelA);

		// Returns an average reading; times = how many times to read.
		// Operates on last selected channel
//		long readAverage(byte times = 10);

		// Returns (read_average() - OFFSET), that is the current value without the tare weight; times = how many readings to do;
		// operates on last selected channel
//		long get_value(byte times = 1);

		// Returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration;
		// times = how many readings to do;
		// operates on last selected channel
//		float get_units(byte times = 1);

		// Set the OFFSET value for tare weight; times = how many times to read the tare value;
		// operates on last selected channel
//		void tare(byte times = 10);

		// Set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
//		void set_scale(float scale = 1.f, bool channel_a = true);

		// Get the current SCALE
//		float get_scale(bool channel_a = true);

		// Set OFFSET, the value that's subtracted from the actual reading (tare weight)
//		void set_offset(long offset = 0, bool channel_a = true);

		// Get the current OFFSET
//		long get_offset(bool channel_a = true);

		// Puts the chip into power down mode
		void powerDown();

		// Wakes up the chip after power down mode
		void powerUp();
		
	private:
		
		bool _selectChannelA;			// Channel A will be selected (for next read)
		bool _selectedChannelA;			// Channel A was selected
		bool _alternateChannels;		// Readings will alternate between channels
		ReadCallback _callbackChannelA; 	// Function to call when new value read from channel A
		ReadCallback _callbackChannelB; 	// Function to call when new value read from channel B
		
		// Interrupt service routine
		static void InterruptHandler();
		
		// Pointer to instance of the HX711 class for the given pin configuration
		static HX711 *pInstance;
		
};

#include "HX711_impl.h"
#endif /* HX711_h */
