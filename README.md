# M031BSP_FMC_IAP_IN_SRAM_LDROM
 M031BSP_FMC_IAP_IN_SRAM_LDROM


udpate @ 2024/04/13

- scenario : 

	- verify checksum and update flow in LDROM

	- app01 binary exist in block1 (flash address : 0x0000) , app02 binary exist in block2 (flash address : 0xF000)
	
	- when power on from LDROM
	
		- calculate the block2 checksum (size :60K - 4)  and compare with checksum pre-stored in this binary last 4 bytes address 
	
		- checksum pre-store by SRECORD , check generateChecksum.bat
	
		- if compare block2 checksum is valid , will erase block1 flash by page and copy firmware from block2 to write in block1

		- when firmware update finish , will perform chip reset and re-start firmware to running on the new firmware
		
		- if block2 checksum is same as block1 checksum , continue to run APP code in APROM

2. use ICP tool , to programming config : boot from LDROM

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/ICP_config.jpg)


3. program LDROM project in LDROM area , 

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/ICP_program_LDROM.jpg)


4. under KEIL project , there are 2 project setting : APP1 , APP2

	- build APP1 and programming and programming to flash : 0xF000 by ICP tool 

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/ICP_program_APROM_block2.jpg)

	- build APP2 and programmin by KEIL directly ( will program to flash address : 0x0000)
	
5. when power on , if compare checksum (block1 , block2) is different , perform update firmware flow

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/checksum_compare_different.jpg)


6. when power on after update firmware ,  

if block1 checksum same as block2 , contine to run block1 normal program

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/checksum_compare_same.jpg)

7. below is flash allocation 

![image](https://github.com/released/M031BSP_FMC_IAP_IN_SRAM_LDROM/blob/main/flash_allocation.jpg)


