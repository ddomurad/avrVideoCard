; ------------------------------------------------------------------------------
;   .:: REGISTERS ::.
; ------------------------------------------------------------------------------

;                                   = r0
;                                   = r1

; GLYPH_INV_REG_[0-7] are temp. registers, used during glyph rendering.
    .def    GLYPH_INV_REG_0         = r2    
    .def    GLYPH_INV_REG_1         = r3
    .def    GLYPH_INV_REG_2         = r4
    .def    GLYPH_INV_REG_3         = r5
    .def    GLYPH_INV_REG_4         = r6
    .def    GLYPH_INV_REG_5         = r7
    .def    GLYPH_INV_REG_6         = r8
    .def    GLYPH_INV_REG_7         = r9

; LINE_CNT_REG_[L,H] keeps track of the current scan line.
    .def    LINE_CNT_REG_L          = r10
    .def    LINE_CNT_REG_H          = r11
;.                                  = r12

; GLB_INV_REG used to invert all the glyps. In practice when GLB_INV_REG
; is set, GLYPH_INV_REG_[0-7] will be inversed before line is rendered.
    .def    GLB_INV_REG             = r13

; SPI_CMD_REG - buffer for the SPI command
    .def    SPI_CMD_REG             = r14

; REG_[A,B,C,D][0,1] are tmp. working registers.
; Used for logic and calculations.
    .def    REG_D0                  = r15
    .def    REG_B0                  = r16
    .def    REG_B1                  = r17
    .def    REG_A0                  = r18
    .def    REG_A1                  = r19
    .def    REG_C0                  = r20
    .def    REG_C1                  = r21

; EXT_CMD_* registers are used for executing extended commands. 
; Extended commands usually run for several frames, 
; a set of designated registers ensures that the state is not altered 
; by other part of the code.
    .def    EXT_CMD_REG             = r22
    .def    EXT_CMD_STATE_REG       = r23
    .def    EXT_CMD_WSTATE_REG_L    = r24
    .def    EXT_CMD_WSTATE_REG_H    = r25


; ------------------------------------------------------------------------------
;   .:: DDR, PORT and PIN CONSTANTS ::.
; ------------------------------------------------------------------------------

; VGA horizontal sync pulse pin.
; We don't need to define the PORT as it wont be used.
; Horizontal sync pulse is generated by the timer1.
    .equ    HSYNC_DDR               = DDRD
    .equ    HSYNC_PIN               = PIND4

; VGA vertical sync pulse pin.
; Here we define the port, as we will generate the signal ourselves.
    .equ    VSYNC_DDR               = DDRD
    .equ    VSYNC_PORT              = PORTD 
    .equ    VSYNC_PIN               = PIND5

; Pin connected to the shift registers load pin.
    .equ    VDATA_LOAD_DDR          = DDRD
    .equ    VDATA_LOAD_PIN          = PIND6

; A whole port is used to set the pixel value in the shift register.
    .equ    VDATA_DDR               = DDRA
    .equ    VDATA_PORT              = PORTA

; Pin setup for the color control circuit.
    .equ    COLOR_PORT              = PORTC
    .equ    COLOR_DDR               = DDRC

    .equ    COLOR_RED_PIN           = PINC0
    .equ    COLOR_GREEN_PIN         = PINC1
    .equ    COLOR_BLUE_PIN          = PINC7

; Pin setup for SPI    
    .equ    SPI_DDR                 = DDRB
    .equ    SPI_PORT                = PORTB
    .equ    SPI_CLK_PIN             = PINB7
    .equ    SPI_SS_PIN              = PINB4
    .equ    SPI_MOSI_PIN            = PINB5
    .equ    SPI_MISO_PIN            = PINB6

; ------------------------------------------------------------------------------
;   .:: Value CONSTANTS ::.
; ------------------------------------------------------------------------------

; This video card uses only the industry standard 640x480@60Hz VGA mode
; Because the MCU uses a 20MHz crystal one scan line takes ~635 ticks
; (insetad of the 800 ticks @25.175MHz)
    .equ    TICKS_PER_SCANLINE      = 635   ; = (20/25.175)*800
    .equ    TICKS_PER_HSYNC_PULSE   = 76    ; = (20/25.175)*96