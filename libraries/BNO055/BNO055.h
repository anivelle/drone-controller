#include "Wire.h"

typedef enum Register {
    CHIP_ID = 0,
    ACC_ID,
    MAG_ID,
    GYR_ID,
    SW_REV_ID_LSB,
    SW_REV_ID_MSB,
    BL_REV_ID,
    PAGE_ID,
    ACC_DATA_X_LSB,
    ACC_DATA_X_MSB,
    ACC_DATA_Y_LSB,
    ACC_DATA_Y_MSB,
    ACC_DATA_Z_LSB,
    ACC_DATA_Z_MSB,
    MAG_DATA_X_LSB,
    MAG_DATA_X_MSB,
    MAG_DATA_Y_LSB,
    MAG_DATA_Y_MSB,
    MAG_DATA_Z_LSB,
    MAG_DATA_Z_MSB,
    GYR_DATA_X_LSB,
    GYR_DATA_X_MSB,
    GYR_DATA_Y_LSB,
    GYR_DATA_Y_MSB,
    GYR_DATA_Z_LSB,
    GYR_DATA_Z_MSB,
    EUL_HEADING_LSB,
    EUL_HEADING_MSB,
    EUL_ROLL_LSB,
    EUL_ROLL_MSB,
    EUL_PITCH_LSB,
    EUL_PITCH_MSB,
    QUA_DATA_W_LSB,
    QUA_DATA_W_MSB,
    QUA_DATA_X_LSB,
    QUA_DATA_X_MSB,
    QUA_DATA_Y_LSB,
    QUA_DATA_Y_MSB,
    QUA_DATA_Z_LSB,
    QUA_DATA_Z_MSB,
    LIA_DATA_X_LSB,
    LIA_DATA_X_MSB,
    LIA_DATA_Y_LSB,
    LIA_DATA_Y_MSB,
    LIA_DATA_Z_LSB,
    LIA_DATA_Z_MSB,
    GRV_DATA_X_LSB,
    GRV_DATA_X_MSB,
    GRV_DATA_Y_LSB,
    GRV_DATA_Y_MSB,
    GRV_DATA_Z_LSB,
    GRV_DATA_Z_MSB,
    TEMP,
    CALIB_STAT,
    ST_RESULT,
    INT_STA,
    SYS_CLK_STATUS,
    SYS_STATUS,
    SYS_ERR,
    UNIT_SEL,
    RESERVED_3C,
    OPR_MODE,
    PWR_MODE,
    SYS_TRIGGER,
    TEMP_SOURCE,
    AXIS_MAP_CONFIG,
    AXIS_MAP_SIGN,
    SIC_MATRIX_LSB0,
    SIC_MATRIX_MSB0,
    SIC_MATRIX_LSB1,
    SIC_MATRIX_MSB1,
    SIC_MATRIX_LSB2,
    SIC_MATRIX_MSB2,
    SIC_MATRIX_LSB3,
    SIC_MATRIX_MSB3,
    SIC_MATRIX_LSB4,
    SIC_MATRIX_MSB4,
    SIC_MATRIX_LSB5,
    SIC_MATRIX_MSB5,
    SIC_MATRIX_LSB6,
    SIC_MATRIX_MSB6,
    SIC_MATRIX_LSB7,
    SIC_MATRIX_MSB7,
    SIC_MATRIX_LSB8,
    SIC_MATRIX_MSB8,
    ACC_OFFSET_X_LSB,
    ACC_OFFSET_X_MSB,
    ACC_OFFSET_Y_LSB,
    ACC_OFFSET_Y_MSB,
    ACC_OFFSET_Z_LSB,
    ACC_OFFSET_Z_MSB,
    MAG_OFFSET_X_LSB,
    MAG_OFFSET_X_MSB,
    MAG_OFFSET_Y_LSB,
    MAG_OFFSET_Y_MSB,
    MAG_OFFSET_Z_LSB,
    MAG_OFFSET_Z_MSB,
    GYR_OFFSET_X_LSB,
    GYR_OFFSET_X_MSB,
    GYR_OFFSET_Y_LSB,
    GYR_OFFSET_Y_MSB,
    GYR_OFFSET_Z_LSB,
    GYR_OFFSET_Z_MSB,
    ACC_RADIUS_LSB,
    ACC_RADIUS_MSB,
    MAG_RADIUS_LSB,
    MAG_RADIUS_MSB,
    ACC_CONFIG = 8,
    MAG_CONFIG,
    GYR_CONFIG_0,
    GYR_CONFIG_1,
    ACC_SLEEP_CONFIG,
    GYR_SLEEP_CONFIG,
    RESERVED_E,
    INT_MSK,
    INT_EN,
    ACC_AM_THRES,
    ACC_INT_SETTINGS,
    ACC_HG_DURATION,
    ACC_HG_THRES,
    ACC_NM_THRES,
    ACC_NM_SET,
    GYT_INT_SET,
    GYR_HR_X_SET,
    GYR_DUR_X,
    GYR_HR_Y_SET,
    GYR_DUR_Y,
    GYR_HR_Z_SET,
    GYR_DUR_Z,
    GYR_AM_THRES,
    GYR_AM_SET,
    UNIQUE_ID0 = 0x50,
    UNIQUE_ID1,
    UNIQUE_ID2,
    UNIQUE_ID3,
    UNIQUE_ID4,
    UNIQUE_ID5,
    UNIQUE_ID6,
    UNIQUE_ID7,
    UNIQUE_ID8,
    UNIQUE_ID9,
    UNIQUE_IDA,
    UNIQUE_IDB,
    UNIQUE_IDC,
    UNIQUE_IDD,
    UNIQUE_IDE,
    UNIQUE_IDF
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

class BNO055 {
  private:
    uint8_t address;
    uint8_t page;
    TwoWire *wire;
    Mode mode;

  public:
    BNO055(TwoWire *i2c);

    /**
     * Directly read a register from the BNO055
     */
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
