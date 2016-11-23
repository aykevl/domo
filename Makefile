

IPADDRESS := $(shell getent hosts wakeup.local | cut -d ' ' -f 1)
OPTIONS := --board esp8266:esp8266:nodemcuv2 --pref serial.port=$(IPADDRESS) --pref serial.port.file=$(IPADDRESS)

verify:
	arduino $(OPTIONS) --verify .

upload:
	arduino $(OPTIONS) --upload .
