/**
  ******************************************************************************
  * @file    bsp_adc.c
  * @brief   ADC1 channel switching and scaling helpers.
  ******************************************************************************
  */

#include "bsp_adc.h"
#include "bsp_dwt.h"
#include "adc.h"

#define BSP_ADC_AVG_SAMPLES     8U
#define BSP_ADC_SAMPLE_TIME     ADC_SAMPLETIME_239CYCLES_5
#define BSP_ADC_POLL_TIMEOUT_MS 5U

static float s_vdda = 3.3f;
static uint8_t s_ready = 0U;

static uint32_t adc_channel_of(bsp_adc_ch_t ch)
{
  switch (ch)
  {
    case BSP_ADC_CH_ADJ_VOLTAGE: return ADC_CHANNEL_4;
    case BSP_ADC_CH_ADJ_CURRENT: return ADC_CHANNEL_5;
    case BSP_ADC_CH_VREFINT:     return ADC_CHANNEL_VREFINT;
    case BSP_ADC_CH_TEMPSENSOR:  return ADC_CHANNEL_TEMPSENSOR;
    default:                     return ADC_CHANNEL_4;
  }
}

static uint8_t adc_sample(uint32_t adc_channel, uint16_t *raw)
{
  ADC_ChannelConfTypeDef config = {0};

  (void)HAL_ADC_Stop(&hadc1);

  config.Channel      = adc_channel;
  config.Rank         = ADC_REGULAR_RANK_1;
  config.SamplingTime = BSP_ADC_SAMPLE_TIME;

  if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK)
  {
    return BSP_ERROR;
  }

  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    return BSP_ERROR;
  }

  if (HAL_ADC_PollForConversion(&hadc1, BSP_ADC_POLL_TIMEOUT_MS) != HAL_OK)
  {
    (void)HAL_ADC_Stop(&hadc1);
    return BSP_ERROR;
  }

  *raw = (uint16_t)HAL_ADC_GetValue(&hadc1);

  (void)HAL_ADC_Stop(&hadc1);

  return BSP_OK;
}

uint8_t bsp_adc_init(void)
{
  s_ready = 0U;

  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    return BSP_ERROR;
  }

  bsp_dwt_delay_us(100U);

  s_vdda = 3.3f;
  s_ready = 1U;

  bsp_adc_update_vdda();

  return BSP_OK;
}

uint8_t bsp_adc_read_raw(bsp_adc_ch_t ch, uint16_t samples, uint16_t *raw)
{
  uint32_t channel;
  uint32_t sum = 0U;
  uint16_t value = 0U;
  uint16_t i;

  if ((raw == 0) || (ch >= BSP_ADC_CH_COUNT))
  {
    return BSP_ERROR;
  }

  if (samples == 0U)
  {
    samples = 1U;
  }

  channel = adc_channel_of(ch);

  for (i = 0U; i < samples; i++)
  {
    if (adc_sample(channel, &value) != BSP_OK)
    {
      return BSP_ERROR;
    }

    sum += value;
  }

  *raw = (uint16_t)(sum / samples);

  return BSP_OK;
}

uint8_t bsp_adc_read_vdda(float *volts)
{
  uint16_t raw = 0U;

  if (volts == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_adc_read_raw(BSP_ADC_CH_VREFINT, BSP_ADC_AVG_SAMPLES, &raw) != BSP_OK)
  {
    return BSP_ERROR;
  }

  if (raw == 0U)
  {
    return BSP_ERROR;
  }

  *volts = (BSP_ADC_VREFINT_NOM_V * 4096.0f) / (float)raw;

  return BSP_OK;
}

void bsp_adc_update_vdda(void)
{
  float value = 0.0f;

  if (bsp_adc_read_vdda(&value) == BSP_OK)
  {
    /* Reject readings outside the plausible supply window. */
    if ((value > 2.7f) && (value < 3.6f))
    {
      s_vdda = value;
    }
  }
}

float bsp_adc_get_vdda(void)
{
  return s_vdda;
}

uint8_t bsp_adc_read_adj_voltage(float *volts)
{
  uint16_t raw = 0U;

  if (volts == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_adc_read_raw(BSP_ADC_CH_ADJ_VOLTAGE, BSP_ADC_AVG_SAMPLES, &raw) != BSP_OK)
  {
    return BSP_ERROR;
  }

  *volts = (((float)raw * s_vdda) / 4096.0f) * BSP_ADC_ADJ_GAIN;

  return BSP_OK;
}

uint8_t bsp_adc_read_adj_current(float *milliamps)
{
  uint16_t raw = 0U;

  if (milliamps == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_adc_read_raw(BSP_ADC_CH_ADJ_CURRENT, BSP_ADC_AVG_SAMPLES, &raw) != BSP_OK)
  {
    return BSP_ERROR;
  }

  /* 1 mV on the shunt amplifier output equals 1 mA. */
  *milliamps = ((float)raw * s_vdda * 1000.0f) / 4096.0f;

  return BSP_OK;
}

uint8_t bsp_adc_read_mcu_temperature(float *celsius)
{
  uint16_t raw = 0U;
  float vsense;

  if (celsius == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_adc_read_raw(BSP_ADC_CH_TEMPSENSOR, BSP_ADC_AVG_SAMPLES, &raw) != BSP_OK)
  {
    return BSP_ERROR;
  }

  vsense = ((float)raw * s_vdda) / 4096.0f;

  *celsius = ((BSP_ADC_V25_V - vsense) / BSP_ADC_TEMP_SLOPE) + 25.0f;

  return BSP_OK;
}
