/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : chat.c
* Author             : AMS - VMA RF  Application team
* Version            : V1.0.0
* Date               : 30-November-2015
* Description        : This file handles bytes received from USB and the init
*                      function.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "gp_timer.h"
#include "ble_const.h"
#include "bluenrg1_stack.h"
#include "app_state.h"
#include "osal.h"
#include "gatt_db.h"
#include "chat.h"
#include "SDK_EVAL_Config.h"
#include "OTA_btl.h"

#include "Configinf.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#define CMD_BUFF_SIZE 512

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint8_t connInfo[20];
volatile int app_flags = SET_CONNECTABLE;
volatile uint16_t connection_handle = 0;

/**
  * @brief  Handle of TX,RX  Characteristics.
  */
#ifdef CLIENT
uint16_t tx_handle;
uint16_t rx_handle;
#endif

/* UUIDs */
UUID_t UUID_Tx;
UUID_t UUID_Rx;

static char cmd[CMD_BUFF_SIZE];

/* Private function prototypes -----------------------------------------------*/
//modify12/7
Current_status_t DiscoveryDevice;
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : CHAT_DeviceInit.
* Description    : Init the Chat device.
* Input          : none.
* Return         : Status.
*******************************************************************************/
uint8_t CHAT_DeviceInit(uint8_t workmode)
{

    uint8_t ret;
    uint8_t role;
    uint16_t service_handle;
    uint16_t dev_name_char_handle;
    uint16_t appearance_char_handle;
    uint8_t name[] = {'B', 'l', 'u', 'e', 'N', 'R', 'G', '1'};

    /* Configure Public address */
    ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, gConfigINFO.MacAddr);
    if(ret != BLE_STATUS_SUCCESS)
    {
        printf("Setting BD_ADDR failed: 0x%02x\r\n", ret);
        return ret;
    }

    /* Set the TX power to -2 dBm */
    aci_hal_set_tx_power_level(1, gConfigINFO.uSendPoewr);

    /* GATT Init */
    ret = aci_gatt_init();
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf ("Error in aci_gatt_init(): 0x%02x\r\n", ret);
        return ret;
    }
    else
    {
        printf ("aci_gatt_init() --> SUCCESS\r\n");
    }

    /* GAP Init */
    role = workmode;
    ret = aci_gap_init(role, 0x00, 0x08, &service_handle,
                       &dev_name_char_handle, &appearance_char_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf ("Error in aci_gap_init() 0x%02x\r\n", ret);
        return ret;
    }
    else
    {
        printf ("aci_gap_init() --> SUCCESS\r\n");
    }

    /* Set the device name */
    ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle,
                                     0, sizeof(name), name);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf ("Error in Gatt Update characteristic value 0x%02x\r\n", ret);
        return ret;
    }
    else
    {
        printf ("aci_gatt_update_char_value() --> SUCCESS\r\n");
    }

#if  SERVER
    ret = Add_Chat_Service();
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error in Add_Chat_Service 0x%02x\r\n", ret);
        return ret;
    }
    else
    {
        printf("Add_Chat_Service() --> SUCCESS\r\n");
    }

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    ret = OTA_Add_Btl_Service();
    if(ret == BLE_STATUS_SUCCESS)
        printf("OTA service added successfully.\n");
    else
        printf("Error while adding OTA service.\n");
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */

#endif

    return BLE_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name  : Process_InputData.
* Description    : Process a command. It should be called when data are received.
* Input          : data_buffer: data address.
*	Nb_bytes: number of received bytes.
* Return         : none.
*******************************************************************************/
void Process_InputData(uint8_t *data_buffer, uint16_t Nb_bytes)
{
    static uint16_t end = 0;
    uint8_t i;
	
    for (i = 0; i < Nb_bytes; i++)
    {
        if(end >= CMD_BUFF_SIZE - 1)
        {
            end = 0;
        }
			 //
			 cmd[end]=data_buffer[i];
			 end++;
		  //�жϽ�����//
        if(cmd[end - 1] == '\n')
        {
            if(end != 1)
            {
                int j = 0;
							//�жϱ�־
							memcpy(gCmdGotFromUart_st.aCmdBuff,cmd,end-1);
							if((cmd[0]=='A')&(cmd[1]=='T')&(cmd[2]=='+'))
								{
									//��ָ��Ƶ����//
			           Osal_MemCpy(gCmdGotFromUart_st.aCmdBuff,&cmd[2],10);
									
									
									//noNONONONONONONONNONONO
                 while(j < end)
                {
                    uint32_t len = MIN(20, end - j);
                    //while(aci_gatt_update_char_value(chatServHandle,TXCharHandle,0,len,(uint8_t *)cmd+j)==BLE_STATUS_INSUFFICIENT_RESOURCES);
                    AtLineProcess(cmd + j);
                    j += len;
                }
								
								
								
            
							  }
								else
								__NOP();
						
						}
            end = 0;
        }
    }
}


/*******************************************************************************
* Function Name  : Make_Connection.
* Description    : If the device is a Client create the connection. Otherwise puts
*                  the device in discoverable mode.
* Input          : none.
* Return         : none.
*******************************************************************************/
void Make_Connection(void)
{
    tBleStatus ret;
    uint8_t i;


#if CLIENT

    tBDAddr bdaddr = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02};

    ret = aci_gap_create_connection(0x4000, 0x4000, PUBLIC_ADDR, bdaddr, PUBLIC_ADDR, 40, 40, 0, 60, 2000 , 2000);
    if (ret != BLE_STATUS_SUCCESS)
    {
        printf("Error while starting connection: 0x%04x\r\n", ret);
        Clock_Wait(100);
    }

#else

    uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME};

    for(i = 1; i < strlen((char const *)gConfigINFO.DviceName.ucModName) + 1; i++)
    {
        local_name[i] = gConfigINFO.DviceName.ucModName[i - 1];
    }
    local_name[i] = '\0';


#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    hci_le_set_scan_response_data(18, BTLServiceUUID4Scan);
#else
    hci_le_set_scan_response_data(0, NULL);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */

    ret = aci_gap_set_discoverable(ADV_IND, gConfigINFO.Adv_Interval_Min, gConfigINFO.Adv_Interval_Max, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                   //sizeof(local_name),
                                   strlen((char const *)local_name), local_name, 0, NULL, gConfigINFO.Con_Interval_Min, gConfigINFO.Con_Interval_Max);
    if (ret != BLE_STATUS_SUCCESS)
        printf ("Error in aci_gap_set_discoverable(): 0x%02x\r\n", ret);
    else
        printf ("aci_gap_set_discoverable() --> SUCCESS\r\n");


#endif
}

/*******************************************************************************
* Function Name  : APP_Tick.
* Description    : Tick to run the application state machine.
* Input          : none.
* Return         : none.
*******************************************************************************/
void APP_Tick(void)
{
#if 0
    if(gSystemMode_st.Mode == SerialMod )
    {


    }
    else if( gSystemMode_st.Mode == CmdMod )
    {


    }

#endif

}/* end APP_Tick() */


/* ***************** BlueNRG-1 Stack Callbacks ********************************/

/*******************************************************************************
 * Function Name  : hci_le_connection_complete_event.
 * Description    : This event indicates that a new connection has been created.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)

{
    connection_handle = Connection_Handle;

    APP_FLAG_SET(CONNECTED);

#if REQUEST_CONN_PARAM_UPDATE
    APP_FLAG_CLEAR(L2CAP_PARAM_UPD_SENT);
    Timer_Set(&l2cap_req_timer, CLOCK_SECOND * 2);
#endif
}/* end hci_le_connection_complete_event() */

/*******************************************************************************
 * Function Name  : hci_disconnection_complete_event.
 * Description    : This event occurs when a connection is terminated.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void hci_disconnection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Reason)
{
    APP_FLAG_CLEAR(CONNECTED);
    /* Make the device connectable again. */
    APP_FLAG_SET(SET_CONNECTABLE);
    APP_FLAG_CLEAR(NOTIFICATIONS_ENABLED);
    APP_FLAG_CLEAR(TX_BUFFER_FULL);

    APP_FLAG_CLEAR(START_READ_TX_CHAR_HANDLE);
    APP_FLAG_CLEAR(END_READ_TX_CHAR_HANDLE);
    APP_FLAG_CLEAR(START_READ_RX_CHAR_HANDLE);
    APP_FLAG_CLEAR(END_READ_RX_CHAR_HANDLE);

#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    OTA_terminate_connection();
#endif

}/* end hci_disconnection_complete_event() */


/*******************************************************************************
 * Function Name  : aci_gatt_attribute_modified_event.
 * Description    : This event occurs when an attribute is modified.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    OTA_Write_Request_CB(Connection_Handle, Attr_Handle, Attr_Data_Length, Attr_Data);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */

    Attribute_Modified_CB(Attr_Handle, Attr_Data_Length, Attr_Data);
}

#if CLIENT

/*******************************************************************************
 * Function Name  : aci_gatt_notification_event.
 * Description    : This event occurs when a notification is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_notification_event(uint16_t Connection_Handle,
                                 uint16_t Attribute_Handle,
                                 uint8_t Attribute_Value_Length,
                                 uint8_t Attribute_Value[])
{
#if CLIENT
    uint16_t attr_handle;

    attr_handle = Attribute_Handle;
    if(attr_handle == tx_handle + 1)
    {
        for(int i = 0; i < Attribute_Value_Length; i++)
            printf("%c", Attribute_Value[i]);
    }
#endif
}

/*******************************************************************************
 * Function Name  : aci_gatt_disc_read_char_by_uuid_resp_event.
 * Description    : This event occurs when a discovery read characteristic by UUID response.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_disc_read_char_by_uuid_resp_event(uint16_t Connection_Handle,
        uint16_t Attribute_Handle,
        uint8_t Attribute_Value_Length,
        uint8_t Attribute_Value[])
{
    printf("aci_gatt_disc_read_char_by_uuid_resp_event, Connection Handle: 0x%04X\n", Connection_Handle);
    if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
    {
        tx_handle = Attribute_Handle;
        printf("TX Char Handle 0x%04X\n", tx_handle);
    }
    else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
    {
        rx_handle = Attribute_Handle;
        printf("RX Char Handle 0x%04X\n", rx_handle);
    }
}

/*******************************************************************************
 * Function Name  : aci_gatt_proc_complete_event.
 * Description    : This event occurs when a GATT procedure complete is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_proc_complete_event(uint16_t Connection_Handle,
                                  uint8_t Error_Code)
{
    if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
    {
        printf("aci_GATT_PROCEDURE_COMPLETE_Event for START_READ_TX_CHAR_HANDLE \r\n");
        APP_FLAG_SET(END_READ_TX_CHAR_HANDLE);
    }
    else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
    {
        printf("aci_GATT_PROCEDURE_COMPLETE_Event for START_READ_TX_CHAR_HANDLE \r\n");
        APP_FLAG_SET(END_READ_RX_CHAR_HANDLE);
    }
}

#endif /* CLIENT */

/*******************************************************************************
 * Function Name  : aci_gatt_tx_pool_available_event.
 * Description    : This event occurs when a TX pool available is received.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_tx_pool_available_event(uint16_t Connection_Handle,
                                      uint16_t Available_Buffers)
{
    /* It allows to notify when at least 2 GATT TX buffers are available */
    APP_FLAG_CLEAR(TX_BUFFER_FULL);
}

/*******************************************************************************
 * Function Name  : aci_gatt_read_permit_req_event.
 * Description    : This event is given when a read request is received
 *                  by the server from the client.
 * Input          : See file bluenrg1_events.h
 * Output         : See file bluenrg1_events.h
 * Return         : See file bluenrg1_events.h
 *******************************************************************************/
void aci_gatt_read_permit_req_event(uint16_t Connection_Handle,
                                    uint16_t Attribute_Handle,
                                    uint16_t Offset)
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    /* Lower/Higher Applications with OTA Service */
    aci_gatt_allow_read(Connection_Handle);
#endif /* ST_OTA_FIRMWARE_UPGRADE_SUPPORT */
}

void aci_hal_end_of_radio_activity_event(uint8_t Last_State,
        uint8_t Next_State,
        uint32_t Next_State_SysTime)
{
#if ST_OTA_FIRMWARE_UPGRADE_SUPPORT
    if (Next_State == 0x02) /* 0x02: Connection event slave */
    {
        OTA_Radio_Activity(Next_State_SysTime);
    }
#endif
}
