# Диспетчеризация вызово посредством C++11 variadic templates

Двнный пример демонстрирует как можно простым и компактным кодом, без использования
внешних кодогенераторов вызывать C++ функции из интерпретатора командной строки.

Пример работы программы:

[vestnik@VestniK-laptop build]$ ./bin/cli 
list
Available functions:
 * add
 * containsBar
 * containsFoo
 * div
 * list
 * mod
 * mul
 * set
 * sub
add 1546 4747
6293
set QWE 16354
OK
div ${QWE} 23
711
div ewrhg 65
Error: T Context::parse(const std::string &) [T = int]: Failed to parse argument 'ewrhg'
containsFoo ${QWE}
false
containsFoo nofoobar
true