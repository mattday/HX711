// Implementation of HX711 template class

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B> *HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance = 0;

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::HX711() :
	_selectChannelA(true), _selectedChannelA(true), _alternateChannels(false)
{
	HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance = this;	// For interrupt callback
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::begin(bool alternateChannels)
{
	_alternateChannels = alternateChannels;
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
/*
template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711::set_gain(byte gain) {
	switch (gain) {
		case 128:		// Channel A, gain factor 128
			HIGAIN_A = 1;
			SELECT_A = true;
			break;
		case 64:		// Channel A, gain factor 64
			HIGAIN_A = 0;
			SELECT_A = true;
			break;
		case 32:		// Channel B, gain factor 32
			SELECT_A = false;
			break;
	}
}

void HX711::set_channel(bool channel_a) {
	SELECT_A = channel_a;
}

void HX711::set_read_handler(ReadCallback &callback, bool channel_a) {
	if (channel_a)
		CALLBACK_A = callback;
	else
		CALLBACK_B = callback;
}
*/


template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::InterruptHandler()
{
	long value;
	bool channelA;
	HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->read(value, channelA);
	if (channelA)
	{
		if (HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->_callbackChannelA != 0)
			HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->_callbackChannelA(value);
	}
	else 
	{
		if (HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->_callbackChannelB != 0)
			HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::pInstance->_callbackChannelB(value);
	}
}
		
template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::setReadHandlerA(ReadCallback &callback) 
{
	_callbackChannelA = callback;
	attachInterrupt(digitalPinToInterrupt(DOUT), HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::InterruptHandler, FALLING);
// TODO
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::setReadHandlerB(ReadCallback &callback)
{
	_callbackChannelB = callback;
	// TODO
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

	/*
	union {
		unsigned long value = 0;
		uint8_t data[4];
	};
	*/
	
	// Initialise reading
	value = 0;
	uint8_t *data = reinterpret_cast<uint8_t *>(&value);

	// Pulse the clock pin 24 times to read the data
	data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// Shift values read from B to match the range of A
	if (!_selectedChannelA && SHIFT_B) {
		value <<= 1;
		if (HIGAIN_A)
			value <<= 1;
	}
	
	// Store the channel to which the value applies
	channelA = _selectedChannelA;
	// Change channels if reads are alternating
	if (_alternateChannels)
		_selectChannelA = !_selectedChannelA;
	// Set the channel and the gain factor for the next reading using the clock pin
	_selectedChannelA = _selectChannelA;
	digitalWrite(PD_SCK, HIGH);				// 25 pulses to select channel A with 128 gain
	digitalWrite(PD_SCK, LOW);
	if (!_selectChannelA || !HIGAIN_A) {
		digitalWrite(PD_SCK, HIGH);			// 26 pulses to select channel B with 32 gain
		digitalWrite(PD_SCK, LOW);
		if (_selectChannelA) {
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

/*
template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
long HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::readAverage(byte times) 
{
	long sum = 0;
	long value;
	bool channelA;
	// Ensure next value is from the last selected channel
	if (_selectChannelA != _selectedChannelA) 
		read(value, channelA);
	for (byte i = 0; i < times; i++) {
		sum += read();
		yield();
	}
	return sum / times;
}

/*
long HX711::get_value(byte times) {
	if (_selectedChannelA)
		return read_average(times) - OFFSET_A;
	else
		return read_average(times) - OFFSET_B;
}

float HX711::get_units(byte times) {
	if (_selectedChannelA)
		return float(get_value(times)) / SCALE_A;
	else
		return float(get_value(times)) / SCALE_B;
}

void HX711::tare(byte times) {
	if (SELECT_A != _selectedChannelA) 
		read();
	long offset = read_average(times);
	set_offset(offset, _selectedChannelA);
}
/*
void HX711::set_scale(float scale, bool channel_a) {
	if (channel_a)
		SCALE_A = scale;
	else
		SCALE_B = scale;
}

float HX711::get_scale(bool channel_a) {
	if (channel_a)
		return SCALE_A;
	else
		return SCALE_B;
}

void HX711::set_offset(long offset, bool channel_a) {
	if (channel_a)
		OFFSET_A = offset;
	else
		OFFSET_B = offset;
}

long HX711::get_offset(bool channel_a) {
	if (channel_a)
		return OFFSET_A;
	else
		return OFFSET_B;
}
*/
template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::powerDown() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);
}

template <byte PD_SCK, byte DOUT, bool HIGAIN_A, bool SHIFT_B>
void HX711<PD_SCK, DOUT, HIGAIN_A, SHIFT_B>::powerUp() {
	digitalWrite(PD_SCK, LOW);
}
