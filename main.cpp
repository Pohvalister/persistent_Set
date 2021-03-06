#include <iostream>
#include <algorithm>
#include "persistent_set.h"

using namespace std;

int rand1() {
    int val = rand() - RAND_MAX / 2;
    if (val != 0)
        return val;
    else
        return 1;
}

int main() {
    persistent_set a;
    a.insert(10);
    a.insert(20);
    persistent_set b = a;
    persistent_set c(a);
    int iter = 1000;
    for (int i = 0; i < iter; i++) {
        a.insert((value_type) rand1());
        b.insert((value_type) rand1());
        c.insert((value_type) rand1());
    }
    auto x = c.find(20);
    x--;

    auto y = c.begin();
    cout << (*c.find(*c.begin()))<<'\n';

    persistent_set d;
    std::vector<value_type> answ;
    for (int i = 0; i < iter; i++) {
        value_type z = (value_type) rand1();
        d.insert((z));
        answ.push_back(z);
    }
    std::random_shuffle(answ.begin(), answ.end());
    persistent_set e(d);
    persistent_set f = d;
    for (int i = 0; i < iter; i++) {

        //cout<<*d.find(answ[i])<<'\n';
        d.erase(d.find(answ[i]));
    }
    int iter2=20;
    persistent_set op;
    answ.clear();
    for (int i = 0; i < iter2; i++) {
        value_type z = (value_type) rand1();
        op.insert((z));
        answ.push_back(z);
    }
    auto o1=op.begin();
    auto o2=op.end();
    for (int i=0;i<iter2;i++){
        o2--;
        cout<<i<<") "<<*o1<<' '<<*o2<<'\n';
        o1++;
    }
    std::sort(answ.begin(),answ.end());
    for (int i=0;i<iter2;i++)
        cout<<answ[i]<<' ';


}


