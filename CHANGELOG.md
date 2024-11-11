# Changes

## 2024-02-09

* First version: Space Invaders as static "C" array works on "native"
* Scaling (scale=4) while rendering. A better approach would be to use a **sprite**

## 2024-02-10

* First attempt to run on actual hardware

## 2024-02-17

How to proceed? I have a disassembler and memory editor on the board. Changing the program name, however, requires to implement a new keyboard with letters and numbers. 

New plan: 

* Load/save a new file format with options and the binary. GIF cartridges contain the source code. This would require to compile on the board. Compiling would better be handled in the browser
* Implement a webserver on the board
* Implement (part of) the Octo web app to edit (and, maybe, debug) programs. This way, I could even offer Octo source code and compilation from Octo to CHIP-8.

## 2024-02-18

Sadly, I could not get neither of my Nunchuks to work.

A small web server allows to edit the program name and disassembled code. Saving is W.I.P.

## 2024-11-11

Moved the SDcard check below the display init.