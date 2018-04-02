#ifndef _INTERRUPT_H
#define _INTERRUPT_H


#define INT_VEC_VBLANK (0x0040)
#define INT_VEC_LCDC (0x0048)
#define INT_VEC_TIMER_OVERFLOW (0x0050)
#define INT_VEC_SERIAL_TRANSFER (0x0058)
#define INT_VEC_P1_IO (0x0060)

/*
	When an interrupt is used a '0' should be stored in the IF register
	before the IE register is set.
*/

// Interrupt Priority enumeration
enum{
	prio_vblank = 1,
	prio_lcdc,
	prio_timer_overflow,
	prio_serial_transfer,
	prio_p1_io
}Int_Priority;

/*
	Use as union Interrupt_Enable *ie_reg;
	Point to address $FF00 in the CPU address space.
*/

// Interrupt enable union
union Interrupt_Enable{
	uint8_t IE; // $FFFF
	struct{
		uint8_t v_blank : 1;
		uint8_t lcdc : 1;
		uint8_t timer_overflow : 1;
		uint8_t serial_transfer_complete : 1;
		uint8_t falling_edge_P1 : 1;
		uint8_t unused : 3;
	}IE_bits;
};

#endif