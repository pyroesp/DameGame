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

enum{
	prio_vblank = 1,
	prio_lcdc,
	prio_timer_overflow,
	prio_serial_transfer,
	prio_p1_io
}Priority;


#endif