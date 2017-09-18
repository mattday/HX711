HX711
=====

An Arduino library to interface with Avia Semiconductor HX711 24-bit ADC for weigh scales. 

Originally forked from https://github.com/bogde/HX711 but no longer compatible.

A lot of the changes just suited the particular project I was working on, and I make no apologies for the resulting library not being more general. Please select whichever library is more appropriate for your application.

Main differences/features are:

1. Template-based to allow some build-time optimisations and for better implementation of callbacks
2. Provides a callback mechanism when a new value has been read from the device. bogde's library supported only polling the device to see if a value was ready for reading. In many applications this is not a useful mode of operation, particulary given that the HX711 conversion rate is relatively slow.
3. No built-in support for any scaling/offet of readings, but this is trivial to implement externally.
4. Different approach to channels: less ambiguous which channel a reading is from, readings can automatically alternate between channels and channel B readings can automatically shift to channel A range.
5. Multiple HX711 devices supported.