#include <assert.h>

#include "persistent_set.h"

void persistent_set::disconnect(node *point) {//nothrow safety
    if (point != nullptr) {
        point->users--;
        if (point->users == 0) {
            disconnect(point->left);
            disconnect(point->right);
            delete point;
        }
    }
}

inline void persistent_set::connect(node *point) {
    if (point != nullptr)
        point->users++;
}

persistent_set::persistent_set() {
    dock = new node(2);
    base = dock;
    surety = 0;
}

persistent_set::persistent_set(persistent_set const &other) {
    dock = other.dock;
    connect(dock);
    base = other.base;
    connect(dock);
    surety = 0;
}

persistent_set &persistent_set::operator=(persistent_set const &rhs) {
    disconnect(base);
    disconnect(dock);
    dock = rhs.dock;
    connect(dock);
    base = rhs.base;
    connect(dock);
    surety++;
    return *this;
}

persistent_set::~persistent_set() {
    disconnect(base);
    disconnect(dock);
}

persistent_set::iterator persistent_set::find_in_child(node *from, value_type x) {
    iterator tmp(from, this, surety);
    while (true) {
        if (tmp.point == dock || tmp.point->val > x) {
            if (tmp.point->left == nullptr)
                return tmp;
            else {
                tmp.upperParents.push(tmp.point);
                tmp.allParents.push(tmp.point);
                disconnect(tmp.point);
                tmp.point = tmp.point->left;
                connect(tmp.point);//tmp.point->users++;
                continue;
            }
        }
        if (tmp.point->val == x) {
            return tmp;
        }
        if (tmp.point->val < x) {
            if (tmp.point->right == nullptr) {
                return tmp;
            } else {
                tmp.lowerParents.push(tmp.point);
                tmp.allParents.push(tmp.point);
                disconnect(tmp.point);
                tmp.point = tmp.point->right;
                connect(tmp.point);//->users++;
                continue;
            }
        }
    }
}

persistent_set::iterator persistent_set::find(value_type x) {
    iterator tmp = find_in_child(base, x);
    if (tmp.point == dock || tmp.point->val != x)
        return iterator(dock, this, surety);
    connect(tmp.point);
    return tmp;
}


std::pair<persistent_set::node *, persistent_set::node *>
persistent_set::createImage(std::stack<node *> u, std::stack<node *> l, std::stack<node *> a) {//a.size!=0
    node *begin;
    node *end;
    begin = new node(a.top()->left, a.top()->right, a.top()->val);
    if (u.size() != 0 && u.top() == a.top())
        u.pop();
    if (l.size() != 0 && l.top() == a.top())
        l.pop();
    a.pop();
    end = begin;
    while (a.size() != 0) {
        if (u.top() != 0 && a.top() == u.top()) {
            end = new node(end, a.top()->right, a.top()->val);
            u.pop();
        } else {
            end = new node(a.top()->left, end, a.top()->val);
            l.pop();
        }
        a.pop();
    }
    return std::make_pair(begin, end);
};

std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type x) {
    iterator tmp = find_in_child(base, x);//ищем саму вершину
    if (tmp.point != dock && tmp.point->val == x) {//если уже есть
        connect(tmp.point);
        return std::make_pair(tmp, false);
    }
    std::pair<node *, node *> imageEnds;
    if (tmp.allParents.size() == 0) {//если не надо никуда идти и слева модно положить ( справа или dock или dock=base)
        node *toInsert = new node(nullptr, nullptr, x);
        node *toInsertParent = new node(toInsert, base->right, base->val);
        if (base == dock) {
            disconnect(dock);
            dock = toInsertParent;
            connect(toInsertParent);//->users++;
        }
        disconnect(base);
        base = toInsertParent;
        connect(toInsertParent);//->users++;
        iterator tmp2 = find_in_child(base, x);
        connect(tmp2.point);//тк в make pair
        return std::make_pair(tmp2, true);//tmp сломан => заново найдем - теперь она точно есть
    } else {
        imageEnds = createImage(tmp.upperParents, tmp.lowerParents,
                                tmp.allParents);//создаем путь от начала до нужной вершины

        node *toInsert = new node(nullptr, nullptr,
                                  x);//добавляем новую вершину ( она будет крайняя, тк мы дошли до nullptr
        node *toInsertParent = new node((tmp.point->val > x ? toInsert : tmp.point->left),
                                        (tmp.point->val > x ? tmp.point->right : toInsert),
                                        tmp.point->val);//изменяем её родителя - половина - на новую, половина куда и была

        if ((base == dock && imageEnds.first == imageEnds.second) ||
            imageEnds.first->val > toInsertParent->val) {//подсоединяем новый путь к сету
            disconnect(imageEnds.first->left);
            imageEnds.first->left = toInsertParent;
            connect(toInsertParent);//->users++;
        } else {
            disconnect(imageEnds.first->right);
            imageEnds.first->right = toInsertParent;
            connect(toInsertParent);//->users++;
        }
        if (base == dock) {
            disconnect(dock);
            dock = imageEnds.second;
            connect(imageEnds.second);//->users++;
        }
        disconnect(base);
        base = imageEnds.second;
        connect(imageEnds.second);//->users++;
        iterator tmp2 = find_in_child(base, x);
        connect(tmp2.point);//тк в make pair
        return std::make_pair(tmp2, true);//tmp сломан => заново найдем - теперь она точно есть
    }
};

void persistent_set::erase(iterator target) {
    target.check_valid();
    bool isImageEndsReal;
    node notRealImageEnds(0);
    std::pair<node *, node *> imageEnds;
    if (target.point == base) {//когда слишком рядом
        imageEnds = std::make_pair(&notRealImageEnds, &notRealImageEnds);
        isImageEndsReal = false;
    } else {
        imageEnds = createImage(target.upperParents, target.lowerParents,
                                target.allParents); //создадим путь от начала до вырезаемой вершины
        isImageEndsReal = true;
    }
    ////если уже можно подставить
    if (target.point->left == nullptr) {
        if (imageEnds.first->left == target.point) {
            disconnect(target.point);
            imageEnds.first->left = target.point->right;
            connect(target.point->right);//->users++;
        } else {
            disconnect(target.point);
            imageEnds.first->right = target.point->right;
            connect(target.point->right);//->users++;
        }
    }
    if (target.point->right == nullptr && target.point->left != nullptr) {
        if (imageEnds.first->left == target.point) {
            disconnect(target.point);
            imageEnds.first->left = target.point->left;
            connect(target.point->left);//->users++;
        } else {
            disconnect(target.point);
            imageEnds.first->right = target.point->left;
            connect(target.point->left);//->users++;
        }
    }
    ////если нельзя
    if (target.point->left != nullptr && target.point->right != nullptr) {
        iterator tmp = find_in_child(target.point->right,
                                     target.point->left->val);//спускаемся до конца правой в левую сторону
        if (tmp.allParents.size() == 0) {//когда слишком рядом
            node *toInsert = new node(target.point->left, target.point->right->right, target.point->right->val);
            (imageEnds.first->left == target.point ? imageEnds.first->left = toInsert
                                                   : imageEnds.first->right = toInsert);
            connect(toInsert);
        } else {
            std::pair<node *, node *> imageEndsCont = createImage(tmp.upperParents, tmp.lowerParents,
                                                                  tmp.allParents);//дублируем этот путь
            imageEndsCont.first->left->left = target.point->left;//переносим левую вершину
            connect(target.point->left);
            (imageEnds.first->left == target.point ? imageEnds.first->left = imageEndsCont.second
                                                   : imageEnds.first->right = imageEndsCont.second);
            connect(imageEndsCont.second);//соединяем концы веревок до корня
        }
    }
    //осталось подсоединить конец к базе
    if (base == dock) {
        disconnect(dock);
        dock = (isImageEndsReal ? imageEnds.second : notRealImageEnds.right);
        connect(isImageEndsReal ? imageEnds.second : notRealImageEnds.right);//->users++;
    }
    disconnect(base);
    base = (isImageEndsReal ? imageEnds.second : notRealImageEnds.right);
    connect(isImageEndsReal ? imageEnds.second : notRealImageEnds.right);//->users++;
    connect(target.point);
}

persistent_set::iterator persistent_set::begin() {//!!!!
    iterator tmp(base, this, surety);

    while (tmp.point->left != nullptr) {
        tmp.upperParents.push(tmp.point);
        tmp.allParents.push(tmp.point);
        disconnect(tmp.point);
        tmp.point = tmp.point->left;
        tmp.point->users++;
    }
    return tmp;
}

persistent_set::iterator persistent_set::end() {
    iterator tmp(base, this, surety);

    while (tmp.point->right != nullptr) {
        tmp.lowerParents.push(tmp.point);
        tmp.allParents.push(tmp.point);
        disconnect(tmp.point);
        tmp.point = tmp.point->right;
        tmp.point->users++;
    }
    return tmp;
}

///////////////////////
void persistent_set::iterator::find_left(iterator &it) {//left, then the rightest or parents right
    if (it.point->left != nullptr) {
        it.upperParents.push(it.point);
        it.allParents.push(it.point);
        disconnect(it.point);
        it.point = it.point->left;
        while (it.point->right != nullptr) {
            it.lowerParents.push(it.point);
            it.allParents.push(it.point);
            it.point = it.point->right;
        }
        connect(it.point);
    } else {
        disconnect(it.point);
        it.point = it.lowerParents.top();
        connect(it.point);
        it.lowerParents.pop();
        it.allParents.pop();
    }
}

void persistent_set::iterator::find_right(iterator &it) {//right, then the lefttest or parents left
    if (it.point->right != nullptr) {
        it.lowerParents.push(it.point);
        it.allParents.push(it.point);
        disconnect(it.point);
        it.point = it.point->right;
        while (it.point->left != nullptr) {
            it.upperParents.push(it.point);
            it.allParents.push(it.point);
            it.point = it.point->left;
        }
        connect(it.point);
    } else {
        disconnect(it.point);
        it.point = it.upperParents.top();
        connect(it.point);
        it.upperParents.pop();
        it.allParents.pop();
    }
}

///////////////////////
inline bool persistent_set::iterator::check_valid() const {
    assert(point != nullptr);
    assert(refer != nullptr);
    assert(check == refer->surety);
    return true;
}

value_type const &persistent_set::iterator::operator*() const {
    check_valid();
    return point->val;
}

persistent_set::iterator &persistent_set::iterator::operator++() {
    check_valid();
    assert(point != refer->dock);
    find_right(*this);
    point->users++;
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator++(int) {
    ++(*this);
    return *this;
}

persistent_set::iterator &persistent_set::iterator::operator--() {
    check_valid();
    find_left(*this);
    point->users++;
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator--(int) {
    --(*this);
    return *this;
}



