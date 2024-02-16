# Make filesystem

fs::
	cp chip8Archive/roms/*.ch8 chip8
	python3 ./roms.py
	cp ../vendor/chip8-test-rom-with-audio/*.ch8 chip8
	echo "test_opcode,7,#FFAA00,#AA4400,#FFAA00,#000000,0,0" >> chip8.txt
	echo "chip8-test-rom-with-audio,7,#FFAA00,#AA4400,#FFAA00,#000000,0,0" >> chip8.txt

run::
	../.pio/build/native/program