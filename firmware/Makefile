build: clean
	gavrasm ./main.asm
	
clean:
	rm main.hex -f
	rm main.lst -f

sim: build
	avr_sim ./main.pro

flash: build
	avrdude -p atmega644 -c stk500v2 -P /dev/ttyACM0 -U flash:w:main.hex