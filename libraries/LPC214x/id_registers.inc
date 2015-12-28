/* Hardware ID */
//We write hardware identifiers here, in its own memory page. The chosen page is 16kiB below the top of
//LPC2148 flash. The top 12kiB (0x7D000-0x7FFFF) is IAP, so there is a total of 4kiB available here.
#define HW_ID_BASE_ADDR 0x0007C000
//The first describes the hardware type:
// 0 for Logomatic
// 1 for Loginator
// 2 for Rocketometer
// 3 for simulator
// remainder to 0xFFFFFFFE are reserved
// 0xFFFFFFFF is unknown (memory block never written to)
ro0(id,HW_TYPE  ,HW_ID_BASE_ADDR+0x00);
//The second word is a serial number, unique to hardware type. Therefore there
//can be a Loginator 0 and Rocketometer 0, but no two Rocketometers are
//both labeled 0. Again, 0xFFFFFFFF is unknown.
ro0(id,HW_SERIAL,HW_ID_BASE_ADDR+0x04);

#ifndef HW_TYPE_LOGOMATIC
#define HW_TYPE_LOGOMATIC    0
#define HW_TYPE_LOGINATOR    1
#define HW_TYPE_ROCKETOMETER 2
#define HW_TYPE_SIMULATOR    3
#endif

#undef HW_ID_BASE_ADDR
