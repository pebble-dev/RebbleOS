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
#include "btstack_event.h"
#include "gap.h"
#include "pebble_bluetooth_gatt.h"
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

//#define CLASSIC

static uint16_t  rfcomm_channel_id;
static uint8_t   spp_service_buffer[98];
static uint8_t device_id_sdp_service_buffer[100];
static uint8_t gatt_service_buffer[70];
static uint8_t   _tx_via_server;
static hci_con_handle_t att_con_handle;
static uint8_t txseq = 0;
static uint8_t _service_found = 0;

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
hci_con_handle_t connection_handle;
gatt_client_characteristic_t characteristic;
static gatt_client_notification_t notification_listener;
static int listener_registered;
static void _remote_service_scan_start();

bd_addr_t _conected_phone_addr;
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

static void att_server_notify_connection_update(void);
static void _pairing_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;
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
static void _hci_main_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static int btstack_remote_connected(void);
static int btstack_gatt_send_now(void);


static uint8_t _att_char_send = 0;
static uint8_t *_att_char_send_buf;
static uint16_t _att_char_send_sz;

enum ppogatt_cmd {
    PPOGATT_CMD_DATA = 0,
    PPOGATT_CMD_ACK = 1,
    PPOGATT_CMD_RESET_REQ = 2,
    PPOGATT_CMD_RESET_ACK = 3,
};

typedef struct pebble_conn_parameters_t {
    /* each byte pair is in reverse byte order */
    uint8_t unk0;
    uint16_t interval_slots;
    uint16_t slave_latency_conn_events;
    uint16_t supervision_timeout;
} __attribute__((__packed__)) pebble_conn_parameters;

typedef struct connectivity_t {
    uint8_t connected:1;
    uint8_t paired:1;
    uint8_t encrypted:1;
    uint8_t bondGateway:1;
    uint8_t pinning:1;
    uint8_t stalePair:1;
    uint8_t unk_0:1;
    uint8_t unk_1:1;
    uint8_t unk_2;
    uint16_t error_code;
} __attribute__((__packed__)) connectivity;

static connectivity pebble_connectivity;
static pebble_conn_parameters pebble_parameters = {
    .unk0 = 0,
    .interval_slots = 0x9c00, // 156 (195ms)
    .slave_latency_conn_events = 0,
    .supervision_timeout = 0x5802 // 600 (6000ms)
};

/* Initial encryption keys */
static const uint8_t ir[16] = {1};
static const uint8_t er[16] = {1};

/* The advertising data for the manufacturer data is this format
typedef struct {
    uint8_t unk0;
    uint8_t serial[12];
    uint8_t model;
    uint8_t colour;
    uint8_t version_major;
    uint8_t version_minor;
    uint16_t padding;
} __attribute__((__packed__)) manufacturer_data;

static const manufacturer_data manu_data = {
    .unk0 = 0,
    .serial = "A301234B00AA",
    .model = 0x08, // 0x08 = snowy, 0x0e = silk
    .colour = 0x19,
    .version_major = 4,
    .version_minor = 3,
    .padding = 0
};
*/

#define DEVICE_ADVERTISEMENT_NAME 'R', 'e', 'b', 'b', 'l', 'e', ' ', '0', '1', '2', '3'
#define DEVICE_SERIAL_NUMBER      'A', '3', '0', '1', '2', '3', '4', 'B', '0', '0', 'A', 'A'
#define DEVICE_MODEL_NUMBER       0x0e
#define DEVICE_COLOR              0x19 // 0x19 SnowyRed
#define FIRMWARE_MAJOR_VERSION    4
#define FIRMWARE_MINOR_VERSION    3

static const uint8_t adv_data[] = {
    /* LEN (including CMD), CMD, DATA... */
    /* Flags general discoverable, BR/EDR not supported */
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, 
    0x0C, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, DEVICE_ADVERTISEMENT_NAME,
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0xD9, 0xFE,
};

#ifdef CLASSIC
static const uint8_t adv_data_ext[] = {
    //0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x01, /* set to LE limited discovery */
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 0xff, 0xca, 0xca, 0xde, 0xaf, 0xde, 0xca, 0xde, 0xde, 0xfa, 0xca, 0xde, 0x00, 0x00, 0x00, 0x00,
    0x15, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'e', 'b', 'b', 'l', 'e', ' ', 'T', 'i', 'm', 'e', ' ', 'L',  'E', ' ', '0', '1', '2', '4', '\0',
    0x0d, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x00, 0x10, 0x01, 0x11, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x58, 0x00,
    0x16, BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x54, 0x01, 0x00, 'A', '3', '0', '1', '2', '3', '4', 'B', '0', '0', 'A', 'A', 0x08, 0x0c, 0x04, 0x03, 0x00, 0x00,
};

#endif

const uint8_t scan_resp_data[] = {
        0x16, BLUETOOTH_DATA_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x54, 0x01, 0x00, DEVICE_ADVERTISEMENT_NAME, DEVICE_MODEL_NUMBER, DEVICE_COLOR, FIRMWARE_MAJOR_VERSION, FIRMWARE_MINOR_VERSION, 0x00, 0x00,
        0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0xD9, 0xFE,
};


/* Bluetooth UART speed configuguration */
static const hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200, /* start slow */
    460800,  /* This is pretty fast, but can we go faster? */
    1,  /* Use hardware flow control */
    NULL
};

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
#ifdef CLASSIC
    hci_set_link_key_db(btstack_link_key_db_memory_instance());
#endif
    hci_set_chipset(btstack_chipset_cc256x_instance()); // Do I need this ??
    
    hci_event_callback_registration.callback = &_hci_main_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    
    /* Init L2CAP */
    l2cap_init();
    l2cap_register_packet_handler(_hci_main_handler);
#ifdef CLASSIC    
    /* init RFCOMM serial layer */
    rfcomm_init();
    rfcomm_register_service(_hci_main_handler, RFCOMM_SERVER_CHANNEL, 0xffff);


    /* init SDP, create record for SPP and register with SDP
     * XXX we could cache this in the flash
     */
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    spp_create_sdp_record(spp_service_buffer, 0x10001, RFCOMM_SERVER_CHANNEL, "RebbleSerial");
    sdp_register_service(spp_service_buffer);
    
    /*memset(gatt_service_buffer, 0, sizeof(gatt_service_buffer));
    gatt_create_sdp_record(gatt_service_buffer, 0x10001, ATT_SERVICE_GAP_SERVICE_START_HANDLE, ATT_SERVICE_GAP_SERVICE_END_HANDLE);
    sdp_register_service(gatt_service_buffer);
    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SDP service record size: %u", de_get_len(spp_service_buffer));
*/
    //device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10003, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_PEBBLE_TECHNOLOGY, 1, 1);
    //sdp_register_service(device_id_sdp_service_buffer);
    sdp_init();

    /* Set our advertisement name 
     * We are set as Pebble Time x so the apps can see us
     */

    gap_set_local_name(BLUETOOTH_MODULE_GAP_NAME);
    gap_set_class_of_device(0x800704);

    gap_set_extended_inquiry_response(adv_data_ext);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    gap_discoverable_control(1);
#endif

    /* setup Low Energy Device Database */
    le_device_db_init();

    /* setup SM: Display only */
    sm_init();
   
    /* turns on pairing using a passkey entry */
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION);

    /* set the enc keys */
    sm_set_ir(ir);
    sm_set_er(er);

    sm_event_callback_registration.callback = &_pairing_handler;
    sm_add_event_handler(&sm_event_callback_registration);
    
    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);    
    att_server_register_packet_handler(_hci_main_handler);

    gatt_client_init();
    
    /* setup advertisements of our name and functionality */
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), (uint8_t*) adv_data);

    gap_scan_response_set_data(sizeof(scan_resp_data), scan_resp_data);
    
    //gap_set_class_of_device(0x800704);

    gap_advertisements_enable(1);
    hci_send_cmd(&hci_le_set_advertise_enable, 1);

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
    SYS_LOG("BTSPP", APP_LOG_LEVEL_ERROR, "rtx %d", len);
    
    static uint8_t buf[256];
    buf[0] = (txseq << 3) | PPOGATT_CMD_DATA;
    memcpy(buf + 1, data, len);
    _tx_buf = buf;
    _tx_buf_len = len + 1;
    /*
     * Call up BTStack and tell it to tell us we can send
     */
#ifdef CLASSIC
    if (rfcomm_channel_id) {
        rfcomm_request_can_send_now_event(rfcomm_channel_id);
    }
#endif
    if (_tx_via_server) {
        att_server_request_can_send_now_event(connection_handle);
    }
    else {
        gatt_client_request_can_write_without_response_event(handle_gatt_client_event, connection_handle);
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
//     bluetooth_tx_complete_from_isr();
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
    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "RDCB");  
    return 0;
}

/* 
 * BLE Write requests 
 */

static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "WRCB %d handle %x", transaction_mode, att_handle);
    for(int i = 0; i < buffer_size; i++) {
        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "%x ", buffer[i]);    
    }
    /* Ignore cancel sent for new connections */
    if (transaction_mode == ATT_TRANSACTION_MODE_CANCEL) return 0;
    
    /* notify incoming */
    
    if (att_handle == ATT_CHARACTERISTIC_30000004_328E_0FBB_C642_1AA6699BDADA_01_CLIENT_CONFIGURATION_HANDLE) {
        /* remote server has notify we can TX to */
        _tx_via_server = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Remote Tx available %d", _tx_via_server);
        att_con_handle = con_handle;
        txseq = 0;
        bluetooth_device_connected();
    } else if (att_handle == ATT_CHARACTERISTIC_00000001_328E_0FBB_C642_1AA6699BDADA_01_CLIENT_CONFIGURATION_HANDLE) {
        /* connectivity */
        att_con_handle = con_handle;
        _tx_via_server = 0;
        att_server_notify_connection_update();
    } else if (att_handle == ATT_CHARACTERISTIC_00000002_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE) {
        //sm_request_pairing(connection_handle);
    } else if (att_handle == ATT_CHARACTERISTIC_00000005_328E_0FBB_C642_1AA6699BDADA_01_CLIENT_CONFIGURATION_HANDLE) {      
        /* parameters */
        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "WRCB PARAMS %d", _tx_via_server);
        //att_server_notify(connection_handle, ATT_CHARACTERISTIC_00000005_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE, (uint8_t*)pebble_parameters_buf, sizeof(pebble_parameters_buf));
        
        _att_char_send = ATT_CHARACTERISTIC_00000005_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE;
        _att_char_send_buf = (uint8_t*)&pebble_parameters;
        _att_char_send_sz =sizeof(pebble_parameters);
        att_server_request_can_send_now_event(connection_handle);
        
    } else if (att_handle == ATT_CHARACTERISTIC_30000006_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE) {
        /* host wrote data to us */
        SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Write on wr characteristic: ");
        //rblprintf_hexdump(buffer, buffer_size);
        bluetooth_data_rx(buffer+1, buffer_size-1);
    }
    
    return 0;
}

static void _pairing_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);
    
    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SM: %x %x", hci_event_packet_get_type(packet), packet_type);

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Just works requested\n");
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Confirming numeric comparison: %"PRIu32"\n", sm_event_numeric_comparison_request_get_passkey(packet));
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Display Passkey: %"PRIu32"\n", sm_event_passkey_display_number_get_passkey(packet));
            break;
        case SM_EVENT_PASSKEY_INPUT_NUMBER:
            SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Passkey Input requested\n");
            sm_passkey_input(sm_event_passkey_input_number_get_handle(packet), 123456);
            break;
        case SM_EVENT_PASSKEY_DISPLAY_CANCEL:
            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "CONN");            
            break;
        case SM_EVENT_PAIRING_COMPLETE:
            switch (sm_event_pairing_complete_get_status(packet)) {
                //sm_event_pairing_complete_get_status(packet)){
                case ERROR_CODE_SUCCESS:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pairing complete, success\n");
                    pebble_connectivity.paired = 1;
                    att_server_notify_connection_update();
                    break;
                case ERROR_CODE_CONNECTION_TIMEOUT:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pairing failed, timeout\n");
                    pebble_connectivity.paired = 0;
                    att_server_notify_connection_update();
                    break;
                case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pairing failed, disconnected\n");
                    pebble_connectivity.paired = 0;
                    pebble_connectivity.connected = 0;
                    pebble_connectivity.bondGateway = 0;
                    pebble_connectivity.encrypted = 0;
                    pebble_connectivity.error_code = 100; // XXX
                    gap_disconnect(connection_handle);
                    att_server_notify_connection_update();
                    break;
                case ERROR_CODE_AUTHENTICATION_FAILURE:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pairing failed, reason = %u\n", sm_event_pairing_complete_get_reason(packet));
                    pebble_connectivity.paired = 0;
                    break;
                default:
                    break;
            }
            
            break;
        
        case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED:               
            break;
    }
}

/*
 * For each protocol type, we get a packet through this runtime handler
 * We check our state machine and then run the relevant function
 */
static void _hci_main_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    bd_addr_t event_addr;
    uint16_t  mtu;
    int i;
    uint8_t event;

    if (packet_type == HCI_EVENT_PACKET)
        event = hci_event_packet_get_type(packet);
    
    if (!bluetooth_is_enabled())
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
                bluetooth_init_complete(INIT_RESP_OK);
                bluetooth_enable();

                SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "GAP CONNECT.");
                break;
        }
        return;
    }

    switch (packet_type)
    {
        case HCI_EVENT_PACKET:
            switch (event)
            {
#ifdef CLASSIC
                case HCI_EVENT_PIN_CODE_REQUEST:
                    /* We do pin handshake */
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Pin code request - using '0000'");

                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    gap_pin_code_response(event_addr, "0000");
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "SSP User Confirmation Auto accept");
                    break;
#endif
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    _tx_via_server = 0;
                    memset(&pebble_connectivity, 0, sizeof(connectivity));
                    break;

                case HCI_EVENT_LE_META:
                    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ATT META %d %x %x", channel, hci_event_le_meta_get_subevent_code(packet), hci_subevent_le_connection_complete_get_role(event));
                    switch(hci_event_le_meta_get_subevent_code(packet)) {
                        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                            connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ATT Connected %d %d %s", packet_type, channel, bd_addr_to_str(&_conected_phone_addr));
                            if (pebble_connectivity.connected == 0) {
                                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "PAIRING");
                                pebble_connectivity.connected = 1;
                                att_server_notify_connection_update();
                                sm_request_pairing(connection_handle);
                            }
                            break;
                        case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                            connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                            break;
                    }
                    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ATT META %x", connection_handle);
                    break;
                case ATT_EVENT_CAN_SEND_NOW:
                    /* server request send. going to be initial notify */
                    if (!_tx_via_server) {
                        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "WRCB NOTIFY %x %x %x", _att_char_send, att_con_handle, connection_handle);
                        int rv = att_server_notify(connection_handle, _att_char_send, (uint8_t*)_att_char_send_buf, _att_char_send_sz);

                        if (rv != ATT_ERROR_SUCCESS) {
                            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ERROR WRCB NOTIFY %x", rv);
                        }
                        
                        break;
                    }

                    if (_tx_buf_len == 0)
                        break;

                    /* notify the remote server connection */
                    btstack_gatt_send_now();
                    
                    break;
                
#ifdef CLASSIC
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
                        bluetooth_device_connected();
                    }
                    break;

                case RFCOMM_EVENT_CAN_SEND_NOW:
                    /* We have been instructed to send data safely. We;re ready */
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM Can Send %d %d", _tx_buf, _tx_buf_len);
                    rfcomm_send(rfcomm_channel_id, _tx_buf, _tx_buf_len);
                    /* data is sent and the we can notify of tx completion. We don't do this is tx isr as the
                     * data can be broken into multiple writes. */
                    bluetooth_tx_complete_from_isr();
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "RFCOMM channel closed");
                    rfcomm_channel_id = 0;
                    bluetooth_device_disconnected();
                    break;
#endif
                case HCI_EVENT_ENCRYPTION_CHANGE: 
                    connection_handle = hci_event_encryption_change_get_connection_handle(packet);
                    SYS_LOG("BTSPP", APP_LOG_LEVEL_INFO, "Connection encrypted: %u\n", hci_event_encryption_change_get_encryption_enabled(packet));
                    if (hci_event_encryption_change_get_encryption_enabled(packet)) {
                        pebble_connectivity.encrypted = 1;
                        pebble_connectivity.bondGateway = 1;
                        pebble_connectivity.pinning = 1;
                        att_server_notify_connection_update();
                        _remote_service_scan_start();
                        pebble_connectivity.encrypted = 1;
                    }
                    pebble_connectivity.encrypted = 0;
                    break;
                
                default:
                    break;
            }
            break;
                
        case RFCOMM_DATA_PACKET:
            /* pack the packet onto the bluetooth generic handler */
            bluetooth_data_rx(packet, size);
            break;

        default:
            break;
    }
}

const char *hw_bluetooth_name() {
    return BLUETOOTH_MODULE_GAP_NAME;
}

void hw_bluetooth_advertising_visible(int vis) {
    /* XXX: not yet implemented */
    SYS_LOG("BT", APP_LOG_LEVEL_ERROR, "NYI: hw_bluetooth_advertising_visible(%d)", vis);
}

void bt_set_callback_get_bond_data(bt_callback_get_bond_data_t cbk) { 

}

void bt_set_callback_request_bond(bt_callback_request_bond_t cbk) {

}

void hw_bluetooth_bond_data_available(const void *data, size_t datalen) { }
void hw_bluetooth_bond_acknowledge(int accepted) { }


static gatt_client_service_t _le_client_service_connection;

static void _remote_service_scan_start()
{
    _service_found = 0;

    gatt_client_discover_primary_services(handle_gatt_client_event, connection_handle);
}


static int btstack_remote_connected(void)
{
   
    bluetooth_device_connected();
}

static int btstack_gatt_send_now(void)
{
   
    if (_tx_via_server) {
        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "TX using server");
        att_server_notify(att_con_handle, ATT_CHARACTERISTIC_30000004_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE, (uint8_t*)  _tx_buf, _tx_buf_len);
    } else {
        SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "TX using client");

        uint8_t status = gatt_client_write_value_of_characteristic_without_response(connection_handle, 
                        characteristic.value_handle,
                        _tx_buf_len, 
                        (uint8_t*)_tx_buf);
        if (status) {
            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "error %02x for write!", status);
            return status;
        }
    }
    /* completing here isn't great, but btstack doesn't have an easy way to track tx complete 
        that being said, the send should be synchronous */
    bluetooth_tx_complete_from_isr();
    txseq++;

    return 0;
}


static uint8_t ackbuf[1];

static void handle_gatt_send_ack(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);
    
    DRV_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Gatt ACK RX ev %x", hci_event_packet_get_type(packet));
    uint8_t rv = 0;
    switch(hci_event_packet_get_type(packet)) {
        case GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE:
            rv = gatt_client_write_value_of_characteristic_without_response(connection_handle, 
                        characteristic.value_handle,
                        1, 
                        (uint8_t*)ackbuf);
            if (rv) {
                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "error %02x on ACK!", rv);
            }
            break;
    }
}

static void request_rx_ack(uint8_t rx_seq)
{
    ackbuf[0] = (rx_seq << 3) | PPOGATT_CMD_ACK;
    gatt_client_request_can_write_without_response_event(handle_gatt_send_ack, connection_handle);
}

static uint8_t cmdbuf[2];

static void handle_gatt_send_cmd(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);
    
    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Gatt CMD RX ev %x", hci_event_packet_get_type(packet));
    uint8_t rv = 0;
    switch(hci_event_packet_get_type(packet)) {
        case GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE:
            rv = gatt_client_write_value_of_characteristic_without_response(connection_handle, 
                        characteristic.value_handle,
                        2, 
                        (uint8_t*)cmdbuf);
            if (rv) {
                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "error %02x on CMD!", rv);
            }

            break;
    }
}

static void request_remote_reset()
{
    cmdbuf[0] = PPOGATT_CMD_RESET_REQ;
    cmdbuf[1] = 0x01;
    gatt_client_request_can_write_without_response_event(handle_gatt_send_cmd, connection_handle);
}

static void request_remote_reset_ready()
{
    cmdbuf[0] = PPOGATT_CMD_RESET_ACK;
    cmdbuf[1] = 0x01;
    gatt_client_request_can_write_without_response_event(handle_gatt_send_cmd, connection_handle);
}

static void handle_gatt_client_notify_rx(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);
    
    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Gatt CMD RX ev %x", hci_event_packet_get_type(packet));
          
    switch(hci_event_packet_get_type(packet)) {
        case GATT_EVENT_NOTIFICATION:
            SYS_LOG("gatt", APP_LOG_LEVEL_INFO, "client: notify: pkt len %d", size);
            int value_handle = little_endian_read_16(packet, 4);
            int value_length = little_endian_read_16(packet, 6);
            const uint8_t * value = gatt_event_notification_get_value(packet);

            uint8_t cmd = value[0] & 7;
            uint8_t seq = value[0] >> 3;
            
            switch (cmd) {
                case PPOGATT_CMD_DATA:
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: data seq %d", seq);
                    /* ack it */
                    request_rx_ack(seq);
                    bluetooth_data_rx(value + 1, value_length - 1);                
                    break;
                case PPOGATT_CMD_ACK:
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: ack seq %d", seq);
                    break;
                case PPOGATT_CMD_RESET_REQ:
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: RESET REQ seq %d", seq);
                    break;
                case PPOGATT_CMD_RESET_ACK:
                    DRV_LOG("bt", APP_LOG_LEVEL_INFO, "rx: RESET ACK seq %d", seq);
                    /* send reset complete */
                    request_remote_reset_ready();
                    txseq = 0;
                    btstack_remote_connected();
                    break;
            }
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Connected!");
            att_server_notify_connection_update();
            request_remote_reset();

            break;
        case GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE:
            /* use our client connection to post data */
            btstack_gatt_send_now();    
            break;
    }
}

static void att_server_notify_connection_update(void)
{
    _att_char_send = ATT_CHARACTERISTIC_00000001_328E_0FBB_C642_1AA6699BDADA_01_VALUE_HANDLE;
    _att_char_send_buf = (uint8_t *)&pebble_connectivity;
    _att_char_send_sz = sizeof(connectivity);
    att_server_request_can_send_now_event(connection_handle);
}

static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);
    
    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "GE ev %x", hci_event_packet_get_type(packet));
    
    gatt_client_service_t _le_client_service;
    gatt_client_characteristic_t _client_characteristic;
    
    switch(hci_event_packet_get_type(packet)) {
        case GATT_EVENT_SERVICE_QUERY_RESULT:
            gatt_event_service_query_result_get_service(packet, &_le_client_service);
            static const uint8_t client_svc_uuid[16] = { 0x10, 0x00, 0x00, 0x00, 0x32, 0x8e, 0x0f, 0xbb, 0xc6, 0x42, 0x1a, 0xa6, 0x69, 0x9b, 0xda, 0xda };
            
            SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "found valid service %x %s, [0x%04x-0x%04x]",
                        _le_client_service.uuid16, uuid128_to_str(_le_client_service.uuid128), _le_client_service.start_group_handle, _le_client_service.end_group_handle);

            if (memcmp(&_le_client_service.uuid128, client_svc_uuid, sizeof(client_svc_uuid)) == 0 && !_service_found) {
                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "service found");
                _service_found = 1;
                _le_client_service_connection = _le_client_service;
                break;
            }
            
            break;
        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
            gatt_event_characteristic_query_result_get_characteristic(packet, &_client_characteristic);
            
            const uint8_t client_char_uuid[16] = { 0x10, 0x00, 0x00, 0x01, 0x32, 0x8e, 0x0f, 0xbb, 0xc6, 0x42, 0x1a, 0xa6, 0x69, 0x9b, 0xda, 0xda };
            //SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ble discovery: characteristic %x %s", _client_characteristic.uuid16, uuid128_to_str(_client_characteristic.uuid128));
            if (memcmp(&_client_characteristic.uuid128, client_char_uuid, sizeof(client_char_uuid)) == 0) {
                characteristic = _client_characteristic;
                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ble discovery: found characteristic %s", uuid128_to_str(characteristic.uuid128));
                _service_found = 2;
            } else {
                //SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "ble discovery: unknown characteristic %x %s", _client_characteristic.uuid16, uuid128_to_str(_client_characteristic.uuid128));
            }
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            if (_service_found == 1) {
                gatt_client_discover_characteristics_for_service(handle_gatt_client_event, connection_handle, &_le_client_service_connection);
            } else if (_service_found == 2) {
                SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "Setting up notify");
                gatt_client_listen_for_characteristic_value_updates(&notification_listener, handle_gatt_client_notify_rx, connection_handle, &characteristic);
                int rv = gatt_client_write_client_characteristic_configuration(handle_gatt_client_notify_rx, connection_handle, &characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                if (rv){
                    SYS_LOG("BTGATT", APP_LOG_LEVEL_INFO, "error %02x for notify setup!", rv);
                }
            }
            break;
        case GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE:
            /* use our client connection to post data */
            btstack_gatt_send_now();    
            break;            
    }
}
