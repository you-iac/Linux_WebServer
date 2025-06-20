//
// Created by 28695 on 2025/6/20.
//

#include "../inc/HttpConnect.h"

int main()
{
    HttpConnect* htcot = HttpConnect::Create(8);
    htcot->Run_OneToN();
}
