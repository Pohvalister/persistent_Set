#include <assert.h>

#include "persistent_set.h"



persistent_set::persistent_set() {
    dock = std::make_shared<node>(node());
    base = dock;
    surety = 0;
}

persistent_set::persistent_set(persistent_set const &other) {
    dock = other.dock;
    base = other.base;
    surety = 0;
}

persistent_set &persistent_set::operator=(persistent_set const &rhs) {
    dock = rhs.dock;
    base = rhs.base;
    surety++;
    return *this;
}


persistent_set::iterator persistent_set::find_in_child(sp from, value_type x) {
    iterator tmp(from, this, surety);
    while (true) {
        if (tmp.point == dock || tmp.point->val > x) {
            if (!tmp.point->left)
                return tmp;
            else {
                tmp.upperParents.push(tmp.point);
                tmp.allParents.push(tmp.point);
                tmp.point = tmp.point->left;
                continue;
            }
        }
        if (tmp.point->val == x) {
            return tmp;
        }
        if (tmp.point->val < x) {
            if (!tmp.point->right) {
                return tmp;
            } else {
                tmp.lowerParents.push(tmp.point);
                tmp.allParents.push(tmp.point);
                tmp.point = tmp.point->right;
                continue;
            }
        }
    }
}

persistent_set::iterator persistent_set::find(value_type x) {
    iterator tmp = find_in_child(base, x);
    if (tmp.point == dock || tmp.point->val != x)
        return iterator(dock, this, surety);
    return tmp;
}


std::pair<spp,spp>
persistent_set::createImage(std::stack<sp>& u, std::stack<sp>& l, std::stack<sp>& a) {
    sp begin=std::make_shared<node>(node(a.top()->left,a.top()->right,a.top()->val));
    if (u.size() != 0 && u.top() == a.top())
        u.pop();
    if (l.size() != 0 && l.top() == a.top())
        l.pop();
    a.pop();
    sp end = begin;
    while (a.size() != 0) {
        if (u.top() != 0 && a.top() == u.top()) {
            sp tmp =std::make_shared<node>(node(end, a.top()->right, a.top()->val));
            end = tmp;//new node(tmp, a.top()->right, a.top()->val);
            u.pop();
        } else {
            sp tmp=std::make_shared<node>(node(a.top()->left, end, a.top()->val));
            end = tmp;
            l.pop();
        }
        a.pop();
    }
    return std::make_pair(begin, end);
};

std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type x) {
    iterator tmp = find_in_child(base, x);//ищем саму вершину
    if (tmp.point != dock && tmp.point->val == x) {//если уже есть
        return std::make_pair(tmp, false);
    }
    if (tmp.allParents.size() == 0) {//если не надо никуда идти и слева модно положить ( справа или dock или dock=base)
        sp toInsert = std::make_shared<node>(node(x));
        sp toInsertParent = std::make_shared<node>(node(toInsert, base->right, base->val));
        if (base == dock) {
            dock = toInsertParent;
        }
        base = toInsertParent;
        iterator tmp2 = find_in_child(base, x);
        return std::make_pair(tmp2, true);//tmp сломан => заново найдем - теперь она точно есть
    } else {
        std::pair<sp, sp> imageEnds;
        imageEnds = createImage(tmp.upperParents, tmp.lowerParents,
                                tmp.allParents);//создаем путь от начала до нужной вершины

        sp toInsert = std::make_shared<node>(node(x));//добавляем новую вершину ( она будет крайняя, тк мы дошли до nullptr
        sp toInsertParent = std::make_shared<node>(node((tmp.point->val > x ? toInsert : tmp.point->left),
                                        (tmp.point->val > x ? tmp.point->right : toInsert),
                                        tmp.point->val));//изменяем её родителя - половина - на новую, половина куда и была

        if ((base == dock && imageEnds.first == imageEnds.second) ||
            imageEnds.first->val > toInsertParent->val) {//подсоединяем новый путь к сету
            imageEnds.first->left = toInsertParent;
        } else {
            imageEnds.first->right = toInsertParent;
        }
        if (base == dock) {
            dock = imageEnds.second;
        }
        base = imageEnds.second;
        iterator tmp2 = find_in_child(base, x);
        return std::make_pair(tmp2, true);//tmp сломан => заново найдем - теперь она точно есть
    }
};

void persistent_set::erase(iterator target) {
    target.check_valid();
    bool isImageEndsReal;
    node notRealImageEnds;
    std::pair<sp, sp> imageEnds;
    if (target.point == base) {//когда слишком рядом
        imageEnds = std::make_pair(std::make_shared<node>(notRealImageEnds), std::make_shared<node>(notRealImageEnds));
        isImageEndsReal = false;
    } else {
        imageEnds = createImage(target.upperParents, target.lowerParents,
                                target.allParents); //создадим путь от начала до вырезаемой вершины
        isImageEndsReal = true;
    }
    ////если уже можно подставить
    if (!target.point->left) {
        if (imageEnds.first->left == target.point) {
            imageEnds.first->left = target.point->right;
        } else {
            imageEnds.first->right = target.point->right;
        }
    }
    if (!target.point->right  && target.point->left) {
        if (imageEnds.first->left == target.point) {
            imageEnds.first->left = target.point->left;
        } else {
            imageEnds.first->right = target.point->left;
        }
    }
    ////если нельзя
    if (target.point->left && target.point->right ) {
        iterator tmp = find_in_child(target.point->right,
                                     target.point->left->val);//спускаемся до конца правой в левую сторону
        if (tmp.allParents.size() == 0) {//когда слишком рядом
            sp toInsert = std::make_shared<node>(node(target.point->left, target.point->right->right, target.point->right->val));
            (imageEnds.first->left == target.point ? imageEnds.first->left = toInsert
                                                   : imageEnds.first->right = toInsert);
        } else {
            std::pair<spp, spp> imageEndsCont = createImage(tmp.upperParents, tmp.lowerParents,
                                                                  tmp.allParents);//дублируем этот путь
            imageEndsCont.first->left->left = target.point->left;//переносим левую вершину
            (imageEnds.first->left == target.point ? imageEnds.first->left = imageEndsCont.second
                                                   : imageEnds.first->right = imageEndsCont.second);
        }
    }
    //осталось подсоединить конец к базе
    if (base == dock) {
        if (isImageEndsReal)
            dock= imageEnds.second;
        else
            dock = notRealImageEnds.right;
    }
    if (isImageEndsReal) base=imageEnds.second; else base= notRealImageEnds.right;
    if (!isImageEndsReal)
        delete &notRealImageEnds;
}

persistent_set::iterator persistent_set::begin() {//!!!!
    iterator tmp(base, this, surety);

    while (tmp.point->left) {
        tmp.upperParents.push(tmp.point);
        tmp.allParents.push(tmp.point);
        tmp.point = tmp.point->left;
        tmp.point->users++;
    }
    return tmp;
}

persistent_set::iterator persistent_set::end() {
    iterator tmp(base, this, surety);

    while (tmp.point->right) {
        tmp.lowerParents.push(tmp.point);
        tmp.allParents.push(tmp.point);
        tmp.point = tmp.point->right;
        tmp.point->users++;
    }
    return tmp;
}

///////////////////////
void persistent_set::iterator::find_left(iterator &it) {//left, then the rightest or parents right
    if (it.point->left) {
        it.upperParents.push(it.point);
        it.allParents.push(it.point);
        it.point = it.point->left;
        while (it.point->right) {
            it.lowerParents.push(it.point);
            it.allParents.push(it.point);
            it.point = it.point->right;
        }
    } else {
        while (it.upperParents.size()==0&&it.allParents.top()==it.upperParents.top()) {
            it.upperParents.pop();
            it.allParents.pop();
        }
        it.point = it.lowerParents.top();
        it.lowerParents.pop();
        it.allParents.pop();
    }
}

void persistent_set::iterator::find_right(iterator &it) {//right, then the lefttest or parents left
    if (it.point->right) {
        it.lowerParents.push(it.point);
        it.allParents.push(it.point);
        it.point = it.point->right;
        while (it.point->left) {
            it.upperParents.push(it.point);
            it.allParents.push(it.point);
            it.point = it.point->left;
        }
    } else {
        while (it.lowerParents.size()!=0 && it.allParents.top()==it.lowerParents.top()){
            it.allParents.pop();
            it.lowerParents.pop();
        }
        it.point = it.upperParents.top();
        it.upperParents.pop();
        it.allParents.pop();
    }
}

///////////////////////
inline bool persistent_set::iterator::check_valid() const {
    assert(point );
    assert(refer );
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
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator++(int) {
    iterator ans(point, refer, check);
    ++(*this);
    return ans;
}

persistent_set::iterator &persistent_set::iterator::operator--() {
    check_valid();
    find_left(*this);
    return *this;
}

persistent_set::iterator persistent_set::iterator::operator--(int) {
    iterator ans(point, refer, check);
    --(*this);
    return ans;
}



