/* 
 * delay utilite for STM8 family
 * COSMIC and SDCC
 * Terentiev Oleg
 * t.oleg@ymail.com
 */
#ifndef __DELAY_H
#define __DELAY_H

#ifndef F_CPU
#warning F_CPU is not defined!
#endif

#define T_COUNT(x) (( F_CPU * x / 1000000UL )-3)/3)

/* 
 * Func delayed N cycles, where N = 3 + ( ticks * 3 )
 * so, ticks = ( N - 3 ) / 3, minimum delay is 6 CLK
 * when tick = 1, because 0 equels 65535
 */

@inline static void _delay_cycl(unsigned short __ticks )
{
	// ldw X, __ticks ; insert automaticaly
	_asm("nop\n $N:\n decw X\n jrne $L\n nop\n ", __ticks);
}

@inline static void _delay_us( const unsigned short __us )
{
	_delay_cycl( (unsigned short)( T_COUNT(__us) );
}

@inline static void _delay_ms( unsigned short __ms )
{
	while ( __ms-- )
	{
		_delay_us(1000);
	}
}

#endif  /* __DELAY_H */

