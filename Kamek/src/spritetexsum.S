.text
.align 4

.global TiltZAdjust
TiltZAdjust: # Puts tilt platforms behind tiles
lis    r0, 0xc120
stw    r0, 0xb4(r29)
stfs   f0, 0x344(r29) # og instruction
blr