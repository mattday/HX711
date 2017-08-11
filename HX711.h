#ifndef HX711_h
#define HX711_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// TODO: We are initially using external interrupts limiting ourselves to only two pins on 
// the 328p. However, we can use level change interrupt to allow more pins to be used. 
// We will need to use a Pin Change library and make a different call to attach the interrupt 
// after checking the pin does not support external interrupt (digitalPinToInterrupt returns 
// NOT_AN_INTERRUPT)

// Template parameters define clock and data pin, whether channel A uses highest gain (128)
// and whether channel B readings will be shifted to match channel A.
template <byte PD_SCK, byte DOUT, bool HIGAIN_A = true, bool SHIFT_B = true>
class HX711
{	
		
	public:

		typedef void (*ReadCallback)(long value, bool channelA);
	
		HX711();

		virtual ~HX711();

		// Initialise the HX711.
		// If alternateChannels is true, readings will be taken alternately from channel A and channel B.
		void begin(bool alternateChannels = false);
		
		// Check if HX711 is ready.
		// From the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
		// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.
		bool isReady();

		// Set the channel from which to read after the next value has been read.
		void setChannelA(bool channelA);
		
		// Set a handler to process value when it becomes available
		// Handler is called by an interrupt, so usual precautions for ISRs apply.
		void setReadHandler(ReadCallback callback);
		
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
		
		bool m_selectChannelA;			// Channel A will be selected (for next read)
		bool m_selectedChannelA;		// Channel A was selected
		bool m_alternateChannels;		// Readings will alternate between channels
		ReadCallback m_readCallback; 	// Function to call when new value is available
		
		// Retrieve reading from chip without first checking it is ready
		void retrieveReading(long &value, bool &channelA);
		
		// Pointer to instance of the HX711 class for the given pin configuration
		static HX711 *pInstance;
		
		// Interrupt service routine
		static void InterruptHandler();
		

		
};

#include "HX711_impl.h"
#endif /* HX711_h */
