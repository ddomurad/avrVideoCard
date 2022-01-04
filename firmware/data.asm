.include "./glyphs/glyphs1.asm"
; .include "./glyphs/der_rouge.asm"
; .include "./glyphs/glyphs2.asm"
; .include "./glyphs/glyphs3.asm"
; .include "./glyphs/glyphs4.asm"
; .include "./glyphs/glyphs5.asm"
; .include "./glyphs/glyphs6.asm"
; .include "./glyphs/glyphs7a.asm"
; .include "./glyphs/glyphs7b.asm"
; .include "./glyphs/glyphs8.asm"
; .include "./glyphs/glyphs9.asm"
; .include "./glyphs/glyphs10.asm"


.dseg
glyphs_ptr:         .byte 58*62
inv_ptr:            .byte 58*8
glyphs_set_ptr:     .byte 1
displ_color_ptr:        .byte 1
ext_cmd_params_ptr:     .byte 5