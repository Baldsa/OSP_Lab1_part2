# OSP_Lab1_part2



**Задание для второй части лабораторной работы по ОСП**

Написать программу, позволяющую выполнять рекурсивный поиск 
совпадений по заданной строке в динамических библиотеках, начиная с 
указанного каталога, с помощью динамических (разделяемых) библиотек-плагинов.


**Пример использования**

./lab12knmN32511 ./libknmN32511.so --dl-sym <STRING_TO_SEARCH> . 


Символ "." обозначет текущая директория. Для произвольной директории поддерживается опция -P, в которой нужно указать путь до библиотеки.
Также поддерживатеся переменная окружения LAB1DEBUG. С помощью, которой можно вывести на каком offset нашлось совпадение.






