# Make filesystem

AT=../../vendor/chip8-test-rom-with-audio

fs::
	(cd ec8; python3 ../roms.py)
	(cd ec8; ../ch8toec8 $(AT)/test_opcode.ch8 0 0)
	(cd ec8; ../ch8toec8 $(AT)/chip8-test-rom-with-audio.ch8 0 0)

run::
	../.pio/build/native/program

ch8toec8: ch8toec8.c
	$(CC) -o ch8toec8 ch8toec8.c
