import json

programs = json.load(open('chip8Archive/programs.json'))

with open('chip8.txt', 'w') as f:
    for name, p in sorted(programs.items()):
        fillColor = p['options']['fillColor'] if 'fillColor' in p['options'] else '#FFAA00'
        backgroundColor = p['options']['backgroundColor'] if 'backgroundColor' in p['options'] else '#AA4400'
        buzzColor = p['options']['buzzColor'] if 'buzzColor' in p['options'] else '#FFAA00'
        quietColor = p['options']['quietColor'] if 'quietColor' in p['options'] else '#000000'
        shiftQuirks = 1 if 'shiftQuirks' in p['options'] and p['options']['shiftQuirks'] else 0
        loadStoreQuirks = 1 if 'loadStoreQuirks' in p['options'] and p['options']['loadStoreQuirks'] else 0

        f.write(f"{name},{p['options']['tickrate']},{fillColor},{backgroundColor},{buzzColor},{quietColor},{shiftQuirks},{loadStoreQuirks}\n")
