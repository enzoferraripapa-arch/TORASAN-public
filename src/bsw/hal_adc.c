/**
 * @file    hal_adc.c
 * @brief   ADC hardware abstraction layer implementation
 * @module  SA-003 HAL (UT-008)
 * @safety  IEC 60730 Class B
 * @req     SR-004, SR-014
 */

#include "hal_adc.h"
#include "../config/mcu_config.h"
#include "../config/safety_config.h"

/* ============================================================
 * HalAdc_Init
 * ============================================================ */
void HalAdc_Init(void)
{
    /* Enable ADC peripheral clock */
    REG_ADCEN = (uint8_t)1U;

    /* Configure ADC: single conversion, software trigger, 10-bit */
    REG_ADM0 = (uint8_t)0x00U;  /* fCLK/64, normal mode */
    REG_ADM1 = (uint8_t)0x20U;  /* Software trigger, one-shot */

    /* Default channel: current sense (ANI0) */
    REG_ADS = (uint8_t)ADC_CH_CURRENT;
}

/* ============================================================
 * HalAdc_StartConversion
 * ============================================================ */
void HalAdc_StartConversion(uint8_t channel)
{
    /* Select ADC channel */
    REG_ADS = channel;

    /* Start conversion */
    REG_ADM0 |= ADC_START_BIT;
}

/* ============================================================
 * HalAdc_IsConversionDone
 * ============================================================ */
uint8_t HalAdc_IsConversionDone(void)
{
    uint8_t result;

    if ((REG_ADIF & ADC_COMPLETE_BIT) != (uint8_t)0U)
    {
        result = STD_TRUE;
    }
    else
    {
        result = STD_FALSE;
    }

    return result;
}

/* ============================================================
 * HalAdc_ReadResult
 * ============================================================ */
uint16_t HalAdc_ReadResult(void)
{
    uint16_t raw;

    /* Read 10-bit result (upper-aligned in 16-bit register) */
    raw = (uint16_t)(REG_ADCR >> 6U);

    /* Clear conversion complete flag */
    REG_ADIF = (uint8_t)0U;

    return raw;
}

/* ============================================================
 * HalAdc_GetCurrent
 * ============================================================ */
uint16_t HalAdc_GetCurrent(void)
{
    uint16_t adc_raw;
    uint32_t current_ma;

    HalAdc_StartConversion((uint8_t)ADC_CH_CURRENT);

    /* Wait for conversion (blocking, within 10ms budget) */
    while (HalAdc_IsConversionDone() == STD_FALSE)
    {
        __NOP();
    }

    adc_raw = HalAdc_ReadResult();

    /* Convert ADC count to mA:
     * current_ma = (adc_raw * VREF_mV) / (ADC_MAX * R_sense)
     * Simplified: assume 5V reference, 10-bit, 0.5 ohm shunt
     * current_ma = adc_raw * 5000 / 1023 / 0.5
     *            = adc_raw * 10000 / 1023
     * Use integer arithmetic: (adc_raw * 10000U) / 1023U */
    current_ma = ((uint32_t)adc_raw * 10000UL) / (uint32_t)ADC_MAX_COUNT;

    return (uint16_t)current_ma;
}

/* ============================================================
 * HalAdc_GetVoltage
 * ============================================================ */
uint16_t HalAdc_GetVoltage(void)
{
    uint16_t adc_raw;
    uint32_t voltage_mv;

    HalAdc_StartConversion((uint8_t)ADC_CH_VOLTAGE);

    /* Wait for conversion */
    while (HalAdc_IsConversionDone() == STD_FALSE)
    {
        __NOP();
    }

    adc_raw = HalAdc_ReadResult();

    /* Convert ADC count to mV:
     * Voltage divider ratio 1:1 assumed (Vmax=5V range)
     * voltage_mv = adc_raw * 5000 / 1023 */
    voltage_mv = ((uint32_t)adc_raw * (uint32_t)ADC_VREF_MV) / (uint32_t)ADC_MAX_COUNT;

    return (uint16_t)voltage_mv;
}
