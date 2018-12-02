/*
 * main.c
 * 修改日期：2014
 * 修改者：
 * 修改内容：为SD项目要求向FLASH不同地址处加多个程序的需求进行改动
 * 引导代码烧写部分不变
 * 应用代码烧写部分，改为循环操作由数组bootCodeArray表征最多7个数组的首地址，由PROGRAM_COUNT
 * 表征要烧写几段应用代码，
 *
 * 修改日期：2014-1-17
 * 考虑到校验时buf1_nor与bootCode等冲突的问题，将SPI_NOR_TEST_SIZE缩减到1MB，MSMC地址
 * 移到0xc300000
 */

#include"c6678.h"
#include "spiBootCode.h"

/********************************************************************
 * 可修改的全局变量
 *******************************************************************/
#define SPI_NOR_TEST_SIZE_512k (512*1024)//512k
#define SPI_NOR_TEST_SIZE (1*1024*1024)//由于L3只有4MB,所以每个只有2MB
#define NOR_RECV_BUF_ADDR  0xc100000
#define SPI_NOR_SECTOR_COUNT       256      /* Total number of data sectors on the NOR */
#define SPI_NOR_SECTOR_SIZE        65536    /* Number of bytes in a data sector */

//第一套：自检程序，共1~23套程序，如：对应程序编号，自检1-1，
#define PROGRAM_COUNT_START             1  //烧写的应用代码数量（每块代码占MB）
#define PROGRAM_COUNT_END               1
#define SLOT_IO			5			//接口板槽位号
int file_byte_count[14] =  {137896, 742856, 115120, 281752, 281752,
		 	 	 	 		137896, 742856, 115120, 742856, 742856,
		 	 	 	 		137896, 742856, 115120, 137896};
#define Maxwords_Read  0x400000 //Freak读bin文件的字数,最大256k
Bool test_nor(void);

int main(void)
{
	int index;
	NOR_STATUS      nor_status;
	unsigned int * Program_Start_Addr = (unsigned int *)0x80000000;
	/* Get the core number. */
	Uint32 coreNum = CSL_chipReadReg (CSL_CHIP_DNUM);
	if(coreNum==0)
	{
		//初始化PLL
	    if (C6678_Pll_Init(PLATFORM_PLL1_PLLM_val)!= TRUE)
	    {
	    	printf("PLL failed to initialize!!!!!!!!!!!!!!!!!!!!!!!!! \n" );
	    }
	    else
	    {
	    	printf("PLL successd to initialize\n" );
	    }
	    /**/

	    C6678_Power_UpDomains();
	    C6678_Ecc_Enable();
	}
	//使能TIMER
	C6678_TimeCounter_Enable();
	C6678_Spi_FPGA_Init();
	//测试SPI_FPGA
	Uint32 g_uiSlotId = C6678_Spi_FPGA_GetSlotId();
	Uint32 DDR_type = DDR3_TYPE_2BANK_8Gb_8P_1600_3U ;
	if(g_uiSlotId == SLOT_IO)
	{
		/********************************************************************
		 * DDR参数 更新
		 *******************************************************************/
		DDR_type = DDR3_TYPE_1BANK_4Gb_4P_1600;
	}
	//初始化DDR
	if (C6678_Ddr3_Init(PLLM_DDR3,DDR_type)!= TRUE)
	{
		printf("*******DDR3 failed to initialize!******* \n" );
	}
	else
	{
		printf("*******DDR3 initialize ok !******* \n" );
	}

	if(coreNum==0)
	{
		//初始化SPI_NORFLASH
		CSL_semReleaseSemaphore (SPI_SW_SEM);
		CSL_semReleaseSemaphore (RADAR_SPI_HW_SEM);

		nor_status=C6678_Spi_Norflash_Init();
		if (nor_status != NOR_EOK)
		{
			printf("SPI_NORFLASH failed to initialize!!!!!!!!!!!!!!!!!!!!!!!!! \n" );
		}
		else
		{
			printf("SPI_NORFLASH successd to initialize! \n" );
		}
	}

	//程序起始地址0x80000000,前0-14按照512k空间为单位递增，15-22按1M空间为单位递增
//	DDR内存空间，应用程序，bootsel值
//	0x80000000,1,自检程序
//	0x80100000,2
//	0x80200000,3
//	0x80300000,4
//	0x80400000,5
//	0x80500000,6
//	0x80600000,7, 网络UDP程序
//	0x80700000,8
//	0x80800000,9
//	0x80900000,10
//	0x80A00000,11
//	0x80B00000,12
//	0x80C00000,13
//	0x80D00000,14
//	0x80e00000.15, 网络加载TCP程序
//	共可用14套程序空间和引导程序

	//********************清零代码区********************
	unsigned int i;
	for(i=0;i<4*1024*1024;i++)
	{
		Program_Start_Addr[i] = 0;
	}
	C6678_TimeCounter_Delaycycles(100000000);

	//开始测试
	if (coreNum ==0)
	{

		printf("\n烧写1段引导代码和%d段应用代码.\n每段代码占据地址%dKB.\n如果有误请修改相应宏.\n"
				,(PROGRAM_COUNT_END-PROGRAM_COUNT_START+1),SPI_NOR_TEST_SIZE_512k/1024);
		printf("\n烧写第%d段到第%d段应用代码.\n",PROGRAM_COUNT_START,PROGRAM_COUNT_END);
		printf("共需擦除块数：%d.\n",(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_END-PROGRAM_COUNT_START+1)/SPI_NOR_SECTOR_SIZE));
		C6678_TimeCounter_Delaycycles(1000000000);
		if(PROGRAM_COUNT_START<=15)
		{
			for(index=(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_START)/SPI_NOR_SECTOR_SIZE);index<(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_END+1)/SPI_NOR_SECTOR_SIZE);index++)
			{
				//擦除SPI_NORFLASH
				C6678_Spi_Norflash_Erase(index);
				printf("已擦除第%d块……\n",index);
			}
		}
		else
		{
			for(index=128+(SPI_NOR_TEST_SIZE*(PROGRAM_COUNT_START-15-1)/SPI_NOR_SECTOR_SIZE);index<128+(SPI_NOR_TEST_SIZE*(PROGRAM_COUNT_END+1-15-1)/SPI_NOR_SECTOR_SIZE);index++)
			{
				//擦除SPI_NORFLASH
				C6678_Spi_Norflash_Erase(index);
				printf("已擦除第%d块……\n",index);
			}
		}

		//测试NOR
		if (test_nor()!= TRUE)
		{
			printf("应用代码烧写失败!!!!!!!!!!!!!!!!!!!!!!!!! \n" );
		}
		else
		{
			printf("应用代码烧写成功！O(∩_∩)O哈哈~ ！！! \n" );
		}

	}

    while(1)
	{
    	C6678_TimeCounter_Delaycycles(5000);
	}
}



Bool test_nor(void)
{
	uint8_t * buf1_nor;
	uint8_t * buf2_nor;
	Uint32 sector,error_cnt;

	unsigned int RealReadBin_Words=0;//fread实际读取的bin文件字数
   //开始测试SPI_NORFLASH
	buf1_nor=(uint8_t *)0x80000000;
	buf2_nor=(uint8_t *)0x90000000;//用于校验烧写正确性
//	PROGRAM_COUNT_START,PROGRAM_COUNT_END
    for(sector = (PROGRAM_COUNT_START-1);sector < PROGRAM_COUNT_END;sector ++)
    {
    	RealReadBin_Words = file_byte_count[sector];
    	printf("烧写第%d段应用代码.\n",sector+1);
    	/* 写入SPI_NORFLASH（起始地址：0 setors；长度：1 setors） */
    	if(sector < 15)
    	{
        	C6678_Spi_Norflash_Write(SPI_NOR_TEST_SIZE_512k*(sector+1), SPI_NOR_TEST_SIZE_512k,
        																(uint8_t *)(0x80000000+sector*SPI_NOR_TEST_SIZE_512k));
    	}
    	else
    	{
        	C6678_Spi_Norflash_Write(SPI_NOR_TEST_SIZE_512k*(1+15)+SPI_NOR_TEST_SIZE*(sector-15), SPI_NOR_TEST_SIZE,
        												(uint8_t *)(0x80000000+SPI_NOR_TEST_SIZE_512k*(15)+(sector-15)*0x100000));
    	}
    	if(sector < 15)
    	{
    		C6678_Spi_Norflash_Read(SPI_NOR_TEST_SIZE_512k*(sector+1), SPI_NOR_TEST_SIZE_512k,
        																(uint8_t *)buf2_nor);
    	}
    	else
    	{
    		C6678_Spi_Norflash_Read(SPI_NOR_TEST_SIZE_512k*(1+15)+SPI_NOR_TEST_SIZE*(sector-15), SPI_NOR_TEST_SIZE,
        												(uint8_t *)buf2_nor);
    	}
    	if(sector < 15)
		{
			error_cnt = memcmp(buf2_nor,(uint8_t *)(0x80000000+sector*SPI_NOR_TEST_SIZE_512k),SPI_NOR_TEST_SIZE_512k);

		}
		else
		{
			error_cnt = memcmp(buf2_nor,(uint8_t *)(0x80000000+SPI_NOR_TEST_SIZE_512k*(15)+(sector-15)*0x100000),SPI_NOR_TEST_SIZE);

		}

    	if(error_cnt != 0)
    	{
			printf("校验出错!错误数量:%d.\n",error_cnt);
			return 0;
    	}

    }
	return 1;
}





