/* btstack_rebble.c
 * The glue logic between BTStack and RebbleOS
 * RebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */
#include "btstack.h"
#include "btstack_rebble.h"
#include "btstack_chipset_cc256x.h"
#include "hal_uart_dma.h"
#include "rebbleos.h"
#include "btstack_spp.h"
#include "hal_time_ms.h"
#include "rbl_bluetooth.h"
#include "platform_config.h"

/* The standard chanel we will use for RFCOMM serial proto comms */
#define RFCOMM_SERVER_CHANNEL 1

#ifndef BLUETOOTH_MODULE_TYPE
#  error You must define BLUETOOTH_MODULE_TYPE of CC2564 or CC2564B
#else
#  if BLUETOOTH_MODULE_TYPE==BLUETOOTH_MODULE_TYPE_CC2564B
#     include "bluetooth_init_cc2564B_1.6_BT_Spec_4.1.c"
#  elif BLUETOOTH_MODULE_TYPE==BLUETOOTH_MODULE_TYPE_CC2564
#     include "bluetooth_init_cc2564_2.14.c"
#  endif
#endif

static uint16_t  rfcomm_channel_id;
static uint8_t   spp_service_buffer[98];
static uint8_t   le_notification_enabled;
static hci_con_handle_t att_con_handle;

static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static uint8_t _bt_enabled = 0;
static btstack_packet_callback_registration_t hci_event_callback_registration;

/* TX outboung buffer pointer.  */
static uint8_t *_tx_buf = NULL;
static uint8_t _tx_buf_len = 0;

/* BTStack handlers */
static void dummy_handler(void);
static void dummy_handler(void){};
static void (*rx_done_handler)(void) = &dummy_handler;
static void (*tx_done_handler)(void) = &dummy_handler;
static void (*cts_irq_handler)(void) = &dummy_handler;


int btstack_main(int argc, const char ** argv);
static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


/* Bluetooth UART speed configuguration */
static const hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200, /* start slow */
    460800,  /* This is pretty fast, but can we go faster? */
    1,  /* Use hardware flow control */
    NULL
};

/*
 * @section Advertisements 
 *
 * @text The Flags attribute in the Advertisement Data indicates if a device is in dual-mode or not.
 * Flag 0x06 indicates LE General Discoverable, BR/EDR not supported although we're actually using BR/EDR.
 * In the past, there have been problems with Anrdoid devices when the flag was not set.
 * Setting it should prevent the remote implementation to try to use GATT over LE/EDR, which is not 
 * implemented by BTstack. So, setting the flag seems like the safer choice (while it's technically incorrect).
 */
/* LISTING_START(advertisements): Advertisement data: Flag 0x06 indicates LE-only device */
static const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, 
    // Name
//    0x0b, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'R', 'e', 'b', 'b', 'l', 'e', 'O', 'S', 'L', 'E', 
    BLUETOOTH_MODULE_NAME_LENGTH + 1, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, BLUETOOTH_MODULE_LE_NAME, 
    // Incomplete List of 16-bit Service Class UUIDs -- FF10 - only valid for testing!
    0x03, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x54, 0x01 // 0x10, 0xff,
};
/* LISTING_END */

uint8_t adv_data_len = sizeof(adv_data);

/*
 * Initialise the underlying BTStack memory and configuration
 */
void bt_device_init(void)
{
#if BLUETOOTH_MODULE_TYPE==BLUETOOTH_MODULE_TYPE_NONE
    return;
#endif
    btstack_memory_init();
    /* Initialise the FREERtos runloop */
    btstack_run_loop_init(btstack_run_loop_freertos_get_instance());
    
#ifdef PACKET_LOGGING
    /* Becuase we want the HCI debug output */
#ifdef DEBUG_UART_SMARTSTRAP 
    /* Becuase we want the HCI debug output */ 
    stm32_power_request(STM32_POWER_AHB1, RCC_AHB1Periph_GPIOE); 
    stm32_power_request(STM32_POWER_APB1, RCC_APB1Periph_UART8); 
#endif 
    /* This is to enable the packet dumps */
    hci_dump_open( NULL, HCI_DUMP_STDOUT );
#endif

    /* Init HCI Layer */
    hci_init(hci_transport_h4_instance(btstack_uart_block_freertos_instance()), (void*) &config);
    hci_set_link_key_db(btstack_link_key_db_memory_instance());
    hci_set_chipset(btstack_chipset_cc256x_instance()); // Do I need this ??
    
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    
    /* Init L2CAP */
    l2cap_init();
    l2cap_register_packet_handler(packet_handler);
    
    /* init RFCOMM serial layer */
    rfcomm_init();
    rfcomm_register_service(packet_handler, RFCOMM_SERVER_CHANNEL, 0xffff);

    /* init SDP, create record for SPP and register with SDP
     * XXX we could cache this in the flash
     */
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    spp_create_sdp_record(spp_service_buffer, 0x10001, RFCOMM_SERVER_CHANNEL, "RebbleSerial");
    sdp_register_service(spp_service_buffer);
    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SDP service record size: %u", de_get_len(spp_service_buffer));
    sdp_init();

    /* Set our advertisement name 
     * We are set as Pebble Time x so the apps can see us
     */
    gap_set_local_name(BLUETOOTH_MODULE_GAP_NAME);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    gap_discoverable_control(1);

    /* setup Low Energy Device Database */
    le_device_db_init();

    /* setup SM: Display only */
    sm_init();

    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);    
    att_server_register_packet_handler(packet_handler);

    /* setup advertisements of our name and functionality */
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);

    /* Power up the bt chipset */
    hci_power_control(HCI_POWER_ON);
    
    /* Go for runloop excection. We are going to block here all being well */
    btstack_run_loop_execute();
}

/*
 * Request we send some data over bluetooth
 * This will return immediately, it wil then be sent async
 */
void bt_device_request_tx(uint8_t *data, uint16_t len)
{
    if (len > HCI_ACL_PAYLOAD_SIZE)
    {
        SYS_LOG("BTSPP", APP_LOG_LEVEL_ERROR, "Data size %d > buffer size %d", len, HCI_ACL_PAYLOAD_SIZE);
        return;
    }

    _tx_buf = data;
    _tx_buf_len = len;
    
    /*
     * Call up BTStack and tell it to tell us we can send
     */
    if (rfcomm_channel_id)
    {
        rfcomm_request_can_send_now_event(rfcomm_channel_id);
    }

    if (le_notification_enabled)
    {
        att_server_request_can_send_now_event(att_con_handle);
    }
}

/*
 * HAL-x functions are stubs that BTStack requires
 * These will proxy BTstack functionality to our own RebbleOS funcs
 */
uint32_t hal_time_ms(void)
{
    TickType_t tick =  xTaskGetTickCount();
    return tick * portTICK_RATE_MS;
}

void hal_cpu_disable_irqs(void)
{
    __disable_irq();
}

void hal_cpu_enable_irqs(void)
{
    __enable_irq();
}

void hal_cpu_enable_irqs_and_sleep(void)
{
    __enable_irq();
    __asm__("wfe"); /* go to sleep if event flag isn't set. if set, just clear it. IRQs set event flag */
}

void hal_uart_dma_set_sleep(uint8_t sleep)
{
    /* later.. */
}

void hal_uart_dma_init(void)
{
    bluetooth_power_cycle();
}

void hal_uart_dma_set_block_received( void (*the_block_handler)(void))
{
    rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void))
{
    tx_done_handler = the_block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void))
{
    if(the_irq_handler)
    {
        hw_bluetooth_enable_cts_irq();
    }
    else
    {
        hw_bluetooth_disable_cts_irq();
    }
    cts_irq_handler = the_irq_handler;
}

int  hal_uart_dma_set_baud(uint32_t baud)
{
    stm32_usart_set_baud(hw_bluetooth_get_usart(), baud);
    return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t size)
{
    stm32_usart_send_dma(hw_bluetooth_get_usart(), (uint32_t *)data, size);
}

void hal_uart_dma_receive_block(uint8_t *data, uint16_t size)
{
    stm32_usart_recv_dma(hw_bluetooth_get_usart(), (uint32_t *)data, size);
}


/* 
 * BTStack wants to reset.
 * Call up to RebbleOS to find out how to do that
 */
void bluetooth_power_cycle(void)
{
    if (hw_bluetooth_power_cycle())
        os_module_init_complete(INIT_RESP_ERROR);
    else
        os_module_init_complete(INIT_RESP_OK);
}

/*
 * USART complete messages
 */

/* TX Was completed. 
 * Call the BTStack implementation callback
 * Call the bluetooth handler tx callback
 */
void bt_stack_tx_done()
{
    (*tx_done_handler)();
    bluetooth_tx_complete_from_isr();
}

/*
 * RX is complete. Tell BTStack
 */
void bt_stack_rx_done()
{
    (*rx_done_handler)();
}

/*
 * We want to go low power and CTS is waking us up
 */
void bt_stack_cts_irq()
{
    if (cts_irq_handler)
    {
        (*cts_irq_handler)();
    }
}


/* ATT Client Read Callback for Dynamic Data
 * - if buffer == NULL, don't copy data, just return size of value
 * - if buffer != NULL, copy data and return number bytes copied
 * @param offset defines start of attribute value
 */
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    UNUSED(con_handle);

    if (att_handle == ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)
    {
        return att_read_callback_handle_blob((const uint8_t *)_tx_buf, _tx_buf_len, offset, buffer, buffer_size);
    }
    return 0;
}

/* 
 * BLE Write requests 
 */
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    /* Ignore cancel sent for new connections */
    if (transaction_mode == ATT_TRANSACTION_MODE_CANCEL) return 0;
    /* find characteristic for handle */
    switch (att_handle)
    {
        case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
            le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
            att_con_handle = con_handle;
            return 0;
        case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Write on test characteristic: ");
            printf_hexdump(buffer, buffer_size);
            return 0;
        default:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "WRITE Callback, handle %04x, mode %u, offset %u, data: ", con_handle, transaction_mode, offset);
            printf_hexdump(buffer, buffer_size);
            return 0;
    }
}

/*
 * For each protocol type, we get a packet through this runtime handler
 * We check our state machine and then run the relevant function
 */
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    bd_addr_t event_addr;
    uint16_t  mtu;
    int i;
    uint8_t event;

    if (packet_type == HCI_EVENT_PACKET)
        event = hci_event_packet_get_type(packet);
    
    if (!_bt_enabled)
    {
        switch (event)
        {
            case BTSTACK_EVENT_STATE:
                if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
                {
                        SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "BTstack Initialising....");
                        return;
                }
                SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "BTstack up and running.");
                _bt_enabled = 1;
                bluetooth_init_complete(INIT_RESP_OK);
                break;
        }
        return;
    }
    
    switch (packet_type)
    {
        case HCI_EVENT_PACKET:              
            switch (event)
            {
                case HCI_EVENT_PIN_CODE_REQUEST:
                    /* We do pin handshake */
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pin code request - using '0000'");
                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    gap_pin_code_response(event_addr, "0000");
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
    //                             SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SSP User Confirmation Request with numeric value '%06"PRIu32"'", little_endian_read_32(packet, 8));
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SSP User Confirmation Auto accept");
                    break;

                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    le_notification_enabled = 0;
                    break;

                case ATT_EVENT_CAN_SEND_NOW:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "ATT %d %d", packet_type, channel);
                    
                    if (_tx_buf_len == 0)
                        break;
                    att_server_notify(att_con_handle, ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE, (uint8_t*)  _tx_buf, _tx_buf_len);
                    break;

                case RFCOMM_EVENT_INCOMING_CONNECTION:
                    /* data: event (8), len(8), address(48), channel (8), rfcomm_cid (16) */
                    rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr); 
                    uint8_t rfcomm_channel_nr = rfcomm_event_incoming_connection_get_server_channel(packet);
                    rfcomm_channel_id = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM channel requested for %s", /*rfcomm_channel_nr, */bd_addr_to_str(event_addr));
                    rfcomm_accept_connection(rfcomm_channel_id);
                    break;
                                        
                case RFCOMM_EVENT_CHANNEL_OPENED:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM CO %d %d", packet_type, channel);
                    /* data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16) */
                    if (rfcomm_event_channel_opened_get_status(packet))
                    {
                        SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM channel open failed, status %d", rfcomm_event_channel_opened_get_status(packet));
                    }
                    else
                    {
                        rfcomm_channel_id = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
                        mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
                        SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM channel open succeeded. New RFCOMM Channel ID %d, max frame size %d", rfcomm_channel_id, mtu);
                    }
                    break;

                case RFCOMM_EVENT_CAN_SEND_NOW:
                    /* We have been instructed to send data safely. We;re ready */
                    rfcomm_send(rfcomm_channel_id, _tx_buf, _tx_buf_len);
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM channel closed");
                    rfcomm_channel_id = 0;
                    break;
                
                default:
                    break;
            }
            break;
                
        case RFCOMM_DATA_PACKET:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RCV: '");
            for (i = 0; i < size; i++)
            {
                SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "0x%x", packet[i]);
            }
            
            /* pack the packet onto the bluetooth generic handler */
            bluetooth_data_rx(packet, size);
            break;

        default:
            break;
    }
}
