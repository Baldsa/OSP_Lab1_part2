#define _GNU_SOURCE // Определение _GNU_SOURCE добавляет определения функции, таких как функции getline() и asprintf()/
#define _XOPEN_SOURCE 500 
#include <dirent.h>   // для работы с каталогами
#include <stdlib.h>   // для работы с функциями стандартной библиотеки
#include <string.h>   // для работы со строками
#include <math.h>     // для математических операций
#include <sys/mman.h> // для работы с отображением файлов в память
#include <sys/types.h>// для работы с типами данных, связанными с системными вызовами
#include <sys/stat.h> // для работы с информацией о файлах
#include <sys/param.h> // для использования макроса MIN()
#include <fcntl.h>    // для работы с файловыми дескрипторами
#include <unistd.h>   // для работы с системными вызовами Unix
#include <errno.h>    // для работы с кодами ошибок
#include <ftw.h>      // для работы с файловой системой
#include <stdio.h>    // для работы со стандартным вводом/выводом

#include "plugin_api.h"

#define MAX_INDENT_LEVEL 128 // Глубина рекурсии 

static char *g_lib_name = "libknmN32511.so"; //Название библиотеки

static char *g_plugin_purpose = "Search for dynamic libraries containing all the specified symbols"; // Цель плагина
 
static char *g_plugin_author = "Kopylov Nikita"; // Имя автора плагина 

#define OPT_DL_SYM "dl-sym" // Описание опции --dl-sym
const char* search_strings[10];
int lenght = 0; 
char *token;
char temp[1024]; 
/*Массив структур для, каждая из которых содержит опцию плагина для программы. Она содержит два поля структуру option, а также поле *opt_descr
    struct plugin_option {
        struct option {
           const char *name;
           int         has_arg;
           int        *flag;
           int         val;
        } opt,
        char *opt_descr
    }
    Поле opt является структурой option, которая содержит информацию об опции:

            name - название опции;
            has_arg - указывает, требует ли опция аргумент;
            flag - указатель на переменную, которая будет установлена в значение val, если опция будет передана;
            val - значение, которое будет установлено в flag, если опция будет передана.
*/

static struct plugin_option g_po_arr[] = {
	{
        {
            OPT_DL_SYM,
            required_argument,
            0, 0,
        },
        "string"
    },
};

static int g_po_arr_len = sizeof(g_po_arr)/sizeof(g_po_arr[0]); // Длина массива структур
//
//  Private functions
//
	void walk_dir(const char *dir); // Обход директории 
	int search_function(const char* filepath, const char** search_strings, int num_strings); // Функция поиска подстроки
	struct flag_option flags = {0}; // структура для флагов опций
//
//  API functions
//
// Функция plugin_get_info заполняет структуру struct plugin_info переданными значениями, 
//полученными из глобальных переменных g_plugin_purpose, g_plugin_author, g_po_arr_len и g_po_arr.
int plugin_get_info(struct plugin_info* ppi) {
    if (!ppi) {
        fprintf(stderr, "ERROR: invalid argument\n");
        return -1;
    }
    
    ppi->plugin_purpose = g_plugin_purpose;
    ppi->plugin_author = g_plugin_author;
    ppi->sup_opts_len = g_po_arr_len;
    ppi->sup_opts = g_po_arr;
    
    return 0;
}

// Макрос длля проверки // 
int plugin_process_file(const char *fname,//  Указатель на строку, содержащую имя обрабатываемого файла.
        struct option in_opts[], //  Массив опций, которые будут использоваться для обработки файла
        size_t in_opts_len, // Размер массива
        struct flag_option* flag)  //  Указатель на структуру, содержащую флаги, которые будут использоваться для обработки файла
         {
// Инициализируем флаги, используя переданный указатель на структуру flag_option
    flags.op_and = flag->op_and;
    flags.op_or = flag->op_or;
    flags.op_not = flag->op_not;
    char *debug = getenv("LAB1DEBUG");
// Проверяем переданные параметры на валидность
    if (!fname || !in_opts || !in_opts_len) {
        errno = EINVAL;
        return -1;
    }
    
    if (debug) {
        for (size_t i = 0; i < in_opts_len; i++) {
            fprintf(stderr, "DEBUG: %s: Got option '%s' with arg '%s'\n",
                g_lib_name, in_opts[i].name, (char*)in_opts[i].flag);
        }
    }
    int got_operator = 0; 
// Проверяем, была ли уже передана опция с заданным именем
    for (size_t i = 0; i < in_opts_len; i++) {
        if (!strcmp(in_opts[i].name, OPT_DL_SYM)) {
           
            if (got_operator) { 
                if(debug){  
                        fprintf(stderr, "DEBUG: %s: Option '%s' was already supplied\n", 
                        g_lib_name, in_opts[i].name); 
                }   
                errno = EINVAL; 
                return -1; 
                 
            } 
            else {
                // Если опция еще не была передана, копируем значение во временный буфер
            strcpy(temp, (char*)in_opts[i].flag);
            got_operator=1;

        }   
        }
        
    }
    // Разбиваем значение, переданное в опции, на отдельные строки и сохраняем их в массив search_strings
    int i = 0;
    token = strtok(temp, ",");
    while (token != NULL){
        search_strings[i] = token;
        i++;
        token = strtok(NULL, ",");
    }
    lenght = i;
    // Обрабатываем файлы
    walk_dir(fname);
    return 0; 
}

void searching_func(const char* filepath, const char** search_strings, int num_strings) {
    FILE* fp = fopen(filepath, "r");
    char buffer[1024];
    char* debug = getenv("LAB1DEBUG");
    char* extension = strrchr(filepath, '.'); // Получаем расширение файла 
    if (extension && !strcmp(extension, ".so")){ // Проверка на нужное расширение (.so)
        if (fp == NULL) {   
            printf("Cannot open file %s\n", filepath);
            return;
        }
        int count = 0; 
        int read_len = 0;
        if (flags.op_not == false){ // Проверка на флаг инфертирования 
        while ((read_len = fread(buffer, 1, sizeof(buffer), fp)) > 0) { // Цикл для считывания данных из файла
            for (int i = 0; i < num_strings; ++i)
            {
                int tmp_leng = strlen(search_strings[i]);
                for (int j = 0; j < (read_len - tmp_leng); j++) { // Цикл для обработки данных из файла 
                    if (memcmp(buffer + j, search_strings[i],tmp_leng) == 0) { // Сравнение данных из файла и нужной подстроки 
                        count++; // Счетчик++
                        if (debug != NULL){ // 
                            printf("Found string '%s' at offset %ld\n", search_strings[i], ftell(fp) - read_len);
                        }
                    }
            }
        }
        }
    }
    if (flags.op_not == true){
         while ((read_len = fread(buffer, 1, sizeof(buffer), fp)) > 0) { // Цикл для считывания данных из файла
            for (int i = 0; i < num_strings; ++i)
            {
                int tmp_leng = strlen(search_strings[i]);
                for (int j = 0; j < (read_len - tmp_leng); j++) { // Цикл для обработки данных из файла 
                    if (memcmp(buffer + j, search_strings[i],tmp_leng) != 0) { // Сравнение данных из файла и нужной подстроки 
                        //printf("%2s equal %s \n",buffer + j,search_strings[i]); 
                        count++; // Счетчик++
                    }
            }
        }
        }
    }
        fclose(fp);
        if (debug != NULL){
            for (int i = 0; i < num_strings; i++) {
                if (count == 0) {
                    printf("Did not find string '%s' in all libraries\n", search_strings[i]);
                    return;
                }
            }
        }
        if (count != 0 && flags.op_not == false){
            printf("Strings ");
        for (int i = 0; i < num_strings; i++){
            printf("\"%s\" " , search_strings[i]);
        }
        printf(" was founded %d times in %s", count, filepath);
        }  
        else if (count != 0 && flags.op_not == true){

            printf("The number of times how many rows were found that are not equal to the original %d in %s\n", count, filepath);
        }
    }
    else return;

}






void print_entry(int level __attribute__((unused)), int type __attribute__((unused)), const char *path) {
    if (!strcmp(path, ".") || !strcmp(path, "..")) // Проверяем, является ли запись текущей директорией или родительской директорией, если да, то пропускаем
        return;
    searching_func(path, search_strings, lenght); // Вызываем функцию поиска searching_func для поиска байтовой последовательности в содержимом файла
}

int walk_func(const char *fpath, const struct stat *sb __attribute__((unused)), int typeflag, struct FTW *ftwbuf) {
    print_entry(ftwbuf->level, typeflag, fpath); // Функция для обработки текущей записи 
    return 0;
}

void walk_dir(const char *dir) { 
    int res = nftw(dir, walk_func, 10, FTW_PHYS); // Вызов функции nftw() для обхода директории
    if (res < 0) { // Если директория не была найдена, выводим сообщение об ошибке
        fprintf(stderr, "ntfw() failed: %s\n", strerror(errno));
    }
}
