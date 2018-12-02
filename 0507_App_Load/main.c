/*
 * main.c
 * �޸����ڣ�2014
 * �޸��ߣ�
 * �޸����ݣ�ΪSD��ĿҪ����FLASH��ͬ��ַ���Ӷ�������������иĶ�
 * ����������д���ֲ���
 * Ӧ�ô�����д���֣���Ϊѭ������������bootCodeArray�������7��������׵�ַ����PROGRAM_COUNT
 * ����Ҫ��д����Ӧ�ô��룬
 *
 * �޸����ڣ�2014-1-17
 * ���ǵ�У��ʱbuf1_nor��bootCode�ȳ�ͻ�����⣬��SPI_NOR_TEST_SIZE������1MB��MSMC��ַ
 * �Ƶ�0xc300000
 */

#include"c6678.h"
#include "spiBootCode.h"

/********************************************************************
 * ���޸ĵ�ȫ�ֱ���
 *******************************************************************/
#define SPI_NOR_TEST_SIZE_512k (512*1024)//512k
#define SPI_NOR_TEST_SIZE (1*1024*1024)//����L3ֻ��4MB,����ÿ��ֻ��2MB
#define NOR_RECV_BUF_ADDR  0xc100000
#define SPI_NOR_SECTOR_COUNT       256      /* Total number of data sectors on the NOR */
#define SPI_NOR_SECTOR_SIZE        65536    /* Number of bytes in a data sector */

//��һ�ף��Լ���򣬹�1~23�׳����磺��Ӧ�����ţ��Լ�1-1��
#define PROGRAM_COUNT_START             1  //��д��Ӧ�ô���������ÿ�����ռMB��
#define PROGRAM_COUNT_END               1
#define SLOT_IO			5			//�ӿڰ��λ��
int file_byte_count[14] =  {137896, 742856, 115120, 281752, 281752,
		 	 	 	 		137896, 742856, 115120, 742856, 742856,
		 	 	 	 		137896, 742856, 115120, 137896};
#define Maxwords_Read  0x400000 //Freak��bin�ļ�������,���256k
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
		//��ʼ��PLL
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
	//ʹ��TIMER
	C6678_TimeCounter_Enable();
	C6678_Spi_FPGA_Init();
	//����SPI_FPGA
	Uint32 g_uiSlotId = C6678_Spi_FPGA_GetSlotId();
	Uint32 DDR_type = DDR3_TYPE_2BANK_8Gb_8P_1600_3U ;
	if(g_uiSlotId == SLOT_IO)
	{
		/********************************************************************
		 * DDR���� ����
		 *******************************************************************/
		DDR_type = DDR3_TYPE_1BANK_4Gb_4P_1600;
	}
	//��ʼ��DDR
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
		//��ʼ��SPI_NORFLASH
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

	//������ʼ��ַ0x80000000,ǰ0-14����512k�ռ�Ϊ��λ������15-22��1M�ռ�Ϊ��λ����
//	DDR�ڴ�ռ䣬Ӧ�ó���bootselֵ
//	0x80000000,1,�Լ����
//	0x80100000,2
//	0x80200000,3
//	0x80300000,4
//	0x80400000,5
//	0x80500000,6
//	0x80600000,7, ����UDP����
//	0x80700000,8
//	0x80800000,9
//	0x80900000,10
//	0x80A00000,11
//	0x80B00000,12
//	0x80C00000,13
//	0x80D00000,14
//	0x80e00000.15, �������TCP����
//	������14�׳���ռ����������

	//********************���������********************
	unsigned int i;
	for(i=0;i<4*1024*1024;i++)
	{
		Program_Start_Addr[i] = 0;
	}
	C6678_TimeCounter_Delaycycles(100000000);

	//��ʼ����
	if (coreNum ==0)
	{

		printf("\n��д1�����������%d��Ӧ�ô���.\nÿ�δ���ռ�ݵ�ַ%dKB.\n����������޸���Ӧ��.\n"
				,(PROGRAM_COUNT_END-PROGRAM_COUNT_START+1),SPI_NOR_TEST_SIZE_512k/1024);
		printf("\n��д��%d�ε���%d��Ӧ�ô���.\n",PROGRAM_COUNT_START,PROGRAM_COUNT_END);
		printf("�������������%d.\n",(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_END-PROGRAM_COUNT_START+1)/SPI_NOR_SECTOR_SIZE));
		C6678_TimeCounter_Delaycycles(1000000000);
		if(PROGRAM_COUNT_START<=15)
		{
			for(index=(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_START)/SPI_NOR_SECTOR_SIZE);index<(SPI_NOR_TEST_SIZE_512k*(PROGRAM_COUNT_END+1)/SPI_NOR_SECTOR_SIZE);index++)
			{
				//����SPI_NORFLASH
				C6678_Spi_Norflash_Erase(index);
				printf("�Ѳ�����%d�顭��\n",index);
			}
		}
		else
		{
			for(index=128+(SPI_NOR_TEST_SIZE*(PROGRAM_COUNT_START-15-1)/SPI_NOR_SECTOR_SIZE);index<128+(SPI_NOR_TEST_SIZE*(PROGRAM_COUNT_END+1-15-1)/SPI_NOR_SECTOR_SIZE);index++)
			{
				//����SPI_NORFLASH
				C6678_Spi_Norflash_Erase(index);
				printf("�Ѳ�����%d�顭��\n",index);
			}
		}

		//����NOR
		if (test_nor()!= TRUE)
		{
			printf("Ӧ�ô�����дʧ��!!!!!!!!!!!!!!!!!!!!!!!!! \n" );
		}
		else
		{
			printf("Ӧ�ô�����д�ɹ���O(��_��)O����~ ����! \n" );
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

	unsigned int RealReadBin_Words=0;//freadʵ�ʶ�ȡ��bin�ļ�����
   //��ʼ����SPI_NORFLASH
	buf1_nor=(uint8_t *)0x80000000;
	buf2_nor=(uint8_t *)0x90000000;//����У����д��ȷ��
//	PROGRAM_COUNT_START,PROGRAM_COUNT_END
    for(sector = (PROGRAM_COUNT_START-1);sector < PROGRAM_COUNT_END;sector ++)
    {
    	RealReadBin_Words = file_byte_count[sector];
    	printf("��д��%d��Ӧ�ô���.\n",sector+1);
    	/* д��SPI_NORFLASH����ʼ��ַ��0 setors�����ȣ�1 setors�� */
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
			printf("У�����!��������:%d.\n",error_cnt);
			return 0;
    	}

    }
	return 1;
}





