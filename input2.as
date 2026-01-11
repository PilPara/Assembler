; macro vaidation input file
; valid macros
; valid macro def line - no other chars and macro name is valid

mcro a_mc          
cmp K, #-6
bne &END
mcroend

; valid macro def line - no other chars and macro name is valid
mcro _b_mc
lea W, r6
inc r6
mcroend
 
;valid with whitespace between and ater macro name
                      mcro                                c_mc
cmp K, #-6
bne &END
mcroend

; valid macro def line - no other chars and macro name is valid
mcro d_mc
lea W, r6
inc r6
                    mcroend


; invalid macros
; invalid macro def line - extra chars  
mcro e_mc and then some more chars
lea W, r6
inc r6
mcroend

; invalid macro def line - extra chars no spaces  
mcro f_mc andthensomemorechars
cmp K, #-6
bne &END
mcroend

; invalid macro def line - no space between 'mcro' and macro name  
mcrog_mc
lea W, r6
inc r6
mcroend

;invalid macro end def - extra chars   
mcro h_mc
cmp K, #-6
bne &END
mcroend andthensomemorechars

;invalid macro names
;stars with a digit 
mcro 1i_mc 
lea W, r6
inc r6
mcroend

;stars with an uppercase 
mcro J_mc
cmp K, #-6
bne &END
mcroend

;macro name too long 
mcro k_mcandthenitsareallyreallylongnamewithnospaces
lea W, r6
inc r6
mcroend

;macro name contains illegal chars 
mcro l_mc!@#$%^&*()
cmp K, #-6
bne &END
mcroend

;macro name missing 
mcro 
lea W, r6
inc r6
mcroend

;macro name duplicate
mcro a_mc
cmp K, #-6
bne &END
mcroend

;macro name is the name of a register 
mcro r0
lea W, r6
inc r6
mcroend

;macro name is the name of a keyword 
mcro lea
cmp K, #-6
bne &END
mcroend

;macro name is the name of a label
mcro HELLO:
lea W, r6
inc r6
mcroend

;macro name is the name of a directive
mcro data
cmp K, #-6
bne &END
mcroend

mcro string
cmp K, #-6
bne &END
mcroend

mcro extern
cmp K, #-6
bne &END
mcroend

mcro entry
cmp K, #-6
bne &END
mcroend