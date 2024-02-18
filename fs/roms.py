import json
import os

programs = json.load(open('../chip8Archive/programs.json'))

for name, p in sorted(programs.items()):
    shiftQuirks = 1 if 'shiftQuirks' in p['options'] and p['options']['shiftQuirks'] else 0
    loadStoreQuirks = 1 if 'loadStoreQuirks' in p['options'] and p['options']['loadStoreQuirks'] else 0

    cmd = f"../ch8toec8 ../chip8Archive/roms/{name}.ch8 {shiftQuirks} {loadStoreQuirks}"
    print(cmd)

    os.system(cmd)
