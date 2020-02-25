#include <iostream>

using namespace std;


class Hello {
    public:
        int* a;
        int b;
        Hello(int &a, int b) {
            this->a = &a;
            this->b = b;
        }
};

void print(Hello x) {
    cout << "Hello(a=" << *x.a << ", b=" << x.b << ")" <<endl;
}


void func(Hello x) {
    *(x.a) = 10;  // although x is copied by value, still a is changed
    x.b = 4;
    return;
}




int main() {
    int y = 1;
    Hello x(y, 2);

    func(x);
    print(x);
    return 0;


}