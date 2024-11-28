# stella-gc

Конфигурация
1. Для начала надо настроить cmake
```bash
mkdir -p cmake-build
cmake -B cmake-build
cmake --build cmake-build
```
2. Для запуска тестов нужно 
   1. в папку tests добавить скомпилированный в C файл на stella
   2. В файле заменить пусть до рантайма с `#include "stella/runtime.h"` до `#include "runtime.h"`
   3. При необходимости в тесте, в необходимых местах добавить `print_gc_alloc_stats();`
   4. В [CmakeLists.txt](CmakeLists.txt) изменить `add_executable(main tests/fibbonachi.c)` на `add_executable(main tests/test.c)`, где test.c желаемый тест
3. Собрать проект с желаемым тестом и запустить
   1. `cmake --build cmake-build`
   2. запустить через `./cmake-build/main`