/**
  ******************************************************************************
  * @file    bsp_adc.h
  * @brief   ADC1 helper for the ADJ rail monitor, VDDA and MCU temperature.
  *
  *          ADC1_IN4 (PA4) : ADJ output voltage, 10 k / 1.5 k divider
  *          ADC1_IN5 (PA5) : ADJ output current, 1 mV == 1 mA
  *          VREFINT        : internal 1.20 V reference -> real VDDA
  *          TEMPSENSOR     : MCU junction temperature
  ******************************************************************************
  */

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_ADC_VREF_MV         1200U
#define BSP_ADC_VREFINT_NOM_V   1.20f
#define BSP_ADC_V25_V           1.43f
#define BSP_ADC_TEMP_SLOPE      0.0043f

#define BSP_ADC_ADJ_R_TOP       10000.0f
#define BSP_ADC_ADJ_R_BOTTOM    1500.0f
#define BSP_ADC_ADJ_GAIN        ((BSP_ADC_ADJ_R_TOP + BSP_ADC_ADJ_R_BOTTOM) / BSP_ADC_ADJ_R_BOTTOM)

typedef enum
{
  BSP_ADC_CH_ADJ_VOLTAGE = 0,   /* ADC1_IN4 */
  BSP_ADC_CH_ADJ_CURRENT,       /* ADC1_IN5 */
  BSP_ADC_CH_VREFINT,
  BSP_ADC_CH_TEMPSENSOR,
  BSP_ADC_CH_COUNT
} bsp_adc_ch_t;

uint8_t bsp_adc_init(void);

/* Raw single conversion, averaged over BSP_ADC_AVG_SAMPLES reads. */
uint8_t bsp_adc_read_raw(bsp_adc_ch_t ch, uint16_t samples, uint16_t *raw);

/* Measured VDDA in volt, derived from VREFINT. */
uint8_t bsp_adc_read_vdda(float *volts);

/* ADJ rail values. */
uint8_t bsp_adc_read_adj_voltage(float *volts);
uint8_t bsp_adc_read_adj_current(float *milliamps);

/* MCU junction temperature. */
uint8_t bsp_adc_read_mcu_temperature(float *celsius);

/* Not thread safe: cached VDDA used by the conversion helpers. */
void bsp_adc_update_vdda(void);

/* Last accepted VDDA value used for the scaling. */
float bsp_adc_get_vdda(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H */
