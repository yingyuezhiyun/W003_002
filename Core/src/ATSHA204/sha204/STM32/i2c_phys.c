/** \file
 *  \brief  I2C Physical Hardware Interface for F28377D (I2CB)
 *  \author Adapted from Atmel STM32 version for F28377D
 *  \date   2024
 *
 * \atsha204_library_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel integrated circuit.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \atsha204_library_license_stop
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
#include "i2c_phys.h"

// =========================================================================
// 配置参数
// =========================================================================
#define I2C_BASE_ADDR       I2CB_BASE               //!< 使用I2CB模块
#define I2C_SYSCLK_HZ       DEVICE_SYSCLK_FREQ      //!< 系统时钟频率
#define I2C_BITRATE_HZ      100000U                 //!< I2C时钟频率 100kHz

// I2CB引脚定义 (根据实际硬件修改)
#define I2C_SDA_PIN         40U                     //!< GPIO40 - SDAB
#define I2C_SCL_PIN         41U                     //!< GPIO41 - SCLB
#define I2C_SDA_PIN_CFG     GPIO_40_SDAB            //!< SDAB引脚配置
#define I2C_SCL_PIN_CFG     GPIO_41_SCLB            //!< SCLB引脚配置

// GPIO40在GPB寄存器中的位偏移 (GPIO40 - GPIO32 = 8)
#define GPIO40_BIT          (1U << (I2C_SDA_PIN - 32U))

// 超时计数
#define I2C_TIMEOUT_COUNT   10000U

// =========================================================================
// 内部函数声明
// =========================================================================
static void i2c_hw_init(void);
static void i2c_gpio_set_output(void);
static void i2c_gpio_restore(void);

// 模块初始化标志
static bool i2c_initialized = false;

// =========================================================================
// 内部函数实现
// =========================================================================

/**
 * \brief 初始化F28377D I2CB硬件模块
 */
static void i2c_hw_init(void)
{
    if (i2c_initialized)
        return;

    // 配置I2CB GPIO引脚
    GPIO_setPinConfig(I2C_SDA_PIN_CFG);
    GPIO_setPinConfig(I2C_SCL_PIN_CFG);
    GPIO_setPadConfig(I2C_SDA_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setPadConfig(I2C_SCL_PIN, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(I2C_SDA_PIN, GPIO_QUAL_ASYNC);
    GPIO_setQualificationMode(I2C_SCL_PIN, GPIO_QUAL_ASYNC);

    // 复位并配置I2C模块
    I2C_disableModule(I2C_BASE_ADDR);

    I2C_initController(I2C_BASE_ADDR, I2C_SYSCLK_HZ, I2C_BITRATE_HZ,
                       I2C_DUTYCYCLE_50);

    I2C_setBitCount(I2C_BASE_ADDR, I2C_BITCOUNT_8);
    I2C_setAddressMode(I2C_BASE_ADDR, I2C_ADDR_MODE_7BITS);

    // 启用FIFO
    I2C_enableFIFO(I2C_BASE_ADDR);

    // 清除所有状态标志
    I2C_clearStatus(I2C_BASE_ADDR, 0xFFFFU);

    // 使能I2C模块
    I2C_enableModule(I2C_BASE_ADDR);

    i2c_initialized = true;
}

/**
 * \brief 将I2C引脚临时配置为GPIO输出模式（用于Wakeup脉冲）
 */
static void i2c_gpio_set_output(void)
{
    EALLOW;
    // GPIO40: 清除GPBMUX1对应位，切换到GPIO模式
    HWREG(GPIOCTRL_BASE + GPIO_O_GPBMUX1) &= ~(0x3U << ((I2C_SDA_PIN - 32U) * 2U));
    HWREG(GPIOCTRL_BASE + GPIO_O_GPBGMUX1) &= ~(0x3U << ((I2C_SDA_PIN - 32U) * 2U));
    // 设置为输出
    HWREG(GPIOCTRL_BASE + GPIO_O_GPBDIR) |= GPIO40_BIT;
    EDIS;
}

/**
 * \brief 恢复I2C引脚为I2C外设功能
 */
static void i2c_gpio_restore(void)
{
    GPIO_setPinConfig(I2C_SDA_PIN_CFG);
    GPIO_setPinConfig(I2C_SCL_PIN_CFG);
}

// =========================================================================
// I2C物理层接口实现
// =========================================================================

/**
 * \brief 初始化I2C物理接口
 */
void i2c_enable(void)
{
    i2c_hw_init();
}

/**
 * \brief 产生I2C Wakeup脉冲
 *
 * 通过临时切换I2C引脚为GPIO模式，产生满足ATSHA204要求的Wakeup脉冲：
 * 1. SDA拉低至少60us（SCL由I2C外设保持低电平）
 * 2. SDA释放（拉高），SCL仍保持低
 * 3. 恢复I2C外设功能
 *
 * \return 操作状态
 */
uint8_t i2c_send_wakeup(void)
{
    // 禁用I2C模块（SCL将被拉低）
    I2C_disableModule(I2C_BASE_ADDR);

    // 将SDA引脚配置为GPIO输出
    i2c_gpio_set_output();

    // SDA拉低 - 产生Wakeup脉冲的低电平部分
    HWREG(GPIOCTRL_BASE + GPIO_O_GPBCLEAR) = GPIO40_BIT;

    // 保持SDA低电平 >= 60us (ATSHA204 Wakeup脉冲宽度要求)
    DEVICE_DELAY_US(60);

    // SDA释放（拉高）
    HWREG(GPIOCTRL_BASE + GPIO_O_GPBSET) = GPIO40_BIT;

    // 等待T_WHI >= 60us (Wakeup脉冲后到通信开始前的延迟)
    DEVICE_DELAY_US(60);

    // 恢复I2C引脚功能
    i2c_gpio_restore();

    // 重新使能I2C模块
    I2C_enableModule(I2C_BASE_ADDR);

    // 重新使能FIFO
    I2C_enableFIFO(I2C_BASE_ADDR);

    // 等待I2C模块稳定
    DEVICE_DELAY_US(10);

    return I2C_FUNCTION_RETCODE_SUCCESS;
}

/**
 * \brief 发送I2C START条件
 *
 * 等待总线空闲后，清除NACK标志，发送START条件
 *
 * \return 操作状态
 */
uint8_t i2c_send_start(void)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    // 等待总线空闲
    while (I2C_isBusBusy(I2C_BASE_ADDR)) {
        if (--timeout == 0)
            return I2C_FUNCTION_RETCODE_TIMEOUT;
    }

    // 清除NACK状态标志
    I2C_clearStatus(I2C_BASE_ADDR, I2C_STS_NO_ACK);

    // 发送START条件
    I2C_sendStartCondition(I2C_BASE_ADDR);

    return I2C_FUNCTION_RETCODE_SUCCESS;
}

/**
 * \brief 发送I2C STOP条件
 *
 * \return 操作状态
 */
uint8_t i2c_send_stop(void)
{
    I2C_sendStopCondition(I2C_BASE_ADDR);

    // 等待STOP条件完成（总线不再繁忙）
    uint32_t timeout = I2C_TIMEOUT_COUNT;
    while (I2C_isBusBusy(I2C_BASE_ADDR)) {
        if (--timeout == 0)
            return I2C_FUNCTION_RETCODE_TIMEOUT;
    }

    return I2C_FUNCTION_RETCODE_SUCCESS;
}

/**
 * \brief 通过I2C发送一个或多个字节
 *
 * 配置I2C为主发送模式，设置数据长度，逐个字节写入数据。
 * 每个字节发送后检查ACK/NACK状态。
 *
 * \param[in] count 要发送的字节数
 * \param[in] data  指向发送数据缓冲区的指针
 * \return 操作状态
 */
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
    uint16_t i;
    uint32_t timeout;

    // 配置为主发送模式，非重复模式
    I2C_setConfig(I2C_BASE_ADDR, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(I2C_BASE_ADDR, count);

    // 逐个发送数据字节
    for (i = 0; i < count; i++) {
        // 等待发送数据就绪 (TX_DATA_RDY)
        timeout = I2C_TIMEOUT_COUNT;
        while (!(I2C_getStatus(I2C_BASE_ADDR) & I2C_STS_TX_DATA_RDY)) {
            // 检查NACK
            if (I2C_getStatus(I2C_BASE_ADDR) & I2C_STS_NO_ACK) {
                I2C_clearStatus(I2C_BASE_ADDR, I2C_STS_NO_ACK);
                I2C_sendStopCondition(I2C_BASE_ADDR);
                return I2C_FUNCTION_RETCODE_NACK;
            }
            if (--timeout == 0) {
                I2C_sendStopCondition(I2C_BASE_ADDR);
                return I2C_FUNCTION_RETCODE_TIMEOUT;
            }
        }

        // 写入数据字节
        I2C_putData(I2C_BASE_ADDR, data[i]);
    }

    return I2C_FUNCTION_RETCODE_SUCCESS;
}

/**
 * \brief 通过I2C接收一个字节（发送ACK）
 *
 * 配置I2C为主接收模式，接收1个字节后发送ACK。
 *
 * \param[out] data 指向接收数据缓冲区的指针
 * \return 操作状态
 */
uint8_t i2c_receive_byte(uint8_t *data)
{
    uint32_t timeout;

    // 配置为主接收模式，接收1字节
    I2C_setConfig(I2C_BASE_ADDR, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(I2C_BASE_ADDR, 1);

    // 发送NACK（最后字节不发送ACK）
    I2C_sendNACK(I2C_BASE_ADDR);

    // 发送START条件开始接收
    I2C_sendStartCondition(I2C_BASE_ADDR);

    // 等待接收数据就绪
    timeout = I2C_TIMEOUT_COUNT;
    while (!(I2C_getStatus(I2C_BASE_ADDR) & I2C_STS_RX_DATA_RDY)) {
        if (--timeout == 0) {
            I2C_sendStopCondition(I2C_BASE_ADDR);
            return I2C_FUNCTION_RETCODE_TIMEOUT;
        }
    }

    // 读取数据
    *data = (uint8_t)I2C_getData(I2C_BASE_ADDR);

    // 发送STOP条件
    I2C_sendStopCondition(I2C_BASE_ADDR);

    // 等待STOP完成
    timeout = I2C_TIMEOUT_COUNT;
    while (I2C_isBusBusy(I2C_BASE_ADDR)) {
        if (--timeout == 0)
            return I2C_FUNCTION_RETCODE_TIMEOUT;
    }

    return I2C_FUNCTION_RETCODE_SUCCESS;
}

/**
 * \brief 通过I2C接收多个字节
 *
 * 配置I2C为主接收模式，使用FIFO接收多个字节。
 * 最后一个字节发送NACK，然后发送STOP条件。
 *
 * \param[in]  count 要接收的字节数
 * \param[out] data  指向接收数据缓冲区的指针
 * \return 操作状态
 */
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
    uint16_t i;
    uint32_t timeout;

    if (count == 0)
        return I2C_FUNCTION_RETCODE_SUCCESS;

    // 配置为主接收模式
    I2C_setConfig(I2C_BASE_ADDR, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(I2C_BASE_ADDR, count);

    // 发送START条件开始接收
    I2C_sendStartCondition(I2C_BASE_ADDR);

    // 逐个接收数据字节
    for (i = 0; i < count; i++) {
        // 等待接收数据就绪
        timeout = I2C_TIMEOUT_COUNT;
        while (!(I2C_getStatus(I2C_BASE_ADDR) & I2C_STS_RX_DATA_RDY)) {
            if (--timeout == 0) {
                I2C_sendStopCondition(I2C_BASE_ADDR);
                return I2C_FUNCTION_RETCODE_TIMEOUT;
            }
        }

        // 最后一个字节发送NACK
        if (i == count - 1) {
            I2C_sendNACK(I2C_BASE_ADDR);
        }

        // 读取数据
        data[i] = (uint8_t)I2C_getData(I2C_BASE_ADDR);
    }

    // 发送STOP条件
    I2C_sendStopCondition(I2C_BASE_ADDR);

    // 等待STOP完成
    timeout = I2C_TIMEOUT_COUNT;
    while (I2C_isBusBusy(I2C_BASE_ADDR)) {
        if (--timeout == 0)
            return I2C_FUNCTION_RETCODE_TIMEOUT;
    }

    return I2C_FUNCTION_RETCODE_SUCCESS;
}
