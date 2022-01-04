;-------------------------------------------------------------------------------
;   .:: ATmega644 glyph video card ::.
;-------------------------------------------------------------------------------

.device ATmega644

; Constants and registers definition.
.include "defs.asm"

.org 0x0000
    rjmp    initialize
.org 0x001C ;TIMER1_COMPB int. Triggered for each scan line.
    rjmp    timer1b_interrupt

; Initialization routine.
.include "init.asm"

; Infinite loop. All logic happens during timer1b interrupts.
loop:    
    rjmp loop

; Rendering and cmd logic
.include "renderer.asm"

; Glyphs data
.include "data.asm"