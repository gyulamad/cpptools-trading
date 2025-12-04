#include <iostream>                // for std
#include "../../misc/EXTERN.hpp"  // for EXTERN
#include "../TestExchange.hpp"     // for TestExchange

using namespace std;

class TestExchange1: public TestExchange {
public:
    TestExchange1(): TestExchange(true, true, false) {}
    virtual ~TestExchange1() {}
};

EXTERN(TestExchange1, (), ())
