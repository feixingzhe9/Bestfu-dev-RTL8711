#define IO_3V_TO_5V

#define I2C_FAIL        0
#define I2C_SUCCESS     1

#define DEBUG_I2C_BL5372   

//ygl added here 20101225, maybe this define should be put in product.conf.
#define USE_DBGUART_GPIO_FOR_RTC_I2C
//#define USE_EXTGAME_GPIO_FOR_RTC_I2C

#if defined(USE_DBGUART_GPIO_FOR_RTC_I2C)
//use debug uart gpio for i2c com here.
#define I2C_BL5372_SCL SPHE1500_PIN_Y1     
#define I2C_BL5372_SDA SPHE1500_PIN_W2
#elif defined(USE_EXTGAME_GPIO_FOR_RTC_I2C)
//use external game gpio for i2c com here.
#define I2C_BL5372_SCL SPHE1500_PIN_Y2
#define I2C_BL5372_SDA SPHE1500_PIN_W1
#else
//use debug uart gpio for i2c com here.
#define I2C_BL5372_SCL SPHE1500_PIN_E6     
#define I2C_BL5372_SDA SPHE1500_PIN_E5
#endif

#define I2C_SPEED 50000 //200---100kbps standard mode ;40---400kbps    fast mode

#ifdef DEBUG_I2C_BL5372
   #define Set_ERR    diag_printf
#else
   #define Set_ERR
#endif


//initial gpio,control by Risk
#define I2C_RISC_CTL() \
    GPIO_M_SET(I2C_BL5372_SCL,1);\
    GPIO_M_SET(I2C_BL5372_SDA,1)

//release I2C
#define Release_I2C()\
    GPIO_E_SET(I2C_BL5372_SCL,0);\
    GPIO_E_SET(I2C_BL5372_SDA,0);\
    GPIO_M_SET(I2C_BL5372_SCL,0);\
    GPIO_M_SET(I2C_BL5372_SDA,0)
    
//output low level
#define I2C_IO_OUTPUT_LOW( ioNum ) \
    GPIO_E_SET( ioNum, 1 );\
    GPIO_O_SET( ioNum, 0 )

//output high level
#ifdef IO_3V_TO_5V
    #define I2C_IO_OUTPUT_HIGH( ioNum )\
        GPIO_E_SET( ioNum, 0 )   
#else        
    #define I2C_IO_OUTPUT_HIGH( ioNum )\
        GPIO_E_SET( ioNum, 1 );\
        GPIO_O_SET( ioNum, 1 )   
#endif