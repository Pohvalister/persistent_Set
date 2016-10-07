#ifndef PERSISTENTSET_PERSISTENT_SET_H
#define PERSISTENTSET_PERSISTENT_SET_H

#include <cstdint>
#include <utility>
#include <cstdlib>
#include <stack>

// Вы можете определить этот тайпдеф по вашему усмотрению.
typedef int32_t value_type;

struct persistent_set {
    // Bidirectional iterator.
    struct iterator;

    // Создает пустой persistent_set.
    persistent_set();

    // Создает копию указанного persistent_set-а.
    persistent_set(persistent_set const &);

    // Изменяет this так, чтобы он содержал те же элементы, что и rhs.
    // Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
    persistent_set &operator=(persistent_set const &rhs);

    // Деструктор. Вызывается при удалении объектов persistent_set.
    // Инвалидирует все итераторы ссылающиеся на элементы этого persistent_set
    // (включая итераторы ссылающиеся на элементы следующие за последними).
    ~persistent_set();

    // Поиск элемента.
    // Возвращает итератор на элемент найденный элемент, либо end().
    iterator find(value_type);

    // Вставка элемента.
    // 1. Если такой ключ уже присутствует, вставка не производится, возвращается итератор
    //    на уже присутствующий элемент и false.
    // 2. Если такого ключа ещё нет, производится вставка, возвращается итератор на созданный
    //    элемент и true.
    // Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
    std::pair<iterator, bool> insert(value_type);

    // Удаление элемента.
    // Инвалидирует все итераторы, принадлежащие persistent_set'у this, включая end().
    void erase(iterator);

    // Возващает итератор на элемент с минимальный ключом.
    iterator begin();

    // Возващает итератор на элемент следующий за элементом с максимальным ключом.
    iterator end();

private:
    struct node {
        uint64_t users = 0;
        value_type val;
        node *left;
        node *right;

        /*node *next;
        node *prev;*/
        node(uint64_t u) {
            left = right = nullptr;
            users = u;
        }

        node(node *l, node *r, value_type v) {
            users = 0;
            val = v;
            left = l;
            if (l != nullptr)
                l->users++;

            right = r;
            if (r != nullptr)
                r->users++;
        }
    };

    node *base;//корень дерева
    node *dock;//последняя вершина
    uint64_t surety;//гарантия вал// идности итераторов
    static void connect(node *);

    static void disconnect(node *);

    iterator find_in_child(node *, value_type);

    std::pair<node *, node *> createImage(std::stack<node *>, std::stack<node *>, std::stack<node *>);
};

struct persistent_set::iterator {
    iterator(node *p, persistent_set *r, uint64_t c) {
        point = p;
        p->users++;
        refer = r;
        check = c;
    }

    ~iterator() {
        disconnect(point);
    }

    // Элемент на который сейчас ссылается итератор.
    // Разыменование итератора end() неопределено.
    // Разыменование невалидного итератора неопределено.
    value_type const &operator*() const;

    // Переход к элементу со следующим по величине ключом.
    // Инкремент итератора end() неопределен.
    // Инкремент невалидного итератора неопределен.
    iterator &operator++();

    iterator operator++(int);

    // Переход к элементу со следующим по величине ключом.
    // Декремент итератора begin() неопределен.
    // Декремент невалидного итератора неопределен.
    iterator &operator--();

    iterator operator--(int);

    inline bool check_valid() const;

    friend struct persistent_set;
private:
    node *point;
    std::stack<node *> upperParents;
    std::stack<node *> lowerParents;
    std::stack<node *> allParents;

    static void find_right(iterator &);

    static void find_left(iterator &);

    persistent_set *refer;
    size_t check;


};

#endif //PERSISTENTSET_PERSISTENT_SET_H
