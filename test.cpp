#include <iostream>
#include <stack>
using namespace std;

int main(int argc, char const *argv[])
{
    int n;
    cin >> n;
    for (int i = 1; i <= n; i++)
    {
        stack<int> s;
        int dif = 5;
        for (int j = i, k = 1; k <= i; j++, k++)
        {
            if (s.empty())
            {
                s.push(j);
            }
            else
            {
                s.push(s.top() + dif--);
            }
        }

        while (s.empty() == false)
        {
            cout << s.top() << " ";
            s.pop();
        }
        cout << endl;
    }
    return 0;
}
