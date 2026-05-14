#include <iostream>
using namespace std;



class A
{
public:
    A ( int x ) { m_X = x; cout << x; }
private:
    int m_X;
};

class B
{
public:
    B ( int x ) { m_A = new A ( x ); }
private:
    A * m_A;
};
int main(){
    B test( 97 );
}
