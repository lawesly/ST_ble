/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : BLE_Chat_main.c
* Author             : AMS - VMA RF Application Team
* Version            : V1.1.0
* Date               : 15-January-2016
* Description        : BlueNRG-1 main file for Chat demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/** @addtogroup BlueNRG1_demonstrations_applications
 * BlueNRG-1 Chat demo \see BLE_Chat_main.c for documentation.
 * 12/7:添加16进制代码回调机制；
 *      添加AT指令回调机制；
 *      添加几个简单的AT命令
 * 12/8：添加默认状态；
 *       添加几个FLASH操作
 * 12/9:修改机制，设置指令格式为AT+[CMD]=指令内容\n
 *
 * 12/10:添加新的机制：当指令内容为?时，表示查询。其他为输入的指令
 *
 *   BUG:断开之后不能从新开启广播；
 *修复了几个我也看不懂的BUG
 *BUG：主机模式下，开启扫描，出现大概率不响应情况
 *未添加低功耗模式
 *
 *@{
 */
/** @} */
/** \cond DOXYGEN_SHOULD_SKIP_THIS
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h"
#include "bluenrg1_stack.h"
#include "gp_timer.h"
#include "app_state.h"
#include "chat.h"
#include "SDK_EVAL_Config.h"
#include "OTA_btl.h"
#include "sleep.h"

/* Private typedef -----------------------------------------------------------*/
#include "AtProcess.h"
#include "Configinf.h"
/* Private define ------------------------------------------------------------*/

/* Enable the Standby - SLEEPMODE_NOTIMER */
#if 0
void sleep(void)
{
  uint8_t ret, wakeup_source, wakeup_level;

  wakeup_source = WAKEUP_IO13|WAKEUP_IO11;
  wakeup_level = (WAKEUP_IOx_LOW << WAKEUP_IO13_SHIFT_MASK) | (WAKEUP_IOx_LOW << WAKEUP_IO11_SHIFT_MASK);

  printf("\r\nEnter to Standby - SLEEPMODE_NOTIMER\r\n");
  ret = BlueNRG_Sleep(SLEEPMODE_NOTIMER, wakeup_source, wakeup_level);
  if (ret != BLE_STATUS_SUCCESS) {
    printf("BlueNRG_Sleep() error 0x%02x\r\n", ret);
    while(1);
  }
  printWakeupSource();
}
//test
#endif

int main(void)
{
    SystemInit();
    //Load configuration information
    if(((uint32_t)(*(uint32_t *)CONFIG_FLAGADDR) == CONFIG_VALID))
    {
        LoadConfig();
    }
    else
    {
        ReadIdFromFlash();
        gConfigINFO = gDefaultConfigINFO;
        SaveConfig();
    }

    SdkEvalIdentification();

    Clock_Init();

    SdkEvalComIOConfig(Process_InputData);

    BleInit(gConfigINFO.BleWorkMode);

    printf("BLE Stack Initialized \n");
    //从机默认开启广播//
    if(gConfigINFO.BleWorkMode == 0x01)
        Start_Adv();
    //else
    //aci_gap_start_general_discovery_proc(0x10, 0x2000, PUBLIC_ADDR, 0x00);

    while(1)
    {
        NVIC_DisableIRQ(UART_IRQn);
        BTLE_StackTick();
        NVIC_EnableIRQ(UART_IRQn);
        // APP_Tick();
    }
}

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
*/
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif

/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
/** \endcond
 */
