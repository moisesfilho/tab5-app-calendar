/**
 * @file main.c
 * @brief Aplicativo de Calendario Mensal para Tab5 OS
 */

#include "tab5_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int s_view_year = 2026;
static int s_view_month = 8;
static int s_today_year = 2026;
static int s_today_month = 8;
static int s_today_day = 29;

static const char *s_month_names[] = {
    "Janeiro", "Fevereiro", "Marco", "Abril", "Maio", "Junho",
    "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"
};

static const char *s_weekday_names[] = {
    "DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"
};

static bool is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int get_days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
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
    char title[128];
    const char *month_name = (s_view_month >= 1 && s_view_month <= 12)
                                 ? s_month_names[s_view_month - 1]
                                 : "Mes";
    snprintf(title, sizeof(title), "Calendario - %s %d", month_name, s_view_year);
    tab5_ui_app_bar_set_title(title);

    char buf[1024];
    size_t offset = 0;

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "====================================\n"
                       "         %s %d\n"
                       "====================================\n"
                       " %-4s %-4s %-4s %-4s %-4s %-4s %-4s\n"
                       "------------------------------------\n",
                       month_name, s_view_year,
                       s_weekday_names[0], s_weekday_names[1], s_weekday_names[2],
                       s_weekday_names[3], s_weekday_names[4], s_weekday_names[5],
                       s_weekday_names[6]);

    int first_day = get_first_weekday(s_view_year, s_view_month);
    int total_days = get_days_in_month(s_view_year, s_view_month);

    int col = 0;
    for (int i = 0; i < first_day; i++) {
        offset += snprintf(buf + offset, sizeof(buf) - offset, "     ");
        col++;
    }

    for (int day = 1; day <= total_days; day++) {
        bool is_today = (s_view_year == s_today_year &&
                         s_view_month == s_today_month &&
                         day == s_today_day);
        if (is_today) {
            offset += snprintf(buf + offset, sizeof(buf) - offset, "[%2d] ", day);
        } else {
            offset += snprintf(buf + offset, sizeof(buf) - offset, " %2d  ", day);
        }
        col++;
        if (col == 7) {
            offset += snprintf(buf + offset, sizeof(buf) - offset, "\n");
            col = 0;
        }
    }

    if (col != 0) {
        offset += snprintf(buf + offset, sizeof(buf) - offset, "\n");
    }

    offset += snprintf(buf + offset, sizeof(buf) - offset,
                       "====================================\n"
                       " Hoje: %02d/%02d/%04d\n",
                       s_today_day, s_today_month, s_today_year);

    tab5_ui_obj_t ta = tab5_ui_get_main_textarea();
    if (ta != NULL) {
        tab5_ui_textarea_set_text(ta, buf);
    }
}

static void on_prev_month(void *user_data)
{
    (void)user_data;
    s_view_month--;
    if (s_view_month < 1) {
        s_view_month = 12;
        s_view_year--;
    }
    tab5_sound_play_beep(1000, 25);
    update_calendar_view();
}

static void on_next_month(void *user_data)
{
    (void)user_data;
    s_view_month++;
    if (s_view_month > 12) {
        s_view_month = 1;
        s_view_year++;
    }
    tab5_sound_play_beep(1200, 25);
    update_calendar_view();
}

static void on_today(void *user_data)
{
    (void)user_data;
    struct tm t = {};
    if (tab5_system_get_time(NULL, &t) == TAB5_OK && t.tm_year > 0) {
        s_today_year = t.tm_year + 1900;
        s_today_month = t.tm_mon + 1;
        s_today_day = t.tm_mday;
    }
    s_view_year = s_today_year;
    s_view_month = s_today_month;
    tab5_sound_play_beep(1500, 40);
    tab5_ui_show_toast("Data redefinida para hoje", 1500);
    update_calendar_view();
}

static void app_init(void)
{
    tab5_system_log(2, "tab5_calendar", "Aplicativo Calendario iniciado");

    struct tm t = {};
    if (tab5_system_get_time(NULL, &t) == TAB5_OK && t.tm_year > 0) {
        s_today_year = t.tm_year + 1900;
        s_today_month = t.tm_mon + 1;
        s_today_day = t.tm_mday;
    }
    s_view_year = s_today_year;
    s_view_month = s_today_month;

    tab5_ui_app_bar_add_action_button("<", on_prev_month, NULL);
    tab5_ui_app_bar_add_action_button(">", on_next_month, NULL);
    tab5_ui_app_bar_add_action_button("Hoje", on_today, NULL);

    update_calendar_view();
    tab5_ui_show_toast("Calendario aberto", 1500);
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
    return 0;
}
