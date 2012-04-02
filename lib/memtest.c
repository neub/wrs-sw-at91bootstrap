/*
 * mtest - Perform a memory test
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <pp_printf.h>
#include <dbgu.h>
#include <debug.h>
//#include <stdlib.h>

/* BEGIN HACKS - to compile barebox code out of barebox */
typedef unsigned char           uchar;
typedef unsigned short          ushort;
typedef unsigned int            uint;
typedef unsigned long           ulong;
typedef volatile unsigned long  vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char  vu_char;

#define putchar __putc
#define printf pp_printf
#define puts dbgu_print

#ifdef DEBUG
#define pr_debug(fmt, arg...)   printf(fmt, ##arg)
#else
#define pr_debug(fmt, arg...)   do {} while(0)
#endif
#define debug(fmt, arg...)      pr_debug(fmt, ##arg)

static inline int ctrlc(void) {return 0;}
static inline void __putc(int c) {printf("%c", c);}

#define CONFIG_CMD_MTEST_ALTERNATIVE

/* END HACKS - to compile barebox code out of barebox */


/**
* Check integrity of the memory with various patterns
**/
int mem_test_integrity(ulong _start, ulong _end, ulong pattern)
{
	vu_long	*addr;
	vu_long *start = (vu_long *)_start;
	vu_long *end   = (vu_long *)_end;
	ulong	val;
	ulong	readback;
	ulong	incr;
	ulong 	iter;
	ulong 	pattern_next;
	ulong 	val_next;
	int rcode;

	incr = 1;
	iter=1;

	for (addr=start,val=pattern; addr<end; addr++) {
			*addr = val;
			val  += incr;
	}


	for (;;) {
		if (ctrlc()) {
			putchar('\n');
			return 1;
		}


		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if(pattern & 0x80000000) {
			pattern_next = -pattern;	/* complement & increment */
		}
		else {
			pattern_next = ~pattern;
		}



		printf ("#0x%08lx: Pattern 0x%08lX ...",iter,pattern);

		for (addr=start,val=pattern, val_next=pattern_next; addr<end; addr++) {
			readback = *addr;
			if (readback != val) {
				printf ("\r\nMem error @ 0x%08X: "
					"found 0x%08lX, expected 0x%08lX\r\n",
					(uint)addr, readback, val);
				rcode = 1;
			}

			//Write the the next value to read			
			*addr=val_next;

			//Increment actual read value and decrement write value.
			val += incr;
			val_next -= incr;
		}

		printf("\tOK\r\n");
		iter++;
		pattern=pattern_next;
		incr = -incr;
		
	}
	return rcode;
}




/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CONFIG_CMD_MTEST_ALTERNATIVE. The complete test
 * loops until interrupted by ctrl-c or by a failure of one of the
 * sub-tests.
 *
 * ref: http://www.barrgroup.com/Embedded-Systems/How-To/Memory-Test-Suite-C
 */
#ifdef CONFIG_CMD_MTEST_ALTERNATIVE
int mem_test(ulong _start, ulong _end, ulong pattern_unused)
{
	vu_long *start = (vu_long *)_start;
	vu_long *end   = (vu_long *)_end;
	vu_long *addr;
	ulong	val;
	ulong	nErr;
	ulong	readback;
	vu_long	addr_mask;
	vu_long	offset;
	vu_long	test_offset;
	vu_long	pattern;
	vu_long	temp;
	vu_long	anti_pattern;
	vu_long	num_words;
#ifdef CFG_MEMTEST_SCRATCH
	vu_long *dummy = (vu_long*)CFG_MEMTEST_SCRATCH;
#else
	vu_long *dummy = start;
#endif
	int	j;


	static const ulong bitpattern[] = {
		0x00000001,	/* single bit */
		0x00000003,	/* two adjacent bits */
		0x00000007,	/* three adjacent bits */
		0x0000000F,	/* four adjacent bits */
		0x00000005,	/* two non-adjacent bits */
		0x00000015,	/* three non-adjacent bits */
		0x00000055,	/* four non-adjacent bits */
		0xaaaaaaaa,	/* alternating 1/0 */
	};


	printf ("\rTesting DDR: 0x%08lX"
		" > 0x%08lX\n\r",
		_start, _end);


	/* XXX: enforce alignment of start and end? */
	if (ctrlc()) {
		putchar ('\n');
		return 1;
	}


	printf ("Testing Data line...\n\r");


	/*
	 * Data line test: write a pattern to the first
	 * location, write the 1's complement to a 'parking'
	 * address (changes the state of the data bus so a
	 * floating bus doen't give a false OK), and then
	 * read the value back. Note that we read it back
	 * into a variable because the next time we read it,
	 * it might be right (been there, tough to explain to
	 * the quality guys why it prints a failure when the
	 * "is" and "should be" are obviously the same in the
	 * error message).
	 *
	 * Rather than exhaustively testing, we test some
	 * patterns by shifting '1' bits through a field of
	 * '0's and '0' bits through a field of '1's (i.e.
	 * pattern and ~pattern).
	 */
	addr = start;
	nErr=0;
	/* XXX */
	if (addr == dummy) ++addr;
	for (j = 0; j < sizeof(bitpattern)/sizeof(bitpattern[0]); j++) {
		val = bitpattern[j];
		for(; val != 0; val <<= 1) {
			*addr  = val;
			*dummy  = ~val; /* clear the test data off of the bus */
			readback = *addr;
			if(readback != val) {
				nErr++;
				printf ("FAILURE (data line)  : "
				"expected 0x%08x, actual 0x%08x @ 0x%08x\n\r",
		val, readback, addr);
			}
			else nErr--;
			
			*addr  = ~val;
			*dummy  = val;
			readback = *addr;
			if(readback != ~val) {
				printf ("FAILURE (data line)~ : "
				"expected 0x%08x, actual 0x%08x @ 0x%08x\n\r",
		~val, readback, addr);
				nErr++;
			}
			else nErr--;
		}
		if(nErr>64) break;
	}
	
	printf ("\tOK: data line\n\r");


	/*
	 * Based on code whose Original Author and Copyright
	 * information follows: Copyright (c) 1998 by Michael
	 * Barr. This software is placed into the public
	 * domain and may be used for any purpose. However,
	 * this notice must not be changed or removed and no
	 * warranty is either expressed or implied by its
	 * publication or distribution.
	 */

	/*
	 * Address line test
	 *
	 * Description: Test the address bus wiring in a
	 *              memory region by performing a walking
	 *              1's test on the relevant bits of the
	 *              address and checking for aliasing.
	 *              This test will find single-bit
	 *              address failures such as stuck -high,
	 *              stuck-low, and shorted pins. The base
	 *              address and size of the region are
	 *              selected by the caller.
	 *
	 * Notes:	For best results, the selected base
	 *              address should have enough LSB 0's to
	 *              guarantee single address bit changes.
	 *              For example, to test a 64-Kbyte
	 *              region, select a base address on a
	 *              64-Kbyte boundary. Also, select the
	 *              region size as a power-of-two if at
	 *              all possible.
	 *
	 * Returns:     0 if the test succeeds, 1 if the test fails.
	 *
	 * ## NOTE ##	Be sure to specify start and end
	 *              addresses such that addr_mask has
	 *              lots of bits set. For example an
	 *              address range of 01000000 02000000 is
	 *              bad while a range of 01000000
	 *              01ffffff is perfect.
	 */

	printf ("Testing addressline (addr mask = 0x%.8lx) ... \n\r",addr_mask);

	addr_mask = ((ulong)end - (ulong)start)/sizeof(vu_long);
	pattern = (vu_long) 0xaaaaaaaa;
	anti_pattern = (vu_long) 0x55555555;

	debug("%s:%d: addr mask = 0x%.8lx\r\n",
		__FUNCTION__, __LINE__,
		addr_mask);
	/*
	 * Write the default pattern at each of the
	 * power-of-two offsets.
	 */
	for (offset = 1; (offset & addr_mask) != 0; offset <<= 1)
		start[offset] = pattern;

	/*
	 * Check for address bits stuck high.
	 */
	test_offset = 0;
	start[test_offset] = anti_pattern;

	for (offset = 1; (offset & addr_mask) != 0; offset <<= 1) {
	    temp = start[offset];
	    if (temp != pattern) {
		printf ("FAILURE: Address bit stuck high @ 0x%.8lx:"
			" expected 0x%.8lx, actual 0x%.8lx\r\n",
			(ulong)&start[offset], pattern, temp);
		return 1;
	    }
	}
	start[test_offset] = pattern;

	/*
	 * Check for addr bits stuck low or shorted.
	 */
	for (test_offset = 1; (test_offset & addr_mask) != 0; test_offset <<= 1) {
	    start[test_offset] = anti_pattern;

	    for (offset = 1; (offset & addr_mask) != 0; offset <<= 1) {
		temp = start[offset];
		if ((temp != pattern) && (offset != test_offset)) {
		    printf ("FAILURE: Address bit stuck low or shorted @"
			" 0x%.8lx: expected 0x%.8lx, actual 0x%.8lx\r\n",
			(ulong)&start[offset], pattern, temp);
		    return 1;
		}
	    }
	    start[test_offset] = pattern;
	}

	printf ("OK: address line\n\r");


	/*
	 * Description: Test the integrity of a physical
	 *		memory device by performing an
	 *		increment/decrement test over the
	 *		entire region. In the process every
	 *		storage bit in the device is tested
	 *		as a zero and a one. The base address
	 *		and the size of the region are
	 *		selected by the caller.
	 *
	 * Returns:     0 if the test succeeds, 1 if the test fails.
	 */
	num_words = ((ulong)end - (ulong)start)/sizeof(vu_long);

	printf ("Testing memory integrity: hold a counter ... \n\r");
	/*
	 * Fill memory with a known pattern.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		start[offset] = pattern;
	}

	/*
	 * Check each location and invert it for the second pass.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
	    temp = start[offset];
	    if (temp != pattern) {
		printf ("FAILURE (read/write) @ 0x%.8lx:"
			" expected 0x%.8lx, actual 0x%.8lx)\r\n",
			(ulong)&start[offset], pattern, temp);
		return 1;
	    }

	    anti_pattern = ~pattern;
	    start[offset] = anti_pattern;
	}

	printf ("OK: memory integrity (counter)\n\r");
	printf ("Testing memory integrity: hold an anti-counter\n\r");

	/*
	 * Check each location for the inverted pattern and zero it.
	 */
	for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
	    anti_pattern = ~pattern;
	    temp = start[offset];
	    if (temp != anti_pattern) {
		printf ("FAILURE (read/write): @ 0x%.8lx:"
			" expected 0x%.8lx, actual 0x%.8lx)\r\n",
			(ulong)&start[offset], anti_pattern, temp);
		return 1;
	    }
	    start[offset] = 0;
	}
	
	printf ("OK: memory integrity (anti-counter)\n\r");
	printf ("OK: bus line, address line and integrity are OK\n\r\n\r");
	printf ("Now it will continue to check integrity with various patterns. (Ctrl+C to exit)...\n\r");

	return mem_test_integrity(_start,_end,pattern_unused);

}
#else
/**
* Small test that check different patterns on the whole range of memory
* only check integrity (holding some values), not shortcut address!
**/
int mem_test(ulong _start, ulong _end, ulong pattern)
{

	printf ("\rTesting Range: 0x%08lX"
			" > 0x%08lX\n\r",
			_start, _end);

	return mem_test_integrity(_start,_end,pattern);
	
}
#endif



