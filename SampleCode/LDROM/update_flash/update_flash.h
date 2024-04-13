/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "NuMicro.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
#define V6M_AIRCR_VECTKEY_DATA                      0x05FA0000UL
#define V6M_AIRCR_SYSRESETREQ                       0x00000004UL


#define APP1_ADDR                                  (FMC_APROM_BASE)
#define APP1_SIZE                                  (60*1024)   
#define APP2_ADDR                                  (APP1_ADDR+APP1_SIZE)//(60*1024)   // 0xF000
#define APP2_SIZE                                  (60*1024)   

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void get_checksum(void);
unsigned char update_ap_check(void);
