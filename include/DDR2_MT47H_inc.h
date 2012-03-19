/**
* Small hacks to make the at91bootstrap works with our DDR memories
* 
* Author: Benoit RAT
*
**/
#ifndef WRS318V3
#define WRS318V3 1 //Version 3.1

#define MT47H32M16HR 0x025E
#ifdef MT47H32M16HR
	//Then define new value
	#define AT91C_DDRC2_NC_XX 	AT91C_DDRC2_NC_DDR10_SDR9	// 10 column bits (1K)
	#define AT91C_DDRC2_NR_XX 	AT91C_DDRC2_NR_13 		// 13 row bits    (8K)
	#define AT91C_DDRC2_TRRD_XX 	AT91C_DDRC2_TRRD_2 		//  2 * 7.5 > 10   ns

	#if MT47H32M16HR == 0x025E //for -25E
		#define AT91C_DDRC2_TRPA_XX 	AT91C_DDRC2_TRPA_2 	//  2 * 7.5 = 15 ns
		#define AT91C_DDRC2_TXARDS_XX 	AT91C_DDRC2_TXARDS_8 	//  
	#endif

	#if MT47H32M16HR == 0x0030 //for -3
		#define AT91C_DDRC2_TRPA_XX 	AT91C_DDRC2_TRPA_3 	//  3 * 7.5 = 22.5 ns
		#define AT91C_DDRC2_TXARDS_XX 	AT91C_DDRC2_TXARDS_7 	// 
	#endif
#else  //Original values for AT
	#define AT91C_DDRC2_NC_XX 	AT91C_DDRC2_NC_DDR10_SDR9 	// 10 column bits (1K)
	#define AT91C_DDRC2_NR_XX 	AT91C_DDRC2_NR_13
	#define AT91C_DDRC2_TRPA_XX 	AT91C_DDRC2_TRPA_0
	#define AT91C_DDRC2_TXARDS_XX 	AT91C_DDRC2_TXARDS_7
	#define AT91C_DDRC2_TRRD_XX 	AT91C_DDRC2_TRRD_1
#endif


#endif