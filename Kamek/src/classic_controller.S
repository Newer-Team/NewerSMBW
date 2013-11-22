#include <asm_setup.S>

#define WPAD_BUTTON_2							0x0001
#define WPAD_BUTTON_1							0x0002
#define WPAD_BUTTON_B							0x0004
#define WPAD_BUTTON_A							0x0008
#define WPAD_BUTTON_MINUS						0x0010
#define WPAD_BUTTON_HOME						0x0080
#define WPAD_BUTTON_LEFT						0x0100
#define WPAD_BUTTON_RIGHT						0x0200
#define WPAD_BUTTON_DOWN						0x0400
#define WPAD_BUTTON_UP							0x0800
#define WPAD_BUTTON_PLUS						0x1000

#define WPAD_CLASSIC_BUTTON_UP					0x0001
#define WPAD_CLASSIC_BUTTON_LEFT				0x0002
#define WPAD_CLASSIC_BUTTON_R					0x0004
#define WPAD_CLASSIC_BUTTON_X					0x0008
#define WPAD_CLASSIC_BUTTON_A					0x0010
#define WPAD_CLASSIC_BUTTON_Y					0x0020
#define WPAD_CLASSIC_BUTTON_B					0x0040
#define WPAD_CLASSIC_BUTTON_L					0x0080
#define WPAD_CLASSIC_BUTTON_ZR					0x0200
#define WPAD_CLASSIC_BUTTON_PLUS				0x0400
#define WPAD_CLASSIC_BUTTON_HOME				0x0800
#define WPAD_CLASSIC_BUTTON_MINUS				0x1000
#define WPAD_CLASSIC_BUTTON_ZL					0x2000
#define WPAD_CLASSIC_BUTTON_DOWN				0x4000
#define WPAD_CLASSIC_BUTTON_RIGHT				0x8000

.set stackSize,0x30;
.set r4Place,0x10;
.set r5Place,0x14;
.set r6Place,0x18;
.set r26Place,0x1c;
.set f8Place,0x20;
.set f9Place,0x24;
.set f10Place,0x28;
.set f11Place,0x2c;

.text
.align 4





.global waggleAdjust
waggleAdjust:
	
	lbz		r5, 0x5C(r3)		# wiimote extension type
	cmpwi	r5, 2				# classic controller

	beq ClassicControllerWaggle

	stfs	f0, 0(r4)
	blr

ClassicControllerWaggle:

	li		r12, 0
	stw		r12, 0(r4)
	blr




.global _switch_buttons
_switch_buttons:
	addi    r1,r1,0x50			# replaced instruction

	stwu	r1,-stackSize(r1)		# provide new stack for saving regs
	mflr    r0
	stw		r4,r4Place(r1)
	stw		r5,r5Place(r1)
	stw		r6,r6Place(r1)
	stw		r26,r26Place(r1)
	stfs	f8,f8Place(r1)
	stfs	f9,f9Place(r1)
	stfs	f10,f10Place(r1)
	stfs	f11,f11Place(r1)

	lbz		r5,0x5C(r26)		# wiimote extension type
	cmpwi	r5, 2				# classic controller
	bne		cleanup

	lwz     r5, 0x60(r26)

Tilt:

	lwz		r5,	0x80(r26)
	cmpwi	r5,	0
	beq		TiltLeftSkip

	lfs     f11, 0x80(r26)

	lis		r5,	0
	stw		r5,	0x80(r26)
	lfs		f10, 0x80(r26)

	fsubs	f11, f10, f11
	
	stfs    f11, 0x58(r26)
	li		r5,	0
	stw     r5,	0x54(r26)

TiltLeftSkip:
	
	lwz		r5,	0x7C(r26)
	cmpwi	r5,	0
	beq		TiltRightSkip
	stw     r5,	0x58(r26)
	li		r5,	0
	stw     r5,	0x54(r26)

TiltRightSkip:
	lwz     r5,0x60(r26)		# wiimote cc data ; KPADInsideStatus->KPADEXStatus_bl->hold
	bl      map_buttons
	lwz     r4,0(r26)			# wiimote ; KPADInsideStatus->hold
	or      r5,r5,r4
	stw     r5,0(r26)			# wiimote ; KPADInsideStatus->hold
	lwz     r5,0x64(r26)		# wiimote cc data ; KPADInsideStatus->KPADEXStatus_bl->trig
	bl      map_buttons
	lwz     r4,4(r26)			# wiimote ; KPADInsideStatus->trig
	or      r5,r5,r4
	stw     r5,4(r26)			# wiimote ; KPADInsideStatus->trig
	lwz     r5,0x68(r26)		# wiimote cc data ; KPADInsideStatus->KPADEXStatus_bl->release
	bl      map_buttons
	lwz     r4,8(r26)			# wiimote ; KPADInsideStatus->release
	or      r5,r5,r4
	stw     r5,8(r26)			# wiimote ; KPADInsideStatus->release

cleanup:
	lwz		r4,r4Place(r1)
	lwz		r5,r5Place(r1)
	lwz		r6,r6Place(r1)
	lwz		r26,r26Place(r1)
	lfs		f8,f8Place(r1)
	lfs		f9,f9Place(r1)
	lfs		f10,f10Place(r1)
	lfs		f11,f11Place(r1)
	mtlr    r0
	addi	r1,r1,stackSize
	blr

# r5 == CC data
# r5 returns the wpad data


map_buttons:
	li      r6,0
	andi.   r4,r5,0x10		// WPAD_CLASSIC_BUTTON_A
	beq-    skip_A
	ori     r6,r6,0x100		// WPAD_BUTTON_2
skip_A:
	andi.   r4,r5,0x800		// WPAD_CLASSIC_BUTTON_HOME
	beq-    skip_home
	ori     r6,r6,0x8000	// WPAD_BUTTON_HOME
skip_home:
	andi.   r4,r5,0x20		// WPAD_CLASSIC_BUTTON_Y
	beq-    skip_Y
	ori     r6,r6,0x800		// WPAD_BUTTON_A
skip_Y:
	andi.   r4,r5,0x8		// WPAD_CLASSIC_BUTTON_X
	beq-    skip_X
	lis     r6,0x7fc0		// float
	stw     r6,0x10(r26)	// set waggle ; KPADInsideStatus->acc->y
skip_X:
	andi.   r4,r5,0x40		// WPAD_CLASSIC_BUTTON_B
	beq-    skip_B
	ori     r6,r6,0x200		// WPAD_BUTTON_1
skip_B:
	andi.   r4,r5,0x80		// WPAD_CLASSIC_BUTTON_ZL
	beq-    skip_ZL
	ori     r6,r6,0x200		// WPAD_BUTTON_1
skip_ZL:
	andi.   r4,r5,0x4		// WPAD_CLASSIC_BUTTON_ZR
	beq-    skip_ZR
	lis     r6,0x7fc0		// float
	stw     r6,0x10(r26)	// set waggle ; KPADInsideStatus->acc->y
skip_ZR:
	andi.   r4,r5,0x400		// WPAD_CLASSIC_BUTTON_PLUS
	beq-    skip_plus
	ori     r6,r6,0x10		// WPAD_BUTTON_PLUS
skip_plus:
	andi.   r4,r5,0x1000	// WPAD_CLASSIC_BUTTON_MINUS
	beq-    skip_minus
	ori     r6,r6,0x1000	// WPAD_BUTTON_MINUS
skip_minus:
	andi.	r4,r5,0xC003	// Any_DPAD
	beq-	DPADOnly		// DPAD and Analog are processed separately to avoid conflicting button presses

	andi.   r4,r5,1			// WPAD_CLASSIC_BUTTON_UP
	beq-    skip_up
	ori     r6,r6,2			// WPAD_BUTTON_RIGHT
skip_up:
	andi.   r4,r5,2			// WPAD_CLASSIC_BUTTON_LEFT
	beq-    skip_left
	ori     r6,r6,8			// WPAD_BUTTON_UP
skip_left:
	andi.   r4,r5,0x8000	// WPAD_CLASSIC_BUTTON_RIGHT
	beq-    skip_right
	ori     r6,r6,4			// WPAD_BUTTON_DOWN
skip_right:
	andi.   r4,r5,0x4000	// WPAD_CLASSIC_BUTTON_DOWN
	beq-    skip_down
	ori     r6,r6,1			// WPAD_BUTTON_LEFT
skip_down:
	mr      r5,r6
	blr

DPADOnly:
	lfs		f8,0x6c(r26)	// left analog x ; KPADInsideStatus->KPADEXStatus_cl->lstick
	lfs		f9,0x70(r26)	// left analog y ; KPADInsideStatus->KPADEXStatus_cl->rstick

	lis		r5,float_one@ha
	lfs		f10,float_one@l(r5)

	lis		r5,float_neg_one@ha
	lfs		f11,float_neg_one@l(r5)

	fcmpu	cr0,f8,f10		// WPAD_R_ANALOG_RIGHT
	blt		skip_right_analog
	ori     r6,r6,4			// WPAD_BUTTON_DOWN
skip_right_analog:
	fcmpu	cr0,f9,f10		// WPAD_R_ANALOG_UP
	blt		skip_up_analog
	ori     r6,r6,2			// WPAD_BUTTON_RIGHT
skip_up_analog:
	fcmpu	cr0,f8,f11		// WPAD_R_ANALOG_LEFT
	bge		skip_down_analog
	ori     r6,r6,8			// WPAD_BUTTON_UP
skip_down_analog:
	fcmpu	cr0,f9,f11		// WPAD_R_ANALOG_DOWN
	bge		skip_left_analog
	ori     r6,r6,1			// WPAD_BUTTON_LEFT
skip_left_analog:
	mr      r5,r6
	blr






.data

float_one:
.float 0.5
float_neg_one:
.float -0.5
float_zero:
.float 0.0
