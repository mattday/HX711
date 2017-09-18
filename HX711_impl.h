// Implementation of HX711 template class. See HX711.h for details of interface.

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B> *HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance = 0;

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::HX711() :
	m_selectChannelA(true), m_selectedChannelA(true), m_alternateChannels(false)
{
	HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance = this;	// For interrupt callback
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::begin(bool alternateChannels)
{
	m_alternateChannels = alternateChannels;
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);
	digitalWrite(PD_SCK, LOW);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::~HX711() 
{
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
bool HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::isReady() 
{
	return digitalRead(DOUT) == LOW;
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
static void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::InterruptHandler()
{
	// Check a reading is ready for retrieval
	if (digitalRead(DOUT) != LOW)
		return;
	long value;
	bool channelA;
	// Retrieve reading and pass it to the callback
	HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->retrieveReading(value, channelA);
	HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->m_readCallback(value, channelA);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::setReadHandler(ReadCallback callback) 
{
	m_readCallback = callback;
	attachInterrupt(digitalPinToInterrupt(DOUT), HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::InterruptHandler, LOW);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
long HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::read()
{
	long value;
	bool channelA;
	read(value, channelA);
	return value;
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::read(long &value, bool &channelA) 
{
	// Wait for the chip to become ready
	while (!isReady()) {
		// Will do nothing on Arduino but prevent resets of ESP8266 (Watchdog Issue)
		yield();
	}
	
	// Retrieve value from chip
	retrieveReading(value, channelA);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::retrieveReading(long &value, bool &channelA) 
{
	// Initialise reading
	value = 0;
	uint8_t *data = reinterpret_cast<uint8_t *>(&value);

	// Pulse the clock pin 24 times to read the data
	data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// Shift values read from B to match the range of A
	if (!m_selectedChannelA && SHIFT_B) {
		value <<= 1;
		if (HIGAIN_A)
			value <<= 1;
	}
	
	// Store the channel to which the value applies
	channelA = m_selectedChannelA;
	// Change channels if reads are alternating
	if (m_alternateChannels)
		m_selectChannelA = !m_selectedChannelA;
	// Set the channel and the gain factor for the next reading using the clock pin
	m_selectedChannelA = m_selectChannelA;
	digitalWrite(PD_SCK, HIGH);				// 25 pulses to select channel A with 128 gain	
	digitalWrite(PD_SCK, LOW);
	if (!m_selectChannelA || !HIGAIN_A) {
		digitalWrite(PD_SCK, HIGH);			// 26 pulses to select channel B with 32 gain
		digitalWrite(PD_SCK, LOW);
		if (m_selectChannelA) {
			digitalWrite(PD_SCK, HIGH);		// 27 pulses to select channel A with 64 gain
			digitalWrite(PD_SCK, LOW);	
		}
	}
	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		data[3] = 0xFF;
	} else {
		data[3] = 0x00;
	}
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void  HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::setChannelA(bool channelA)
{
	m_selectChannelA = channelA;
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::powerDown() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::powerUp() {
	digitalWrite(PD_SCK, LOW);
}


