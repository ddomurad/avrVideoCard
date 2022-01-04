; -----------------------------------------------------------------------------
;    .:: MCU Initialization Routine ::.
;
;   Steps:
;   1) Initialize stack
;   2) Initialize gp. registers
;   3) Initialize 16bit timer1
;   4) Initialize 8bit timer2
;   5) Initialize shift register and color
;   6) Initialize SPI
;   7) Initialize RAM
;   8) Enable interrupts
; -----------------------------------------------------------------------------

initialize:
; Initialize stack.
    ldi     REG_B0, LOW(RAMEND)
    ldi     REG_B1, HIGH(RAMEND)
    out     SPL, REG_B0
    out     SPH, REG_B1

; Initialize gp. register. Not all registers have to be initialized at reset.
    clr     LINE_CNT_REG_L
    clr     LINE_CNT_REG_H
    clr     GLB_INV_REG
    clr     SPI_CMD_REG
    
    clr     EXT_CMD_REG
    clr     EXT_CMD_STATE_REG
    ldi     EXT_CMD_WSTATE_REG_L, LOW(glyphs_ptr)
    ldi     EXT_CMD_WSTATE_REG_H, HIGH(glyphs_ptr)

    ; Clear index registers.
    clr     XL
    clr     XH
    clr     YL
    clr     YH
    clr     ZL
    clr     ZH

    ; Clear the T flag. The T flag is used for tracking SPI_CMD state. 
    ; low => read cmd, and set T high.
    ; high => read data, exec cmd, set T lwo.
    ; This could be easly refactored to use a gp. register instead.
    clt

; Initialize 16bit timer1.
    ; This timer is used to generate the H-Sync pulse, and trigger an interrupt
    ; for every scan line.
    ; Timer1 setup: 
    ;   Fast PWM
    ;   TOP: OCR1A = TICKS_PER_SCANLINE
    ;   OCR1B = TICKS_PER_SCANLINE - TICKS_PER_HSYNC_PULSE
    ;   CLK prescaller: None
    ;   Clear OCnA/OCnB on Compare Match, set OCnA/OCnB at BOTTOM (non-inverting mode)
    ;   Enable TIMER1_COMPB interrupt
    
    ldi     REG_B0, (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) |(1<<WGM10)
    sts     TCCR1A, REG_B0
    
    ldi     REG_B1, (1<<WGM13) | (1<<WGM12) | (1<<CS10)
    sts     TCCR1B, REG_B1

    ldi     REG_B0, LOW(TICKS_PER_SCANLINE)
    ldi     REG_B1, HIGH(TICKS_PER_SCANLINE)
    sts     OCR1AH, REG_B1
    sts     OCR1AL, REG_B0

    ldi     REG_B0, LOW(TICKS_PER_SCANLINE - TICKS_PER_HSYNC_PULSE)
    ldi     REG_B1, HIGH(TICKS_PER_SCANLINE - TICKS_PER_HSYNC_PULSE)
    sts     OCR1BH, REG_B1
    sts     OCR1BL, REG_B0

    ldi     REG_B0, (1<<OCIE1B)
    sts     TIMSK1, REG_B0
    
    ; Setup timer1 output pin B
    sbi     HSYNC_DDR, HSYNC_PIN
    sbi     VSYNC_DDR, VSYNC_PIN
    sbi     VSYNC_PORT, VSYNC_PIN

; Initialize 8bit timer2
    ; This timer is used to generate a Load pulse for the shift register.
    ; This way we dont have to generate this pulse our selves,
    ; gaining 2 extra cycles for the color inv. feature. 
    ; Timer2 setp:
    ;   Fast PWM
    ;   TOP: OCR1A = 7
    ;   OCR1B = 6
    ;   CLK prescaller: None
    ;   Clear OCnA/OCnB on Compare Match, set OCnA/OCnB at BOTTOM (non-inverting mode)
    ldi     REG_B0, (1<<COM2B1) | (0<<COM2B0) | (1<<WGM21) |(1<<WGM20)
    sts     TCCR2A, REG_B0
    
    ldi     REG_B1, (0<<CS20) | (1<<WGM22)
    sts     TCCR2B, REG_B1
    
    ldi     REG_B0, 7
    sts     OCR2A, REG_B0

    ldi     REG_B1, 6
    sts     OCR2B, REG_B1
    

; Initialize shift register and color
    ldi     REG_B0, 0xff
    out     VDATA_DDR, REG_B0
    sbi     VDATA_LOAD_DDR, VDATA_LOAD_PIN
    
    ldi     REG_B0, 0xff
    sbi     COLOR_DDR, COLOR_RED_PIN
    sbi     COLOR_DDR, COLOR_GREEN_PIN
    sbi     COLOR_DDR, COLOR_BLUE_PIN

    sbi     COLOR_PORT, COLOR_RED_PIN
    sbi     COLOR_PORT, COLOR_GREEN_PIN
    sbi     COLOR_PORT, COLOR_BLUE_PIN
    
; Initialize SPI
    ; SPI setup:
    ;   SPI: Enabled
    ;   Interrupts: Disabled
    ;   Order: MSB
    ;   Mode: Slave
    ;   Clk Polarity: Leading Edge = Rising
    ;   Clk Phase: Leading Edge = Setup
    ldi     REG_B1, (0<<SPIE) | (1 << SPE) | (0 << DORD) | (0 << MSTR) | (0 << CPOL) | (1 << CPHA)
    out     SPCR, REG_B1

    ; Initialize SPI pins.
    cbi     SPI_DDR, SPI_CLK_PIN
    cbi     SPI_DDR, SPI_SS_PIN
    cbi     SPI_DDR, SPI_MOSI_PIN
    sbi     SPI_DDR, SPI_MISO_PIN

; Initialize RAM
    ; Initialize default glyphs set
    ldi     REG_B0, HIGH(glyphs*2)
    ldi     XL, LOW(glyphs_set_ptr)
    ldi     XH, HIGH(glyphs_set_ptr)
    st      X, REG_B0

    ; Set X index to glyphs_ptr
    ldi     XL, LOW(glyphs_ptr)
    ldi     XH, HIGH(glyphs_ptr)
    ; Load end of glyphs + inv data
    ldi     REG_B0, LOW(glyphs_ptr + 4060)
    ldi     REG_B1, HIGH(glyphs_ptr + 4060)

    clr     REG_A0
_ram_init_loop:             ; do {
    st      X+, REG_A0      ; glyphs_ptr[X++] = reg_a0;
    inc     REG_A0          ; reg_a0++;
    cp      XL, REG_B0      ; 
    cpc     XH, REG_B1      ; 
    brne    _ram_init_loop  ; } while(X != REG_B);

    ; Set text color to white
    ldi     REG_A0, 0x07
    sts     displ_color_ptr, REG_A0
    
    ; Initialize extended comamnd
    ; Set: X=0, Y=0, W=62, H=58, GLYPH=0
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)

    ldi     REG_A0, 0
    st      X+, REG_A0
    st      X+, REG_A0

    ldi     REG_A0, 62
    st      X+, REG_A0

    ldi     REG_A0, 58
    st      X+, REG_A0

    ldi     REG_A0, 0
    st      X+, REG_A0

; Enable interrupts
    sei