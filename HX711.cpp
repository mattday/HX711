#include <Arduino.h>
#include <HX711.h>

HX711::HX711(byte dout, byte pd_sck, bool channel_a_hi_gain, bool channel_b_shift) {
	begin(dout, pd_sck, channel_a_hi_gain, channel_b_shift);
}

HX711::HX711() {
}

HX711::~HX711() {
}

void HX711::begin(byte dout, byte pd_sck, bool channel_a_hi_gain, bool channel_b_shift) {
	PD_SCK = pd_sck;
	DOUT = dout;
	HIGAIN_A = channel_a_hi_gain;
	SHIFT_B = channel_b_shift;
	SELECT_A = true;
	SELECTED_A = true;
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);
	digitalWrite(PD_SCK, LOW);
}

bool HX711::is_ready() {
	return digitalRead(DOUT) == LOW;
}

void HX711::set_gain(byte gain) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			//GAIN_A = 1;
			HIGAIN_A = 1;
			SELECT_A = true;
			break;
		case 64:		// channel A, gain factor 64
			//GAIN_A = 3;
			HIGAIN_A = 0;
			SELECT_A = true;
			break;
		case 32:		// channel B, gain factor 32
			SELECT_A = false;
			break;
	}
}

void HX711::set_channel(bool channel_a) {
	SELECT_A = channel_a;
}

long HX711::read() {
	// wait for the chip to become ready
	while (!is_ready()) {
		// Will do nothing on Arduino but prevent resets of ESP8266 (Watchdog Issue)
		yield();
	}

	union {
		unsigned long value = 0;
		uint8_t data[4];
	};

	// Pulse the clock pin 24 times to read the data
	data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// Shift values read from B to match the range of A
	if (!SELECTED_A && SHIFT_B) {
		value <<= 1;
		if (HIGAIN_A)
			value <<= 1;
	}
	
	// Set the channel and the gain factor for the next reading using the clock pin
	SELECTED_A = SELECT_A;
	digitalWrite(PD_SCK, HIGH);				// 25 pulses to select channel A with 128 gain
	digitalWrite(PD_SCK, LOW);
	if (!SELECT_A || !HIGAIN_A) {
		digitalWrite(PD_SCK, HIGH);			// 26 pulses to select channel B with 32 gain
		digitalWrite(PD_SCK, LOW);
		if (SELECT_A) {
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

	return static_cast<long>(value);
}

long HX711::read_average(byte times) {
	long sum = 0;
	// Ensure next value is from the last selected channel
	if (SELECT_A != SELECTED_A) 
		read();
	for (byte i = 0; i < times; i++) {
		sum += read();
		yield();
	}
	return sum / times;
}

long HX711::get_value(byte times) {
	if (SELECTED_A)
		return read_average(times) - OFFSET_A;
	else
		return read_average(times) - OFFSET_B;
}

float HX711::get_units(byte times) {
	if (SELECTED_A)
		return float(get_value(times)) / SCALE_A;
	else
		return float(get_value(times)) / SCALE_B;
}

void HX711::tare(byte times) {
	if (SELECT_A != SELECTED_A) 
		read();
	long offset = read_average(times);
	set_offset(offset, SELECTED_A);
}

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

void HX711::power_down() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);
}

void HX711::power_up() {
	digitalWrite(PD_SCK, LOW);
}
