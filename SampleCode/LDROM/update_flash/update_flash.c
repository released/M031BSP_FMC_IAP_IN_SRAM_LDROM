/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

// #include "misc_config.h"

#include "update_flash.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/

// struct flag_32bit flag_UPDATE_FLASH_CTL;
// #define FLAG_UPDATE_FLASH_REVERSE0	                 		(flag_UPDATE_FLASH_CTL.bit0)
// #define FLAG_UPDATE_FLASH_REVERSE1                   		(flag_UPDATE_FLASH_CTL.bit1)
// #define FLAG_UPDATE_FLASH_REVERSE2                  		(flag_UPDATE_FLASH_CTL.bit2)
// #define FLAG_UPDATE_FLASH_REVERSE3                			(flag_UPDATE_FLASH_CTL.bit3)
// #define FLAG_UPDATE_FLASH_REVERSE4                 			(flag_UPDATE_FLASH_CTL.bit4)
// #define FLAG_UPDATE_FLASH_REVERSE5                			(flag_UPDATE_FLASH_CTL.bit5)
// #define FLAG_UPDATE_FLASH_REVERSE6                  		(flag_UPDATE_FLASH_CTL.bit6)
// #define FLAG_UPDATE_FLASH_REVERSE7                  		(flag_UPDATE_FLASH_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/


uint32_t size_addr = APP2_ADDR;

// #define VECMAP_SRAM_BASE                    				0x20000000UL
// extern void *__Vectors;                         /* see startup file */
// #define VECTPR_SRAM_LEN                                    (0x200) // 0x200
// uint8_t u8Vectors_SRAM[VECTPR_SRAM_LEN] __attribute__((section("VectorSection")));

volatile uint32_t u32FMCChecksum = 0;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/
// extern int IsDebugFifoEmpty(void);
// extern volatile uint32_t u32FMCChecksum;
// extern uint8_t verify_application_chksum(uint32_t target_addr , uint32_t target_size , uint32_t checksum_addr);


void iap_jump_to_app(void)
{
    printf("[B]Jump to <APPLICATION>\r\n");
    while(!UART_IS_TX_EMPTY(UART0));
    
    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Enable FMC ISP function */
    FMC_Open();
    /* Mask all interrupt before changing VECMAP to avoid wrong interrupt handler fetched */        
    __set_PRIMASK(1);    
    FMC_SetVectorPageAddr(APP1_ADDR);                               /* Set vector remap to APROM address 0x0      */

    // SYS->IPRST0 |= SYS_IPRST0_CHIPRST_Msk;                          /* Let CPU reset. Will boot from APROM.       */

    SYS->RSTSTS = (SYS_RSTSTS_PORF_Msk | SYS_RSTSTS_PINRF_Msk);
    FMC->ISPCTL &= ~(FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_BS_Msk);
    SCB->AIRCR = (V6M_AIRCR_VECTKEY_DATA | V6M_AIRCR_SYSRESETREQ);

}


uint32_t caculate_crc32_checksum(uint32_t start, uint32_t size)
{
    volatile uint32_t addr, data;

    // printf("HW CRC32\r\n");
    CRC_Open(CRC_32, (CRC_WDATA_RVS | CRC_CHECKSUM_RVS | CRC_CHECKSUM_COM), 0xFFFFFFFF, CRC_WDATA_32);
    
    for(addr = start; addr < (start+size) ; addr += 4){
        data = FMC_Read(addr);
        CRC_WRITE_DATA(data);
    }
    return CRC_GetChecksum();
}

void get_checksum(void)
{
    // uint32_t u32FMCChecksum = 0;
    uint32_t addr_start = APP1_ADDR;
    // uint32_t size = DATA_FLASH_OFFSET;  //0x1C000;    
    uint32_t size = APP1_SIZE;  //0x1C000;

    SYS_UnlockReg();
    FMC_Open();
    
    u32FMCChecksum = caculate_crc32_checksum(addr_start, (size - 4));//(g_apromSize - FMC_FLASH_PAGE_SIZE)
    
    FMC_Close();
    SYS_LockReg();

    printf("\r\n[B]block1=0x%08X\r\n" , u32FMCChecksum);
}


uint8_t verify_application_chksum(uint32_t target_addr , uint32_t target_size , uint32_t checksum_addr)
{
    uint32_t chksum_cal, chksum_app;
    
    // printf("Verify Checksum\r\n");
    
    chksum_cal = caculate_crc32_checksum(target_addr, target_size);//(g_apromSize - FMC_FLASH_PAGE_SIZE)
    printf("[B]addr:0x%08X,Caculated:0x%08X\r\n",target_addr ,chksum_cal);
    
    chksum_app = FMC_Read(checksum_addr);    
    printf("[B]addr:0x%08X,In APROM:0x%08X\r\n",target_addr ,chksum_app);
    
    if (chksum_cal == chksum_app) {
        // printf("Verify ........<PASS>\r\n");
        return TRUE;
    } else {
        // printf("Verify ........<FAIL>\r\n");
        return FALSE;
    }
}


int32_t update_ap_OnSRAM(void)
{
	int32_t resp_e = 0;
	int32_t resp_w = 0;
    uint32_t u32Addr = 0;
    uint32_t u32Data = 0;
    uint32_t u32Tmp = 0;
    uint32_t u32Cnt = 0;

    /*
        .erase AP1 page
        .copy AP2 data , write to AP1 

    */
    // SystemCoreClock_OnSRAM = clk_src;
	// SYS_UnlockReg_OnSRAM();
    // FMC_Open_OnSRAM();
	// FMC_ENABLE_AP_UPDATE_OnSRAM();
    
    printf("[B]START update\r\n");    
    printf(".");

    /* Mask all interrupt before changing VECMAP to avoid wrong interrupt handler fetched */    
    __set_PRIMASK(1);

	SYS_UnlockReg();
    FMC_Open();
	FMC_ENABLE_AP_UPDATE();

    u32Addr = APP1_ADDR;

    #if 0   // erase all
    for(u32Cnt = 0; u32Cnt < APP1_SIZE ; u32Cnt += FMC_FLASH_PAGE_SIZE)
    {
        resp_e = FMC_Erase(u32Addr + u32Cnt);

        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Erase failed!\r\n");
            return -1;
        }

        #if 1
        printf("[OnSRAM]addr:0x%08X erase done(%d)\r\n" ,u32Addr + u32Cnt,resp_e);
        #endif  
    }
    #endif

    for(u32Cnt = 0; u32Cnt < APP1_SIZE ; u32Cnt += 4)
    {        

        #if 1   // erase page
        if( ((u32Addr + u32Cnt)%FMC_FLASH_PAGE_SIZE) == 0)
        {
            #if 0
            printf("[OnSRAM-erase start]addr:0x%08X\r\n" ,u32Addr + u32Cnt);
            #endif 

            resp_e = FMC_Erase(u32Addr + u32Cnt);

            if (g_FMC_i32ErrCode != 0)
            {
                printf("[B-e]FMC_Erase failed!\r\n");
                return -1;
            }

            #if 0
            printf("[OnSRAM-erase done]addr:0x%08X (%d)\r\n" ,u32Addr + u32Cnt,resp_e);
            #endif    
            (void)(resp_e);          
        }
        #else
        if((u32Addr & (FMC_FLASH_PAGE_SIZE - 1)) == 0)
        {
            // resp_e = FMC_Erase(u32Addr);

            #if 1
            printf("[OnSRAM]addr:0x%08X erase done(%d)\r\n" ,u32Addr,resp_e);
            #endif
        }
        #endif

        u32Data = FMC_Read(APP2_ADDR + u32Cnt);
        #if 0   //debug
        printf("[B-r1]addr:0x%08X ,r:0x%08X\r\n",u32Addr,u32Data);
        #endif

	    resp_w = FMC_Write(u32Addr + u32Cnt,u32Data);
        if (g_FMC_i32ErrCode != 0)
        {
            printf("[B]FMC_Write failed!\r\n");
            return -1;
        }
        #if 0   //debug
        printf("[B-w]addr:0x%08X ,w:0x%08X ,status:0x%02X\r\n",u32Addr,u32Data,resp_w);
        #endif

        // read back compare        
        u32Tmp = FMC_Read(u32Addr + u32Cnt);
        if (g_FMC_i32ErrCode != 0)
        {
            printf("[B]FMC_Read failed!\r\n");
            return -1;
        }
        
        if (u32Data != u32Tmp)
        {
            printf("[B][Read/Write FAIL]\r\n");
            return -1;
        }

        #if 1   //debug
        if (u32Tmp != u32Data)
        {
            printf("[B-r2]addr:0x%08X ,w:0x%08X ,r:0x%08X,status:0x%02X ,%s\r\n",u32Addr + u32Cnt ,u32Data ,u32Tmp ,resp_w  ,(u32Tmp == u32Data) ? ("OK") : ("NG") );        
        }
        
        #endif
        
    }

    printf("\r\n");

    printf("[B]update finish,reset\r\n");
    UART_WAIT_TX_EMPTY(UART0);

    FMC_DISABLE_AP_UPDATE();
    FMC_Close();
    SYS_LockReg();

    __set_PRIMASK(0);

	SYS_UnlockReg();
    FMC_Open();
    SYS->IPRST0 |= SYS_IPRST0_CHIPRST_Msk;  //SYS_ResetChip();

    return resp_w;
}


// void Remap_ISR_OnSRAM(void)
// {
//     if (FMC_GetVECMAP() != VECMAP_SRAM_BASE)
//     {
//         printf("Before setting VECMAP=0x%x\n\r", FMC_GetVECMAP());
//         UART_WAIT_TX_EMPTY(UART0);

//         /* Mask all interrupt before changing VECMAP to avoid wrong interrupt handler fetched */
//         __set_PRIMASK(1);

//         memcpy(u8Vectors_SRAM, &__Vectors, VECTPR_SRAM_LEN);

//         /* Unlock protected register */
//         SYS_UnlockReg();

//         /* Enable FMC ISP function */
//         FMC_Open();

//         /* set VECMAP to SRAM */
//         FMC_SetVectorPageAddr((uint32_t)&u8Vectors_SRAM);


//         /* Disable FMC ISP function */
//         FMC_Close();

//         /* Lock protected Register */
//         SYS_LockReg();

//         __set_PRIMASK(0);

//         printf("After setting VECMAP=0x%x\n\r", FMC_GetVECMAP());
//     }
// }


unsigned char update_ap_check(void)
{
	uint8_t resp = 0;

	uint32_t ap2_checksum_value = 0;
	// uint32_t ap1_checksum_value = 0;
	uint32_t ap2_checksum_addr = 0;
	uint32_t ap1_checksum_addr = 0;

    SYS_UnlockReg();
    FMC_Open();
    
	ap2_checksum_addr = APP1_SIZE + APP2_SIZE - 4;
	ap1_checksum_addr = APP1_SIZE - 4;
	resp = verify_application_chksum(APP2_ADDR , APP2_SIZE - 4 , ap2_checksum_addr );

	/*
		. check AP2 checksum correction
			- if correnct , compare to AP1
				- if checksum not same as AP1 , update
				- if checksum same as AP1 , continue to main code
			- if not correct , means AP2 data corruption , continue to main code
	*/

	if (resp)
	{
		ap2_checksum_value = FMC_Read(ap2_checksum_addr);

    	printf("[B]block1:0x%08X,block2::0x%08X\r\n" , u32FMCChecksum,ap2_checksum_value);

		if (ap2_checksum_value != u32FMCChecksum)
		{			
			// Remap_ISR_OnSRAM();
			update_ap_OnSRAM();
		}
		else
		{
			printf("[B]block2 checksum same as block1\r\n");
            iap_jump_to_app();
		}
	}
    else
    {
        printf("[B]block2 comapre fail\r\n");
        resp = verify_application_chksum(APP1_ADDR , APP1_SIZE - 4 , ap1_checksum_addr );
        if (resp)
        {
            printf("[B]block1 checksum OK\r\n");
            iap_jump_to_app();
        }
        else
        {
            printf("stuck in boot loader\r\n");
        }
    }

    // FMC_Close();
    // SYS_LockReg();

    // printf("%s = 0x%08X\r\n" , __FUNCTION__ , resp);

	return resp;

}
