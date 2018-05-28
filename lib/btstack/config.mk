CFLAGS_bt += -Ilib/btstack/btstack/platform/freertos/
CFLAGS_bt += -Ilib/btstack/btstack/3rd-party/bluedroid/encoder/include
CFLAGS_bt += -Ilib/btstack/btstack/3rd-party/bluedroid/decoder/include
CFLAGS_bt += -Ilib/btstack/btstack/3rd-party/micro-ecc
CFLAGS_bt += -Ilib/btstack/btstack/chipset/cc256x
CFLAGS_bt += -Ilib/btstack/btstack/platform/embedded
CFLAGS_bt += -Ilib/btstack/btstack/src
CFLAGS_bt += -Ilib/btstack

CFLAGS_bt += -Ilib/btstack

CFLAGS_bt += -DREBBLE_BT_STACK=btstack


#lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/alloc.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/bitalloc.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/bitalloc-sbc.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/bitstream-decode.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/decoder-oina.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/decoder-private.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/decoder-sbc.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/dequant.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/framing.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/framing-sbc.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/oi_codec_version.c
# SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/readsamplesjoint.inc
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/synthesis-8-generated.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/synthesis-dct8.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/decoder/srce/synthesis-sbc.c

#lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_analysis.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_dct.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_dct_coeffs.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_enc_bit_alloc_mono.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_enc_bit_alloc_ste.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_enc_coeffs.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_encoder.c
SRCS_bt += lib/btstack/btstack/3rd-party/bluedroid/encoder/srce/sbc_packing.c

#btstack/chipset/cc256x/
SRCS_bt += lib/btstack/btstack/chipset/cc256x/ant_cmd.c
SRCS_bt += lib/btstack/btstack/chipset/cc256x/btstack_chipset_cc256x.c

#btstack/platform/freertos/:
SRCS_bt += lib/btstack/btstack/platform/freertos/btstack_run_loop_freertos.c
SRCS_bt += lib/btstack/btstack/platform/freertos/btstack_uart_block_freertos.c

#btstack/src/:
SRCS_bt += lib/btstack/btstack/src/ad_parser.c
SRCS_bt += lib/btstack/btstack/src/btstack_linked_list.c
SRCS_bt += lib/btstack/btstack/src/btstack_memory.c
SRCS_bt += lib/btstack/btstack/src/btstack_memory_pool.c
SRCS_bt += lib/btstack/btstack/src/btstack_ring_buffer.c
SRCS_bt += lib/btstack/btstack/src/btstack_run_loop.c
SRCS_bt += lib/btstack/btstack/src/btstack_slip.c
SRCS_bt += lib/btstack/btstack/src/btstack_tlv.c
SRCS_bt += lib/btstack/btstack/src/btstack_util.c
SRCS_bt += lib/btstack/btstack/src/hci.c
SRCS_bt += lib/btstack/btstack/src/hci_cmd.c
SRCS_bt += lib/btstack/btstack/src/hci_dump.c
SRCS_bt += lib/btstack/btstack/src/hci_transport_h4.c
SRCS_bt += lib/btstack/btstack/src/hci_transport_h5.c
SRCS_bt += lib/btstack/btstack/src/l2cap.c
SRCS_bt += lib/btstack/btstack/src/l2cap_signaling.c

#btstack/src/ble:
SRCS_bt += lib/btstack/btstack/src/ble/ancs_client.c
SRCS_bt += lib/btstack/btstack/src/ble/att_db.c
# SRCS_bt += lib/btstack/btstack/src/ble/att_db_util.c
SRCS_bt += lib/btstack/btstack/src/ble/att_dispatch.c
SRCS_bt += lib/btstack/btstack/src/ble/att_server.c
SRCS_bt += lib/btstack/btstack/src/ble/gatt_client.c
SRCS_bt += lib/btstack/btstack/src/ble/le_device_db_memory.c
# SRCS_bt += lib/btstack/btstack/src/ble/le_device_db_tlv.c
SRCS_bt += lib/btstack/btstack/src/ble/sm.c


#btstack/src/ble/gatt-service:
SRCS_bt += lib/btstack/btstack/src/ble/gatt-service/battery_service_server.c
SRCS_bt += lib/btstack/btstack/src/ble/gatt-service/device_information_service_server.c
SRCS_bt += lib/btstack/btstack/src/ble/gatt-service/hids_device.c

#btstack/src/classic:
SRCS_bt += lib/btstack/btstack/src/classic/a2dp_sink.c
SRCS_bt += lib/btstack/btstack/src/classic/a2dp_source.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp_acceptor.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp_initiator.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp_sink.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp_source.c
SRCS_bt += lib/btstack/btstack/src/classic/avdtp_util.c
SRCS_bt += lib/btstack/btstack/src/classic/avrcp.c
SRCS_bt += lib/btstack/btstack/src/classic/avrcp_controller.c
SRCS_bt += lib/btstack/btstack/src/classic/avrcp_target.c
SRCS_bt += lib/btstack/btstack/src/classic/bnep.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_cvsd_plc.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_link_key_db_memory.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_link_key_db_static.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_link_key_db_tlv.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_sbc_encoder_bluedroid.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_sbc_decoder_bluedroid.c
SRCS_bt += lib/btstack/btstack/src/classic/btstack_sbc_plc.c
SRCS_bt += lib/btstack/btstack/src/classic/device_id_server.c
SRCS_bt += lib/btstack/btstack/src/classic/goep_client.c
SRCS_bt += lib/btstack/btstack/src/classic/hfp_ag.c
SRCS_bt += lib/btstack/btstack/src/classic/hfp.c
SRCS_bt += lib/btstack/btstack/src/classic/hfp_gsm_model.c
SRCS_bt += lib/btstack/btstack/src/classic/hfp_hf.c
SRCS_bt += lib/btstack/btstack/src/classic/hfp_msbc.c
SRCS_bt += lib/btstack/btstack/src/classic/hid_device.c
SRCS_bt += lib/btstack/btstack/src/classic/hsp_ag.c
SRCS_bt += lib/btstack/btstack/src/classic/hsp_hs.c
SRCS_bt += lib/btstack/btstack/src/classic/obex_iterator.c
SRCS_bt += lib/btstack/btstack/src/classic/pan.c
SRCS_bt += lib/btstack/btstack/src/classic/pbap_client.c
SRCS_bt += lib/btstack/btstack/src/classic/rfcomm.c
SRCS_bt += lib/btstack/btstack/src/classic/sdp_client.c
# SRCS_bt += lib/btstack/btstack/src/classic/sdp_client_rfcomm.c
SRCS_bt += lib/btstack/btstack/src/classic/sdp_client_rfcomm.h
SRCS_bt += lib/btstack/btstack/src/classic/sdp_server.c
SRCS_bt += lib/btstack/btstack/src/classic/sdp_util.c
SRCS_bt += lib/btstack/btstack/src/classic/spp_server.c

#btstack/3rd-party/micro-ecc/:
#SRCS_bt += lib/btstack/btstack/3rd-party/micro-ecc/asm_arm.inc
#SRCS_bt += lib/btstack/btstack/3rd-party/micro-ecc/asm_avr.inc
SRCS_bt += lib/btstack/btstack/3rd-party/micro-ecc/uECC.c



# LDFLAGS_bt = $(LDFLAGS_stm32f4xx)
# LIBS_bt = $(LIBS_stm32f4xx)


