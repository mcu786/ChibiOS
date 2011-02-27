/**
 * @file STM32/i2c_lld.h
 * @brief STM32 I2C subsystem low level driver header.
 * @addtogroup STM32_I2C
 * @{
 */

#ifndef _I2C_LLD_H_
#define _I2C_LLD_H_

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note The default is @p TRUE.
 */
#if !defined(STM32_I2C_USE_I2C1) || defined(__DOXYGEN__)
#define STM32_I2C_USE_I2C1              TRUE
#endif

/**
 * @brief I2C2 driver enable switch.
 * @details If set to @p TRUE the support for I2C2 is included.
 * @note The default is @p TRUE.
 */
#if !defined(STM32_I2C_USE_I2C2) || defined(__DOXYGEN__)
#define STM32_I2C_USE_I2C2              TRUE
#endif

/**
 * @brief I2C1 interrupt priority level setting.
 * @note @p BASEPRI_KERNEL >= @p STM32_I2C_I2C1_IRQ_PRIORITY > @p PRIORITY_PENDSV.
 */
#if !defined(STM32_I2C_I2C1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_I2C_I2C1_IRQ_PRIORITY     0xA0
#endif

/**
 * @brief I2C2 interrupt priority level setting.
 * @note @p BASEPRI_KERNEL >= @p STM32_I2C_I2C2_IRQ_PRIORITY > @p PRIORITY_PENDSV.
 */
#if !defined(STM32_I2C_I2C2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_I2C_I2C2_IRQ_PRIORITY     0xA0
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CSlaveConfig I2CSlaveConfig;


/**
 * @brief   I2C notification callback type.
 * @details This function must be used to send start or stop events to I2C bus,
 *          and change states of I2CDriver.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object triggering the
 *                      callback
 * @param[in] i2cscfg   pointer to the @p I2CSlaveConfig object triggering the
 *                      callback
 */
typedef void (*i2ccallback_t)(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg);


/**
 * @brief   I2C error notification callback type.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object triggering the
 *                      callback
 * @param[in] i2cscfg   pointer to the @p I2CSlaveConfig object triggering the
 *                      callback
 */
typedef void (*i2cerrorcallback_t)(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg);

typedef enum {
  opmodeI2C,
  opmodeSMBusDevice,
  opmodeSMBusHost,
} I2C_opMode_t;

typedef enum {
  stdDutyCycle,
  fastDutyCycle_2,
  fastDutyCycle_16_9,
} I2C_DutyCycle_t;

/**
 * @brief Driver configuration structure.
 */
typedef struct {
  I2C_opMode_t    opMode;           /*!< Specifies the I2C mode.*/
  uint32_t        ClockSpeed;       /*!< Specifies the clock frequency. Must be set to a value lower than 400kHz */
  I2C_DutyCycle_t FastModeDutyCycle;/*!< Specifies the I2C fast mode duty cycle */
  uint8_t         OwnAddress7;      /*!< Specifies the first device 7-bit own address. */
  uint16_t        OwnAddress10;     /*!< Specifies the second part of device own address in 10-bit mode. Set to NULL if not used. */
} I2CConfig;



/**
 * @brief I2C transmission data block size.
 */
typedef uint8_t i2cblock_t;


/**
 * @brief Structure representing an I2C slave configuration.
 */
struct I2CSlaveConfig{
  /**
   * @brief Callback pointer.
   * @note  Transfer finished callback. Invoke when all data transferred, or
   *        by DMA buffer events
   *        If set to @p NULL then the callback is disabled.
   */
  i2ccallback_t         id_callback;

  /**
   * @brief Callback pointer.
   * @note  This callback will be invoked when error condition occur.
   *        If set to @p NULL then the callback is disabled.
   */
  i2cerrorcallback_t    id_err_callback;

  i2cblock_t            *rxbuf;     // pointer to buffer
  size_t                rxdepth;    // depth of buffer
  size_t                rxbytes;    // count of bytes to sent in one transmission
  size_t                rxbufhead;  // head pointer to current data byte

  i2cblock_t            *txbuf;
  size_t                txdepth;
  size_t                txbytes;
  size_t                txbufhead;

  /**
   * @brief   Contain slave address and some flags.
   * @details Bits 0..9 contain slave address in 10-bit mode.
   *
   *          Bits 0..6 contain slave address in 7-bit mode.
   *
   *          Bits 10..14 are not used in 10-bit mode.
   *          Bits  7..14 are not used in 7-bit mode.
   *
   *          Bit 15 is used to switch between 10-bit and 7-bit modes
   *          (0 denotes 7-bit mode).
   */
  uint16_t              address;

  //TODO: merge rw_bit, restart and address in one 16-bit variable.
  uint8_t               rw_bit;
  bool_t                restart;    // send restart if TRUE. Else sent stop event after complete data tx/rx


#if I2C_USE_WAIT
  /**
   * @brief Thread waiting for I/O completion.
   */
  Thread                *thread;
#endif /* I2C_USE_WAIT */
};



/**
 * @brief Structure representing an I2C driver.
 */
struct I2CDriver{
  /**
   * @brief Driver state.
   */
  i2cstate_t            id_state;
#if I2C_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
#if CH_USE_MUTEXES || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the bus.
   */
  Mutex                 id_mutex;
#elif CH_USE_SEMAPHORES
  Semaphore             id_semaphore;
#endif
#endif /* I2C_USE_MUTUAL_EXCLUSION */
  /**
   * @brief Current configuration data.
   */
  I2CConfig             *id_config;
  /**
   * @brief Current slave configuration data.
   */
  I2CSlaveConfig        *id_slave_config;

  /* End of the mandatory fields.*/

  /**
   * @brief Pointer to the I2Cx registers block.
   */
  I2C_TypeDef           *id_i2c;
} ;


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/** @cond never*/
#if STM32_I2C_USE_I2C1
extern I2CDriver I2CD1;
#endif

#if STM32_I2C_USE_I2C2
extern I2CDriver I2CD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void i2c_lld_init(void);
void i2c_lld_start(I2CDriver *i2cp);
void i2c_lld_stop(I2CDriver *i2cp);
void i2c_lld_set_clock(I2CDriver *i2cp);
void i2c_lld_set_opmode(I2CDriver *i2cp);
void i2c_lld_set_own_address(I2CDriver *i2cp);

void i2c_lld_master_start(I2CDriver *i2cp);
void i2c_lld_master_stop(I2CDriver *i2cp);

void i2c_lld_master_transmit(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg);
void i2c_lld_master_receive(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg);

void i2c_lld_master_transmit_NI(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg, bool_t restart);
void i2c_lld_master_receive_NI(I2CDriver *i2cp, I2CSlaveConfig *i2cscfg);

#ifdef __cplusplus
}
#endif
/** @endcond*/

#endif // CH_HAL_USE_I2C

#endif // _I2C_LLD_H_