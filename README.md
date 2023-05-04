# OSP_Lab1_part2



**Цель работы**:
=====================
Написать программу, позволяющую выполнять рекурсивный поиск 
совпадений по заданной строке в динамических библиотеках, начиная с 
указанного каталога, с помощью динамических (разделяемых) библиотек-плагинов.


**Описание проекта**:
Лабораторная работа предполагает написание программы на языке 
программирования C, которая будет осуществлять рекурсивный поиск в 
каталогах при помощи функции nftw() и находить заданные строки в 
динамических библиотеках. Для выполнения поиска в каталогах будет 
использоваться функция nftw(), которая позволяет обойти все файлы и 
подкаталоги в заданном каталоге и его подкаталогах. Для каждого найденного 
файла программа будет выполнять проверку на наличие заданных байт внутри 
файла.
Программа на вход принимает 3 аргумента, не включая название самой программы <program_name> <path_to_lib> <option> <substring>. Добавлена возможность использовать переменную 
окружения LAB1DEBUG. С помощью ее можно вывести отладочную 
информацию на консоль. А именно вывести точное место, где был найдено 
совпадение байтов.
  
**Пример использования**
***
./lab12knmN32511 ./libknmN32511.so --dl-sym <STRING_TO_SEARCH> .
  
Символ "." обозначет текущая директория. Для произвольной директории поддерживается опция -P, в которой нужно указать путь до библиотеки. Также поддерживатеся переменная окружения LAB1DEBUG. С помощью, которой можно вывести на каком offset нашлось совпадение.





