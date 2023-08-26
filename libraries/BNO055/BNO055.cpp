#include "BNO055.h"

BNO055::BNO055(TwoWire *wire) {
    address = 0x28;
    page = 0;
}
