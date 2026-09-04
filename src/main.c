/**
 * @file main.c
 * @brief Aplicativo de Calendario Mensal Desacoplado para Tab5 OS
 */

#include "tab5_sdk.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CAL_TOTAL_CELLS 42

static int s_view_year = 2026;
static int s_view_month = 9;
static int s_today_year = 2026;
static int s_today_month = 9;
static int s_today_day = 3;
static int s_selected_day = 0;

static tab5_ui_obj_t s_lbl_month_year = TAB5_UI_INVALID_OBJ;
static tab5_ui_obj_t s_btn_prev = TAB5_UI_INVALID_OBJ;
static tab5_ui_obj_t s_btn_next = TAB5_UI_INVALID_OBJ;
static tab5_ui_obj_t s_btn_today = TAB5_UI_INVALID_OBJ;
static tab5_ui_obj_t s_day_cells[CAL_TOTAL_CELLS];
static tab5_ui_obj_t s_day_labels[CAL_TOTAL_CELLS];
static int s_cell_days[CAL_TOTAL_CELLS];

static const char *s_month_names[] = {
    "Janeiro", "Fevereiro", "Marco", "Abril", "Maio", "Junho",
    "Julho",   "Agosto",    "Setembro", "Outubro", "Novembro", "Dezembro"};

static const char *s_weekday_names[] = {"DOM", "SEG", "TER", "QUA",
                                        "QUI", "SEX", "SAB"};

static bool is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int get_days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30,
                               31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 30;
    }
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

/* Retorna o dia da semana para o primeiro dia do mes (0 = Domingo, 1 = Segunda, ...) */
static int get_first_weekday(int year, int month)
{
    /* Algoritmo de Sakamoto */
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = year - (month < 3 ? 1 : 0);
    return (y + y / 4 - y / 100 + y / 400 + t[month - 1] + 1) % 7;
}

static void update_calendar_view(void)
{
    uint32_t pal_surface = tab5_ui_theme_get_color(TAB5_UI_COLOR_SURFACE);
    uint32_t pal_surface_alt = tab5_ui_theme_get_color(TAB5_UI_COLOR_SURFACE_ALT);
    uint32_t pal_accent = tab5_ui_theme_get_color(TAB5_UI_COLOR_ACCENT);
    uint32_t pal_accent_soft = tab5_ui_theme_get_color(TAB5_UI_COLOR_ACCENT_SOFT);
    uint32_t pal_text = tab5_ui_theme_get_color(TAB5_UI_COLOR_TEXT);
    uint32_t pal_text_muted = tab5_ui_theme_get_color(TAB5_UI_COLOR_TEXT_MUTED);

    char title[128];
    const char *month_name = (s_view_month >= 1 && s_view_month <= 12)
                                 ? s_month_names[s_view_month - 1]
                                 : "Mes";
    snprintf(title, sizeof(title), "Calendario - %s %d", month_name,
             s_view_year);
    tab5_ui_app_bar_set_title(title);

    char month_year_str[64];
    snprintf(month_year_str, sizeof(month_year_str), "%s %d", month_name,
             s_view_year);
    if (s_lbl_month_year != TAB5_UI_INVALID_OBJ) {
        tab5_ui_label_set_text(s_lbl_month_year, month_year_str);
    }

    int first_dow = get_first_weekday(s_view_year, s_view_month);
    int days_in_cur = get_days_in_month(s_view_year, s_view_month);

    int prev_y = s_view_year;
    int prev_m = s_view_month - 1;
    if (prev_m < 1) {
        prev_m = 12;
        prev_y--;
    }
    int days_in_prev = get_days_in_month(prev_y, prev_m);

    for (int i = 0; i < CAL_TOTAL_CELLS; i++) {
        tab5_ui_obj_t cell = s_day_cells[i];
        tab5_ui_obj_t lbl = s_day_labels[i];
        if (cell == TAB5_UI_INVALID_OBJ || lbl == TAB5_UI_INVALID_OBJ) {
            continue;
        }

        char day_str[16];
        if (i < first_dow) {
            /* Dias do mes anterior */
            int d = days_in_prev - first_dow + 1 + i;
            s_cell_days[i] = -d;
            snprintf(day_str, sizeof(day_str), "%d", d);
            tab5_ui_label_set_text(lbl, day_str);
            tab5_ui_obj_set_style_bg(cell, pal_surface_alt, 100);
            tab5_ui_obj_set_style_text_color(lbl, pal_text_muted, 128);
        } else if (i < first_dow + days_in_cur) {
            /* Dias do mes atual */
            int d = i - first_dow + 1;
            s_cell_days[i] = d;
            snprintf(day_str, sizeof(day_str), "%d", d);
            tab5_ui_label_set_text(lbl, day_str);

            bool is_today = (s_view_year == s_today_year &&
                             s_view_month == s_today_month &&
                             d == s_today_day);
            bool is_selected = (s_selected_day == d);

            if (is_today) {
                tab5_ui_obj_set_style_bg(cell, pal_accent, 255);
                tab5_ui_obj_set_style_text_color(lbl, pal_surface, 255);
            } else if (is_selected) {
                tab5_ui_obj_set_style_bg(cell, pal_accent_soft, 255);
                tab5_ui_obj_set_style_text_color(lbl, pal_text, 255);
            } else {
                tab5_ui_obj_set_style_bg(cell, 0, 0);
                tab5_ui_obj_set_style_text_color(lbl, pal_text, 255);
            }
        } else {
            /* Dias do proximo mes */
            int d = i - (first_dow + days_in_cur) + 1;
            s_cell_days[i] = -d;
            snprintf(day_str, sizeof(day_str), "%d", d);
            tab5_ui_label_set_text(lbl, day_str);
            tab5_ui_obj_set_style_bg(cell, pal_surface_alt, 100);
            tab5_ui_obj_set_style_text_color(lbl, pal_text_muted, 128);
        }
    }
}

static void on_prev_month(void)
{
    s_view_month--;
    if (s_view_month < 1) {
        s_view_month = 12;
        s_view_year--;
    }
    s_selected_day = 0;
    tab5_sound_play_beep(1000, 25);
    update_calendar_view();
}

static void on_next_month(void)
{
    s_view_month++;
    if (s_view_month > 12) {
        s_view_month = 1;
        s_view_year++;
    }
    s_selected_day = 0;
    tab5_sound_play_beep(1200, 25);
    update_calendar_view();
}

static void on_today(void)
{
    struct tm t = {};
    if (tab5_system_get_time(NULL, &t) == TAB5_OK && t.tm_year > 0) {
        s_today_year = t.tm_year + 1900;
        s_today_month = t.tm_mon + 1;
        s_today_day = t.tm_mday;
    }
    s_view_year = s_today_year;
    s_view_month = s_today_month;
    s_selected_day = s_today_day;
    tab5_sound_play_beep(1500, 40);
    tab5_ui_show_toast("Data redefinida para hoje", 1500);
    update_calendar_view();
}

static void build_calendar_ui(void)
{
    uint32_t pal_surface = tab5_ui_theme_get_color(TAB5_UI_COLOR_SURFACE);
    uint32_t pal_surface_alt = tab5_ui_theme_get_color(TAB5_UI_COLOR_SURFACE_ALT);
    uint32_t pal_border = tab5_ui_theme_get_color(TAB5_UI_COLOR_BORDER);
    uint32_t pal_text = tab5_ui_theme_get_color(TAB5_UI_COLOR_TEXT);
    uint32_t pal_text_muted = tab5_ui_theme_get_color(TAB5_UI_COLOR_TEXT_MUTED);

    tab5_ui_obj_t scr = tab5_ui_get_screen();

    // Container raiz da tela do aplicativo (preenche largura, transparente, sem moldura modal)
    tab5_ui_obj_t main_cont = tab5_ui_container_create(scr);
    tab5_ui_obj_set_size(main_cont, TAB5_UI_PCT(100), TAB5_UI_SIZE_CONTENT);
    tab5_ui_obj_set_align(main_cont, TAB5_UI_ALIGN_TOP_MID, 0, 104);
    tab5_ui_obj_set_flex_flow(main_cont, TAB5_UI_FLEX_FLOW_COLUMN);
    tab5_ui_obj_set_style_bg(main_cont, 0, 0);
    tab5_ui_obj_set_style_border(main_cont, 0, 0);
    tab5_ui_obj_set_pad(main_cont, 12);
    tab5_ui_obj_set_gap(main_cont, 12);

    // 1. Cabecalho de navegacao
    tab5_ui_obj_t header_cont = tab5_ui_container_create(main_cont);
    tab5_ui_obj_set_size(header_cont, TAB5_UI_PCT(100), 52);
    tab5_ui_obj_set_flex_flow(header_cont, TAB5_UI_FLEX_FLOW_ROW);
    tab5_ui_obj_set_style_bg(header_cont, 0, 0);
    tab5_ui_obj_set_style_border(header_cont, 0, 0);
    tab5_ui_obj_set_pad(header_cont, 0);
    tab5_ui_obj_set_gap(header_cont, 8);

    s_btn_prev = tab5_ui_btn_create(header_cont, " < ");
    tab5_ui_obj_set_size(s_btn_prev, 46, 46);
    tab5_ui_obj_set_style_radius(s_btn_prev, 8);
    tab5_ui_obj_set_style_bg(s_btn_prev, pal_surface, 255);
    tab5_ui_obj_set_style_border(s_btn_prev, pal_border, 1);
    tab5_ui_obj_set_style_text_color(s_btn_prev, pal_text, 255);

    s_lbl_month_year = tab5_ui_label_create(header_cont, "Setembro 2026");
    tab5_ui_obj_set_flex_grow(s_lbl_month_year, 1);
    tab5_ui_obj_set_size(s_lbl_month_year, 0, 46);
    tab5_ui_obj_set_align(s_lbl_month_year, TAB5_UI_ALIGN_CENTER, 0, 0);
    tab5_ui_obj_set_style_text_color(s_lbl_month_year, pal_text, 255);

    s_btn_today = tab5_ui_btn_create(header_cont, "Hoje");
    tab5_ui_obj_set_size(s_btn_today, 70, 46);
    tab5_ui_obj_set_style_radius(s_btn_today, 8);
    tab5_ui_obj_set_style_bg(s_btn_today, pal_surface, 255);
    tab5_ui_obj_set_style_border(s_btn_today, pal_border, 1);
    tab5_ui_obj_set_style_text_color(s_btn_today, pal_text, 255);

    s_btn_next = tab5_ui_btn_create(header_cont, " > ");
    tab5_ui_obj_set_size(s_btn_next, 46, 46);
    tab5_ui_obj_set_style_radius(s_btn_next, 8);
    tab5_ui_obj_set_style_bg(s_btn_next, pal_surface, 255);
    tab5_ui_obj_set_style_border(s_btn_next, pal_border, 1);
    tab5_ui_obj_set_style_text_color(s_btn_next, pal_text, 255);

    // 2. Linha dos dias da semana
    tab5_ui_obj_t wdays_cont = tab5_ui_container_create(main_cont);
    tab5_ui_obj_set_size(wdays_cont, TAB5_UI_PCT(100), 38);
    tab5_ui_obj_set_flex_flow(wdays_cont, TAB5_UI_FLEX_FLOW_ROW);
    tab5_ui_obj_set_style_bg(wdays_cont, pal_surface_alt, 255);
    tab5_ui_obj_set_style_border(wdays_cont, pal_border, 1);
    tab5_ui_obj_set_style_radius(wdays_cont, 6);
    tab5_ui_obj_set_pad(wdays_cont, 0);
    tab5_ui_obj_set_gap(wdays_cont, 0);

    for (int i = 0; i < 7; i++) {
        tab5_ui_obj_t wday_box = tab5_ui_container_create(wdays_cont);
        tab5_ui_obj_set_flex_grow(wday_box, 1);
        tab5_ui_obj_set_size(wday_box, 0, TAB5_UI_PCT(100));
        tab5_ui_obj_set_style_bg(wday_box, 0, 0);
        tab5_ui_obj_set_style_radius(wday_box, 0);
        tab5_ui_obj_set_pad(wday_box, 0);
        tab5_ui_obj_set_gap(wday_box, 0);
        if (i < 6) {
            tab5_ui_obj_set_style_border(wday_box, pal_border, 1);
        } else {
            tab5_ui_obj_set_style_border(wday_box, 0, 0);
        }

        tab5_ui_obj_t wday_lbl =
            tab5_ui_label_create(wday_box, s_weekday_names[i]);
        tab5_ui_obj_set_align(wday_lbl, TAB5_UI_ALIGN_CENTER, 0, 0);
        tab5_ui_obj_set_style_text_color(wday_lbl, pal_text_muted, 255);
    }

    // 3. Grade de dias unificada (tabela com bordas conectadas)
    tab5_ui_obj_t grid_cont = tab5_ui_container_create(main_cont);
    tab5_ui_obj_set_size(grid_cont, TAB5_UI_PCT(100), TAB5_UI_SIZE_CONTENT);
    tab5_ui_obj_set_flex_flow(grid_cont, TAB5_UI_FLEX_FLOW_COLUMN);
    tab5_ui_obj_set_style_bg(grid_cont, pal_surface, 255);
    tab5_ui_obj_set_style_border(grid_cont, pal_border, 1);
    tab5_ui_obj_set_style_radius(grid_cont, 8);
    tab5_ui_obj_set_pad(grid_cont, 0);
    tab5_ui_obj_set_gap(grid_cont, 0);

    for (int r = 0; r < 6; r++) {
        tab5_ui_obj_t row_cont = tab5_ui_container_create(grid_cont);
        tab5_ui_obj_set_size(row_cont, TAB5_UI_PCT(100), 52);
        tab5_ui_obj_set_flex_flow(row_cont, TAB5_UI_FLEX_FLOW_ROW);
        tab5_ui_obj_set_style_bg(row_cont, 0, 0);
        tab5_ui_obj_set_style_radius(row_cont, 0);
        tab5_ui_obj_set_pad(row_cont, 0);
        tab5_ui_obj_set_gap(row_cont, 0);
        if (r < 5) {
            tab5_ui_obj_set_style_border(row_cont, pal_border, 1);
        } else {
            tab5_ui_obj_set_style_border(row_cont, 0, 0);
        }

        for (int c = 0; c < 7; c++) {
            int idx = r * 7 + c;
            tab5_ui_obj_t cell = tab5_ui_container_create(row_cont);
            tab5_ui_obj_set_flex_grow(cell, 1);
            tab5_ui_obj_set_size(cell, 0, TAB5_UI_PCT(100));
            tab5_ui_obj_set_style_bg(cell, 0, 0);
            tab5_ui_obj_set_style_radius(cell, 0);
            tab5_ui_obj_set_pad(cell, 0);
            tab5_ui_obj_set_gap(cell, 0);
            if (c < 6) {
                tab5_ui_obj_set_style_border(cell, pal_border, 1);
            } else {
                tab5_ui_obj_set_style_border(cell, 0, 0);
            }
            tab5_ui_obj_set_clickable(cell, true);

            tab5_ui_obj_t lbl = tab5_ui_label_create(cell, " ");
            tab5_ui_obj_set_align(lbl, TAB5_UI_ALIGN_CENTER, 0, 0);
            tab5_ui_obj_set_style_text_color(lbl, pal_text, 255);

            s_day_cells[idx] = cell;
            s_day_labels[idx] = lbl;
            s_cell_days[idx] = 0;
        }
    }
}

static void app_init(void)
{
    tab5_system_log(2, "tab5_calendar",
                    "Aplicativo Calendario desacoplado iniciado");
    tab5_ui_app_bar_set_title("Calendario");

    struct tm t = {};
    if (tab5_system_get_time(NULL, &t) == TAB5_OK && t.tm_year > 0) {
        s_today_year = t.tm_year + 1900;
        s_today_month = t.tm_mon + 1;
        s_today_day = t.tm_mday;
    }
    s_view_year = s_today_year;
    s_view_month = s_today_month;
    s_selected_day = s_today_day;

    build_calendar_ui();
    update_calendar_view();
}

static void app_resume(void)
{
    tab5_system_log(2, "tab5_calendar", "Calendario retomado");
}

static void app_pause(void)
{
    tab5_system_log(2, "tab5_calendar", "Calendario pausado");
}

static void app_destroy(void)
{
    tab5_system_log(2, "tab5_calendar", "Calendario finalizado");
}

TAB5_APP_EXPORT void tab5_app_on_theme_changed(bool dark)
{
    (void)dark;
    tab5_ui_clear_content();
    build_calendar_ui();
}

TAB5_APP_EXPORT void tab5_app_on_ui_event(tab5_ui_obj_t obj,
                                          uint32_t event_type,
                                          int32_t event_val)
{
    (void)event_val;
    if (event_type != TAB5_UI_EVENT_CLICKED) {
        return;
    }

    if (obj == s_btn_prev) {
        on_prev_month();
    } else if (obj == s_btn_next) {
        on_next_month();
    } else if (obj == s_btn_today) {
        on_today();
    } else {
        for (int i = 0; i < CAL_TOTAL_CELLS; i++) {
            if (obj == s_day_cells[i] || obj == s_day_labels[i]) {
                if (s_cell_days[i] > 0) {
                    s_selected_day = s_cell_days[i];
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Data: %02d/%02d/%04d",
                             s_selected_day, s_view_month, s_view_year);
                    tab5_ui_show_toast(buf, 1500);
                    tab5_sound_play_beep(1800, 30);
                    update_calendar_view();
                }
                break;
            }
        }
    }
}

TAB5_APP_EXPORT int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    tab5_lifecycle_callbacks_t cbs = {
        .on_init = app_init,
        .on_resume = app_resume,
        .on_pause = app_pause,
        .on_destroy = app_destroy,
        .on_open_file = NULL,
    };

    tab5_lifecycle_register(&cbs);
    app_init();
    return 0;
}
