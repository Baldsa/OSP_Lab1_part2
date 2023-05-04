#ifndef _PLUGIN_API_H
#define _PLUGIN_API_H

#include <getopt.h>
#include <stdbool.h>

/*
    Структура, описывающая опцию, поддерживаемую плагином.
*/
struct plugin_option {
    /* Опция в формате, поддерживаемом getopt_long (man 3 getopt_long). */
    struct option opt;
    /* Описание опции, которое предоставляет плагин. */
    const char *opt_descr;
};


struct flag_option {
	bool op_and;
	bool op_or;
	bool op_not;
};

/*
    Структура, содержащая информацию о плагине.
*/
struct plugin_info {
    /* Назначение плагина */
    const char *plugin_purpose;
    /* Автор плагина, например "Иванов Иван Иванович, N32xx" */
    const char *plugin_author;
    /* Длина списка опций */
    size_t sup_opts_len;
    /* Список опций, поддерживаемых плагином */
    struct plugin_option *sup_opts;
};


int plugin_get_info(struct plugin_info* ppi);



int plugin_process_file(const char *fname,
        struct option in_opts[],
        size_t in_opts_len,
        struct flag_option* flags);        
#endif
