// Library to interface with Avia Semiconductor HX711 24-bit ADC for weigh scales. 
//
// Originally forked from https://github.com/bogde/HX711 but no longer compatible.
//
// A lot of the changes just suited the particular project I was working on, and I make no apologies for the 
// resulting library not being more general. Please select whichever library is more appropriate for your 
// application.
//
// Main differences/features are:
//
//   a) template-based to allow some build-time optimisations and for better implementation of (b)
//   b) provides a callback mechanism when a new value has been read from the device. bogde's library 
// 	    supported only polling the device to see if a value was ready for reading. This is not a useful operating 
// 		mode in many applications, particulary given that the HX711 conversion rate is relatively slow.
//   c) no built-in support for any scaling/offet of readings, but this is trivial to implement externally.
//   d) different approach to channels: less ambiguous which channel a reading is from, readings can automatically
//      alternate between channels and channel B readings can automatically shift to channel A range.
//   e) multiple HX711 devices supported.

#ifndef HX711_h
#define HX711_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Note: We are initially using external interrupts limiting ourselves to only two pins on 
// the 328p. However, we can use level change interrupt to allow more pins to be used. 
// We will need to use a Pin Change library and make a different call to attach the interrupt 
// after checking the pin does not support external interrupt (digitalPinToInterrupt returns 
// NOT_AN_INTERRUPT)

// Note: Alternate reading between channels makes the HX711 very slow. The datasheet specifies
// settling time of 400ms (for low output rate) or 50ms (for high output rate) when changing gain 
// or channel. This can mean only approx one reading from each channel per second on a typical board.

// Template parameters define clock and data pin, whether channel A uses highest gain (128)
// and whether channel B readings will be shifted to match channel A range.
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
