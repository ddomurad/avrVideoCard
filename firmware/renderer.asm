;-------------------------------------------------------------------------------
;   .:: VIDEO RENNDERING, SPI HANDLING, CMD EXECUTION ::.
;-------------------------------------------------------------------------------


; MACROS
;-------------------------------------------------------------------------------
; FIX_JITTER 
; Used: top of timer1b_interrupt routine
;
; This macro will add one extra cycle to the timer1b_interrupt routine, 
; when TCNT1L bit0 is set. The jitter is caused by the rjmp in the 
; mian.asm infinite loop. Sometimes timer1b_interrupt will be triggered during 
; the 2 cycle jump - this will delay the timer1b_interrupt execution by one cycle.
.macro fix_jitter
    nop
    lds     REG_A0, TCNT1L         
    sbic    REG_A0, 0
    rjmp    _fix_jitter_jmp
_fix_jitter_jmp:
.endmacro

;-------------------------------------------------------------------------------
; DELAY_VSYNC_PULSE
; Used: render_porch, render_sync_pulse
; Ticks: (REG_A0=9)*3 + 3 -1 = 29
; Delays execution of routine by fixed ammount of cycles. 
; Used to align execution tiems in  render_porch, render_sync_pulse
.macro delay_vsync_pulse
    ldi     REG_A0, 9           ; REG_A0 = 9;
_render_wait_loop:              ; do {
    subi     REG_A0, 0x01       ;   REG_A0--;
    brne    _render_wait_loop   ; } while(REG_A0 != 0);
    nop
    nop
.endmacro 

;-------------------------------------------------------------------------------
; ENABLE_TIMER2
; Used: render_video_line
;
; Will enable timer2, that will automaticaly load data into the shift register
.macro enable_timer2
    nop     ; todo reuse nops!
    nop
    ldi     REG_B1, 0x00
    sts     TCNT2, REG_B1
    ldi     REG_B1, (1<<CS20) | (1<<WGM22)
    ; ldi     ZL, TCCR2B
    ; st      Z, REG_B1
    nop     ;todo reuse nop!
    sts     TCCR2B, REG_B1
.endmacro

;-------------------------------------------------------------------------------
; DISABLE_TIMER2
; Used: render_video_line
;
; Will disable timer2
.macro disable_timer2
    ldi    REG_B1, (0<<CS20) | (1<<WGM22)    
    nop     ;todo  reuse nops!
    nop
    sts    TCCR2B, REG_B1
.endmacro

;-------------------------------------------------------------------------------
; RENDER_GLYPH
; Used: render_video_line
; Ticks: 8
;
; 1. Loads glyph index from RAM
; 2. loads glyph line from FLASH
; 3. If inv bit set, inverts loaded glyph data
; 4. Ouput 8 bits of glyph data into the VDATA_PORT
.macro render_glyph
    ld      ZL, X+                  ; ZL = RAM[X++];
    lpm     REG_B0,  Z              ; REG_B0 = FLASH[Z];
    sbrc    @0,  @1                 ; if (GLYPH_INV_REG_@0 & (1<<@1)) {
    com     REG_B0                  ; REG_B0 ^= REG_B0; }
    out     VDATA_PORT, REG_B0      ; VDATA_PORT = REG_B0;
.endmacro

; READ AND HANDLE SPI COMMANDS
; Used by: render_video_line, exec_cmd_read_spi, handle_last_line
;
; (This macro could be refactored to a subroutine.)
; This code will check if SPI data is avaiable
; If not then we are done. Just wait a fixed amount of nops.
; If data avaiable read it and handle it.
.macro read_spi
    ; check is SPI data is avaiable
    in      REG_A0, SPSR            ;
    sbrs    REG_A0, SPIF            ;
    rjmp    _read_spi_no_data       ; if(! (SPSR & 1<<SPSR)) {goto _read_spi_no_data;}
    
    ; read SPI data
    in      REG_A0, SPDR            ; REG_A0 = SPDR; 
    brts    _read_spi_state_1       ;
_read_spi_state_0:                  ; if(T_flag == false) {
    ; responde with same byte as received
    out     SPDR, REG_A0            ; SPDR = REG_A0;
    ; store the data as SPI command
    mov     SPI_CMD_REG, REG_A0     ; SPI_CMD_REG = REG_A0
    ; set the T flag to true. Next recived byte will be handled as data.
    set                             ; T_flag = true;
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop                         
    nop
    nop
    nop                         
    nop
    nop    
    rjmp    _read_spi_end           ;}  // if(T_flag == false)
_read_spi_state_1:                  ;else {
    ; a  zero register is needed later
    ldi     REG_A1, 0x00            ; REG_A1 = 0x00;
    out     SPDR, REG_A0            ; responde with same byte as received
    sbrc    SPI_CMD_REG, 0          ; if(SPI_CMD_REG & 0x01) {
    mov     YL, REG_A0              ;   YL = SPDR; }
    sbrc    SPI_CMD_REG, 1          ; if(SPI_CMD_REG & 0x01) {
    mov     YH, REG_A0              ;   YH = SPDR; }
    sbrc    SPI_CMD_REG, 2          ; if(SPI_CMD_REG & 0x01) {
    st      Y+, REG_A0              ;   RAM[Y++] = SPDR;}
    sbrc    SPI_CMD_REG, 3          ; if(SPI_CMD_REG & 0x01) {
    st      Y, REG_A0               ;   RAM[Y] = SPDR;}
    sbrc    SPI_CMD_REG, 4          ; if(SPI_CMD_REG & 0x01) {
    st      -Y, REG_A0              ;  RAM[--Y] = SPDR;} 
    sbrc    SPI_CMD_REG, 5          ; if(SPI_CMD_REG & 0x01) {
    mov     EXT_CMD_REG, REG_A0     ;   EXT_CMD_REG = SPDR; }
    sbrc    SPI_CMD_REG, 6          ; if(SPI_CMD_REG & 0x01) {
    add     YL, REG_A0              ; 
    sbrc    SPI_CMD_REG, 6          ;   Y += SPDR;
    adc     YH, REG_A1              ; }
    ; if bit 7 is set in the command, this block will take one extra clock cycle 
    ; to execute. This is done to align executaion time of all commands. 
    sbrs    SPI_CMD_REG, 7          ; if(SPI_CMD_REG & 0x01) {
    rjmp    _read_spi_state_1b      ;   goto _read_spi_state_1b;}
_read_spi_state_1b:    
    ; clear T flag. Next SPI data will be handled as a command 
    clt                             ; T_flag = false;
    rjmp    _read_spi_end           ; goto _read_spi_end

_read_spi_no_data:                  ;
    ; When there is no SPI data, we will chek if the SS pin is LOW
    ; if it is we will clear the T flag. This way the master can interrupt 
    ; the comunication, and reset the protocol state. 
    sbic    SPI_PORT, SPI_SS_PIN    ; if(!SPI_PORT & (1<<SPI_SS_PIN)) {
    clt                             ;   T_flat = false; }
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop                         
    nop                         
    nop
    nop
    nop
    nop                         
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
_read_spi_end:
.endmacro

;-------------------------------------------------------------------------------
; TIMER1B_INTERRUPT routine
; 
; Executed for every scan line, just after the h-sync pulse starts. So there 
; is some time for setup, data loading, and spi handling before the frames starts.
timer1b_interrupt:
    fix_jitter
    
    ldi     REG_A0, 0x01                ;
    ldi     REG_A1, 0x00                ; REG_A = 0x0001;
    
    add     LINE_CNT_REG_L, REG_A0      ; 
    adc     LINE_CNT_REG_H, REG_A1      ; LINE_CNT_REG += REG_A; 
    
    ldi     REG_B0, LOW(464)            ; 
    ldi     REG_B1, HIGH(464)           ; REG_B = 464;
    cp      LINE_CNT_REG_L, REG_B0      ;
    cpc     LINE_CNT_REG_H, REG_B1      ;
    brlo    render_video_line           ; if(LINE_CNT_REG < REG_B) { goto render_video_line;}

    ldi     REG_B0, LOW(482)            ; 
    ldi     REG_B1, HIGH(482)           ; REG_B = 482;
    cp      LINE_CNT_REG_L, REG_B0      ;
    cpc     LINE_CNT_REG_H, REG_B1      ;
    brlo    _render_porch_jmp           ; if(LINE_CNT_REG < REG_B) {goto _render_porch_jmp;}

    ldi     REG_B0, LOW(484)            ;
    ldi     REG_B1, HIGH(484)           ; REG_B = 484;
    cp      LINE_CNT_REG_L, REG_B0      ;
    cpc     LINE_CNT_REG_H, REG_B1      ;
    brlo    _render_sync_pulse_jmp      ; if(LINE_CNT_REG < REG_B) {goto _render_sync_pulse_jmp;}

    ldi     REG_B0, LOW(524)            ;
    ldi     REG_B1, HIGH(524)           ; REG_B = 524;
    cp      LINE_CNT_REG_L, REG_B0      ;
    cpc     LINE_CNT_REG_H, REG_B1      ;
    brlo    _render_porch_jmp           ; if(LINE_CNT_REG < REG_B) {goto _render_porch_jmp;}

    ldi     REG_B0, LOW(524)            ;
    ldi     REG_B1, HIGH(524)           ; REG_B = 524; //this is redundant
    cp      LINE_CNT_REG_L, REG_B0      ;
    cpc     LINE_CNT_REG_H, REG_B1      ; 
    breq    _handle_last_line_jmp           ; if(LINE_CNT_REG == REG_B) {goto _handle_last_line_jmp;}
reti

; brlo and breq are not able to jump that far, thus we use a proxy here.
_render_porch_jmp:
    rjmp    render_porch
_render_sync_pulse_jmp:
    rjmp    render_sync_pulse
_handle_last_line_jmp:
    rjmp    handle_last_line

;-------------------------------------------------------------------------------
; RENDERS VIDEO LINE + handle SPI
; Use by: timer1b_interrupt
render_video_line:
    read_spi
    
    nop
    nop
    nop
    
    ; CALCULATE GLYPH DATA ADDR OFFSET FOR CURRENT LINE
    ; REG_A0 stores address offset to glyphs data (FLASH mem)
    ; Each gliph is 8 lines tall, for each (0-8) glyph horizontal lines.
    ; we set an offset to the Z index. See glyph data scructure.
    ; for scan lines 0,8,16,... => REG_A0 = 0x0;
    ; for scan line 1,9,17 => REG_A0 = 0x02;
    ; for scan line 2,10,18 => REG_A0 = 0x04;
    ; etc.... 
    ; REG_A0 is shifter left in order to multiply it by 2. This can be optimized though !!!
    ; Right now, glyphs take up 2 times more data, than needed. 
    mov     REG_A0, LINE_CNT_REG_L
    andi    REG_A0, 0x07
    lsl     REG_A0                  ; REG_A0 = (LINE_CNT_REG_L & 0x07)*2;

    ; STORE CURRENT X INDEX
    mov     REG_C0, XL              ; 
    mov     REG_C1, XH              ; REG_C = X;

    ; PRELOAD ALL INV REGISTERS
    ; The invert registers are used to mark glyphs that should be inverted
    ; Invert data is stored under *inv_ptr
    ; ths block of code calculates the *inv_ptr offset based on the scan line number
    mov     REG_A1, LINE_CNT_REG_L  ;
    andi    REG_A1, 0xf8            ; REG_A1 = LINE_CNT_REG_L & 0xf8;
    
    ldi     XL, LOW(inv_ptr)        ;
    ldi     XH, HIGH(inv_ptr)       ; X = inv_ptr;
    ; apply the offset to inv_ptr
    add     XL, REG_A1              ; 
    adc     XH, LINE_CNT_REG_H      ; inv_ptr += (LINE_CNT_REG_H<<8) + REG_A1;

    ; load all inv registers for current line
    ld      GLYPH_INV_REG_0, X+     ; GLYPH_INV_REG_0 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_1, X+     ; GLYPH_INV_REG_1 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_2, X+     ; GLYPH_INV_REG_2 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_3, X+     ; GLYPH_INV_REG_3 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_4, X+     ; GLYPH_INV_REG_4 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_5, X+     ; GLYPH_INV_REG_5 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_6, X+     ; GLYPH_INV_REG_6 = RAM[inv_ptr++];
    ld      GLYPH_INV_REG_7, X+     ; GLYPH_INV_REG_7 = RAM[inv_ptr++];
    
    ; APPLY GLOBAL INVERSE
    ; when GLB_INV_REG bit 0 is set, all inv registers are inversed. 
    sbrs    GLB_INV_REG, 0                  ;
    rjmp    _render_video_line_skip_invert  ; if(GLB_INV_REG & 0x01) {
    com     GLYPH_INV_REG_0                 ; GLYPH_INV_REG_0 ^= GLYPH_INV_REG_0;
    com     GLYPH_INV_REG_1                 ; GLYPH_INV_REG_1 ^= GLYPH_INV_REG_1;
    com     GLYPH_INV_REG_2                 ; GLYPH_INV_REG_2 ^= GLYPH_INV_REG_2;
    com     GLYPH_INV_REG_3                 ; GLYPH_INV_REG_3 ^= GLYPH_INV_REG_3;
    com     GLYPH_INV_REG_4                 ; GLYPH_INV_REG_4 ^= GLYPH_INV_REG_4;
    com     GLYPH_INV_REG_5                 ; GLYPH_INV_REG_5 ^= GLYPH_INV_REG_5;
    com     GLYPH_INV_REG_6                 ; GLYPH_INV_REG_6 ^= GLYPH_INV_REG_6;
    com     GLYPH_INV_REG_7                 ; GLYPH_INV_REG_7 ^= GLYPH_INV_REG_7;
    rjmp    _render_video_line_invert_done  ;
_render_video_line_skip_invert:             ; else { // do nothing
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
    nop                                     ;
_render_video_line_invert_done:             ; }

    ; Set X index to point to glyphs_set_ptr
    ldi     XL, LOW(glyphs_set_ptr)
    ldi     XH, HIGH(glyphs_set_ptr)        ; X = glyphs_set_ptr;

    ; enable timer2 that will automaticaly load data to the shift register
    ; there are some ticks that we have to wait before we can render the first glyph
    ; so we can use it for some logic
    enable_timer2
    ; LOAD glyphs ZH addr from *glyphs_set_ptr and add the offset for current scan line
    ld      REG_B1, X           ; REG_B1 = RAM[glyphs_set_ptr]; 
    nop                         ;
    mov     ZH, REG_B1          ; 
    add     ZH, REG_A0          ; ZH = RAM[glyphs_set_ptr] + REG_A0;
    ; RESTORE X index from REG_C
    mov     XL, REG_C0          ;
    mov     XH, REG_C1          ; X = REG_C;
    
    ; RENDER GLYPHS 
    ; X is a RAM pointer for glyphs index data
    ; Z is FLASH pointer for glyphs pixel data
    render_glyph GLYPH_INV_REG_0, 0
    render_glyph GLYPH_INV_REG_0, 1
    render_glyph GLYPH_INV_REG_0, 2
    render_glyph GLYPH_INV_REG_0, 3
    render_glyph GLYPH_INV_REG_0, 4
    render_glyph GLYPH_INV_REG_0, 5
    render_glyph GLYPH_INV_REG_0, 6
    render_glyph GLYPH_INV_REG_0, 7    ;8

    render_glyph GLYPH_INV_REG_1, 0
    render_glyph GLYPH_INV_REG_1, 1
    render_glyph GLYPH_INV_REG_1, 2
    render_glyph GLYPH_INV_REG_1, 3
    render_glyph GLYPH_INV_REG_1, 4
    render_glyph GLYPH_INV_REG_1, 5
    render_glyph GLYPH_INV_REG_1, 6
    render_glyph GLYPH_INV_REG_1, 7    ;16
    
    render_glyph GLYPH_INV_REG_2, 0
    render_glyph GLYPH_INV_REG_2, 1
    render_glyph GLYPH_INV_REG_2, 2
    render_glyph GLYPH_INV_REG_2, 3
    render_glyph GLYPH_INV_REG_2, 4
    render_glyph GLYPH_INV_REG_2, 5
    render_glyph GLYPH_INV_REG_2, 6
    render_glyph GLYPH_INV_REG_2, 7    ;24

    render_glyph GLYPH_INV_REG_3, 0
    render_glyph GLYPH_INV_REG_3, 1
    render_glyph GLYPH_INV_REG_3, 2
    render_glyph GLYPH_INV_REG_3, 3
    render_glyph GLYPH_INV_REG_3, 4
    render_glyph GLYPH_INV_REG_3, 5
    render_glyph GLYPH_INV_REG_3, 6
    render_glyph GLYPH_INV_REG_3, 7    ;32

    render_glyph GLYPH_INV_REG_4, 0
    render_glyph GLYPH_INV_REG_4, 1
    render_glyph GLYPH_INV_REG_4, 2
    render_glyph GLYPH_INV_REG_4, 3
    render_glyph GLYPH_INV_REG_4, 4
    render_glyph GLYPH_INV_REG_4, 5
    render_glyph GLYPH_INV_REG_4, 6
    render_glyph GLYPH_INV_REG_4, 7    ;40

    render_glyph GLYPH_INV_REG_5, 0
    render_glyph GLYPH_INV_REG_5, 1
    render_glyph GLYPH_INV_REG_5, 2
    render_glyph GLYPH_INV_REG_5, 3
    render_glyph GLYPH_INV_REG_5, 4
    render_glyph GLYPH_INV_REG_5, 5
    render_glyph GLYPH_INV_REG_5, 6
    render_glyph GLYPH_INV_REG_5, 7    ;48

    render_glyph GLYPH_INV_REG_6, 0
    render_glyph GLYPH_INV_REG_6, 1
    render_glyph GLYPH_INV_REG_6, 2
    render_glyph GLYPH_INV_REG_6, 3
    render_glyph GLYPH_INV_REG_6, 4
    render_glyph GLYPH_INV_REG_6, 5
    render_glyph GLYPH_INV_REG_6, 6
    render_glyph GLYPH_INV_REG_6, 7    ;56

    render_glyph GLYPH_INV_REG_7, 0
    render_glyph GLYPH_INV_REG_7, 1
    render_glyph GLYPH_INV_REG_7, 2
    render_glyph GLYPH_INV_REG_7, 3
    render_glyph GLYPH_INV_REG_7, 4
    render_glyph GLYPH_INV_REG_7, 5    ;62

    ; DISABLE TIMER
    disable_timer2
    
    ; CLEAR VDATA_PORT used by the shift register
    ldi     REG_B1, 0x00
    out     VDATA_PORT, REG_B1      ; VDATA_PORT = 0x00;
    
    ; if this is not the 8th glyph line, 
    ; go back by 62 glyphs. This will result in rendering the same set of glyphs 
    ; in the next line. Only after the last (8th) line we will move to the next 
    ; glyph indices row.
    cpi      REG_A0, 0x0e           ;
    breq    __skip_x_reset          ; 
    sbiw    XH:XL, 62               ; 
__skip_x_reset:                     ; if(REG_A0 != 0x0e) { X -= 62; } 
reti

;-------------------------------------------------------------------------------
; GENERATE V-PORCH SIGANL + handle SPI
; Use by: timer1b_interrupt
render_porch:
    ; there is some time before we have to change VSYNC_PIN state
    ; we use that time to do some setup for the extended commands
    rcall    setup_ext_cmd              ; setup_ext_cmd();

    delay_vsync_pulse               ; after the setup, we still have some time left
    sbi     VSYNC_PORT, VSYNC_PIN   ; set VSYNC_PIN -> HIGH

    ; now we can handle the extended commands and spi
    rjmp    exec_cmd_read_spi       ; goto exec_cmd_read_spi
reti

;-------------------------------------------------------------------------------
; GENERATE V-SYNC SIGNAL + handle SPI
render_sync_pulse:
    ; there is some time before we have to change VSYNC_PIN state
    ; we use that time to do some setup for the extended commands
    rcall    setup_ext_cmd

    delay_vsync_pulse
    nop 
    nop
    nop
    nop
    nop                             ; after the setup, we still have some time left
    cbi     VSYNC_PORT, VSYNC_PIN   ; set VSYNC_PIN -> LOW

    ; now we can handle the extended commands and spi
    rjmp    exec_cmd_read_spi
reti

;-------------------------------------------------------------------------------
; HANDLE LAST LINE
; Reads SPI. Resets line counter. Updates text colors, and gloval inv reg.
handle_last_line:
    read_spi        ; handle SPI

    ; Read color settings
    ldi     XL, LOW(displ_color_ptr)    
    ldi     XH, HIGH(displ_color_ptr)   ; X = displ_color_ptr
    ld      REG_B0, X                   ; REG_B0 = *displ_color_ptr

    ; Update color pins
    sbrc    REG_B0, 0                   ; if(*displ_color_ptr & 0x01) {
    sbi     COLOR_PORT, COLOR_RED_PIN   ;   COLOR_PORT |= (1<<COLOR_RED_PIN); }
    sbrs    REG_B0, 0                   ; else {
    cbi     COLOR_PORT, COLOR_RED_PIN   ;   COLOR_PORT &= ~(1<<COLOR_RED_PIN); }

    sbrc    REG_B0, 1                   ; if(*displ_color_ptr & 0x02) {
    sbi     COLOR_PORT, COLOR_BLUE_PIN;   COLOR_PORT |= (1<<COLOR_BLUE_PIN); }
    sbrs    REG_B0, 1                   ; else {
    cbi     COLOR_PORT, COLOR_BLUE_PIN  ;   COLOR_PORT &= ~(1<<COLOR_BLUE_PIN); }

    sbrc    REG_B0, 2                   ; if(*displ_color_ptr & 0x04) {
    sbi     COLOR_PORT, COLOR_GREEN_PIN ;   COLOR_PORT |= (1<<COLOR_GREEN_PIN); }
    sbrs    REG_B0, 2                   ; else {
    cbi     COLOR_PORT, COLOR_GREEN_PIN ;   COLOR_PORT &= ~(1<<COLOR_GREEN_PIN); }
    
    ; Update global invert register
    clr     GLB_INV_REG                 ; GLB_INV_REG = 0x00;
    sbrc    REG_B0, 7                   ; if(*displ_color_ptr & 0x80) {
    inc     GLB_INV_REG                 ;   GLB_INV_REG ++; }

    ; Reset lince counter
    ; set te VLINE CNT to 0xFFFF, next video line will add +1 and set it to 0x0000
    ldi     REG_B0, 0xff            
    mov     LINE_CNT_REG_L, REG_B0       
    mov     LINE_CNT_REG_H, REG_B0      ; LINE_CNT_REG = 0xffff
    ldi     XL, LOW(glyphs_ptr)
    ldi     XH, HIGH(glyphs_ptr)        ; Reset X index 
    
    wdr                                 ; Let us not forget about the watchdog
reti


;-------------------------------------------------------------------------------
; SETUP EXTENDED COMMAND
; if the bit 7 is set in reg EXT_CMD_REG, we skip the setup
; eg. if EXT_CMD_REG == 0x88, even thoug this is a push right command, we will
; skip the cmd setup for it.
setup_ext_cmd:
    sbrc    EXT_CMD_REG, 7             ; if(EXT_CMD_REG & 1<<7) {
    rjmp    _setup_push_spkip_setup    ;    goto _setup_push_spkip_setup; }
    
    sbrc    EXT_CMD_REG, 0             ; if(EXT_CMD_REG & 1<<0) {
    rjmp    setup_push_up_cmd          ;    goto setup_push_up_cmd; }

    sbrc    EXT_CMD_REG, 1             ; if(EXT_CMD_REG & 1<<1) {
    rjmp    setup_push_down_cmd        ;    goto setup_push_down_cmd; }

    sbrc    EXT_CMD_REG, 2             ; if(EXT_CMD_REG & 1<<2) {
    rjmp    setup_push_left_cmd        ;    goto setup_push_left_cmd; }

    sbrc    EXT_CMD_REG, 3             ; if(EXT_CMD_REG & 1<<3) {
    rjmp    setup_push_right_cmd       ;    goto setup_push_right_cmd; }

    rjmp    _setup_push_no_setup

_setup_push_spkip_setup:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

_setup_push_no_setup:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
ret

;-------------------------------------------------------------------------------
; EXECUTE EXTENDED COMMAND
exec_cmd_read_spi:
    sbrc    EXT_CMD_REG, 7                  ; if(EXT_CMD_REG & 0x07) {
    rjmp    mov_EXT_CMD_WSTATE_REG_to_screen_end  ; goto mov_EXT_CMD_WSTATE_REG_to_screen_end;}

    sbrc    EXT_CMD_REG, 0                 
    rjmp    exec_push_up_cmd
    
    sbrc    EXT_CMD_REG, 1
    rjmp    exec_push_down_cmd
    
    sbrc    EXT_CMD_REG, 2
    rjmp    exec_push_left_cmd

    sbrc    EXT_CMD_REG, 3
    rjmp    exec_push_right_cmd

    sbrc    EXT_CMD_REG, 4
    rjmp    exec_clear_cmd

    read_spi
    clr     ZH
    wdr
reti

;-------------------------------------------------------------------------------
mov_EXT_CMD_WSTATE_REG_to_screen_end:
    cbr     EXT_CMD_REG, 0x80
    ldi     EXT_CMD_WSTATE_REG_L, LOW(glyphs_ptr+62*57)
    ldi     EXT_CMD_WSTATE_REG_H, HIGH(glyphs_ptr+62*57)
    ldi     EXT_CMD_STATE_REG, 57
    clr     ZH
reti

;-------------------------------------------------------------------------------

setup_push_up_cmd:  ;+5
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)
    ld      REG_B0, X+      ; left
    ld      REG_B1, X+      ; top
    ld      REG_C0, X+      ; width
    ld      REG_C1, X+      ; height
    ld      REG_D0, X+      ; glyph
    ; -------------------------
    mov     XL, EXT_CMD_WSTATE_REG_L   ;1
    mov     XH, EXT_CMD_WSTATE_REG_H   ;1
    mov     ZL, EXT_CMD_WSTATE_REG_L   ;1
    mov     ZH, EXT_CMD_WSTATE_REG_H   ;1
    adiw    XH:XL, 62           ;1
    ; ------------------------------
    ldi     REG_A0, 0x00
    add     XL, REG_B0
    adc     XH, REG_A0
    add     ZL, REG_B0
    adc     ZH, REG_A0          ;+27

    nop
    nop
    nop
    nop                         ;31
    nop
    nop
    nop
    nop
    nop
ret                             

;-------------------------------------------------------------------------------
exec_push_up_cmd:
    adiw    EXT_CMD_WSTATE_REG_L:EXT_CMD_WSTATE_REG_H, 62
    
    cp      EXT_CMD_STATE_REG, REG_B1
    brlo    _exec_push_up_skip
    
    dec     REG_B1
    add     REG_B1, REG_C1
    cp      EXT_CMD_STATE_REG, REG_B1
    breq    finish_scroll_cmd

_exec_push_up_cmd_loop:
    ld      REG_A0, X+  ;;
    st      Z+, REG_A0  ;;
    subi    REG_C0, 1
    brne    _exec_push_up_cmd_loop
_exec_push_up_skip:
    inc     EXT_CMD_STATE_REG
    clr     ZH
reti

;-------------------------------------------------------------------------------
setup_push_down_cmd:        ;+7
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)
    ld      REG_B0, X+      ; left
    ld      REG_B1, X+      ; top
    ld      REG_C0, X+      ; width
    ld      REG_C1, X+      ; height    ; +10
    ld      REG_D0, X+      ; glyph
    ; -------------------------
    mov     XL, EXT_CMD_WSTATE_REG_L   ;1
    mov     XH, EXT_CMD_WSTATE_REG_H   ;1
    mov     ZL, EXT_CMD_WSTATE_REG_L   ;1
    mov     ZH, EXT_CMD_WSTATE_REG_H   ;1
    sbiw    XH:XL, 62           ;1  ; +17
    ; ------------------------------
    ldi     REG_A0, 0x00
    add     XL, REG_B0
    adc     XH, REG_A0
    add     ZL, REG_B0
    adc     ZH, REG_A0          ;+29
    nop
    nop                         ;31
    nop
    nop
    nop
    nop
    nop
ret                             ;4 //+15

;-------------------------------------------------------------------------------
exec_push_down_cmd:
    sbiw    EXT_CMD_WSTATE_REG_L:EXT_CMD_WSTATE_REG_H, 62
    cp      EXT_CMD_STATE_REG, REG_B1
    breq    finish_scroll_cmd

    add     REG_B1, REG_C1
    cp      EXT_CMD_STATE_REG, REG_B1
    brge    _exec_push_down_skip

_exec_push_down_cmd_loop:
    ld      REG_A0, X+  ;;
    st      Z+, REG_A0  ;;
    subi    REG_C0, 1
    brne    _exec_push_down_cmd_loop
_exec_push_down_skip:
    dec     EXT_CMD_STATE_REG
    clr     ZH
reti

;-------------------------------------------------------------------------------
finish_scroll_cmd:
_finish_scroll_cmd_loop:
    st      Z+, REG_D0
    subi    REG_C0, 1
    brne    _finish_scroll_cmd_loop
rjmp  finish_cmd

;-------------------------------------------------------------------------------
setup_push_left_cmd:                ;+9
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)
    ld      REG_B0, X+      ; left
    ld      REG_B1, X+      ; top
    ld      REG_C0, X+      ; width
    ld      REG_C1, X+      ; height   
    ld      REG_D0, X+      ; glyph     ; rel +12
    ; -------------------------
    mov     XL, EXT_CMD_WSTATE_REG_L   ;1
    mov     XH, EXT_CMD_WSTATE_REG_H   ;1
    mov     ZL, EXT_CMD_WSTATE_REG_L   ;1
    mov     ZH, EXT_CMD_WSTATE_REG_H   ;1
    adiw    XH:XL, 1            ;1      ; rel +17
    ; ------------------------------
    ldi     REG_A0, 0x00
    add     XL, REG_B0
    adc     XH, REG_A0
    add     ZL, REG_B0
    adc     ZH, REG_A0          ; rel +22
    nop
    nop
    nop
    nop
    nop
ret

;-------------------------------------------------------------------------------
exec_push_left_cmd:
    adiw    EXT_CMD_WSTATE_REG_L:EXT_CMD_WSTATE_REG_H, 62

    cp      EXT_CMD_STATE_REG, REG_B1
    brlo    _exec_push_left_cmd_skip
    
    add     REG_B1, REG_C1
    cp      EXT_CMD_STATE_REG, REG_B1
    breq    finish_cmd
    
    subi    REG_C0, 1

_exec_push_left_cmd_loop:
    ld      REG_A0, X+  ;;
    st      Z+, REG_A0  ;;
    subi    REG_C0, 1
    brne    _exec_push_left_cmd_loop
    
    st      Z, REG_D0

_exec_push_left_cmd_skip:
    inc     EXT_CMD_STATE_REG
    clr     ZH
reti

;-------------------------------------------------------------------------------
setup_push_right_cmd:                ;+11
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)
    ld      REG_B0, X+      ; left
    ld      REG_B1, X+      ; top
    ld      REG_C0, X+      ; width
    ld      REG_C1, X+      ; height   
    ld      REG_D0, X+      ; glyph     ; rel +12
    ; -------------------------
    mov     XL, EXT_CMD_WSTATE_REG_L   ;1
    mov     XH, EXT_CMD_WSTATE_REG_H   ;1
    mov     ZL, EXT_CMD_WSTATE_REG_L   ;1
    mov     ZH, EXT_CMD_WSTATE_REG_H   ;1
    sbiw    XH:XL, 1            ;1      ; rel +17
    ; ------------------------------
    ldi     REG_A0, 0x00
    add     XL, REG_B0
    adc     XH, REG_A0
    add     ZL, REG_B0
    adc     ZH, REG_A0          ; rel +22
    
    add     XL, REG_C0
    adc     XH, REG_A0
    add     ZL, REG_C0
    adc     ZH, REG_A0          ; rel +26
ret

;-------------------------------------------------------------------------------
exec_push_right_cmd:    
sbiw    EXT_CMD_WSTATE_REG_L:EXT_CMD_WSTATE_REG_H, 62
    
    cp      EXT_CMD_STATE_REG, REG_B1
    brlo    finish_cmd
    
    add     REG_B1, REG_C1
    cp      EXT_CMD_STATE_REG, REG_B1
    brge    _exec_push_right_cmd_skip

_exec_push_right_cmd_loop:
    ld      REG_A0, -X  ;;
    st      -Z, REG_A0  ;;
    subi    REG_C0, 1
    brne    _exec_push_right_cmd_loop

    st      Z, REG_D0
_exec_push_right_cmd_skip:
    dec     EXT_CMD_STATE_REG
    clr     ZH
reti

;-------------------------------------------------------------------------------
finish_cmd:
    ldi     EXT_CMD_REG, 0x00
    ldi     EXT_CMD_STATE_REG, 0x00
    ldi     EXT_CMD_WSTATE_REG_L, LOW(glyphs_ptr)
    ldi     EXT_CMD_WSTATE_REG_H, HIGH(glyphs_ptr)
reti

;-------------------------------------------------------------------------------
exec_clear_cmd:
    ldi     XL, LOW(ext_cmd_params_ptr)
    ldi     XH, HIGH(ext_cmd_params_ptr)
    ld      REG_A0, X+      ; left
    ld      REG_A1, X+      ; top
    ld      REG_B0, X+      ; width
    ld      REG_B1, X+      ; height
    ld      REG_C0, X+      ; glyph
    
    mov     XL, EXT_CMD_WSTATE_REG_L   ;1
    mov     XH, EXT_CMD_WSTATE_REG_H   ;1
    
    ldi     REG_C1, 0x00
    add     XL, REG_A0
    adc     XH, REG_C1
    
    adiw    EXT_CMD_WSTATE_REG_L:EXT_CMD_WSTATE_REG_H, 62

    cp      EXT_CMD_STATE_REG, REG_A1
    brlo    _exec_clear_cmd_skip

_exec_clear_cmd_loop:
    st      X+, REG_C0
    subi    REG_B0, 1
    brne    _exec_clear_cmd_loop
_exec_clear_cmd_skip:
    inc     EXT_CMD_STATE_REG    
    clr     ZH
    add     REG_A1, REG_B1
    cp      EXT_CMD_STATE_REG, REG_A1
    breq    finish_cmd
reti
