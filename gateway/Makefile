

IPADDRESS := $(shell getent hosts gateway.local | cut -d ' ' -f 1)
OPTIONS := --board esp8266:esp8266:d1_mini --pref serial.port=$(IPADDRESS) --pref serial.port.file=$(IPADDRESS)

verify:
	arduino $(OPTIONS) --verify .

upload:
	arduino $(OPTIONS) --upload .
