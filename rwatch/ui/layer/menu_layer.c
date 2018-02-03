/* menu_layer.c
 *
 * MenuLayer component.
 * RebbleOS
 */
#include "menu_layer.h"
#include "menu.h"
#include "utils.h"

extern void graphics_draw_bitmap_in_rect(GContext *, GBitmap *, GRect);

static void menu_layer_update_proc(Layer *layer, GContext *nGContext);

#define MenuRow(section, row, y, h) ((MenuCellSpan){ (y), (h), 0, false, MenuIndex((section), (row)) })
#define MenuHeader(section, y, h) ((MenuCellSpan){ (y), (h), 0, true, MenuIndex((section), 0) })

#define BUTTON_REPEAT_INTERVAL_MS 600
#define BUTTON_LONG_CLICK_DELAY_MS 500
#define ANIMATE_ON_CLICK true

MenuLayer *menu_layer_create(GRect frame)
{
    MenuLayer *mlayer = (MenuLayer *)app_calloc(1, sizeof(MenuLayer));
    mlayer->layer = layer_create(frame);
    mlayer->scroll_layer = scroll_layer_create(GRect(0, 0, frame.size.w, frame.size.h));
    mlayer->layer->container = mlayer;

    mlayer->cells_count = 0;
    mlayer->selected = MenuIndex(0, 0);
    mlayer->end_index = MenuIndex(0, 1);
    mlayer->bg_color = GColorWhite;
    mlayer->bg_hi_color = GColorBlack;
    mlayer->fg_color = GColorBlack;
    mlayer->fg_hi_color = GColorWhite;

    mlayer->isBottomPaddingEnabled = true;
#ifdef PBL_RECT
    mlayer->isCenterFocused = false;
#else
    mlayer->isCenterFocused = true;
#endif

    layer_set_update_proc(mlayer->layer, menu_layer_update_proc);

    scroll_layer_add_child(mlayer->scroll_layer, mlayer->layer);

    return mlayer;
}

void menu_layer_destroy(MenuLayer *menu)
{
    scroll_layer_destroy(menu->scroll_layer);
    if (menu->cells_count > 0)
        app_free(menu->cells);
    app_free(menu);
}

int16_t menu_index_compare(const MenuIndex *a, const MenuIndex *b)
{
    return (a->section == b->section) ? a->row - b->row : a->section - b->section;
}

Layer *menu_layer_get_layer(const MenuLayer *menu_layer)
{
    return scroll_layer_get_layer(menu_layer->scroll_layer);
}

ScrollLayer *menu_layer_get_scroll_layer(const MenuLayer *menu_layer)
{
    return menu_layer->scroll_layer;
}

// Selected index handling -------------

static uint16_t get_num_rows(MenuLayer *menu_layer, uint16_t section)
{
    return menu_layer->callbacks.get_num_rows(menu_layer, section, menu_layer->context);
}

static bool has_next_index(const MenuLayer *menu_layer, const MenuIndex *index, bool up)
{
    // check if we can scroll in given direction
    return (up && (index->section > 0 || index->row > 0))
           || (!up && (index->section < menu_layer->end_index.section || index->row < menu_layer->end_index.row - 1));
}

static MenuIndex get_next_index(MenuLayer *menu_layer, bool up)
{
    MenuIndex index = menu_layer->selected;

    if (!has_next_index(menu_layer, &index, up))
        return index;

    if (up && index.row == 0)
    {
        --index.section;
        index.row = get_num_rows(menu_layer, index.section) - (uint16_t) 1;
    } else if (!up && index.row + 1 == get_num_rows(menu_layer, index.section))
    {
        ++index.section;
        index.row = 0;
    } else
    {
        index.row += up ? -1 : 1;
    }
    return index;
}

void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align, bool animated)
{
    menu_layer_set_selected_index(menu_layer, get_next_index(menu_layer, up), scroll_align, animated);
}

static MenuCellSpan *get_cell_span(MenuLayer *menu_layer, const MenuIndex *index)
{
    // TODO: optimize, binary search should be enough
    for (size_t cell = 0; cell < menu_layer->cells_count; ++cell)
        if (menu_index_compare(index, &menu_layer->cells[cell].index) == 0 && !menu_layer->cells[cell].header)
            return &menu_layer->cells[cell];

    return NULL;
}

static int16_t _get_aligned_edge_position(int16_t height, MenuRowAlign align)
{
    switch (align)
    {
        case MenuRowAlignCenter:
            return height / 2;
        case MenuRowAlignBottom:
            return height;
        default:
            return 0;
    }
}

void _menu_layer_update_scroll_offset(MenuLayer* menu_layer, MenuRowAlign scroll_align, bool animated) {
    MenuIndex index = menu_layer_get_selected_index(menu_layer);
    MenuCellSpan *cell = get_cell_span(menu_layer, &index);
    if (cell && scroll_align != MenuRowAlignNone)
    {
        if (menu_layer->isCenterFocused)
            scroll_align = MenuRowAlignCenter;
        GSize size = layer_get_frame(menu_layer->layer).size;
        int16_t span_pos = cell->y + _get_aligned_edge_position(cell->h, scroll_align);
        int16_t frame_pos = _get_aligned_edge_position(size.h, scroll_align);

        int16_t full_content_height = scroll_layer_get_content_size(menu_layer->scroll_layer).h;
        if (menu_layer->isBottomPaddingEnabled)
            full_content_height += MENU_BOTTOM_PADDING;

        GPoint new_offset = scroll_layer_get_content_offset(menu_layer->scroll_layer);
        new_offset.y = -(span_pos - frame_pos);
        if (menu_layer->isCenterFocused == false) {
            int16_t min_offset = MIN(size.h - full_content_height, 0);
            new_offset.y = CLAMP(new_offset.y, min_offset, 0);
        }
        scroll_layer_set_content_offset(menu_layer->scroll_layer, new_offset, animated);
    }
}

void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index, MenuRowAlign scroll_align, bool animated)
{
    if (menu_index_compare(&menu_layer->selected, &index) != 0)
    {
        menu_layer->selected = index;
        
        _menu_layer_update_scroll_offset(menu_layer, scroll_align, animated);
        layer_mark_dirty(menu_layer->layer);
    }
}

MenuIndex menu_layer_get_selected_index(const MenuLayer *menu_layer)
{
    return menu_layer->selected;
}


void menu_layer_pad_bottom_enable(MenuLayer *menu_layer, bool enable)
{
    if (menu_layer->isBottomPaddingEnabled != enable) {
        menu_layer->isBottomPaddingEnabled = enable;

        MenuIndex index = menu_layer->selected;
        if (!has_next_index(menu_layer, &index, false))
            _menu_layer_update_scroll_offset(menu_layer, MenuRowAlignCenter, false);
    }
}

bool menu_layer_get_center_focused(MenuLayer *menu_layer)
{
    return menu_layer->isCenterFocused;
}

void menu_layer_set_center_focused(MenuLayer *menu_layer, bool center_focused)
{
    if (menu_layer->isCenterFocused != center_focused) {
        menu_layer->isCenterFocused = center_focused;

        _menu_layer_update_scroll_offset(menu_layer, MenuRowAlignCenter, false);
    }
}

bool menu_layer_is_index_selected(const MenuLayer *menu_layer, MenuIndex *index)
{
    return menu_index_compare(&menu_layer->selected, index) == 0;
}

// Cell data --------------------

void menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context, MenuLayerCallbacks callbacks)
{
    assert(callbacks.get_num_rows && "'get_num_rows' callback has to be specified");
    menu_layer->callbacks = callbacks;
    menu_layer->context = callback_context;

    menu_layer_reload_data(menu_layer);
}

void menu_layer_reload_data(MenuLayer *menu_layer)
{
    int16_t sections = 1;
    if (menu_layer->callbacks.get_num_sections)
        sections = menu_layer->callbacks.get_num_sections(menu_layer, menu_layer->context);

    uint16_t last_section = (uint16_t) (sections - 1);
    menu_layer->end_index = MenuIndex(last_section, get_num_rows(menu_layer, last_section));

    // count cells
    size_t cells = 0;
    for (uint16_t section = 0; section < sections; ++section)
    {
        if (menu_layer->callbacks.get_header_height)
            ++cells;

        cells += menu_layer->callbacks.get_num_rows(menu_layer, section, menu_layer->context);
    }

    // allocate cells array if needed
    if (menu_layer->cells_count != cells)
    {
        if (menu_layer->cells_count > 0)
            free(menu_layer->cells);

        menu_layer->cells_count = cells;
        if (cells > 0)
           menu_layer->cells = (MenuCellSpan *)app_calloc(cells, sizeof(MenuCellSpan));
    }

    // generate cells
    size_t cell = 0;
    int16_t y = 0;
    int16_t h = 0;
    for (uint16_t section = 0; section < sections; ++section)
    {
        if (menu_layer->callbacks.get_header_height)
        {
            h = menu_layer->callbacks.get_header_height(menu_layer, section, menu_layer->context);
            menu_layer->cells[cell++] = MenuHeader(section, y, h);
            y += h;
            // TODO: add space for separator
        }

        uint16_t rows = menu_layer->callbacks.get_num_rows(menu_layer, section, menu_layer->context);
        for (uint16_t row = 0; row < rows; ++row)
        {
            MenuIndex index = MenuIndex(section, row);
            h = menu_layer->callbacks.get_cell_height
                ? menu_layer->callbacks.get_cell_height(menu_layer, &index, menu_layer->context)
                : MENU_CELL_BASIC_CELL_HEIGHT;
            menu_layer->cells[cell++] = MenuRow(section, row, y, h);
            y += h;
            // TODO: add space for separator
        }
    }


    GSize size = layer_get_frame(menu_layer->layer).size;
    size.h = y;
    scroll_layer_set_content_size(menu_layer->scroll_layer, size);
    _menu_layer_update_scroll_offset(menu_layer, MenuRowAlignCenter, false);
    layer_mark_dirty(menu_layer->layer);
}

// Input handling -------------

void menu_layer_set_click_config_provider(MenuLayer *menu_layer, ClickConfigProvider provider)
{
    menu_layer->click_config_provider = provider;
}

static void down_single_click_handler(ClickRecognizerRef _, MenuLayer *menu_layer)
{
    menu_layer_set_selected_next(menu_layer, false, MenuRowAlignCenter, ANIMATE_ON_CLICK);
}

static void up_single_click_handler(ClickRecognizerRef _, MenuLayer *menu_layer)
{
    menu_layer_set_selected_next(menu_layer, true, MenuRowAlignCenter, ANIMATE_ON_CLICK);
}

static void select_single_click_handler(ClickRecognizerRef _, MenuLayer *menu_layer)
{
    if (menu_layer->callbacks.select_click)
        menu_layer->callbacks.select_click(menu_layer, &menu_layer->selected, menu_layer->context);
}

static void select_long_click_handler(ClickRecognizerRef _, MenuLayer *menu_layer)
{
    if (menu_layer->callbacks.select_long_click)
        menu_layer->callbacks.select_long_click(menu_layer, &menu_layer->selected, menu_layer->context);
}

static void menu_layer_click_config_provider(MenuLayer *menu_layer)
{
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, BUTTON_REPEAT_INTERVAL_MS,
                                            (ClickHandler) down_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, BUTTON_REPEAT_INTERVAL_MS,
                                            (ClickHandler) up_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_single_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, BUTTON_LONG_CLICK_DELAY_MS, NULL,
                                (ClickHandler) select_long_click_handler);
    window_set_click_context(BUTTON_ID_DOWN, menu_layer);
    window_set_click_context(BUTTON_ID_UP, menu_layer);
    window_set_click_context(BUTTON_ID_SELECT, menu_layer);

    if (menu_layer->click_config_provider)
        menu_layer->click_config_provider(menu_layer->context);
}

void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window)
{
    window_set_click_config_provider_with_context(window, (ClickConfigProvider) menu_layer_click_config_provider,
                                                  menu_layer);
}

// Drawing --------------------

bool menu_cell_layer_is_highlighted(const Layer *cell_layer)
{
    return menu_layer_is_index_selected(cell_layer->container, &((MenuCellSpan *) cell_layer->callback_data)->index);
}

void menu_layer_set_normal_colors(MenuLayer *menu_layer, GColor background, GColor foreground)
{
    menu_layer->bg_color = background;
    menu_layer->fg_color = foreground;
    layer_mark_dirty(menu_layer->layer);
}

void menu_layer_set_highlight_colors(MenuLayer *menu_layer, GColor background, GColor foreground)
{
    menu_layer->bg_hi_color = background;
    menu_layer->fg_hi_color = foreground;
    layer_mark_dirty(menu_layer->layer);
}

static void menu_layer_draw_cell(GContext *context, const MenuLayer *menu_layer,
                                 MenuCellSpan *span,
                                 Layer *layer)
{
    bool highlighted = menu_layer_is_index_selected(menu_layer, &span->index) && !span->header;
    graphics_context_set_fill_color(context, highlighted ? menu_layer->bg_hi_color : menu_layer->bg_color);
    graphics_context_set_text_color(context, highlighted ? menu_layer->fg_hi_color : menu_layer->fg_color);
    
    // background
    if (menu_layer->callbacks.draw_background)
        menu_layer->callbacks.draw_background(context, layer, highlighted, menu_layer->context);
    else
#ifdef PBL_RECT
        graphics_fill_rect(context, GRect(0, 0, layer->frame.size.w, layer->frame.size.h), 0, GCornerNone);
#endif
    
    // cell content
    if (span->header)
    {
        if (menu_layer->callbacks.draw_header)
            menu_layer->callbacks.draw_header(context, layer, span->index.section, menu_layer->context);
    } else if (menu_layer->callbacks.draw_row)
        menu_layer->callbacks.draw_row(context, layer, &span->index, menu_layer->context);
    
    // TODO: draw separator
}

static void menu_layer_update_proc(Layer *layer, GContext *nGContext)
{
    MenuLayer *menu_layer = (MenuLayer *) layer->container;
    GRect frame = layer_get_frame(layer);
    
#ifndef PBL_RECT
    if (!menu_layer->callbacks.draw_background)
    {
        graphics_context_set_fill_color(nGContext, menu_layer->bg_hi_color);
        GPoint scroll_offset = scroll_layer_get_content_offset(menu_layer->scroll_layer);
        GRect background_rect = GRect(0, 
                              (layer->frame.size.h / 2) - (MENU_CELL_BASIC_CELL_HEIGHT / 2) - scroll_offset.y, 
                              layer->frame.size.w, 
                              MENU_CELL_BASIC_CELL_HEIGHT);
        graphics_fill_rect(nGContext, background_rect, 0, GCornerNone);
    }
#endif

    for (size_t cell = 0; cell < menu_layer->cells_count; ++cell)
    {
        // TODO: only draw visible cells based on layer frame/bounds
        MenuCellSpan *span = menu_layer->cells + cell;
        layer->callback_data = span;
        layer->frame = GRect(0, span->y, frame.size.w, span->h);
        // TODO: update bounds

        GRect offset = nGContext->offset;
        layer_apply_frame_offset(layer, nGContext);

        menu_layer_draw_cell(nGContext, menu_layer, span, layer);

        nGContext->offset = offset;
    }

    layer->frame = frame;
}

void menu_cell_basic_draw(GContext *ctx, const Layer *layer, const char *title,
                          const char *subtitle, GBitmap *icon)
{
    GRect frame = layer_get_frame(layer);
#ifdef PBL_RECT
    menu_cell_basic_draw_ex(ctx, frame, title, subtitle, icon, MENU_DEFAULT_TEXT_ALIGNMENT);
#else
    // the basic cell does not support icons on chalk
    menu_cell_basic_draw_ex(ctx, frame, title, subtitle, NULL, MENU_DEFAULT_TEXT_ALIGNMENT);
#endif
}

void menu_cell_basic_draw_ex(GContext *ctx, GRect frame, const char *title,
                          const char *subtitle, GBitmap *icon, GTextAlignment align)
{
    int16_t x = MENU_CELL_PADDING + frame.origin.x;
    if (icon)
    {
        GSize icon_size = icon->raw_bitmap_size;
        graphics_draw_bitmap_in_rect(ctx, icon, GRect(x, (frame.size.h - icon_size.h) / 2, icon_size.w, icon_size.h));
        x += icon_size.w + MENU_CELL_PADDING;
    }
    
    bool has_subtitle = false;
    
    if (subtitle)
    {
        has_subtitle = true;
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
        GRect subtitle_rect;
        subtitle_rect = GRect(x, frame.size.h / 2 - 2,
                              frame.size.w - x - MENU_CELL_PADDING, frame.size.h);
        graphics_draw_text(ctx, subtitle, font, subtitle_rect,
                               GTextOverflowModeTrailingEllipsis, align, 0);
    }

    if (title)
    {
        GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        GRect title_rect = GRect(x, frame.size.h / 2 - ( has_subtitle ? 26 : 18 ),
                                 frame.size.w - x - MENU_CELL_PADDING, frame.size.h);
        graphics_draw_text(ctx, title, title_font, title_rect,
                               GTextOverflowModeTrailingEllipsis, align, 0);
    }
}

void menu_cell_title_draw(GContext *ctx, const Layer *layer, const char *title)
{
    if (title)
    {
        GTextAlignment align = MENU_DEFAULT_TEXT_ALIGNMENT;
        GRect frame = layer_get_frame(layer);
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
        GRect title_rect = GRect(MENU_CELL_PADDING, frame.size.h / 2 - 18,
                                 frame.size.w - 2 * MENU_CELL_PADDING, frame.size.h);
        graphics_draw_text(ctx, title, font, title_rect,
                               GTextOverflowModeTrailingEllipsis, align, 0);
    }
}

void menu_cell_basic_header_draw(GContext *ctx, const Layer *layer, const char *title)
{
    if (title)
    {
        GTextAlignment align = MENU_DEFAULT_TEXT_ALIGNMENT;
        GRect frame = layer_get_frame(layer);
        GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
        GRect title_rect = GRect(MENU_CELL_PADDING, frame.size.h / 2 - 7,
                                 frame.size.w - 2 * MENU_CELL_PADDING, frame.size.h);
        graphics_draw_text(ctx, title, font, title_rect,
                               GTextOverflowModeTrailingEllipsis, align, 0);
    }
}
