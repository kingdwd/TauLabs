/**
 ******************************************************************************
 * @file       board_hw_defs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @author     Virtual Robotix Network Team, http://www.virtualrobotix.com Copyright (C) 2013.
 * @addtogroup VRBrain VRBrain System
 * @{
 * @addtogroup VRBrain VRBrain Core
 * @{
 * @brief Defines board specific static initializers for hardware for the VRBrain board.
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <pios_config.h>
#include <pios_board_info.h>

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>
static const struct pios_led pios_leds[] = {
		[PIOS_LED_RED] = {
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin   = GPIO_Pin_15,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
		[PIOS_LED_GREEN] = {
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin   = GPIO_Pin_13,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
		[PIOS_LED_YELLOW] = {
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin   = GPIO_Pin_14,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
		[PIOS_LED_1] = {// OUT1
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin   = GPIO_Pin_2,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
		[PIOS_LED_2] = {// OUT2
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin   = GPIO_Pin_3,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
		[PIOS_LED_3] = {// OUT3
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin   = GPIO_Pin_4,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
#if defined(PIOS_INCLUDE_CH4_OUT) // OUT4 or ADC0(PA0)
		[PIOS_LED_4] = {
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin   = GPIO_Pin_5,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.remap = 0,
			.active_high = true,
		},
#endif	/* PIOS_INCLUDE_CH4_OUT */
	};

static const struct pios_led_cfg pios_led_cfg = {
	.leds     = pios_leds,
	.num_leds = NELEMENTS(pios_leds),
};

const struct pios_led_cfg * PIOS_BOARD_HW_DEFS_GetLedCfg (uint32_t board_revision)
{
	return &pios_led_cfg;
}

#endif	/* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_SPI)
#include <pios_spi_priv.h>
void PIOS_SPI_baro_flash_irq_handler(void);
void DMA2_Stream0_IRQHandler(void) __attribute__((alias("PIOS_SPI_baro_flash_irq_handler")));
void DMA2_Stream3_IRQHandler(void) __attribute__((alias("PIOS_SPI_baro_flash_irq_handler")));
static const struct pios_spi_cfg pios_spi_baro_flash_cfg = {
		.regs = SPI1,
		.remap = GPIO_AF_SPI1,
		.init   = {
			.SPI_Mode              = SPI_Mode_Master,
			.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
			.SPI_DataSize          = SPI_DataSize_8b,
			.SPI_NSS               = SPI_NSS_Soft,
			.SPI_FirstBit          = SPI_FirstBit_MSB,
			.SPI_CRCPolynomial     = 7,
			.SPI_CPOL              = SPI_CPOL_High,
			.SPI_CPHA              = SPI_CPHA_2Edge,
			.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
		},
		.use_crc = false,
		.dma = {
			.irq = {
				.flags   = (DMA_IT_TCIF0 | DMA_IT_TEIF0 | DMA_IT_HTIF0),
				.init    = {
					.NVIC_IRQChannel                   = DMA2_Stream0_IRQn,
					.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
					.NVIC_IRQChannelSubPriority        = 0,
					.NVIC_IRQChannelCmd                = ENABLE,
				},
			},

			.rx = {
				.channel = DMA2_Stream0,
				.init    = {
	                .DMA_Channel            = DMA_Channel_3,
					.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
					.DMA_DIR                = DMA_DIR_PeripheralToMemory,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_Medium,
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
	                /* .DMA_FIFOThreshold */
	                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
	                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
			.tx = {
				.channel = DMA2_Stream3,
				.init    = {
	                .DMA_Channel            = DMA_Channel_3,
					.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
					.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_High,
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
	                /* .DMA_FIFOThreshold */
	                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
	                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
		},
		.sclk = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_5,
				.GPIO_Speed = GPIO_Speed_100MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_UP
			},
		},
		.miso = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_6,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_UP
			},
		},
		.mosi = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_7,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_UP
			},
		},
		.slave_count = 2,
		.ssel = { {
				.gpio = GPIOE,  // CS_MS5611
				.init = {
					.GPIO_Pin   = GPIO_Pin_0,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				}},{
					.gpio = GPIOE, // CS_AT45DB161D
					.init = {
						.GPIO_Pin   = GPIO_Pin_12,
						.GPIO_Speed = GPIO_Speed_50MHz,
						.GPIO_Mode  = GPIO_Mode_OUT,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd = GPIO_PuPd_UP
					} },
		}
	};

static uint32_t pios_spi_baro_flash_id;
void PIOS_SPI_baro_flash_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_baro_flash_id);
}


/* SPI2 Interface
 *      - Used for gyro communications
 */
void PIOS_SPI_GYRO_irq_handler(void);
void DMA1_Stream3_IRQHandler(void) __attribute__((alias("PIOS_SPI_gyro_irq_handler")));
void DMA1_Stream4_IRQHandler(void) __attribute__((alias("PIOS_SPI_gyro_irq_handler")));
static const struct pios_spi_cfg pios_spi_gyro_cfg = {
		.regs = SPI2,
		.remap = GPIO_AF_SPI2,
		.init = {
			.SPI_Mode              = SPI_Mode_Master,
			.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
			.SPI_DataSize          = SPI_DataSize_8b,
			.SPI_NSS               = SPI_NSS_Soft,
			.SPI_FirstBit          = SPI_FirstBit_MSB,
			.SPI_CRCPolynomial     = 7,
			.SPI_CPOL              = SPI_CPOL_High,
			.SPI_CPHA              = SPI_CPHA_2Edge,
			.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
		},
		.use_crc = false,
		.dma = {
			.irq = {
				// Note this is the stream ID that triggers interrupts (in this case RX)
				.flags = (DMA_IT_TCIF3 | DMA_IT_TEIF3 | DMA_IT_HTIF3),
				.init = {
					.NVIC_IRQChannel = DMA1_Stream3_IRQn,
					.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
					.NVIC_IRQChannelSubPriority = 0,
					.NVIC_IRQChannelCmd = ENABLE,
				},
			},

			.rx = {
				.channel = DMA1_Stream3,
				.init = {
					.DMA_Channel            = DMA_Channel_0,
					.DMA_PeripheralBaseAddr = (uint32_t) & (SPI2->DR),
					.DMA_DIR                = DMA_DIR_PeripheralToMemory,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_Medium,
					//TODO: Enable FIFO
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
	                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
	                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
	                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
			.tx = {
				.channel = DMA1_Stream4,
				.init = {
					.DMA_Channel            = DMA_Channel_0,
					.DMA_PeripheralBaseAddr = (uint32_t) & (SPI2->DR),
					.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_Medium,
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
	                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
	                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
	                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
		},
		.sclk = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_13,
				.GPIO_Speed = GPIO_Speed_100MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.miso = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_14,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.mosi = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_15,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.slave_count = 1,
		.ssel = { {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin = GPIO_Pin_10,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_OUT,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
			},
		} },
	};

uint32_t pios_spi_gyro_id;
void PIOS_SPI_gyro_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_gyro_id);
}


/*
 * SPI3 Interface
 * Used for Flash and the RFM22B
 */
void PIOS_SPI_telem_irq_handler(void);
void DMA1_Stream0_IRQHandler(void) __attribute__((alias("PIOS_SPI_telem_irq_handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__((alias("PIOS_SPI_telem_irq_handler")));
static const struct pios_spi_cfg pios_spi_telem_cfg = {
	.regs = SPI3,
	.remap = GPIO_AF_SPI3,
	.init = {
		.SPI_Mode              = SPI_Mode_Master,
		.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize          = SPI_DataSize_8b,
		.SPI_NSS               = SPI_NSS_Soft,
		.SPI_FirstBit          = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial     = 7,
		.SPI_CPOL              = SPI_CPOL_Low,
		.SPI_CPHA              = SPI_CPHA_1Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
	},
	.use_crc = false,
	.dma = {
		.irq = {
			// Note this is the stream ID that triggers interrupts (in this case RX)
			.flags = (DMA_IT_TCIF0 | DMA_IT_TEIF0 | DMA_IT_HTIF0),
			.init = {
				.NVIC_IRQChannel = DMA1_Stream0_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},

		.rx = {
			.channel = DMA1_Stream0,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_PeripheralToMemory,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				//TODO: Enable FIFO
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
		.tx = {
			.channel = DMA1_Stream5,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
	},
	.sclk = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.miso = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.mosi = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_12,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.slave_count = 1,
	.ssel = {
		{      // RFM22b
		.gpio = GPIOD,
		.init = {
			.GPIO_Pin = GPIO_Pin_0,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_UP
		} },
	},
};

uint32_t pios_spi_telem_id;
void PIOS_SPI_telem_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_telem_id);
}

#if defined(PIOS_INCLUDE_RFM22B)
#include <pios_rfm22b_priv.h>

static const struct pios_exti_cfg pios_exti_rfm22b_cfg __exti_config = {
	.vector = PIOS_RFM22_EXT_Int,
	.line = EXTI_Line1,
	.pin = {
		.gpio = GPIOE,
		.init = {
			.GPIO_Pin = GPIO_Pin_1,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line1, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Falling,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

const struct pios_rfm22b_cfg pios_rfm22b_vrbrain_cfg = {
	.spi_cfg = &pios_spi_telem_cfg,
	.exti_cfg = &pios_exti_rfm22b_cfg,
	.RFXtalCap = 0x7f,
	.slave_num = 0,
	.gpio_direction = GPIO0_TX_GPIO1_RX,
};

const struct pios_rfm22b_cfg * PIOS_BOARD_HW_DEFS_GetRfm22Cfg (uint32_t board_revision)
{
			return &pios_rfm22b_vrbrain_cfg;
}

#endif /* PIOS_INCLUDE_RFM22B */

#endif /* PIOS_INCLUDE_SPI */


#if defined(PIOS_OVERO_SPI)
/* SPI3 Interface
 *      - Used for flash communications
 */
#include <pios_overo_priv.h>
void PIOS_OVERO_irq_handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__((alias("PIOS_OVERO_irq_handler")));
static const struct pios_overo_cfg pios_overo_cfg = {
	.regs = SPI3,
	.remap = GPIO_AF_SPI3,
	.init = {
		.SPI_Mode              = SPI_Mode_Slave,
		.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize          = SPI_DataSize_8b,
		.SPI_NSS               = SPI_NSS_Hard,
		.SPI_FirstBit          = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial     = 7,
		.SPI_CPOL              = SPI_CPOL_High,
		.SPI_CPHA              = SPI_CPHA_2Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
	},
	.use_crc = false,
	.dma = {
		.irq = {
			// Note this is the stream ID that triggers interrupts (in this case TX)
			.flags = (DMA_IT_TCIF7),
			.init = {
				.NVIC_IRQChannel = DMA1_Stream7_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},

		.rx = {
			.channel = DMA1_Stream0,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_PeripheralToMemory,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Circular,
				.DMA_Priority           = DMA_Priority_Medium,
				//TODO: Enable FIFO
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
				.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
				.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
				.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
		.tx = {
			.channel = DMA1_Stream7,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Circular,
				.DMA_Priority           = DMA_Priority_Medium,
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
				.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
				.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
				.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
	},
	.sclk = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.miso = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.mosi = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_12,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
	},
	.slave_count = 1,
	.ssel = { {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin = GPIO_Pin_15,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_UP
		},
	} },
};
uint32_t pios_overo_id = 0;
void PIOS_OVERO_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_OVERO_DMA_irq_handler(pios_overo_id);
}
#else

#endif /* PIOS_OVERO_SPI */





#include <pios_usart_priv.h>

#ifdef PIOS_INCLUDE_COM_TELEM
/*
 * Telemetry on main USART
 */
static const struct pios_usart_cfg pios_usart_telem_cfg = {
		.regs = USART3,
		.remap = GPIO_AF_USART3,
		.init = {
			.USART_BaudRate = 57600,
			.USART_WordLength = USART_WordLength_8b,
			.USART_Parity = USART_Parity_No,
			.USART_StopBits = USART_StopBits_1,
			.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
		},
		.irq = {
			.init = {
				.NVIC_IRQChannel = USART3_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.rx = {
			.gpio = GPIOD,
			.init = {
				.GPIO_Pin   = GPIO_Pin_9,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
		.tx = {
			.gpio = GPIOD,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
	};

#endif /* PIOS_COM_TELEM */

#ifdef PIOS_INCLUDE_GPS
/*
 * GPS USART
 */
static const struct pios_usart_cfg pios_usart_gps_cfg = {
		.regs = USART2,
		.remap = GPIO_AF_USART2,
		.init = {
			.USART_BaudRate = 57600,
			.USART_WordLength = USART_WordLength_8b,
			.USART_Parity = USART_Parity_No,
			.USART_StopBits = USART_StopBits_1,
			.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
		},
		.irq = {
			.init = {
				.NVIC_IRQChannel = USART2_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.rx = {
			.gpio = GPIOD,
			.init = {
				.GPIO_Pin   = GPIO_Pin_6,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
		.tx = {
			.gpio = GPIOD,
			.init = {
				.GPIO_Pin   = GPIO_Pin_5,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
	};

#endif /* PIOS_INCLUDE_GPS */

#ifdef PIOS_INCLUDE_COM_AUX
/*
 * AUX USART (UART label on rev2)
 */
static const struct pios_usart_cfg pios_usart_aux_cfg = {
		.regs = USART1,
		.remap = GPIO_AF_USART1,
		.init = {
			.USART_BaudRate = 57600,
			.USART_WordLength = USART_WordLength_8b,
			.USART_Parity = USART_Parity_No,
			.USART_StopBits = USART_StopBits_1,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
		},
		.irq = {
			.init = {
				.NVIC_IRQChannel = USART1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.rx = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_10,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
		.tx = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_9,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
		},
};

#endif /* PIOS_COM_AUX */

#ifdef PIOS_INCLUDE_COM_AUXSBUS
/*
 * AUX USART SBUS ( UART/ S Bus label on rev2)
 */
static const struct pios_usart_cfg pios_usart_auxsbus_cfg = {
	.regs = UART4,
	.remap = GPIO_AF_UART4,
	.init = {
		.USART_BaudRate = 57600,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = UART4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_1,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_0,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_INCLUDE_COM_AUXSBUS */

#ifdef PIOS_INCLUDE_COM_FLEXI
/*
 * FLEXI PORT
 */
static const struct pios_usart_cfg pios_usart_flexi_cfg = {
	.regs = USART3,
	.remap = GPIO_AF_USART3,
	.init = {
		.USART_BaudRate = 57600,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = USART3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_INCLUDE_COM_FLEXI */

#if defined(PIOS_INCLUDE_DSM)
/*
 * Spektrum/JR DSM USART
 */
#include <pios_dsm_priv.h>

static const struct pios_usart_cfg pios_usart1_dsm_cfg = {
		.regs = USART1,
		.remap = GPIO_AF_USART1,
		.init = {
			.USART_BaudRate = 115200,
			.USART_WordLength = USART_WordLength_8b,
			.USART_Parity = USART_Parity_No,
			.USART_StopBits = USART_StopBits_1,
			.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
			.USART_Mode = USART_Mode_Rx,
		},
		.irq = {
			.init = {
				.NVIC_IRQChannel = USART1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.rx = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_10,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
			.pin_source = GPIO_PinSource7,
		},
	};

	static const struct pios_dsm_cfg pios_dsm_aux_cfg = {
		.bind = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_10,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_OUT,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_NOPULL
			},
		},
	};

#endif	/* PIOS_INCLUDE_DSM */

#if defined(PIOS_INCLUDE_SBUS)
/*
 * S.Bus USART
 */
#include <pios_sbus_priv.h>

static const struct pios_usart_cfg pios_usart_sbus_auxsbus_cfg = {
	.regs = UART4,
	.init = {
		.USART_BaudRate            = 100000,
		.USART_WordLength          = USART_WordLength_8b,
		.USART_Parity              = USART_Parity_Even,
		.USART_StopBits            = USART_StopBits_2,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode                = USART_Mode_Rx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = UART4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_1,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_0,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL
		},
	},
};

static const struct pios_sbus_cfg pios_sbus_cfg = {
	/* Inverter configuration */
	.inv = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_3,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_Speed = GPIO_Speed_2MHz,
		},
	},
	.gpio_clk_func = RCC_AHB1PeriphClockCmd,
	.gpio_clk_periph = RCC_AHB1Periph_GPIOB,
	.gpio_inv_enable = Bit_SET,
};

#endif	/* PIOS_INCLUDE_SBUS */

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */
void PIOS_I2C_mag_adapter_ev_irq_handler(void);
void PIOS_I2C_mag_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler()
__attribute__ ((alias("PIOS_I2C_mag_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler()
__attribute__ ((alias("PIOS_I2C_mag_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_mag_adapter_cfg = {
	.regs = I2C2,
	.remap = GPIO_AF_I2C2,
	.init = {
		.I2C_Mode = I2C_Mode_I2C,
		.I2C_OwnAddress1 = 0,
		.I2C_Ack = I2C_Ack_Enable,
		.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
		.I2C_DutyCycle = I2C_DutyCycle_2,
		.I2C_ClockSpeed = 400000,	/* bits/s */
	},
	.transfer_timeout_ms = 50,
	.scl = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_10,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
		},
	},
	.sda = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_11,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
		},
	},
	.event = {
		.flags = 0,	/* FIXME: check this */
		.init = {
			.NVIC_IRQChannel = I2C2_EV_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.error = {
		.flags = 0,	/* FIXME: check this */
		.init = {
			.NVIC_IRQChannel = I2C2_ER_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
};

uint32_t pios_i2c_mag_adapter_id;
void PIOS_I2C_mag_adapter_ev_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_mag_adapter_id);
}

void PIOS_I2C_mag_adapter_er_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_ER_IRQ_Handler(pios_i2c_mag_adapter_id);
}


#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler (void);
void RTC_WKUP_IRQHandler() __attribute__ ((alias ("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
	.clksrc = RCC_RTCCLKSource_HSE_Div16, // Divide 8 Mhz crystal down to 1
	// For some reason it's acting like crystal is 16 Mhz.  This clock is then divided
	// by another 16 to give a nominal 62.5 khz clock
	.prescaler = 100, // Every 100 cycles gives 625 Hz
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = RTC_WKUP_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

void PIOS_RTC_IRQ_Handler (void)
{
	PIOS_RTC_irq_handler ();
}

#endif

#include "pios_tim_priv.h"

// Set up timers that only have inputs on APB2
static const TIM_TimeBaseInitTypeDef tim_1_time_base = {
	.TIM_Prescaler = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = 0xFFFF,
	.TIM_RepetitionCounter = 0x0000,
};

static const TIM_TimeBaseInitTypeDef tim_2_3_time_base = {
	.TIM_Prescaler = (PIOS_PERIPHERAL_APB1_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
	.TIM_RepetitionCounter = 0x0000,
};

static const TIM_TimeBaseInitTypeDef tim_8_time_base = {
	.TIM_Prescaler = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = 0xFFFF,
	.TIM_RepetitionCounter = 0x0000,
};

static const TIM_TimeBaseInitTypeDef tim_10_11_time_base = {
	.TIM_Prescaler = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
	.TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
	.timer = TIM1,
	.time_base_init = &tim_1_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM1_CC_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_2_cfg = {
	.timer = TIM2,
	.time_base_init = &tim_2_3_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_3_cfg = {
	.timer = TIM3,
	.time_base_init = &tim_2_3_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_8_cfg = {
	.timer = TIM8,
	.time_base_init = &tim_8_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM8_CC_IRQn, //TIM1_BRK_TIM8_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_10_cfg = {
	.timer = TIM10,
	.time_base_init = &tim_10_11_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM1_UP_TIM10_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_11_cfg = {
	.timer = TIM11,
	.time_base_init = &tim_10_11_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM1_TRG_COM_TIM11_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

/*
 * Pios servo configuration structures

 	OUTPUTS
	1: TIM2_CH2 (PA1)
	2: TIM2_CH3 (PA2)
	3: TIM2_CH4 (PA3)
	4: TIM3_CH2 (PB5)
	5: TIM3_CH3 (PB0)
	6: TIM3_CH4 (PB1)
	7: TIM10_CH1 (PB8)
	8: TIM11_CH1 (PB9)
*/

/**
 * Pios servo configuration structures
 */
#include <pios_servo_priv.h>
static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
		{  // servo 1
				.timer = TIM2,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_1,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource1,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 2
				.timer = TIM2,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_2,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource2,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 3
				.timer = TIM2,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_3,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource3,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 4
				.timer = TIM3,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_5,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource5,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 5
				.timer = TIM3,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_0,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource0,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 6
				.timer = TIM3,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_1,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource1,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 7
				.timer = TIM10,
				.timer_chan = TIM_Channel_1,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_8,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource8,
				},
				.remap = GPIO_AF_TIM10,
			},
			{// servo 8
				.timer = TIM11,
				.timer_chan = TIM_Channel_1,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_9,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource9,
				},
				.remap = GPIO_AF_TIM11,
			},
		};

/*
 * OUTPUTS with extra outputs on receiverport

	1:  TIM2_CH2 (PA1)
	2:  TIM2_CH3 (PA2)
	3:  TIM2_CH4 (PA3)
	4:  TIM3_CH2 (PB5)
	5:  TIM3_CH3 (PB0)
	6:  TIM3_CH4 (PB1)
	7:  TIM10_CH1(PB8)
	8:  TIM11_CH1(PB9)
	9:  TIM1_CH2 (PE11) (IN2)
	10: TIM1_CH3 (PE13) (IN3)
	11: TIM1_CH4 (PE14) (IN4)
	12: TIM8_CH1 (PC6)  (IN5)
	13: TIM8_CH2 (PC7)  (IN6)
	14: TIM8_CH3 (PC8)  (IN7)
	15: TIM8_CH4 (PC9)  (IN8)
*/

static const struct pios_tim_channel pios_tim_servoport_rcvrport_pins[] = {
		{  // servo 1
				.timer = TIM2,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_1,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource1,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 2
				.timer = TIM2,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_2,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource2,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 3
				.timer = TIM2,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOA,
					.init = {
						.GPIO_Pin = GPIO_Pin_3,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource3,
				},
				.remap = GPIO_AF_TIM2,
			},
			{// servo 4
				.timer = TIM3,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_5,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource5,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 5
				.timer = TIM3,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_0,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource0,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 6
				.timer = TIM3,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_1,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource1,
				},
				.remap = GPIO_AF_TIM3,
			},
			{// servo 7
				.timer = TIM10,
				.timer_chan = TIM_Channel_1,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_8,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource8,
				},
				.remap = GPIO_AF_TIM10,
			},
			{// servo 8
				.timer = TIM11,
				.timer_chan = TIM_Channel_1,
				.pin = {
					.gpio = GPIOB,
					.init = {
						.GPIO_Pin = GPIO_Pin_9,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource9,
				},
				.remap = GPIO_AF_TIM11,
			},
			{ // servo 9 - rc chanel 2
				.timer = TIM1,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOE,
					.init = {
						.GPIO_Pin = GPIO_Pin_11,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource11,
				},
				.remap = GPIO_AF_TIM1,
			},
			{// servo 10 - rc chanel 3
				.timer = TIM1,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOE,
					.init = {
						.GPIO_Pin = GPIO_Pin_13,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource13,
				},
				.remap = GPIO_AF_TIM1,
			},
			{// servo 11 - rc chanel 4
				.timer = TIM1,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOE,
					.init = {
						.GPIO_Pin = GPIO_Pin_14,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource14,
				},
				.remap = GPIO_AF_TIM1,
			},
			{// servo 12 - rc chanel 5
				.timer = TIM8,
				.timer_chan = TIM_Channel_1,
				.pin = {
					.gpio = GPIOC,
					.init = {
						.GPIO_Pin = GPIO_Pin_6,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource6,
				},
				.remap = GPIO_AF_TIM8,
			},
			{// servo 13 - rc chanel 6
				.timer = TIM8,
				.timer_chan = TIM_Channel_2,
				.pin = {
					.gpio = GPIOC,
					.init = {
						.GPIO_Pin = GPIO_Pin_7,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource7,
				},
				.remap = GPIO_AF_TIM8,
			},
			{// servo 14 - rc chanel 7
				.timer = TIM8,
				.timer_chan = TIM_Channel_3,
				.pin = {
					.gpio = GPIOC,
					.init = {
						.GPIO_Pin = GPIO_Pin_8,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource8,
				},
				.remap = GPIO_AF_TIM8,
			},
			{// servo 15 - rc chanel 8
				.timer = TIM8,
				.timer_chan = TIM_Channel_4,
				.pin = {
					.gpio = GPIOC,
					.init = {
						.GPIO_Pin = GPIO_Pin_9,
						.GPIO_Speed = GPIO_Speed_2MHz,
						.GPIO_Mode  = GPIO_Mode_AF,
						.GPIO_OType = GPIO_OType_PP,
						.GPIO_PuPd  = GPIO_PuPd_UP
					},
					.pin_source = GPIO_PinSource9,
				},
				.remap = GPIO_AF_TIM8,
			},
};

const struct pios_servo_cfg pios_servo_cfg = {
	.tim_oc_init = {
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = PIOS_SERVOS_INITIAL_POSITION,
		.TIM_OCPolarity = TIM_OCPolarity_High,
		.TIM_OCNPolarity = TIM_OCPolarity_High,
		.TIM_OCIdleState = TIM_OCIdleState_Reset,
		.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	},
	.channels = pios_tim_servoport_all_pins,
	.num_channels = NELEMENTS(pios_tim_servoport_all_pins),
};

const struct pios_servo_cfg pios_servo_rcvr_cfg = {
	.tim_oc_init = {
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = PIOS_SERVOS_INITIAL_POSITION,
		.TIM_OCPolarity = TIM_OCPolarity_High,
		.TIM_OCNPolarity = TIM_OCPolarity_High,
		.TIM_OCIdleState = TIM_OCIdleState_Reset,
		.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	},
	.channels = pios_tim_servoport_rcvrport_pins,
	.num_channels = NELEMENTS(pios_tim_servoport_rcvrport_pins),
};

/* 	INPUTS
	1: TIM1_CH1 (PE9)
	2: TIM1_CH2 (PE11)
	3: TIM1_CH3 (PE13)
	4: TIM1_CH4 (PE14)
	5: TIM8_CH1 (PC6)
	6: TIM8_CH2 (PC7)
	7: TIM8_CH3 (PC8)
	8: TIM8_CH4 (PC9)
*/
/*
 * PWM Inputs
 */
#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM)
#include <pios_pwm_priv.h>
static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
		{// rc chanel 1
			.timer = TIM1,
			.timer_chan = TIM_Channel_1,
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin = GPIO_Pin_9,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource9,
			},
			.remap = GPIO_AF_TIM1,
		},
		{ //rc chanel 2
			.timer = TIM1,
			.timer_chan = TIM_Channel_2,
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin = GPIO_Pin_11,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource11,
			},
			.remap = GPIO_AF_TIM1,
		},
		{//rc chanel 3
			.timer = TIM1,
			.timer_chan = TIM_Channel_3,
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin = GPIO_Pin_13,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource13,
			},
			.remap = GPIO_AF_TIM1,
		},
		{//rc chanel 4
			.timer = TIM1,
			.timer_chan = TIM_Channel_4,
			.pin = {
				.gpio = GPIOE,
				.init = {
					.GPIO_Pin = GPIO_Pin_14,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource14,
			},
			.remap = GPIO_AF_TIM1,
		},
		{//rc chanel 5
			.timer = TIM8,
			.timer_chan = TIM_Channel_1,
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin = GPIO_Pin_6,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource6,
			},
			.remap = GPIO_AF_TIM8,
		},
		{//rc chanel 6
			.timer = TIM8,
			.timer_chan = TIM_Channel_2,
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin = GPIO_Pin_7,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource7,
			},
			.remap = GPIO_AF_TIM8,
		},
		{//rc chanel 7
			.timer = TIM8,
			.timer_chan = TIM_Channel_3,
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin = GPIO_Pin_8,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource8,
			},
			.remap = GPIO_AF_TIM8,
		},
		{//rc chanel 8
			.timer = TIM8,
			.timer_chan = TIM_Channel_4,
			.pin = {
				.gpio = GPIOC,
				.init = {
					.GPIO_Pin = GPIO_Pin_9,
					.GPIO_Speed = GPIO_Speed_2MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd  = GPIO_PuPd_UP
				},
				.pin_source = GPIO_PinSource9,
			},
			.remap = GPIO_AF_TIM8,
		},
	};

const struct pios_pwm_cfg pios_pwm_cfg = {
	.tim_ic_init = {
		.TIM_ICPolarity = TIM_ICPolarity_Rising,
		.TIM_ICSelection = TIM_ICSelection_DirectTI,
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,
		.TIM_ICFilter = 0x0,
	},
	.channels = pios_tim_rcvrport_all_channels,
	.num_channels = NELEMENTS(pios_tim_rcvrport_all_channels),
};
#endif

/*
 * PPM Input
 */
#if defined(PIOS_INCLUDE_PPM)
#include <pios_ppm_priv.h>
static const struct pios_ppm_cfg pios_ppm_cfg = {
	.tim_ic_init = {
		.TIM_ICPolarity = TIM_ICPolarity_Rising,
		.TIM_ICSelection = TIM_ICSelection_DirectTI,
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,
		.TIM_ICFilter = 0x0,
		.TIM_Channel = TIM_Channel_2,
	},
	/* Use only the first channel for ppm */
	.channels = &pios_tim_rcvrport_all_channels[0],
	.num_channels = 1,
};

#endif //PPM

#if defined(PIOS_INCLUDE_GCSRCVR)
#include "pios_gcsrcvr_priv.h"
#endif	/* PIOS_INCLUDE_GCSRCVR */

#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"
#endif /* PIOS_INCLUDE_RCVR */

#if defined(PIOS_INCLUDE_USB)
#include "pios_usb_priv.h"

static const struct pios_usb_cfg pios_usb_main_cfg = {
		.irq = {
			.init    = {
				.NVIC_IRQChannel                   = OTG_FS_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
				.NVIC_IRQChannelSubPriority        = 3,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		.vsense = {
				.gpio = GPIOD,
				.init = {
					.GPIO_Pin   = GPIO_Pin_4,
					.GPIO_Speed = GPIO_Speed_25MHz,
					.GPIO_Mode  = GPIO_Mode_IN,
					.GPIO_OType = GPIO_OType_OD,
			},
		}
	};

#include "pios_usb_board_data_priv.h"
#include "pios_usb_desc_hid_cdc_priv.h"
#include "pios_usb_desc_hid_only_priv.h"
#include "pios_usbhook.h"

#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM_MSG)

#include <pios_com_msg_priv.h>

#endif /* PIOS_INCLUDE_COM_MSG */

#if defined(PIOS_INCLUDE_USB_HID)
#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
	.data_if = 0,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};
#endif /* PIOS_INCLUDE_USB_HID */

#if defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_cdc_priv.h>

const struct pios_usb_cdc_cfg pios_usb_cdc_cfg = {
	.ctrl_if = 1,
	.ctrl_tx_ep = 2,

	.data_if = 2,
	.data_rx_ep = 3,
	.data_tx_ep = 3,
};
#endif	/* PIOS_INCLUDE_USB_CDC */
