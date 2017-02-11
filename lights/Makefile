
ARDUINO=arduino

BOARD=arduino:avr:nano:cpu=atmega328
PORT=/dev/ttyUSB0
OPTIONS=--board $(BOARD) --port $(PORT)

verify:
	$(ARDUINO) $(OPTIONS) --verify .

upload:
	$(ARDUINO) $(OPTIONS) --upload .
