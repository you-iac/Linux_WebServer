//
// Created by 28695 on 2025/6/17.
//



#include <iostream>
#include "processpool.h"

int main()
{
    std::cout << "hello world!!!";
    processpool<int> *ppl = processpool<int>::create(1,16);
    ppl->run();
}
