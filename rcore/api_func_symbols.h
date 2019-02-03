#include "common.h"
#include "rebbleos.h"
#include "librebble.h"
#include "graphics_wrapper.h"
#include "battery_state_service.h"

GBitmap *gbitmap_create_with_resource_proxy(uint32_t resource_id);
bool persist_exists(void);
bool persist_exists(void) { return false; }

typedef void (*VoidFunc)(void);
typedef void (*UnimplFunc)(void);


#define UNIMPL(FN) void FN(){ SYS_LOG("API", APP_LOG_LEVEL_WARNING, "== Unimplemented: %s ==\n", __func__); }
UNIMPL(_accel_data_service_subscribe__deprecated);
UNIMPL(_accel_data_service_unsubscribe);
UNIMPL(_accel_service_peek);
UNIMPL(_accel_service_set_samples_per_update);
UNIMPL(_accel_service_set_sampling_rate);
UNIMPL(_accel_tap_service_subscribe);
UNIMPL(_accel_tap_service_unsubscribe);
UNIMPL(_action_bar_layer_legacy2_add_to_window);
UNIMPL(_action_bar_layer_legacy2_clear_icon);
UNIMPL(_action_bar_layer_legacy2_create);
UNIMPL(_action_bar_layer_legacy2_destroy);
UNIMPL(_action_bar_layer_legacy2_get_layer);
UNIMPL(_action_bar_layer_legacy2_remove_from_window);
UNIMPL(_action_bar_layer_legacy2_set_background_color_2bit);
UNIMPL(_action_bar_layer_legacy2_set_click_config_provider);
UNIMPL(_action_bar_layer_legacy2_set_context);
UNIMPL(_action_bar_layer_legacy2_set_icon);
UNIMPL(_animation_legacy2_create);
UNIMPL(_animation_legacy2_destroy);
UNIMPL(_animation_legacy2_get_context);
UNIMPL(_animation_legacy2_is_scheduled);
UNIMPL(_animation_legacy2_schedule);
UNIMPL(_animation_legacy2_set_curve);
UNIMPL(_animation_legacy2_set_delay);
UNIMPL(_animation_legacy2_set_duration);
UNIMPL(_animation_legacy2_set_handlers);
UNIMPL(_animation_legacy2_set_implementation);
UNIMPL(_animation_legacy2_unschedule);
UNIMPL(_animation_legacy2_unschedule_all);
UNIMPL(_app_comm_get_sniff_interval);
UNIMPL(_app_comm_set_sniff_interval);
UNIMPL(_app_message_deregister_callbacks);
UNIMPL(_app_message_open);
UNIMPL(_app_sync_deinit);
UNIMPL(_app_sync_get);
UNIMPL(_app_sync_init);
UNIMPL(_app_sync_set);
UNIMPL(_atan2_lookup);
UNIMPL(_atol);
UNIMPL(_bitmap_layer_set_background_color_2bit);
UNIMPL(_bluetooth_connection_service_peek);
UNIMPL(_bluetooth_connection_service_subscribe);
UNIMPL(_bluetooth_connection_service_unsubscribe);
UNIMPL(_click_number_of_clicks_counted);
UNIMPL(_click_recognizer_get_button_id);
UNIMPL(_clock_copy_time_string);
UNIMPL(_data_logging_create);
UNIMPL(_data_logging_finish);
UNIMPL(_data_logging_log);
UNIMPL(_dict_calc_buffer_size);
UNIMPL(_dict_calc_buffer_size_from_tuplets);
UNIMPL(_dict_find);
UNIMPL(_dict_merge);
UNIMPL(_dict_read_begin_from_buffer);
UNIMPL(_dict_read_first);
UNIMPL(_dict_read_next);
UNIMPL(_dict_serialize_tuplets);
UNIMPL(_dict_serialize_tuplets_to_buffer__deprecated);
UNIMPL(_dict_serialize_tuplets_to_buffer_with_iter);
UNIMPL(_dict_write_begin);
UNIMPL(_dict_write_cstring);
UNIMPL(_dict_write_data);
UNIMPL(_dict_write_end);
UNIMPL(_dict_write_int);
UNIMPL(_dict_write_int16);
UNIMPL(_dict_write_int32);
UNIMPL(_dict_write_int8);
UNIMPL(_dict_write_tuplet);
UNIMPL(_dict_write_uint16);
UNIMPL(_dict_write_uint32);
UNIMPL(_dict_write_uint8);
UNIMPL(_gmtime);
UNIMPL(_gpath_draw_filled_legacy);
UNIMPL(_gpoint_equal);
UNIMPL(_graphics_context_set_compositing_mode);
UNIMPL(_graphics_context_set_fill_color_2bit);
UNIMPL(_graphics_context_set_stroke_color_2bit);
UNIMPL(_graphics_context_set_text_color_2bit);
UNIMPL(_graphics_draw_round_rect);
UNIMPL(_graphics_text_layout_get_max_used_size);
UNIMPL(_grect_align);
UNIMPL(_grect_clip);
UNIMPL(_grect_contains_point);
UNIMPL(_grect_crop);
UNIMPL(_grect_is_empty);
UNIMPL(_grect_standardize);
UNIMPL(_gsize_equal);
UNIMPL(_inverter_layer_create);
UNIMPL(_inverter_layer_destroy);
UNIMPL(_inverter_layer_get_layer);
UNIMPL(_layer_get_clips);
UNIMPL(_layer_get_data);
UNIMPL(_layer_set_clips);
UNIMPL(_light_enable);
UNIMPL(_light_enable_interaction);
UNIMPL(_menu_layer_legacy2_create);
UNIMPL(_menu_layer_legacy2_set_callbacks__deprecated);
UNIMPL(_number_window_create);
UNIMPL(_number_window_destroy);
UNIMPL(_number_window_get_value);
UNIMPL(_number_window_set_label);
UNIMPL(_number_window_set_max);
UNIMPL(_number_window_set_min);
UNIMPL(_number_window_set_step_size);
UNIMPL(_number_window_set_value);
UNIMPL(_persist_delete);
UNIMPL(_persist_get_size);
UNIMPL(_persist_read_bool);
UNIMPL(_persist_read_data__deprecated);
UNIMPL(_persist_read_int);
UNIMPL(_persist_read_string__deprecated);
UNIMPL(_persist_write_bool);
UNIMPL(_persist_write_data__deprecated);
UNIMPL(_persist_write_int);
UNIMPL(_persist_write_string);
UNIMPL(_property_animation_legacy2_create);
UNIMPL(_property_animation_legacy2_create_layer_frame);
UNIMPL(_property_animation_legacy2_destroy);
UNIMPL(_property_animation_legacy2_update_gpoint);
UNIMPL(_property_animation_legacy2_update_grect);
UNIMPL(_property_animation_legacy2_update_int16);
UNIMPL(_psleep);
UNIMPL(_rot_bitmap_layer_create);
UNIMPL(_rot_bitmap_layer_destroy);
UNIMPL(_rot_bitmap_layer_increment_angle);
UNIMPL(_rot_bitmap_layer_set_angle);
UNIMPL(_rot_bitmap_layer_set_corner_clip_color_2bit);
UNIMPL(_rot_bitmap_set_compositing_mode);
UNIMPL(_rot_bitmap_set_src_ic);
UNIMPL(_strncat);
UNIMPL(_text_layer_legacy2_create);
UNIMPL(_text_layer_legacy2_destroy);
UNIMPL(_text_layer_legacy2_get_content_size);
UNIMPL(_text_layer_legacy2_get_layer);
UNIMPL(_text_layer_legacy2_get_text);
UNIMPL(_text_layer_legacy2_set_background_color_2bit);
UNIMPL(_text_layer_legacy2_set_font);
UNIMPL(_text_layer_legacy2_set_overflow_mode);
UNIMPL(_text_layer_legacy2_set_size);
UNIMPL(_text_layer_legacy2_set_text);
UNIMPL(_text_layer_legacy2_set_text_alignment);
UNIMPL(_text_layer_legacy2_set_text_color_2bit);
UNIMPL(_window_get_fullscreen);
UNIMPL(_window_set_background_color_2bit);
UNIMPL(_window_set_fullscreen);
UNIMPL(_window_set_status_bar_icon);
UNIMPL(_window_stack_pop_all);
UNIMPL(_app_focus_service_subscribe);
UNIMPL(_app_focus_service_unsubscribe);
UNIMPL(_app_message_get_context);
UNIMPL(_app_message_inbox_size_maximum);
UNIMPL(_app_message_outbox_begin);
UNIMPL(_app_message_outbox_send);
UNIMPL(_app_message_outbox_size_maximum);
UNIMPL(_app_message_register_inbox_dropped);
UNIMPL(_app_message_register_inbox_received);
UNIMPL(_app_message_register_outbox_failed);
UNIMPL(_app_message_register_outbox_sent);
UNIMPL(_app_message_set_context);
UNIMPL(_dict_serialize_tuplets_to_buffer);
UNIMPL(_persist_read_data);
UNIMPL(_persist_read_string);
UNIMPL(_persist_write_data);
UNIMPL(_dict_size);
UNIMPL(_n_graphics_text_layout_get_content_size);
UNIMPL(_accel_data_service_subscribe);
UNIMPL(_menu_layer_legacy2_set_callbacks);
UNIMPL(_number_window_get_window);
UNIMPL(_gbitmap_create_blank_2bit);
UNIMPL(_click_recognizer_is_repeating);
UNIMPL(_accel_raw_data_service_subscribe);
UNIMPL(_app_worker_is_running);
UNIMPL(_app_worker_kill);
UNIMPL(_app_worker_launch);
UNIMPL(_app_worker_message_subscribe);
UNIMPL(_app_worker_message_unsubscribe);
UNIMPL(_app_worker_send_message);
UNIMPL(_worker_event_loop);
UNIMPL(_worker_launch_app);
UNIMPL(_compass_service_peek);
UNIMPL(_compass_service_set_heading_filter);
UNIMPL(_compass_service_subscribe);
UNIMPL(_compass_service_unsubscribe);
UNIMPL(_uuid_equal);
UNIMPL(_uuid_to_string);
UNIMPL(_animation_legacy2_set_custom_curve);
UNIMPL(_watch_info_get_color);
UNIMPL(_watch_info_get_firmware_version);
UNIMPL(_watch_info_get_model);
UNIMPL(_graphics_capture_frame_buffer_2bit);
UNIMPL(_graphics_frame_buffer_is_captured);
UNIMPL(_clock_to_timestamp);
UNIMPL(_launch_reason);
UNIMPL(_wakeup_cancel);
UNIMPL(_wakeup_cancel_all);
UNIMPL(_wakeup_get_launch_event);
UNIMPL(_wakeup_query);
UNIMPL(_wakeup_schedule);
UNIMPL(_wakeup_service_subscribe);
UNIMPL(_clock_is_timezone_set);
UNIMPL(_i18n_get_system_locale);
UNIMPL(__localeconv_r);
UNIMPL(_setlocale);
UNIMPL(_gcolor_equal);
UNIMPL(___profiler_init);
UNIMPL(___profiler_print_stats);
UNIMPL(___profiler_start);
UNIMPL(___profiler_stop);
UNIMPL(_rot_bitmap_layer_set_corner_clip_color);
UNIMPL(_clock_get_timezone);
UNIMPL(_animation_get_context);
UNIMPL(_animation_is_scheduled);
UNIMPL(_animation_set_curve);
UNIMPL(_animation_set_custom_curve);
UNIMPL(_animation_set_delay);
UNIMPL(_animation_set_handlers);
UNIMPL(_animation_unschedule);
UNIMPL(_animation_unschedule_all);
UNIMPL(_property_animation_from);
UNIMPL(_property_animation_subject);
UNIMPL(_property_animation_to);
UNIMPL(_gbitmap_sequence_create_with_resource);
UNIMPL(_gbitmap_sequence_destroy);
UNIMPL(_gbitmap_sequence_get_bitmap_size);
UNIMPL(_gbitmap_sequence_get_current_frame_idx);
UNIMPL(_gbitmap_sequence_get_total_num_frames);
UNIMPL(_gbitmap_sequence_update_bitmap_next_frame);
UNIMPL(_animation_clone);
UNIMPL(_animation_get_delay);
UNIMPL(_animation_get_duration);
UNIMPL(_animation_get_play_count);
UNIMPL(_animation_get_elapsed);
UNIMPL(_animation_get_reverse);
UNIMPL(_animation_sequence_create);
UNIMPL(_animation_sequence_create_from_array);
UNIMPL(_animation_set_play_count);
UNIMPL(_animation_set_elapsed);
UNIMPL(_animation_set_reverse);
UNIMPL(_animation_spawn_create);
UNIMPL(_animation_spawn_create_from_array);
UNIMPL(_animation_get_curve);
UNIMPL(_animation_get_custom_curve);
UNIMPL(_animation_get_implementation);
UNIMPL(_launch_get_args);
UNIMPL(_gbitmap_sequence_get_play_count);
UNIMPL(_gbitmap_sequence_restart);
UNIMPL(_gbitmap_sequence_set_play_count);
UNIMPL(_gbitmap_sequence_update_bitmap_by_elapsed);
UNIMPL(_graphics_draw_rotated_bitmap);
UNIMPL(_difftime);
//UNIMPL(_time_ms);
UNIMPL(_gcolor_legible_over);
UNIMPL(_app_focus_service_subscribe_handlers);
UNIMPL(_action_menu_set_result_window);
UNIMPL(_dictation_session_create);
UNIMPL(_dictation_session_destroy);
UNIMPL(_dictation_session_enable_confirmation);
UNIMPL(_dictation_session_start);
UNIMPL(_dictation_session_stop);
UNIMPL(_smartstrap_attribute_begin_write);
UNIMPL(_smartstrap_attribute_create);
UNIMPL(_smartstrap_attribute_destroy);
UNIMPL(_smartstrap_attribute_end_write);
UNIMPL(_smartstrap_attribute_get_attribute_id);
UNIMPL(_smartstrap_attribute_get_service_id);
UNIMPL(_smartstrap_attribute_read);
UNIMPL(_smartstrap_service_is_available);
UNIMPL(_smartstrap_set_timeout);
UNIMPL(_smartstrap_subscribe);
UNIMPL(_smartstrap_unsubscribe);
UNIMPL(_connection_service_peek_pebble_app_connection);
UNIMPL(_connection_service_peek_pebblekit_connection);
UNIMPL(_connection_service_subscribe);
UNIMPL(_connection_service_unsubscribe);
UNIMPL(_dictation_session_enable_error_dialogs);
UNIMPL(_gbitmap_get_data_row_info);
UNIMPL(_grect_inset);
UNIMPL(_gpoint_from_polar);
UNIMPL(_graphics_draw_arc);
UNIMPL(_graphics_fill_radial);
UNIMPL(_grect_centered_from_polar);
UNIMPL(_graphics_text_attributes_create);
UNIMPL(_graphics_text_attributes_destroy);
UNIMPL(_graphics_text_attributes_enable_paging);
UNIMPL(_graphics_text_attributes_enable_screen_text_flow);
UNIMPL(_graphics_text_attributes_restore_default_paging);
UNIMPL(_graphics_text_attributes_restore_default_text_flow);
UNIMPL(_graphics_text_layout_get_content_size_with_attributes);
UNIMPL(_layer_convert_rect_to_screen);
UNIMPL(_text_layer_enable_screen_text_flow_and_paging);
UNIMPL(_health_service_activities_iterate);
UNIMPL(_health_service_any_activity_accessible);
UNIMPL(_health_service_events_subscribe);
UNIMPL(_health_service_events_unsubscribe);
UNIMPL(_health_service_get_minute_history);
UNIMPL(_health_service_metric_accessible);
UNIMPL(_health_service_peek_current_activities);
UNIMPL(_health_service_sum);
UNIMPL(_health_service_sum_today);
UNIMPL(_time_start_of_today);
UNIMPL(_health_service_metric_averaged_accessible);
UNIMPL(_health_service_sum_averaged);
UNIMPL(_health_service_get_measurement_system_for_display);
UNIMPL(_gdraw_command_frame_get_command_list);
UNIMPL(_unimpl613);
UNIMPL(_unimpl614);
UNIMPL(_unimpl615);
UNIMPL(_unimpl616);
UNIMPL(_unimpl617);
UNIMPL(_unimpl618);
UNIMPL(_unimpl619);
UNIMPL(_unimpl620);
UNIMPL(_unimpl621);
UNIMPL(_unimpl623);
UNIMPL(_unimpl624);
UNIMPL(_unimpl625);
UNIMPL(_unimpl626);
UNIMPL(_unimpl628);
UNIMPL(_unimpl629);
UNIMPL(_unimpl630);
UNIMPL(_unimpl631);

const VoidFunc sym[] = {

    [31]  = (VoidFunc)app_event_loop,                                                           // app_event_loop@0000007c
    [34]  = (VoidFunc)app_log_trace,                                                            // app_log@00000088
                                                                                                
    [47]  = (VoidFunc)app_timer_cancel,                                                         // app_timer_cancel@000000bc
    [48]  = (VoidFunc)app_timer_register,                                                       // app_timer_register@000000c0
    [49]  = (VoidFunc)app_timer_reschedule,                                                     // app_timer_reschedule@000000c4
                                                                                                
    [51]  = (VoidFunc)atoi,                                                                     // atoi@000000cc
    [53]  = (VoidFunc)battery_state_service_peek,                                               // battery_state_service_peek@000000d4
    [54]  = (VoidFunc)battery_state_service_subscribe,                                          // battery_state_service_subscribe@000000d8
    [55]  = (VoidFunc)battery_state_service_unsubscribe,                                        // battery_state_service_unsubscribe@000000dc                                                                                               
    [56]  = (VoidFunc)bitmap_layer_create,                                                      // bitmap_layer_create@000000e0
    [57]  = (VoidFunc)bitmap_layer_destroy,                                                     // bitmap_layer_destroy@000000e4
    [58]  = (VoidFunc)bitmap_layer_get_layer,                                                   // bitmap_layer_get_layer@000000e8
    [59]  = (VoidFunc)bitmap_layer_set_alignment,                                               // bitmap_layer_set_alignment@000000ec
                                                                                                
    [61]  = (VoidFunc)bitmap_layer_set_bitmap,                                                  // bitmap_layer_set_bitmap@000000f4
    [62]  = (VoidFunc)bitmap_layer_set_compositing_mode,                                        // bitmap_layer_set_compositing_mode@000000f8
          
    [69]  = (VoidFunc)pbl_clock_is_24h_style,                                                   // clock_is_24h_style@00000114
    [70]  = (VoidFunc)cos_lookup,                                                               // cos_lookup@00000118
          
    [96]  = (VoidFunc)fonts_get_system_font,                                                    // fonts_get_system_font@00000180
    [97]  = (VoidFunc)fonts_load_custom_font_proxy,                                             // fonts_load_custom_font@00000184
    [98]  = (VoidFunc)fonts_unload_custom_font,                                                 // fonts_unload_custom_font@00000188
    [99]  = (VoidFunc)app_free,                                                                 // free@0000018c
    [100] = (VoidFunc)gbitmap_create_as_sub_bitmap,                                            // gbitmap_create_as_sub_bitmap@00000190
    [101] = (VoidFunc)gbitmap_create_with_data,                                                // gbitmap_create_with_data@00000194
    [102] = (VoidFunc)gbitmap_create_with_resource_proxy,                                      // gbitmap_create_with_resource@00000198
    [103] = (VoidFunc)gbitmap_destroy,                                                         // gbitmap_destroy@0000019c

    [105] = (VoidFunc)n_gpath_create,                                                          // gpath_create@000001a4
    [106] = (VoidFunc)n_gpath_destroy,                                                         // gpath_destroy@000001a8

    [108] = (VoidFunc)gpath_draw_app,                                                          // gpath_draw_outline@000001b0
    [109] = (VoidFunc)gpath_move_to_app,                                                       // gpath_move_to@000001b4
    [110] = (VoidFunc)gpath_rotate_to_app,                                                     // gpath_rotate_to@000001b8

    [116] = (VoidFunc)graphics_draw_bitmap_in_rect,                                            // graphics_draw_bitmap_in_rect@000001d0
    [117] = (VoidFunc)graphics_draw_circle,                                                    // graphics_draw_circle@000001d4
    [118] = (VoidFunc)graphics_draw_line,                                                      // graphics_draw_line@000001d8
    [119] = (VoidFunc)graphics_draw_pixel,                                                     // graphics_draw_pixel@000001dc
    [120] = (VoidFunc)graphics_draw_rect,                                                      // graphics_draw_rect@000001e0
                                                                                            
    [122] = (VoidFunc)graphics_fill_circle,                                                    // graphics_fill_circle@000001e8
    [123] = (VoidFunc)graphics_fill_rect,                                                      // graphics_fill_rect@000001ec
                                                                                            
    [127] = (VoidFunc)n_grect_center_point,                                                    // grect_center_point@000001fc
                                                                                            
    [131] = (VoidFunc)grect_equal,                                                             // grect_equal@0000020c
                                                                                            
    [138] = (VoidFunc)layer_add_child,                                                         // layer_add_child@00000228
    [139] = (VoidFunc)layer_create,                                                            // layer_create@0000022c
    [140] = (VoidFunc)layer_create_with_data,                                                  // layer_create_with_data@00000230
    [141] = (VoidFunc)layer_destroy,                                                           // layer_destroy@00000234
    [142] = (VoidFunc)layer_get_bounds,                                                        // layer_get_bounds@00000238
                                                                                            
    [145] = (VoidFunc)layer_get_frame,                                                         // layer_get_frame@00000244
    [146] = (VoidFunc)layer_get_hidden,                                                        // layer_get_hidden@00000248
    [147] = (VoidFunc)layer_get_window,                                                        // layer_get_window@0000024c
    [148] = (VoidFunc)layer_insert_above_sibling,                                              // layer_insert_above_sibling@00000250
    [149] = (VoidFunc)layer_insert_below_sibling,                                              // layer_insert_below_sibling@00000254
    [150] = (VoidFunc)layer_mark_dirty,                                                        // layer_mark_dirty@00000258
    [151] = (VoidFunc)layer_remove_child_layers,                                               // layer_remove_child_layers@0000025c
    [152] = (VoidFunc)layer_remove_from_parent,                                                // layer_remove_from_parent@00000260
    [153] = (VoidFunc)layer_set_bounds,                                                        // layer_set_bounds@00000264
                                                                                            
    [155] = (VoidFunc)layer_set_frame,                                                         // layer_set_frame@0000026c
    [156] = (VoidFunc)layer_set_hidden,                                                        // layer_set_hidden@00000270
    [157] = (VoidFunc)layer_set_update_proc,                                                   // layer_set_update_proc@00000274

    [160] = (VoidFunc)rebble_time_get_tm,                                                      // localtime__deprecated@00000280
    [161] = (VoidFunc)app_malloc,                                                              // malloc@00000284
    [162] = (VoidFunc)memcpy,                                                                  // memcpy@00000288
    [163] = (VoidFunc)memmove,                                                                 // memmove@0000028c
    [164] = (VoidFunc)memset,                                                                  // memset@00000290
    [165] = (VoidFunc)menu_cell_basic_draw,                                                    // menu_cell_basic_draw@00000294
    [166] = (VoidFunc)menu_cell_basic_header_draw,                                             // menu_cell_basic_header_draw@00000298
    [167] = (VoidFunc)menu_cell_title_draw,                                                    // menu_cell_title_draw@0000029c
    [168] = (VoidFunc)menu_index_compare,                                                      // menu_index_compare@000002a0
                                                                                              
    [170] = (VoidFunc)menu_layer_destroy,                                                      // menu_layer_destroy@000002a8
    [171] = (VoidFunc)menu_layer_get_layer,                                                    // menu_layer_get_layer@000002ac
    [172] = (VoidFunc)menu_layer_get_scroll_layer,                                             // menu_layer_get_scroll_layer@000002b0
    [173] = (VoidFunc)menu_layer_get_selected_index,                                           // menu_layer_get_selected_index@000002b4
    [174] = (VoidFunc)menu_layer_reload_data,                                                  // menu_layer_reload_data@000002b8
                                                                                              
    [176] = (VoidFunc)menu_layer_set_click_config_onto_window,                                 // menu_layer_set_click_config_onto_window@000002c0
    [177] = (VoidFunc)menu_layer_set_selected_index,                                           // menu_layer_set_selected_index@000002c4
    [178] = (VoidFunc)menu_layer_set_selected_next,                                            // menu_layer_set_selected_next@000002c8
                                                                                              
    [188] = (VoidFunc)persist_exists,                                                          // persist_exists@000002f0

    [205] = (VoidFunc)rand,                                                                    // rand@00000334
    [206] = (VoidFunc)resource_get_handle,                                                     // resource_get_handle@00000338
    [207] = (VoidFunc)resource_load,                                                           // resource_load@0000033c
    [208] = (VoidFunc)resource_load_byte_range,                                                // resource_load_byte_range@00000340
    
    [209] = (VoidFunc)resource_size,                                                           // resource_size@00000344

    [217] = (VoidFunc)scroll_layer_add_child,                                                  // scroll_layer_add_child@00000364
    [218] = (VoidFunc)scroll_layer_create,                                                     // scroll_layer_create@00000368
    [219] = (VoidFunc)scroll_layer_destroy,                                                    // scroll_layer_destroy@0000036c
    [220] = (VoidFunc)scroll_layer_get_content_offset,                                         // scroll_layer_get_content_offset@00000370
    [221] = (VoidFunc)scroll_layer_get_content_size,                                           // scroll_layer_get_content_size@00000374
    [222] = (VoidFunc)scroll_layer_get_layer,                                                  // scroll_layer_get_layer@00000378
    [223] = (VoidFunc)scroll_layer_get_shadow_hidden,                                          // scroll_layer_get_shadow_hidden@0000037c
    [224] = (VoidFunc)scroll_layer_scroll_down_click_handler,                                  // scroll_layer_scroll_down_click_handler@00000380
    [225] = (VoidFunc)scroll_layer_scroll_up_click_handler,                                    // scroll_layer_scroll_up_click_handler@00000384
    [226] = (VoidFunc)scroll_layer_set_callbacks,                                              // scroll_layer_set_callbacks@00000388
    [227] = (VoidFunc)scroll_layer_set_click_config_onto_window,                               // scroll_layer_set_click_config_onto_window@0000038c
    [228] = (VoidFunc)scroll_layer_set_content_offset,                                         // scroll_layer_set_content_offset@00000390
    [229] = (VoidFunc)scroll_layer_set_content_size,                                           // scroll_layer_set_content_size@00000394
    [230] = (VoidFunc)scroll_layer_set_context,                                                // scroll_layer_set_context@00000398
    [231] = (VoidFunc)scroll_layer_set_frame,                                                  // scroll_layer_set_frame@0000039c
    [232] = (VoidFunc)scroll_layer_set_shadow_hidden,                                          // scroll_layer_set_shadow_hidden@000003a0
    [233] = (VoidFunc)simple_menu_layer_create,                                                // simple_menu_layer_create@000003a4
    [234] = (VoidFunc)simple_menu_layer_destroy,                                               // simple_menu_layer_destroy@000003a8
    [235] = (VoidFunc)simple_menu_layer_get_layer,                                             // simple_menu_layer_get_layer@000003ac
    [236] = (VoidFunc)simple_menu_layer_get_selected_index,                                    // simple_menu_layer_get_selected_index@000003b0
    [237] = (VoidFunc)simple_menu_layer_set_selected_index,                                    // simple_menu_layer_set_selected_index@000003b4
    [238] = (VoidFunc)sin_lookup,                                                              // sin_lookup@000003b8
    [239] = (VoidFunc)snprintf,                                                                // snprintf@000003bc
    [240] = (VoidFunc)srand,                                                                   // srand@000003c0
    [241] = (VoidFunc)strcat,                                                                  // strcat@000003c4
    [242] = (VoidFunc)strcmp,                                                                  // strcmp@000003c8
    [243] = (VoidFunc)strcpy,                                                                  // strcpy@000003cc
    [244] = (VoidFunc)strftime,                                                                // strftime@000003d0
    [245] = (VoidFunc)strlen,                                                                  // strlen@000003d4
                                                                                               
    [247] = (VoidFunc)strncmp,                                                                 // strncmp@000003dc
    [248] = (VoidFunc)strncpy,                                                                 // strncpy@000003e0
                                                                                               
    [262] = (VoidFunc)tick_timer_service_subscribe,                                            // tick_timer_service_subscribe@00000418
    [263] = (VoidFunc)tick_timer_service_unsubscribe,                                          // tick_timer_service_unsubscribe@0000041c
    [264] = (VoidFunc)pbl_time_deprecated,                                                     // time__deprecated@00000420
    [265] = (VoidFunc)rcore_time_ms,                                                           // time_ms_deprecated@00000424
    [266] = (VoidFunc)vibes_cancel,                                                            // vibes_cancel@00000428
    [267] = (VoidFunc)vibes_double_pulse,                                                      // vibes_double_pulse@0000042c
    [268] = (VoidFunc)vibes_enqueue_custom_pattern,                                            // vibes_enqueue_custom_pattern@00000430
    [269] = (VoidFunc)vibes_long_pulse,                                                        // vibes_long_pulse@00000434
    [270] = (VoidFunc)vibes_short_pulse,                                                       // vibes_short_pulse@00000438
    [271] = (VoidFunc)window_create,                                                           // window_create@0000043c
    [272] = (VoidFunc)window_destroy,                                                          // window_destroy@00000440
    [273] = (VoidFunc)window_get_click_config_provider,                                        // window_get_click_config_provider@00000444
                                                                                               
    [275] = (VoidFunc)window_get_root_layer,                                                   // window_get_root_layer@0000044c
    [276] = (VoidFunc)window_is_loaded,                                                        // window_is_loaded@00000450
                                                                                               
    [278] = (VoidFunc)window_set_click_config_provider,                                        // window_set_click_config_provider@00000458
    [279] = (VoidFunc)window_set_click_config_provider_with_context,                           // window_set_click_config_provider_with_context@0000045c

    [282] = (VoidFunc)window_set_window_handlers,                                              // window_set_window_handlers@00000468
    [283] = (VoidFunc)window_stack_contains_window,                                            // window_stack_contains_window@0000046c
    [284] = (VoidFunc)window_stack_get_top_window,                                             // window_stack_get_top_window@00000470
    [285] = (VoidFunc)window_stack_pop,                                                        // window_stack_pop@00000474
                                                                                               
    [287] = (VoidFunc)window_stack_push,                                                       // window_stack_push@0000047c
    [288] = (VoidFunc)window_stack_remove,                                                     // window_stack_remove@00000480
                                                                                               
    [291] = (VoidFunc)window_get_user_data,                                                    // window_get_user_data@0000048c
    [292] = (VoidFunc)window_set_user_data,                                                    // window_set_user_data@00000490
                                                                                               
    [303] = (VoidFunc)window_long_click_subscribe,                                             // window_long_click_subscribe@000004bc
    [304] = (VoidFunc)window_multi_click_subscribe,                                            // window_multi_click_subscribe@000004c0
    [305] = (VoidFunc)window_raw_click_subscribe,                                              // window_raw_click_subscribe@000004c4
    [306] = (VoidFunc)window_set_click_context,                                                // window_set_click_context@000004c8
    [307] = (VoidFunc)window_single_click_subscribe,                                           // window_single_click_subscribe@000004cc
    [308] = (VoidFunc)window_single_repeating_click_subscribe,                                 // window_single_repeating_click_subscribe@000004d0
    [309] = (VoidFunc)graphics_draw_text,                                                      // graphics_draw_text@000004d4
                                                                                               
    [316] = (VoidFunc)simple_menu_layer_get_menu_layer,                                        // simple_menu_layer_get_menu_layer@000004f0

    [318] = (VoidFunc)app_calloc,                                                              // calloc@000004f8

    [319] = (VoidFunc)bitmap_layer_get_bitmap,                                                 // bitmap_layer_get_bitmap@000004fc
                                                                                               
    [321] = (VoidFunc)window_get_click_config_context,                                         // window_get_click_config_context@00000504
    [323] = (VoidFunc)app_realloc,                                                             // realloc@0000050c
    [335] = (VoidFunc)app_heap_bytes_free,                                                     // heap_bytes_free@0000053c
    [336] = (VoidFunc)app_heap_bytes_used,                                                     // heap_bytes_used@00000540
    [343] = (VoidFunc)gpath_fill_app,                                                          // gpath_draw_filled@0000055c

    [350] = (VoidFunc)graphics_release_frame_buffer,                                           // graphics_release_frame_buffer@00000578
    
                                                                                               
    [363] = (VoidFunc)mktime,                                                                  // mktime@000005ac
                                                                                               
    [370] = (VoidFunc)bitmap_layer_set_background_color,                                       // bitmap_layer_set_background_color@000005c8
    [371] = (VoidFunc)graphics_context_set_fill_color,                                         // graphics_context_set_fill_color@000005cc
    [372] = (VoidFunc)graphics_context_set_stroke_color,                                       // graphics_context_set_stroke_color@000005d0
    [373] = (VoidFunc)graphics_context_set_text_color,                                         // graphics_context_set_text_color@000005d4
                                                                                               
    [377] = (VoidFunc)window_set_background_color,                                             // window_set_background_color@000005e4
                                                                                               
    [379] = (VoidFunc)localtime,                                                               // localtime@000005ec
    [380] = (VoidFunc)animation_create,                                                        // animation_create@000005f0
    [381] = (VoidFunc)animation_destroy,                                                       // animation_destroy@000005f4
                                                                                               
    [384] = (VoidFunc)animation_schedule,                                                      // animation_schedule@00000600
                                                                                               
    [388] = (VoidFunc)animation_set_duration,                                                  // animation_set_duration@00000610
                                                                                               
    [390] = (VoidFunc)animation_set_implementation,                                            // animation_set_implementation@00000618
                                                                                               
    [393] = (VoidFunc)gbitmap_create_blank,                                                    // gbitmap_create_blank@00000624
    [394] = (VoidFunc)graphics_capture_frame_buffer,                                           // graphics_capture_frame_buffer@00000628
    [395] = (VoidFunc)graphics_capture_frame_buffer_format,                                    // graphics_capture_frame_buffer_format@0000062c
    [396] = (VoidFunc)property_animation_create,                                               // property_animation_create@00000630
    [397] = (VoidFunc)property_animation_create_layer_frame,                                   // property_animation_create_layer_frame@00000634
    [398] = (VoidFunc)property_animation_destroy,                                              // property_animation_destroy@00000638
                                                                                               
    [400] = (VoidFunc)property_animation_get_animation,                                        // property_animation_get_animation@00000640
                                                                                               
    [403] = (VoidFunc)property_animation_update_gpoint,                                        // property_animation_update_gpoint@0000064c
    [404] = (VoidFunc)property_animation_update_grect,                                         // property_animation_update_grect@00000650
    [405] = (VoidFunc)property_animation_update_int16,                                         // property_animation_update_int16@00000654
    [406] = (VoidFunc)gbitmap_create_blank_with_palette,                                       // gbitmap_create_blank_with_palette@00000658
    [407] = (VoidFunc)gbitmap_get_bounds,                                                      // gbitmap_get_bounds@0000065c
    [408] = (VoidFunc)gbitmap_get_bytes_per_row,                                               // gbitmap_get_bytes_per_row@00000660
    [409] = (VoidFunc)gbitmap_get_data,                                                        // gbitmap_get_data@00000664
    [410] = (VoidFunc)gbitmap_get_format,                                                      // gbitmap_get_format@00000668
    [411] = (VoidFunc)gbitmap_get_palette,                                                     // gbitmap_get_palette@0000066c
    [412] = (VoidFunc)gbitmap_set_bounds,                                                      // gbitmap_set_bounds@00000670
    [413] = (VoidFunc)gbitmap_set_data,                                                        // gbitmap_set_data@00000674
    [414] = (VoidFunc)gbitmap_set_palette,                                                     // gbitmap_set_palette@00000678
                                                                                               
    [421] = (VoidFunc)gbitmap_create_from_png_data,                                            // gbitmap_create_from_png_data@00000694
                                                                                               
    [439] = (VoidFunc)menu_layer_create,                                                       // menu_layer_create@000006dc
    [444] = (VoidFunc)graphics_context_set_antialiased,                                        // graphics_context_set_antialiased@000006f0
    [445] = (VoidFunc)graphics_context_set_stroke_width,                                       // graphics_context_set_stroke_width@000006f4
    [446] = (VoidFunc)action_bar_layer_add_to_window,                                          // action_bar_layer_add_to_window@000006f8
    [447] = (VoidFunc)action_bar_layer_clear_icon,                                             // action_bar_layer_clear_icon@000006fc
    [448] = (VoidFunc)action_bar_layer_create,                                                 // action_bar_layer_create@00000700
    [449] = (VoidFunc)action_bar_layer_destroy,                                                // action_bar_layer_destroy@00000704
    [450] = (VoidFunc)action_bar_layer_get_layer,                                              // action_bar_layer_get_layer@00000708
    [451] = (VoidFunc)action_bar_layer_remove_from_window,                                     // action_bar_layer_remove_from_window@0000070c
    [452] = (VoidFunc)action_bar_layer_set_background_color,                                   // action_bar_layer_set_background_color@00000710
    [453] = (VoidFunc)action_bar_layer_set_click_config_provider,                              // action_bar_layer_set_click_config_provider@00000714
    [454] = (VoidFunc)action_bar_layer_set_context,                                            // action_bar_layer_set_context@00000718
    [455] = (VoidFunc)action_bar_layer_set_icon,                                               // action_bar_layer_set_icon@0000071c
    [456] = (VoidFunc)action_bar_layer_set_icon_animated,                                      // action_bar_layer_set_icon_animated@00000720
                                                                                               
    [458] = (VoidFunc)gbitmap_create_palettized_from_1bit,                                     // gbitmap_create_palettized_from_1bit@00000728
    [459] = (VoidFunc)menu_cell_layer_is_highlighted,                                          // menu_cell_layer_is_highlighted@0000072c
                                                                                               
    [461] = (VoidFunc)action_bar_layer_set_icon_press_animation,                               // action_bar_layer_set_icon_press_animation@00000734
    [462] = (VoidFunc)text_layer_create,                                                       // text_layer_create@00000738
    [463] = (VoidFunc)text_layer_destroy,                                                      // text_layer_destroy@0000073c
    [464] = (VoidFunc)text_layer_get_content_size,                                             // text_layer_get_content_size@00000740
    [465] = (VoidFunc)text_layer_get_layer,                                                    // text_layer_get_layer@00000744
    [466] = (VoidFunc)text_layer_get_text,                                                     // text_layer_get_text@00000748
    [467] = (VoidFunc)text_layer_set_background_color,                                         // text_layer_set_background_color@0000074c
    [468] = (VoidFunc)text_layer_set_font,                                                     // text_layer_set_font@00000750
    [469] = (VoidFunc)text_layer_set_overflow_mode,                                            // text_layer_set_overflow_mode@00000754
    [470] = (VoidFunc)text_layer_set_size,                                                     // text_layer_set_size@00000758
    [471] = (VoidFunc)text_layer_set_text,                                                     // text_layer_set_text@0000075c
    [472] = (VoidFunc)text_layer_set_text_alignment,                                           // text_layer_set_text_alignment@00000760
    [473] = (VoidFunc)text_layer_set_text_color,                                               // text_layer_set_text_color@00000764
    [474] = (VoidFunc)n_gdraw_command_draw,                                                    // gdraw_command_draw@00000768
    [475] = (VoidFunc)n_gdraw_command_frame_draw,                                              // gdraw_command_frame_draw@0000076c
    [476] = (VoidFunc)n_gdraw_command_frame_get_duration,                                      // gdraw_command_frame_get_duration@00000770
    [477] = (VoidFunc)n_gdraw_command_frame_set_duration,                                      // gdraw_command_frame_set_duration@00000774
    [478] = (VoidFunc)n_gdraw_command_get_fill_color,                                          // gdraw_command_get_fill_color@00000778
    [479] = (VoidFunc)n_gdraw_command_get_hidden,                                              // gdraw_command_get_hidden@0000077c
    [480] = (VoidFunc)n_gdraw_command_get_num_points,                                          // gdraw_command_get_num_points@00000780
    [481] = (VoidFunc)n_gdraw_command_get_path_open,                                           // gdraw_command_get_path_open@00000784
    [482] = (VoidFunc)n_gdraw_command_get_point,                                               // gdraw_command_get_point@00000788
    [483] = (VoidFunc)n_gdraw_command_get_radius,                                              // gdraw_command_get_radius@0000078c
    [484] = (VoidFunc)n_gdraw_command_get_stroke_color,                                        // gdraw_command_get_stroke_color@00000790
    [485] = (VoidFunc)n_gdraw_command_get_stroke_width,                                        // gdraw_command_get_stroke_width@00000794
    [486] = (VoidFunc)n_gdraw_command_get_type,                                                // gdraw_command_get_type@00000798
    [487] = (VoidFunc)n_gdraw_command_image_clone,                                             // gdraw_command_image_clone@0000079c
    [488] = (VoidFunc)n_gdraw_command_image_create_with_resource,                              // gdraw_command_image_create_with_resource@000007a0
    [489] = (VoidFunc)n_gdraw_command_image_destroy,                                           // gdraw_command_image_destroy@000007a4
    [490] = (VoidFunc)n_gdraw_command_image_draw,                                              // gdraw_command_image_draw@000007a8
    [491] = (VoidFunc)n_gdraw_command_image_get_bounds_size,                                   // gdraw_command_image_get_bounds_size@000007ac
    [492] = (VoidFunc)n_gdraw_command_image_get_command_list,                                  // gdraw_command_image_get_command_list@000007b0
    [493] = (VoidFunc)n_gdraw_command_image_set_bounds_size,                                   // gdraw_command_image_set_bounds_size@000007b4
    [494] = (VoidFunc)n_gdraw_command_list_draw,                                               // gdraw_command_list_draw@000007b8
    [495] = (VoidFunc)n_gdraw_command_list_get_command,                                        // gdraw_command_list_get_command@000007bc
    [496] = (VoidFunc)n_gdraw_command_list_get_num_commands,                                   // gdraw_command_list_get_num_commands@000007c0
    [497] = (VoidFunc)n_gdraw_command_list_iterate,                                            // gdraw_command_list_iterate@000007c4
    [498] = (VoidFunc)n_gdraw_command_sequence_clone,                                          // gdraw_command_sequence_clone@000007c8
    [499] = (VoidFunc)n_gdraw_command_sequence_create_with_resource,                           // gdraw_command_sequence_create_with_resource@000007cc
    [500] = (VoidFunc)n_gdraw_command_sequence_destroy,                                        // gdraw_command_sequence_destroy@000007d0
    [501] = (VoidFunc)n_gdraw_command_sequence_get_bounds_size,                                // gdraw_command_sequence_get_bounds_size@000007d4
    [502] = (VoidFunc)n_gdraw_command_sequence_get_frame_by_elapsed,                           // gdraw_command_sequence_get_frame_by_elapsed@000007d8
    [503] = (VoidFunc)n_gdraw_command_sequence_get_frame_by_index,                             // gdraw_command_sequence_get_frame_by_index@000007dc
    [504] = (VoidFunc)n_gdraw_command_sequence_get_num_frames,                                 // gdraw_command_sequence_get_num_frames@000007e0
    [505] = (VoidFunc)n_gdraw_command_sequence_get_play_count,                                 // gdraw_command_sequence_get_play_count@000007e4
    [506] = (VoidFunc)n_gdraw_command_sequence_get_total_duration,                             // gdraw_command_sequence_get_total_duration@000007e8
    [507] = (VoidFunc)n_gdraw_command_sequence_set_bounds_size,                                // gdraw_command_sequence_set_bounds_size@000007ec
    [508] = (VoidFunc)n_gdraw_command_sequence_set_play_count,                                 // gdraw_command_sequence_set_play_count@000007f0
    [509] = (VoidFunc)n_gdraw_command_set_fill_color,                                          // gdraw_command_set_fill_color@000007f4
    [510] = (VoidFunc)n_gdraw_command_set_hidden,                                              // gdraw_command_set_hidden@000007f8
    [511] = (VoidFunc)n_gdraw_command_set_path_open,                                           // gdraw_command_set_path_open@000007fc
    [512] = (VoidFunc)n_gdraw_command_set_point,                                               // gdraw_command_set_point@00000800
    [513] = (VoidFunc)n_gdraw_command_set_radius,                                              // gdraw_command_set_radius@00000804
    [514] = (VoidFunc)n_gdraw_command_set_stroke_color,                                        // gdraw_command_set_stroke_color@00000808
    [515] = (VoidFunc)n_gdraw_command_set_stroke_width,                                        // gdraw_command_set_stroke_width@0000080c
    [516] = (VoidFunc)property_animation_create_bounds_origin,                                 // property_animation_create_bounds_origin@00000810
    [517] = (VoidFunc)property_animation_update_uint32,                                        // property_animation_update_uint32@00000814
    [518] = (VoidFunc)gpath_draw_app,                                                          // gpath_draw_outline_open@00000818
    [519] = (VoidFunc)pbl_time_t_deprecated,                                                     // time@0000081c
    [520] = (VoidFunc)menu_layer_set_highlight_colors,                                         // menu_layer_set_highlight_colors@00000820
    [521] = (VoidFunc)menu_layer_set_normal_colors,                                            // menu_layer_set_normal_colors@00000824
    [522] = (VoidFunc)menu_layer_set_callbacks,                                                // menu_layer_set_callbacks@00000828
    [523] = (VoidFunc)menu_layer_pad_bottom_enable,                                            // menu_layer_pad_bottom_enable@0000082c
    [524] = (VoidFunc)status_bar_layer_create,                                                 // status_bar_layer_create@00000830
    [525] = (VoidFunc)status_bar_layer_destroy,                                                // status_bar_layer_destroy@00000834
    [526] = (VoidFunc)status_bar_layer_get_background_color,                                   // status_bar_layer_get_background_color@00000838
    [527] = (VoidFunc)status_bar_layer_get_foreground_color,                                   // status_bar_layer_get_foreground_color@0000083c
    [528] = (VoidFunc)status_bar_layer_get_layer,                                              // status_bar_layer_get_layer@00000840
    [529] = (VoidFunc)status_bar_layer_set_colors,                                             // status_bar_layer_set_colors@00000844
    [530] = (VoidFunc)status_bar_layer_set_separator_mode,                                     // status_bar_layer_set_separator_mode@00000848
                                                                                               
    [534] = (VoidFunc)property_animation_update_gcolor8,                                       // property_animation_update_gcolor8@00000858
                                                                                               
    [536] = (VoidFunc)action_menu_close,                                                       // action_menu_close@00000860
    [537] = (VoidFunc)action_menu_freeze,                                                      // action_menu_freeze@00000864
    [538] = (VoidFunc)action_menu_get_context,                                                 // action_menu_get_context@00000868
    [539] = (VoidFunc)action_menu_get_root_level,                                              // action_menu_get_root_level@0000086c
    [540] = (VoidFunc)action_menu_hierarchy_destroy,                                           // action_menu_hierarchy_destroy@00000870
    [541] = (VoidFunc)action_menu_item_get_action_data,                                        // action_menu_item_get_action_data@00000874
    [542] = (VoidFunc)action_menu_item_get_label,                                              // action_menu_item_get_label@00000878
    [543] = (VoidFunc)action_menu_level_add_action,                                            // action_menu_level_add_action@0000087c
    [544] = (VoidFunc)action_menu_level_add_child,                                             // action_menu_level_add_child@00000880
    [545] = (VoidFunc)action_menu_level_create,                                                // action_menu_level_create@00000884
    [546] = (VoidFunc)action_menu_level_set_display_mode,                                      // action_menu_level_set_display_mode@00000888
    [547] = (VoidFunc)action_menu_open,                                                        // action_menu_open@0000088c

    [549] = (VoidFunc)action_menu_unfreeze,                                                    // action_menu_unfreeze@00000894
                                                                                               
    [572] = (VoidFunc)content_indicator_configure_direction,                                   // content_indicator_configure_direction@000008f0
    [573] = (VoidFunc)content_indicator_create,                                                // content_indicator_create@000008f4
    [574] = (VoidFunc)content_indicator_destroy,                                               // content_indicator_destroy@000008f8
    [575] = (VoidFunc)content_indicator_get_content_available,                                 // content_indicator_get_content_available@000008fc
    [576] = (VoidFunc)content_indicator_set_content_available,                                 // content_indicator_set_content_available@00000900
    [577] = (VoidFunc)scroll_layer_get_content_indicator,                                      // scroll_layer_get_content_indicator@00000904
    [578] = (VoidFunc)menu_layer_get_center_focused,                                           // menu_layer_get_center_focused@00000908
    [579] = (VoidFunc)menu_layer_set_center_focused,                                           // menu_layer_set_center_focused@0000090c
                                                                                               
    [592] = (VoidFunc)layer_convert_point_to_screen,                                           // layer_convert_point_to_screen@00000940
                                                                                               
    [594] = (VoidFunc)scroll_layer_get_paging,                                                 // scroll_layer_get_paging@00000948
    [595] = (VoidFunc)scroll_layer_set_paging,                                                 // scroll_layer_set_paging@0000094c
    
    [597] = (UnimplFunc)text_layer_restore_default_text_flow_and_paging,                       // text_layer_restore_default_text_flow_and_paging@00000954
    [598] = (VoidFunc)menu_layer_is_index_selected,                                            // menu_layer_is_index_selected@00000958
    
    [622] = (VoidFunc)layer_get_unobstructed_bounds,
    [627] = (VoidFunc)rocky_event_loop_with_resource,
    
    
    /* These functions are not yet implemented */
    

    [0]   = (UnimplFunc)_accel_data_service_subscribe__deprecated,                             // accel_data_service_subscribe__deprecated@00000000
    [1]   = (UnimplFunc)_accel_data_service_unsubscribe,                                       // accel_data_service_unsubscribe@00000004
    [2]   = (UnimplFunc)_accel_service_peek,                                                   // accel_service_peek@00000008
    [3]   = (UnimplFunc)_accel_service_set_samples_per_update,                                 // accel_service_set_samples_per_update@0000000c
    [4]   = (UnimplFunc)_accel_service_set_sampling_rate,                                      // accel_service_set_sampling_rate@00000010
    [5]   = (UnimplFunc)_accel_tap_service_subscribe,                                          // accel_tap_service_subscribe@00000014
    [6]   = (UnimplFunc)_accel_tap_service_unsubscribe,                                        // accel_tap_service_unsubscribe@00000018
    [7]   = (UnimplFunc)_action_bar_layer_legacy2_add_to_window,                               // action_bar_layer_legacy2_add_to_window@0000001c
    [8]   = (UnimplFunc)_action_bar_layer_legacy2_clear_icon,                                  // action_bar_layer_legacy2_clear_icon@00000020
    [9]   = (UnimplFunc)_action_bar_layer_legacy2_create,                                      // action_bar_layer_legacy2_create@00000024
    [10]  = (UnimplFunc)_action_bar_layer_legacy2_destroy,                                     // action_bar_layer_legacy2_destroy@00000028
    [11]  = (UnimplFunc)_action_bar_layer_legacy2_get_layer,                                   // action_bar_layer_legacy2_get_layer@0000002c
    [12]  = (UnimplFunc)_action_bar_layer_legacy2_remove_from_window,                          // action_bar_layer_legacy2_remove_from_window@00000030
    [13]  = (UnimplFunc)_action_bar_layer_legacy2_set_background_color_2bit,                   // action_bar_layer_legacy2_set_background_color_2bit@00000034
    [14]  = (UnimplFunc)_action_bar_layer_legacy2_set_click_config_provider,                   // action_bar_layer_legacy2_set_click_config_provider@00000038
    [15]  = (UnimplFunc)_action_bar_layer_legacy2_set_context,                                 // action_bar_layer_legacy2_set_context@0000003c
    [16]  = (UnimplFunc)_action_bar_layer_legacy2_set_icon,                                    // action_bar_layer_legacy2_set_icon@00000040
    [17]  = (UnimplFunc)_animation_legacy2_create,                                             // animation_legacy2_create@00000044
    [18]  = (UnimplFunc)_animation_legacy2_destroy,                                            // animation_legacy2_destroy@00000048
    [19]  = (UnimplFunc)_animation_legacy2_get_context,                                        // animation_legacy2_get_context@0000004c
    [20]  = (UnimplFunc)_animation_legacy2_is_scheduled,                                       // animation_legacy2_is_scheduled@00000050
    [21]  = (UnimplFunc)_animation_legacy2_schedule,                                           // animation_legacy2_schedule@00000054
    [22]  = (UnimplFunc)_animation_legacy2_set_curve,                                          // animation_legacy2_set_curve@00000058
    [23]  = (UnimplFunc)_animation_legacy2_set_delay,                                          // animation_legacy2_set_delay@0000005c
    [24]  = (UnimplFunc)_animation_legacy2_set_duration,                                       // animation_legacy2_set_duration@00000060
    [25]  = (UnimplFunc)_animation_legacy2_set_handlers,                                       // animation_legacy2_set_handlers@00000064
    [26]  = (UnimplFunc)_animation_legacy2_set_implementation,                                 // animation_legacy2_set_implementation@00000068
    [27]  = (UnimplFunc)_animation_legacy2_unschedule,                                         // animation_legacy2_unschedule@0000006c
    [28]  = (UnimplFunc)_animation_legacy2_unschedule_all,                                     // animation_legacy2_unschedule_all@00000070
    [29]  = (UnimplFunc)_app_comm_get_sniff_interval,                                          // app_comm_get_sniff_interval@00000074
    [30]  = (UnimplFunc)_app_comm_set_sniff_interval,                                          // app_comm_set_sniff_interval@00000078
    [35]  = (UnimplFunc)_app_message_deregister_callbacks,                                     // app_message_deregister_callbacks@0000008c
    [36]  = (UnimplFunc)_app_message_open,                                                     // app_message_open@00000090
    [43]  = (UnimplFunc)_app_sync_deinit,                                                      // app_sync_deinit@000000ac
    [44]  = (UnimplFunc)_app_sync_get,                                                         // app_sync_get@000000b0
    [45]  = (UnimplFunc)_app_sync_init,                                                        // app_sync_init@000000b4
    [46]  = (UnimplFunc)_app_sync_set,                                                         // app_sync_set@000000b8
    [50]  = (UnimplFunc)_atan2_lookup,                                                         // atan2_lookup@000000c8
    [52]  = (UnimplFunc)_atol,                                                                 // atol@000000d0
    [60]  = (UnimplFunc)_bitmap_layer_set_background_color_2bit,                               // bitmap_layer_set_background_color_2bit@000000f0
    [63]  = (UnimplFunc)_bluetooth_connection_service_peek,                                    // bluetooth_connection_service_peek@000000fc
    [64]  = (UnimplFunc)_bluetooth_connection_service_subscribe,                               // bluetooth_connection_service_subscribe@00000100
    [65]  = (UnimplFunc)_bluetooth_connection_service_unsubscribe,                             // bluetooth_connection_service_unsubscribe@00000104
    [66]  = (UnimplFunc)_click_number_of_clicks_counted,                                       // click_number_of_clicks_counted@00000108
    [67]  = (UnimplFunc)_click_recognizer_get_button_id,                                       // click_recognizer_get_button_id@0000010c
    [68]  = (UnimplFunc)_clock_copy_time_string,                                               // clock_copy_time_string@00000110
    [71]  = (UnimplFunc)_data_logging_create,                                                  // data_logging_create@0000011c
    [72]  = (UnimplFunc)_data_logging_finish,                                                  // data_logging_finish@00000120
    [73]  = (UnimplFunc)_data_logging_log,                                                     // data_logging_log@00000124
    [74]  = (UnimplFunc)_dict_calc_buffer_size,                                                // dict_calc_buffer_size@00000128
    [75]  = (UnimplFunc)_dict_calc_buffer_size_from_tuplets,                                   // dict_calc_buffer_size_from_tuplets@0000012c
    [76]  = (UnimplFunc)_dict_find,                                                            // dict_find@00000130
    [77]  = (UnimplFunc)_dict_merge,                                                           // dict_merge@00000134
    [78]  = (UnimplFunc)_dict_read_begin_from_buffer,                                          // dict_read_begin_from_buffer@00000138
    [79]  = (UnimplFunc)_dict_read_first,                                                      // dict_read_first@0000013c
    [80]  = (UnimplFunc)_dict_read_next,                                                       // dict_read_next@00000140
    [81]  = (UnimplFunc)_dict_serialize_tuplets,                                               // dict_serialize_tuplets@00000144
    [82]  = (UnimplFunc)_dict_serialize_tuplets_to_buffer__deprecated,                         // dict_serialize_tuplets_to_buffer__deprecated@00000148
    [83]  = (UnimplFunc)_dict_serialize_tuplets_to_buffer_with_iter,                           // dict_serialize_tuplets_to_buffer_with_iter@0000014c
    [84]  = (UnimplFunc)_dict_write_begin,                                                     // dict_write_begin@00000150
    [85]  = (UnimplFunc)_dict_write_cstring,                                                   // dict_write_cstring@00000154
    [86]  = (UnimplFunc)_dict_write_data,                                                      // dict_write_data@00000158
    [87]  = (UnimplFunc)_dict_write_end,                                                       // dict_write_end@0000015c
    [88]  = (UnimplFunc)_dict_write_int,                                                       // dict_write_int@00000160
    [89]  = (UnimplFunc)_dict_write_int16,                                                     // dict_write_int16@00000164
    [90]  = (UnimplFunc)_dict_write_int32,                                                     // dict_write_int32@00000168
    [91]  = (UnimplFunc)_dict_write_int8,                                                      // dict_write_int8@0000016c
    [92]  = (UnimplFunc)_dict_write_tuplet,                                                    // dict_write_tuplet@00000170
    [93]  = (UnimplFunc)_dict_write_uint16,                                                    // dict_write_uint16@00000174
    [94]  = (UnimplFunc)_dict_write_uint32,                                                    // dict_write_uint32@00000178
    [95]  = (UnimplFunc)_dict_write_uint8,                                                     // dict_write_uint8@0000017c
    [104] = (UnimplFunc)_gmtime,                                                               // gmtime@000001a0
    [107] = (UnimplFunc)_gpath_draw_filled_legacy,                                             // gpath_draw_filled_legacy@000001ac
    [111] = (UnimplFunc)_gpoint_equal,                                                         // gpoint_equal@000001bc
    [112] = (UnimplFunc)_graphics_context_set_compositing_mode,                                // graphics_context_set_compositing_mode@000001c0
    [113] = (UnimplFunc)_graphics_context_set_fill_color_2bit,                                 // graphics_context_set_fill_color_2bit@000001c4
    [114] = (UnimplFunc)_graphics_context_set_stroke_color_2bit,                               // graphics_context_set_stroke_color_2bit@000001c8
    [115] = (UnimplFunc)_graphics_context_set_text_color_2bit,                                 // graphics_context_set_text_color_2bit@000001cc
    [121] = (UnimplFunc)_graphics_draw_round_rect,                                             // graphics_draw_round_rect@000001e4
    [125] = (UnimplFunc)_graphics_text_layout_get_max_used_size,                               // graphics_text_layout_get_max_used_size@000001f4
    [126] = (UnimplFunc)_grect_align,                                                          // grect_align@000001f8
    [128] = (UnimplFunc)_grect_clip,                                                           // grect_clip@00000200
    [129] = (UnimplFunc)_grect_contains_point,                                                 // grect_contains_point@00000204
    [130] = (UnimplFunc)_grect_crop,                                                           // grect_crop@00000208
    [132] = (UnimplFunc)_grect_is_empty,                                                       // grect_is_empty@00000210
    [133] = (UnimplFunc)_grect_standardize,                                                    // grect_standardize@00000214
    [134] = (UnimplFunc)_gsize_equal,                                                          // gsize_equal@00000218
    [135] = (UnimplFunc)_inverter_layer_create,                                                // inverter_layer_create@0000021c
    [136] = (UnimplFunc)_inverter_layer_destroy,                                               // inverter_layer_destroy@00000220
    [137] = (UnimplFunc)_inverter_layer_get_layer,                                             // inverter_layer_get_layer@00000224
    [143] = (UnimplFunc)_layer_get_clips,                                                      // layer_get_clips@0000023c
    [144] = (UnimplFunc)_layer_get_data,                                                       // layer_get_data@00000240
    [154] = (UnimplFunc)_layer_set_clips,                                                      // layer_set_clips@00000268
    [158] = (UnimplFunc)_light_enable,                                                         // light_enable@00000278
    [159] = (UnimplFunc)_light_enable_interaction,                                             // light_enable_interaction@0000027c
    [169] = (UnimplFunc)_menu_layer_legacy2_create,                                            // menu_layer_legacy2_create@000002a4
    [175] = (UnimplFunc)_menu_layer_legacy2_set_callbacks__deprecated,                         // menu_layer_legacy2_set_callbacks__deprecated@000002bc
    [179] = (UnimplFunc)_number_window_create,                                                 // number_window_create@000002cc
    [180] = (UnimplFunc)_number_window_destroy,                                                // number_window_destroy@000002d0
    [181] = (UnimplFunc)_number_window_get_value,                                              // number_window_get_value@000002d4
    [182] = (UnimplFunc)_number_window_set_label,                                              // number_window_set_label@000002d8
    [183] = (UnimplFunc)_number_window_set_max,                                                // number_window_set_max@000002dc
    [184] = (UnimplFunc)_number_window_set_min,                                                // number_window_set_min@000002e0
    [185] = (UnimplFunc)_number_window_set_step_size,                                          // number_window_set_step_size@000002e4
    [186] = (UnimplFunc)_number_window_set_value,                                              // number_window_set_value@000002e8
    [187] = (UnimplFunc)_persist_delete,                                                       // persist_delete@000002ec
    [189] = (UnimplFunc)_persist_get_size,                                                     // persist_get_size@000002f4
    [190] = (UnimplFunc)_persist_read_bool,                                                    // persist_read_bool@000002f8
    [191] = (UnimplFunc)_persist_read_data__deprecated,                                        // persist_read_data__deprecated@000002fc
    [192] = (UnimplFunc)_persist_read_int,                                                     // persist_read_int@00000300
    [193] = (UnimplFunc)_persist_read_string__deprecated,                                      // persist_read_string__deprecated@00000304
    [194] = (UnimplFunc)_persist_write_bool,                                                   // persist_write_bool@00000308
    [195] = (UnimplFunc)_persist_write_data__deprecated,                                       // persist_write_data__deprecated@0000030c
    [196] = (UnimplFunc)_persist_write_int,                                                    // persist_write_int@00000310
    [197] = (UnimplFunc)_persist_write_string,                                                 // persist_write_string@00000314
    [198] = (UnimplFunc)_property_animation_legacy2_create,                                    // property_animation_legacy2_create@00000318
    [199] = (UnimplFunc)_property_animation_legacy2_create_layer_frame,                        // property_animation_legacy2_create_layer_frame@0000031c
    [200] = (UnimplFunc)_property_animation_legacy2_destroy,                                   // property_animation_legacy2_destroy@00000320
    [201] = (UnimplFunc)_property_animation_legacy2_update_gpoint,                             // property_animation_legacy2_update_gpoint@00000324
    [202] = (UnimplFunc)_property_animation_legacy2_update_grect,                              // property_animation_legacy2_update_grect@00000328
    [203] = (UnimplFunc)_property_animation_legacy2_update_int16,                              // property_animation_legacy2_update_int16@0000032c
    [204] = (UnimplFunc)_psleep,                                                               // psleep@00000330
    [210] = (UnimplFunc)_rot_bitmap_layer_create,                                              // rot_bitmap_layer_create@00000348
    [211] = (UnimplFunc)_rot_bitmap_layer_destroy,                                             // rot_bitmap_layer_destroy@0000034c
    [212] = (UnimplFunc)_rot_bitmap_layer_increment_angle,                                     // rot_bitmap_layer_increment_angle@00000350
    [213] = (UnimplFunc)_rot_bitmap_layer_set_angle,                                           // rot_bitmap_layer_set_angle@00000354
    [214] = (UnimplFunc)_rot_bitmap_layer_set_corner_clip_color_2bit,                          // rot_bitmap_layer_set_corner_clip_color_2bit@00000358
    [215] = (UnimplFunc)_rot_bitmap_set_compositing_mode,                                      // rot_bitmap_set_compositing_mode@0000035c
    [216] = (UnimplFunc)_rot_bitmap_set_src_ic,                                                // rot_bitmap_set_src_ic@00000360
    [246] = (UnimplFunc)_strncat,                                                              // strncat@000003d8
    [249] = (UnimplFunc)_text_layer_legacy2_create,                                            // text_layer_legacy2_create@000003e4
    [250] = (UnimplFunc)_text_layer_legacy2_destroy,                                           // text_layer_legacy2_destroy@000003e8
    [251] = (UnimplFunc)_text_layer_legacy2_get_content_size,                                  // text_layer_legacy2_get_content_size@000003ec
    [252] = (UnimplFunc)_text_layer_legacy2_get_layer,                                         // text_layer_legacy2_get_layer@000003f0
    [253] = (UnimplFunc)_text_layer_legacy2_get_text,                                          // text_layer_legacy2_get_text@000003f4
    [254] = (UnimplFunc)_text_layer_legacy2_set_background_color_2bit,                         // text_layer_legacy2_set_background_color_2bit@000003f8
    [255] = (UnimplFunc)_text_layer_legacy2_set_font,                                          // text_layer_legacy2_set_font@000003fc
    [256] = (UnimplFunc)_text_layer_legacy2_set_overflow_mode,                                 // text_layer_legacy2_set_overflow_mode@00000400
    [257] = (UnimplFunc)_text_layer_legacy2_set_size,                                          // text_layer_legacy2_set_size@00000404
    [258] = (UnimplFunc)_text_layer_legacy2_set_text,                                          // text_layer_legacy2_set_text@00000408
    [259] = (UnimplFunc)_text_layer_legacy2_set_text_alignment,                                // text_layer_legacy2_set_text_alignment@0000040c
    [260] = (UnimplFunc)_text_layer_legacy2_set_text_color_2bit,                               // text_layer_legacy2_set_text_color_2bit@00000410
    [274] = (UnimplFunc)_window_get_fullscreen,                                                // window_get_fullscreen@00000448
    [277] = (UnimplFunc)_window_set_background_color_2bit,                                     // window_set_background_color_2bit@00000454
    [280] = (UnimplFunc)_window_set_fullscreen,                                                // window_set_fullscreen@00000460
    [281] = (UnimplFunc)_window_set_status_bar_icon,                                           // window_set_status_bar_icon@00000464
    [286] = (UnimplFunc)_window_stack_pop_all,                                                 // window_stack_pop_all@00000478
    [289] = (UnimplFunc)_app_focus_service_subscribe,                                          // app_focus_service_subscribe@00000484
    [290] = (UnimplFunc)_app_focus_service_unsubscribe,                                        // app_focus_service_unsubscribe@00000488
    [293] = (UnimplFunc)_app_message_get_context,                                              // app_message_get_context@00000494
    [294] = (UnimplFunc)_app_message_inbox_size_maximum,                                       // app_message_inbox_size_maximum@00000498
    [295] = (UnimplFunc)_app_message_outbox_begin,                                             // app_message_outbox_begin@0000049c
    [296] = (UnimplFunc)_app_message_outbox_send,                                              // app_message_outbox_send@000004a0
    [297] = (UnimplFunc)_app_message_outbox_size_maximum,                                      // app_message_outbox_size_maximum@000004a4
    [298] = (UnimplFunc)_app_message_register_inbox_dropped,                                   // app_message_register_inbox_dropped@000004a8
    [299] = (UnimplFunc)_app_message_register_inbox_received,                                  // app_message_register_inbox_received@000004ac
    [300] = (UnimplFunc)_app_message_register_outbox_failed,                                   // app_message_register_outbox_failed@000004b0
    [301] = (UnimplFunc)_app_message_register_outbox_sent,                                     // app_message_register_outbox_sent@000004b4
    [302] = (UnimplFunc)_app_message_set_context,                                              // app_message_set_context@000004b8
    [310] = (UnimplFunc)_dict_serialize_tuplets_to_buffer,                                     // dict_serialize_tuplets_to_buffer@000004d8
    [311] = (UnimplFunc)_persist_read_data,                                                    // persist_read_data@000004dc
    [312] = (UnimplFunc)_persist_read_string,                                                  // persist_read_string@000004e0
    [313] = (UnimplFunc)_persist_write_data,                                                   // persist_write_data@000004e4
    [314] = (UnimplFunc)_dict_size,                                                            // dict_size@000004e8
    [315] = (UnimplFunc)_n_graphics_text_layout_get_content_size,                              // graphics_text_layout_get_content_size@000004ec
    [317] = (UnimplFunc)_accel_data_service_subscribe,                                         // accel_data_service_subscribe@000004f4
    [320] = (UnimplFunc)_menu_layer_legacy2_set_callbacks,                                     // menu_layer_legacy2_set_callbacks@00000500
    [322] = (UnimplFunc)_number_window_get_window,                                             // number_window_get_window@00000508
        
    [324] = (UnimplFunc)_gbitmap_create_blank_2bit,                                            // gbitmap_create_blank_2bit@00000510
    [325] = (UnimplFunc)_click_recognizer_is_repeating,                                        // click_recognizer_is_repeating@00000514
    [326] = (UnimplFunc)_accel_raw_data_service_subscribe,                                     // accel_raw_data_service_subscribe@00000518
    [327] = (UnimplFunc)_app_worker_is_running,                                                // app_worker_is_running@0000051c
    [328] = (UnimplFunc)_app_worker_kill,                                                      // app_worker_kill@00000520
    [329] = (UnimplFunc)_app_worker_launch,                                                    // app_worker_launch@00000524
    [330] = (UnimplFunc)_app_worker_message_subscribe,                                         // app_worker_message_subscribe@00000528
    [331] = (UnimplFunc)_app_worker_message_unsubscribe,                                       // app_worker_message_unsubscribe@0000052c
    [332] = (UnimplFunc)_app_worker_send_message,                                              // app_worker_send_message@00000530
    [333] = (UnimplFunc)_worker_event_loop,                                                    // worker_event_loop@00000534
    [334] = (UnimplFunc)_worker_launch_app,                                                    // worker_launch_app@00000538
    [337] = (UnimplFunc)_compass_service_peek,                                                 // compass_service_peek@00000544
    [338] = (UnimplFunc)_compass_service_set_heading_filter,                                   // compass_service_set_heading_filter@00000548
    [339] = (UnimplFunc)_compass_service_subscribe,                                            // compass_service_subscribe@0000054c
    [340] = (UnimplFunc)_compass_service_unsubscribe,                                          // compass_service_unsubscribe@00000550
    [341] = (UnimplFunc)_uuid_equal,                                                           // uuid_equal@00000554
    [342] = (UnimplFunc)_uuid_to_string,                                                       // uuid_to_string@00000558
    [344] = (UnimplFunc)_animation_legacy2_set_custom_curve,                                   // animation_legacy2_set_custom_curve@00000560
    [345] = (UnimplFunc)_watch_info_get_color,                                                 // watch_info_get_color@00000564
    [346] = (UnimplFunc)_watch_info_get_firmware_version,                                      // watch_info_get_firmware_version@00000568
    [347] = (UnimplFunc)_watch_info_get_model,                                                 // watch_info_get_model@0000056c
    [348] = (UnimplFunc)_graphics_capture_frame_buffer_2bit,                                   // graphics_capture_frame_buffer_2bit@00000570
    [349] = (UnimplFunc)_graphics_frame_buffer_is_captured,                                    // graphics_frame_buffer_is_captured@00000574
    [351] = (UnimplFunc)_clock_to_timestamp,                                                   // clock_to_timestamp@0000057c
    [352] = (UnimplFunc)_launch_reason,                                                        // launch_reason@00000580
    [353] = (UnimplFunc)_wakeup_cancel,                                                        // wakeup_cancel@00000584
    [354] = (UnimplFunc)_wakeup_cancel_all,                                                    // wakeup_cancel_all@00000588
    [355] = (UnimplFunc)_wakeup_get_launch_event,                                              // wakeup_get_launch_event@0000058c
    [356] = (UnimplFunc)_wakeup_query,                                                         // wakeup_query@00000590
    [357] = (UnimplFunc)_wakeup_schedule,                                                      // wakeup_schedule@00000594
    [358] = (UnimplFunc)_wakeup_service_subscribe,                                             // wakeup_service_subscribe@00000598
    [359] = (UnimplFunc)_clock_is_timezone_set,                                                // clock_is_timezone_set@0000059c
    [360] = (UnimplFunc)_i18n_get_system_locale,                                               // i18n_get_system_locale@000005a0
    [361] = (UnimplFunc)__localeconv_r,                                                        // _localeconv_r@000005a4
    [362] = (UnimplFunc)_setlocale,                                                            // setlocale@000005a8
    [364] = (UnimplFunc)_gcolor_equal,                                                         // gcolor_equal@000005b0
    [365] = (UnimplFunc)___profiler_init,                                                      // __profiler_init@000005b4
    [366] = (UnimplFunc)___profiler_print_stats,                                               // __profiler_print_stats@000005b8
    [367] = (UnimplFunc)___profiler_start,                                                     // __profiler_start@000005bc
    [368] = (UnimplFunc)___profiler_stop,                                                      // __profiler_stop@000005c0
    [374] = (UnimplFunc)_rot_bitmap_layer_set_corner_clip_color,                               // rot_bitmap_layer_set_corner_clip_color@000005d8
    [378] = (UnimplFunc)_clock_get_timezone,                                                   // clock_get_timezone@000005e8
    [382] = (UnimplFunc)_animation_get_context,                                                // animation_get_context@000005f8
    [383] = (UnimplFunc)_animation_is_scheduled,                                               // animation_is_scheduled@000005fc
    [385] = (UnimplFunc)_animation_set_curve,                                                  // animation_set_curve@00000604
    [386] = (UnimplFunc)_animation_set_custom_curve,                                           // animation_set_custom_curve@00000608
    [387] = (UnimplFunc)_animation_set_delay,                                                  // animation_set_delay@0000060c
    [389] = (UnimplFunc)_animation_set_handlers,                                               // animation_set_handlers@00000614
    [391] = (UnimplFunc)_animation_unschedule,                                                 // animation_unschedule@0000061c
    [392] = (UnimplFunc)_animation_unschedule_all,                                             // animation_unschedule_all@00000620
    [399] = (UnimplFunc)_property_animation_from,                                              // property_animation_from@0000063c
    [401] = (UnimplFunc)_property_animation_subject,                                           // property_animation_subject@00000644
    [402] = (UnimplFunc)_property_animation_to,                                                // property_animation_to@00000648
    [415] = (UnimplFunc)_gbitmap_sequence_create_with_resource,                                // gbitmap_sequence_create_with_resource@0000067c
    [416] = (UnimplFunc)_gbitmap_sequence_destroy,                                             // gbitmap_sequence_destroy@00000680
    [417] = (UnimplFunc)_gbitmap_sequence_get_bitmap_size,                                     // gbitmap_sequence_get_bitmap_size@00000684
    [418] = (UnimplFunc)_gbitmap_sequence_get_current_frame_idx,                               // gbitmap_sequence_get_current_frame_idx@00000688
    [419] = (UnimplFunc)_gbitmap_sequence_get_total_num_frames,                                // gbitmap_sequence_get_total_num_frames@0000068c
    [420] = (UnimplFunc)_gbitmap_sequence_update_bitmap_next_frame,                            // gbitmap_sequence_update_bitmap_next_frame@00000690
    [422] = (UnimplFunc)_animation_clone,                                                      // animation_clone@00000698
    [423] = (UnimplFunc)_animation_get_delay,                                                  // animation_get_delay@0000069c
    [424] = (UnimplFunc)_animation_get_duration,                                               // animation_get_duration@000006a0
    [425] = (UnimplFunc)_animation_get_play_count,                                             // animation_get_play_count@000006a4
    [426] = (UnimplFunc)_animation_get_elapsed,                                                // animation_get_elapsed@000006a8
    [427] = (UnimplFunc)_animation_get_reverse,                                                // animation_get_reverse@000006ac
    [428] = (UnimplFunc)_animation_sequence_create,                                            // animation_sequence_create@000006b0
    [429] = (UnimplFunc)_animation_sequence_create_from_array,                                 // animation_sequence_create_from_array@000006b4
    [430] = (UnimplFunc)_animation_set_play_count,                                             // animation_set_play_count@000006b8
    [431] = (UnimplFunc)_animation_set_elapsed,                                                // animation_set_elapsed@000006bc
    [432] = (UnimplFunc)_animation_set_reverse,                                                // animation_set_reverse@000006c0
    [433] = (UnimplFunc)_animation_spawn_create,                                               // animation_spawn_create@000006c4
    [434] = (UnimplFunc)_animation_spawn_create_from_array,                                    // animation_spawn_create_from_array@000006c8
    [435] = (UnimplFunc)_animation_get_curve,                                                  // animation_get_curve@000006cc
    [436] = (UnimplFunc)_animation_get_custom_curve,                                           // animation_get_custom_curve@000006d0
    [437] = (UnimplFunc)_animation_get_implementation,                                         // animation_get_implementation@000006d4
    [438] = (UnimplFunc)_launch_get_args,                                                      // launch_get_args@000006d8
    [441] = (UnimplFunc)_gbitmap_sequence_get_play_count,                                      // gbitmap_sequence_get_play_count@000006e4
    [442] = (UnimplFunc)_gbitmap_sequence_restart,                                             // gbitmap_sequence_restart@000006e8
    [443] = (UnimplFunc)_gbitmap_sequence_set_play_count,                                      // gbitmap_sequence_set_play_count@000006ec
    [457] = (UnimplFunc)_gbitmap_sequence_update_bitmap_by_elapsed,                            // gbitmap_sequence_update_bitmap_by_elapsed@00000724
    [460] = (UnimplFunc)_graphics_draw_rotated_bitmap,                                         // graphics_draw_rotated_bitmap@00000730
    [531] = (UnimplFunc)_difftime,                                                             // difftime@0000084c
    [532] = (VoidFunc)rcore_time_ms,                                                              // time_ms@00000850
    [533] = (UnimplFunc)_gcolor_legible_over,                                                  // gcolor_legible_over@00000854
    [535] = (UnimplFunc)_app_focus_service_subscribe_handlers,                                 // app_focus_service_subscribe_handlers@0000085c
    [548] = (UnimplFunc)_action_menu_set_result_window,                                        // action_menu_set_result_window@00000890
    [550] = (UnimplFunc)_dictation_session_create,                                             // dictation_session_create@00000898
    [551] = (UnimplFunc)_dictation_session_destroy,                                            // dictation_session_destroy@0000089c
    [552] = (UnimplFunc)_dictation_session_enable_confirmation,                                // dictation_session_enable_confirmation@000008a0
    [553] = (UnimplFunc)_dictation_session_start,                                              // dictation_session_start@000008a4
    [554] = (UnimplFunc)_dictation_session_stop,                                               // dictation_session_stop@000008a8
    [555] = (UnimplFunc)_smartstrap_attribute_begin_write,                                     // smartstrap_attribute_begin_write@000008ac
    [556] = (UnimplFunc)_smartstrap_attribute_create,                                          // smartstrap_attribute_create@000008b0
    [557] = (UnimplFunc)_smartstrap_attribute_destroy,                                         // smartstrap_attribute_destroy@000008b4
    [558] = (UnimplFunc)_smartstrap_attribute_end_write,                                       // smartstrap_attribute_end_write@000008b8
    [559] = (UnimplFunc)_smartstrap_attribute_get_attribute_id,                                // smartstrap_attribute_get_attribute_id@000008bc
    [560] = (UnimplFunc)_smartstrap_attribute_get_service_id,                                  // smartstrap_attribute_get_service_id@000008c0
    [561] = (UnimplFunc)_smartstrap_attribute_read,                                            // smartstrap_attribute_read@000008c4
    [562] = (UnimplFunc)_smartstrap_service_is_available,                                      // smartstrap_service_is_available@000008c8
    [563] = (UnimplFunc)_smartstrap_set_timeout,                                               // smartstrap_set_timeout@000008cc
    [564] = (UnimplFunc)_smartstrap_subscribe,                                                 // smartstrap_subscribe@000008d0
    [565] = (UnimplFunc)_smartstrap_unsubscribe,                                               // smartstrap_unsubscribe@000008d4
    [566] = (UnimplFunc)_connection_service_peek_pebble_app_connection,                        // connection_service_peek_pebble_app_connection@000008d8
    [567] = (UnimplFunc)_connection_service_peek_pebblekit_connection,                         // connection_service_peek_pebblekit_connection@000008dc
    [568] = (UnimplFunc)_connection_service_subscribe,                                         // connection_service_subscribe@000008e0
    [569] = (UnimplFunc)_connection_service_unsubscribe,                                       // connection_service_unsubscribe@000008e4
    [570] = (UnimplFunc)_dictation_session_enable_error_dialogs,                               // dictation_session_enable_error_dialogs@000008e8
    [571] = (UnimplFunc)_gbitmap_get_data_row_info,                                            // gbitmap_get_data_row_info@000008ec
    [580] = (UnimplFunc)_grect_inset,                                                          // grect_inset@00000910
    [581] = (UnimplFunc)_gpoint_from_polar,                                                    // gpoint_from_polar@00000914
    [582] = (UnimplFunc)_graphics_draw_arc,                                                    // graphics_draw_arc@00000918
    [583] = (UnimplFunc)_graphics_fill_radial,                                                 // graphics_fill_radial@0000091c
    [584] = (UnimplFunc)_grect_centered_from_polar,                                            // grect_centered_from_polar@00000920
    [585] = (UnimplFunc)_graphics_text_attributes_create,                                      // graphics_text_attributes_create@00000924
    [586] = (UnimplFunc)_graphics_text_attributes_destroy,                                     // graphics_text_attributes_destroy@00000928
    [587] = (UnimplFunc)_graphics_text_attributes_enable_paging,                               // graphics_text_attributes_enable_paging@0000092c
    [588] = (UnimplFunc)_graphics_text_attributes_enable_screen_text_flow,                     // graphics_text_attributes_enable_screen_text_flow@00000930
    [589] = (UnimplFunc)_graphics_text_attributes_restore_default_paging,                      // graphics_text_attributes_restore_default_paging@00000934
    [590] = (UnimplFunc)_graphics_text_attributes_restore_default_text_flow,                   // graphics_text_attributes_restore_default_text_flow@00000938
    [591] = (UnimplFunc)_graphics_text_layout_get_content_size_with_attributes,                // graphics_text_layout_get_content_size_with_attributes@0000093c
    [593] = (UnimplFunc)_layer_convert_rect_to_screen,                                         // layer_convert_rect_to_screen@00000944
    [596] = (UnimplFunc)_text_layer_enable_screen_text_flow_and_paging,                        // text_layer_enable_screen_text_flow_and_paging@00000950
    [599] = (UnimplFunc)_health_service_activities_iterate,                                    // health_service_activities_iterate@0000095c
    [600] = (UnimplFunc)_health_service_any_activity_accessible,                               // health_service_any_activity_accessible@00000960
    [601] = (UnimplFunc)_health_service_events_subscribe,                                      // health_service_events_subscribe@00000964
    [602] = (UnimplFunc)_health_service_events_unsubscribe,                                    // health_service_events_unsubscribe@00000968
    [603] = (UnimplFunc)_health_service_get_minute_history,                                    // health_service_get_minute_history@0000096c
    [604] = (UnimplFunc)_health_service_metric_accessible,                                     // health_service_metric_accessible@00000970
    [605] = (UnimplFunc)_health_service_peek_current_activities,                               // health_service_peek_current_activities@00000974
    [606] = (UnimplFunc)_health_service_sum,                                                   // health_service_sum@00000978
    [607] = (UnimplFunc)_health_service_sum_today,                                             // health_service_sum_today@0000097c
    [608] = (UnimplFunc)_time_start_of_today,                                                  // time_start_of_today@00000980
    [609] = (UnimplFunc)_health_service_metric_averaged_accessible,                            // health_service_metric_averaged_accessible@00000984
    [610] = (UnimplFunc)_health_service_sum_averaged,                                          // health_service_sum_averaged@00000988
    [611] = (UnimplFunc)_health_service_get_measurement_system_for_display,                    // health_service_get_measurement_system_for_display@0000098c
    [612] = (UnimplFunc)_gdraw_command_frame_get_command_list,                                 // gdraw_command_frame_get_command_list@00000990 
    [613] = (UnimplFunc)_unimpl613,
    [614] = (UnimplFunc)_unimpl614,
    [615] = (UnimplFunc)_unimpl615,
    [616] = (UnimplFunc)_unimpl616,
    [617] = (UnimplFunc)_unimpl617,
    [618] = (UnimplFunc)_unimpl618,
    [619] = (UnimplFunc)_unimpl619,
    [620] = (UnimplFunc)_unimpl620,
    [621] = (UnimplFunc)_unimpl621,
    [623] = (UnimplFunc)_unimpl623,
    [624] = (UnimplFunc)_unimpl624,
    [625] = (UnimplFunc)_unimpl625,
    [626] = (UnimplFunc)_unimpl626,
    [628] = (UnimplFunc)_unimpl628,
    [629] = (UnimplFunc)_unimpl629,
    [630] = (UnimplFunc)_unimpl630,
    [631] = (UnimplFunc)_unimpl631,
};
