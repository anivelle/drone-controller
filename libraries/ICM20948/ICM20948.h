#include "Wire.h"

typedef enum Register {
};

typedef enum Axis {
    X,
    Y,
    Z,
    W_QUA = 0,
    X_QUA,
    Y_QUA,
    Z_QUA,
    HEADING = 0,
    ROLL,
    PITCH
};

typedef enum Mode {
    CONFIG,
    ACCONLY,
    MAGONLY,
    GYROONLY,
    ACCMAG,
    ACCGYRO,
    MAGGYRO,
    AMG,
    IMU,
    COMPASS,
    M4G,
    NDOF_FMC_OFF,
    NDOF
};

typedef enum Sys_Trigger {
    SELF_TEST = 1,
    RST_SYS = 1 << 5,
    RST_INT = 1 << 6,
    CLK_SEL = 1 << 7
};

typedef enum Interrupts {
    ACC_BSX_DRDY = 1,
    MAG_DRDY,
    GYRO_AM = 1 << 2,
    GYR_HIGH_RATE = 1 << 3,
    GYR_DRDY = 1 << 4,
    ACC_HIGH_G = 1 << 5,
    ACC_AM = 1 << 6,
    ACC_NM = 8
};

class ICM20948 {
  private:
    uint8_t address;
    uint8_t page;
    TwoWire *wire;
    Mode mode;

  public:
    ICM20948(TwoWire *i2c);

    // Direct access
    int16_t readRegister(Register reg);
    size_t writeRegister(Register reg, uint8_t data);

    int changePage(uint8_t page);
    int changeMode(Mode mode);
    // Raw Data
    int16_t readAcc(Axis axis); // Accelerometer
    int16_t readMag(Axis axis); // Magnetometer
    int16_t readGyr(Axis axis); // Gyroscope

    // Fusion data
    int16_t readQua(Axis axis); // Quaternion data
    int16_t readEul(Axis axis); // Euler angles
    int16_t readLia(Axis axis); // Linear Acceleration
};
