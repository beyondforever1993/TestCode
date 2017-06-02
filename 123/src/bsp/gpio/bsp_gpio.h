#ifndef    __BSP_GPIO_H
#define    __BSP_GPIO_H

/**********************LED*****************************/
#define BRD_LED_POWER_CONNECTED_PORT        (3)
#define BRD_LED_POWER_CONNECTED_PIN         (31ul)

#define BRD_LED_BD_CONNECTED_PORT           (3)//(1)
#define BRD_LED_BD_CONNECTED_PIN            (30)//(13)

#define BRD_LED_GPRS_CONNECTED_PORT         (3)//(1)
#define BRD_LED_GPRS_CONNECTED_PIN          (29)//(18)

#define BRD_LED_ONLINE_CONNECTED_PORT       (3)
#define BRD_LED_ONLINE_CONNECTED_PIN        (28)

#define BRD_LED_DATA1_PORT                  (3)//(1)
#define BRD_LED_DATA1_PIN                   (27)

#define BRD_LED_DATA2_PORT                  (3)//(1)
#define BRD_LED_DATA2_PIN                   (23)

/**********************GPRS*****************************/
//Modle control preset
#define BRD_SW_GPRS_ON_PORT                (3)
#define BRD_SW_GPRS_ON_PIN                 (26)

#define BRD_SW_GPRS_RST_PORT               (3)
#define BRD_SW_GPRS_RST_PIN                (25)

//Power switch preset
#define BRD_POWER_SW_GPRS_PORT             (3)
#define BRD_POWER_SW_GPRS_PIN              (24)

/**********************Sensor**********************/
#define BRD_8V_PWR_PORT                     (1)//
#define BRD_8V_PWR_PIN                      (12)

#define BRD_5V_PWR_PORT                     (0)//
#define BRD_5V_PWR_PIN                      (19)

#define BRD_12V_PWR_PORT                    (0)//
#define BRD_12V_PWR_PIN                     (18)

#define BRD_SEN_PWR_PORT                    (3)//
#define BRD_SEN_PWR_PIN                     (22)

#define BRD_PWM_IN_PORT                     (2)//
#define BRD_PWM_IN_PIN                      (6)
#define BRD_PWM_IN_FUNC                     (1)

#define BRD_PWM_OUT_PORT                    (2)//
#define BRD_PWM_OUT_PIN                     (0)
#define BRD_PWM_OUT_FUNC                    (1)

#define BRD_485_OE_PORT                     (1)
#define BRD_485_OE_PIN                      (19)

#define BRD_RAIN_PORT                       (0)
#define BRD_RAIN_PIN                        (4)

#define BRD_AD0_PORT                        (0)
#define BRD_AD0_PIN                         (23)
#define BRD_AD0_FUNC                        1

#define BRD_AD1_PORT                        (0)
#define BRD_AD1_PIN                         (24)
#define BRD_AD1_FUNC                        1

#define BRD_AD2_PORT                        (0)
#define BRD_AD2_PIN                         (25)
#define BRD_AD2_FUNC                        1

#define BRD_AD3_PORT                        (0)
#define BRD_AD3_PIN                         (26)
#define BRD_AD3_FUNC                        1

/**********************»»√ÙµÁ◊Ë**********************/
#define TSENSOR_OUT_ST_PORT                 1
#define TSENSOR_OUT_ST_PIN                  15

#define TSENSOR_OUT_TT_PORT                 1 
#define TSENSOR_OUT_TT_PIN                  16


#define TSENSOR_IN_CAP_PORT                 1
#define TSENSOR_IN_CAP_PIN                  14
#define TSENSOR_IN_CAP_FUNC                 (3)


/**********************UART0*****************************/
#define UART0_TX_PORT                       0
#define UART0_TX_PIN                        2
#define UART0_TX_FUNC                       1

#define UART0_RX_PORT                       0
#define UART0_RX_PIN                        3
#define UART0_RX_FUNC                       1

/**********************UART1*****************************/
#define UART1_TX_PORT                       0
#define UART1_TX_PIN                        15
#define UART1_TX_FUNC                       1

#define UART1_RX_PORT                       0
#define UART1_RX_PIN                        16
#define UART1_RX_FUNC                       1

/**********************UART2*****************************/
#define UART2_TX_PORT                       0
#define UART2_TX_PIN                        10
#define UART2_TX_FUNC                       1

#define UART2_RX_PORT                       0
#define UART2_RX_PIN                        11
#define UART2_RX_FUNC                       1

/**********************UART3*****************************/
#define UART3_TX_PORT                       4
#define UART3_TX_PIN                        28
#define UART3_TX_FUNC                       2

#define UART3_RX_PORT                       4
#define UART3_RX_PIN                        29
#define UART3_RX_FUNC                       2

/**********************I2C1*****************************/
#define I2C1_SCL_PORT                       0
#define I2C1_SCL_PIN                        1

#define I2C1_SDA_PORT                       0
#define I2C1_SDA_PIN                        0

/**********************SPI*****************************/
#define FLASH_WP_PORT                       2
#define FLASH_WP_PIN                        25
#define FLASH_HOLD_PORT                     2
#define FLASH_HOLD_PIN                      24

#define FLASH_CS_PORT                       2
#define FLASH_CS_PIN                        23
#define FLASH_MOSI_PORT                     2
#define FLASH_MOSI_PIN                      27
#define FLASH_MISO_PORT                     2
#define FLASH_MISO_PIN                      26
#define FLASH_CLK_PORT                      2
#define FLASH_CLK_PIN                       22

#endif
