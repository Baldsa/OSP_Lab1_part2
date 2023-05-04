#include <errno.h> // Подключение стандартной библиотеки для работы с кодами ошибок
#include <stdlib.h>// Подключение стандартной библиотеки для работы с памятью и строками
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>// Подключение библиотеки для динамической загрузки библиотек в процессе выполнения программы

#include "plugin_api.h"// Подключение пользовательской библиотеки, которая содержит API для работы с плагинами

int main(int argc, char *argv[]) {
    int opts_to_pass_len = 0; // Определяем переменную для хранения длины массива опций для передачи
    struct option *opts_to_pass = NULL;// Определяем указатель на массив опций для передачи и устанавливаем его в NULL
    struct option *longopts = NULL;// Определяем указатель на массив опций длинных параметров и устанавливаем его в NULL
    opterr = 0;// Устанавливаем флаг, чтобы отключить вывод ошибок getopt()
    
    // Minimum number of arguments is 4:  
    // $ program_name lib_name --opt1 file_to_check
    if (argc < 4) {    // Проверяем, что число аргументов не меньше 4    
        fprintf(stdout, "Usage: lab1call /path/to/libabcNXXXX.so [options_for_lib] /path/to/file\n");
        return 0;
    }
    char *lib_name = strdup(argv[1]);// Создаем копию имени библиотеки из первого аргумента
    bool version = 0, help = 0; // Устанавливаем переменные типа bool version и help в false(0)

    int option;// Определяем переменную для хранения текущей опции
    // Выделяем память для массива аргументов командной строки save_argv
    // и копируем туда аргументы из argv
    char** save_argv = calloc(argc, sizeof(char*));
    memcpy(save_argv, argv, argc * sizeof(char*));
    struct flag_option flags_opt = {0}; // Определяем структуру flag_option и инициализируем ее поля нулями
    while ((option = getopt(argc, save_argv, "P:AO:Nvh")) != -1) { // Анализируем аргументы командной строки
        switch (option) {
            case 'P':
                strcpy(lib_name, optarg);
                break;
            case 'A':
                flags_opt.op_and = 1;
                flags_opt.op_or = 0;
                break;
            case 'O':
                flags_opt.op_or = 1;
                flags_opt.op_and = 0;
                break;
            case 'N':
                flags_opt.op_not = 1;
                break;
            case 'v':
                version = 1;
                break;
            case 'h':
                help = 1;
                break;
            default:
                break;
        }
    }
    
    if (help) {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf("Options:\n");
        printf("  -P DIR   Specify the directory containing plugins\n");
        printf("  -A       Combine plugin options with AND operator (default)\n");
        printf("  -O       Combine plugin options with OR operator\n");
        printf("  -N       Negate plugin option search condition\n");
        printf("  -v       Print version information and exit\n");
        printf("  -h       Show this help message and exit\n");
        printf("  --dl-sym Checks whether the dynamic library contains the specified substring\n");
        printf("Example: ./lab12knmN32511 ./libknmN32511.so --dl-sym Nikita .\n");
        exit(EXIT_SUCCESS);
    } 
    // Name of the file to analyze. Should be passed as the last argumtent.
    char *file_name = strdup(argv[argc-1]);// Создание копии строки с именем файла
    
    struct plugin_info pi = {0};// Создание структуры информации о плагине, инициализация нулями
    
    void *dl = dlopen(lib_name, RTLD_LAZY);// Получение дескриптора динамической библиотеки
    if (!dl) {
        fprintf(stderr, "ERROR: dlopen() failed: %s\n", dlerror());
        goto END;
    }

    // Check for plugin_get_info() func
    void *func = dlsym(dl, "plugin_get_info"); // Получение указателя на функцию plugin_get_info()
    if (!func) {
        fprintf(stderr, "ERROR: dlsym() failed: %s\n", dlerror());
        goto END;
    }
    // Определение типа функции plugin_get_info() и приведение указателя на функцию к этому типу
    typedef int (*pgi_func_t)(struct plugin_info*);
    pgi_func_t pgi_func = (pgi_func_t)func;            
    // Вызов функции plugin_get_info() с передачей структуры plugin_info в качестве аргумента
    int ret = pgi_func(&pi);
    if (ret < 0) {        
        fprintf(stderr, "ERROR: plugin_get_info() failed\n");
        goto END;
    }
    
    if (version) {
    // Plugin info       
    fprintf(stdout, "Plugin purpose: %s\n", pi.plugin_purpose);
    fprintf(stdout, "Plugin author: %s\n", pi.plugin_author);
    fprintf(stdout, "Supported options: ");
    if (pi.sup_opts_len > 0) {
        for (size_t i = 0; i < pi.sup_opts_len; i++) {
            fprintf(stdout, " --%s\t\t Description: %s", pi.sup_opts[i].opt.name, pi.sup_opts[i].opt_descr);
        }
    }
    else {
        fprintf(stdout, "none (!?)\n");
    }
    fprintf(stdout, "\n");
        exit(EXIT_SUCCESS);
    }
    // If library supports no options then we have to stop
    if (pi.sup_opts_len == 0) { // Проверка на содержание дин. библиотекой опции, если нет то выход их программы
        fprintf(stderr, "ERROR: library supports no options! How so?\n");
        goto END;
    }

    // Get pointer to plugin_process_file()
    func = dlsym(dl, "plugin_process_file");// Получение указателя на функцию plugin_process_file()
    if (!func) {
        fprintf(stderr, "ERROR: no plugin_process_file() function found\n");
        goto END;
    }
    //  создаём тип данных ppf_func_t, который является указателем на функцию, принимающую четыре аргумента: строку const char*, массив опций struct option*, размер этого массива size_t, и структуру struct
    // flag_option*, и возвращающую целочисленное значение.
    typedef int (*ppf_func_t)(const char*, struct option*, size_t, struct flag_option*);
    ppf_func_t ppf_func = (ppf_func_t)func;            
   
    // Prepare array of options for getopt_long
    longopts = calloc(pi.sup_opts_len + 1, sizeof(struct option));
    if (!longopts) {
        fprintf(stderr, "ERROR: calloc() failed: %s\n", strerror(errno));
        goto END;
    }
    
    // Copy option information
    for (size_t i = 0; i < pi.sup_opts_len; i++) {
        // Mind this!
        // getopt_long() requires array of struct option in its longopts arg,
        // but pi.sup_opts is array of  structs, not option structs.
        memcpy(longopts + i, &pi.sup_opts[i].opt, sizeof(struct option));
    }
    
    // Prepare array of actually used options that will be passed to 
    // plugin_process_file() (Maximum pi.sup_opts_len options)
    opts_to_pass = calloc(pi.sup_opts_len, sizeof(struct option));
    if (!opts_to_pass) {
        fprintf(stderr, "ERROR: calloc() failed: %s\n", strerror(errno));
        goto END;
    }
    
    // Now process options for the lib
    while (1) {
        int opt_ind = 0;
        ret = getopt_long(argc, argv, "", longopts, &opt_ind);
        if (ret == -1) break;
        
        if (ret != 0) {
            fprintf(stderr, "ERROR: failed to parse options\n");
            goto END;
        }

#ifndef ALLOW_OPT_ABBREV
        // glibc quirk: no proper way to disable option abbreviations
        // https://stackoverflow.com/questions/5182041/turn-off-abbreviation-in-getopt-long-optarg-h
        int idx = (longopts + opt_ind)->has_arg ? 2 : 1;
        const char *actual_opt_name = argv[optind - idx] + 2; // +2 for -- before option
        const char *found_opt_name = (longopts + opt_ind)->name;
        if (strcmp(actual_opt_name, found_opt_name)) {
            // It's probably abbreviated name, which we do not allow
            fprintf(stderr, "ERROR: unknown option: %s\n", argv[optind - idx]);
            goto END;
        }
#endif
        
        // Check how many options we got up to this moment
        if ((size_t)opts_to_pass_len == pi.sup_opts_len) {
            fprintf(stderr, "ERROR: too many options!\n");
            goto END;
        }

        // Add this option to array of options actually passed to plugin_process_file()
        memcpy(opts_to_pass + opts_to_pass_len, longopts + opt_ind, sizeof(struct option));
        // Argument (if any) is passed in flag
        if ((longopts + opt_ind)->has_arg) {
            // Mind this!
            // flag is of type int*, but we are passing char* here (it's ok to do so).
            (opts_to_pass + opts_to_pass_len)->flag = (int*)strdup(optarg);
        }
        opts_to_pass_len++;        
    }

    if (getenv("LAB1DEBUG")) {
        fprintf(stderr, "DEBUG: opts_to_pass_len = %d\n", opts_to_pass_len);
        for (int i = 0; i < opts_to_pass_len; i++) {
            fprintf(stderr, "DEBUG: passing option '%s' with arg '%s'\n",
                (opts_to_pass + i)->name,
                (char*)(opts_to_pass + i)->flag);
        }
    }
 
    // Call plugin_process_file()
    errno = 0;
    ret = ppf_func(file_name, opts_to_pass, opts_to_pass_len, &flags_opt);
    fprintf(stdout, "\nplugin_process_file() returned %d\n", ret);
    if (ret < 0) {
        fprintf(stdout, "Error information: %s\n", strerror(errno));
    }
    
    END:
    if (opts_to_pass) {
        for (int i = 0; i < opts_to_pass_len; i++)
            free( (opts_to_pass + i)->flag );
        free(opts_to_pass);
    }
    if (save_argv) free(save_argv);
    if (longopts) free(longopts);
    if (lib_name) free(lib_name);
    if (file_name) free(file_name);
    if (dl) dlclose(dl);

    return 0;
}
