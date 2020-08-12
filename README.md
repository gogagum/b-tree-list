# b-tree-list

## Краткое описание

Репозиторий содержит реализацию структуктуры данных с доступом, вставкой и удалением по
 индексу, сохраняющей данные в файл. Для простого доступа к участкам файла используется отображение
 на память 
 ([mapped_file](https://www.boost.org/doc/libs/1_72_0/libs/iostreams/doc/classes/mapped_file.html)
 из boost). Организация упорядоченного хранения объектов происходит с использованием b-дерева.
 
## Интерфейс

-     BTreeList(const std::string &filename, size_t size = 0);
Конструктор. `filename` - название файла для сохранения, `size` - изначальный размер.

-     Insert(unsigned index, const ElementType& e);
Вставть. `index` - позиция, куда вставить, `e` - элемент для вставки.

-     Insert(unsigned index, IteratorType begin, IteratorType end);
Вставить из контейнера по итераторам `begin` и `end`.

-     ElementType Extract(unsigned index);
Извлечь. `index` - позиция элемента, который удалить.

-     Set(unsigned index, const ElementType& e);
Установить элемент. `index` - позиция, куда вставить, `e` - элемент для установки.

-     ElementType Get(unsigned index);
Получить элемент. `index` - позиция элемента.

-     ElementType& operator[](unsigned index);
Оператор доступа по индексу.

-     ElementType operator[](unsigned index) const;
Оператор доступа по индексу.

-     size_t Size() const;
Узнать размер структуры.

## Анализ времени работы

Добавлен [python-notebook файл](./stress_tests/analysis/speed-analysis.ipynb), который
 содержит отчёт о времени выполнения некоторых операций над структурой.

