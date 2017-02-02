
ARDUINO=/home/ayke/lib/arduino-1.7.7-linux64/arduino

BOARD=arduino:avr:nano:cpu=atmega328
OPTIONS=--board $(BOARD) --port /dev/ttyUSB0

verify:
	$(ARDUINO) $(OPTIONS) --verify .

upload:
	$(ARDUINO) $(OPTIONS) --upload .
