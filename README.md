# b-tree-list

## Краткое описание

Репозиторий содержит реализацию структуктуры данных с доступом, вставкой и удалением по
 индексу, сохраняющей данные в файл. Для простого доступа к участкам файла используется отображение
 на память 
 ([mapped_file](https://www.boost.org/doc/libs/1_72_0/libs/iostreams/doc/classes/mapped_file.html)
 из boost). Организация упорядоченного хранения объектов происходит с использованием b-дерева.

## Требования / зависимости

`C++17`, `boost 1.73`
 
## Интерфейс

-     BTreeList(const std::string &filename, bool rebuild_flag = true);
Конструктор. `filename` - название файла для сохранения, `rebuild_flag` - переменная,
 отвечающая за перестраивание дерева в деструкторе.

-     BTreeList(const std::string &filename, SizeType size, bool rebuild_flag = true);
Конструктор. Создаёт файл для дерева размера `size`, игнорируя возможно существующий
 файл с названием `filename`.

-     BTreeList(const std::string &filename,
                size_t size,
                const ElementType &element_to_fill,
                bool rebuild_flag = true);
Конструктор. Создаёт файл для дерева размера `size`, игнорируя возможно существующий
 файл с названием `filename` и заполняет структуру копиями элемента
 `element_to_fill`.

-      BTreeList(const std::string &filename,
                 IteratorType begin, IteratorType end,
                 bool rebuild_flag = true);
Конструктор. Создаёт файл для дерева с размером, соответствующим итераторам `begin` и
`end`, игнорируя возможно существующий файл с названием `filename`.

-     Insert(unsigned index, const ElementType& e);
Вставть. `index` - позиция, куда вставить, `e` - элемент для вставки.

-     Insert(unsigned index, IteratorType begin, IteratorType end);
Вставить из контейнера по итераторам `begin` и `end`.

-     ElementType Extract(unsigned index);
Извлечь. `index` - позиция элемента, который удалить.

-     ElementType& operator[](unsigned index);
Оператор доступа по индексу.

-     ElementType operator[](unsigned index) const;
Оператор доступа по индексу.

-     size_t Size() const;
Узнать размер структуры.

## Анализ времени работы

[python-notebook файл](./stress_tests/analysis/after_adding_memcpy/speed-analysis.ipynb)
 содержит отчёт о времени выполнения некоторых операций над структурой.