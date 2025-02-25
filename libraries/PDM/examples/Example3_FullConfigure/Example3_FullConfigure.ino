/* 
  Author: Nathan Seidle
  Created: July 24, 2019
  License: MIT. See SparkFun Arduino Apollo3 Project for more information

  This example shows how to configure PDM using the pdm config struct so 
  that you can access all settings in one step.
*/

//Global variables needed for PDM library
#define pdmDataBufferSize 4096 //Default is array of 4096 * 32bit
uint32_t pdmDataBuffer[pdmDataBufferSize];

//Global variables needed for the FFT in this sketch
float g_fPDMTimeDomain[pdmDataBufferSize * 2];
float g_fPDMFrequencyDomain[pdmDataBufferSize * 2];
float g_fPDMMagnitudes[pdmDataBufferSize * 2];
uint32_t sampleFreq;

//Enable these defines for additional debug printing
#define PRINT_PDM_DATA 0
#define PRINT_FFT_DATA 0

#include <PDM.h> //Include PDM library included with the Aruino_Apollo3 core
AP3_PDM myPDM; //Create instance of PDM class

//Math library needed for FFT
#define ARM_MATH_CM4
#include <arm_math.h>

const am_hal_pdm_config_t newConfig = {
    //Basic PDM setup pulled from SDK PDM example
    .eClkDivider = AM_HAL_PDM_MCLKDIV_1,
    .eLeftGain = AM_HAL_PDM_GAIN_0DB,
    .eRightGain = AM_HAL_PDM_GAIN_0DB,
    .ui32DecimationRate = 64,
    .bHighPassEnable = 0,
    .ui32HighPassCutoff = 0xB,
    .ePDMClkSpeed = AM_HAL_PDM_CLK_6MHZ,
    .bInvertI2SBCLK = 0,
    .ePDMClkSource = AM_HAL_PDM_INTERNAL_CLK,
    .bPDMSampleDelay = 0,
    .bDataPacking = 1,
    .ePCMChannels = AM_HAL_PDM_CHANNEL_RIGHT,
    .ui32GainChangeDelay = 1,
    .bI2SEnable = 0,
    .bSoftMute = 0,
    .bLRSwap = 0,
};

void setup()
{
  Serial.begin(9600);
  Serial.println("SparkFun PDM Example");

  // Turn on the PDM with default settings
  if (myPDM.begin() == false) //Use Data, clock defines from variant file
  {
    Serial.println("PDM Init failed. Are you sure these pins are PDM capable?");
    while (1);
  }
  Serial.println("PDM Initialized");

  myPDM.updateConfig(newConfig); //Send config struct
  
  printPDMConfig();
    
  myPDM.getData(pdmDataBuffer, pdmDataBufferSize); //This clears the current PDM FIFO and starts DMA
}

void loop()
{
  noInterrupts();

  if (myPDM.available())
  {
    printLoudest();

    while (PRINT_PDM_DATA || PRINT_FFT_DATA);

    // Start converting the next set of PCM samples.
    myPDM.getData(pdmDataBuffer, pdmDataBufferSize);
  }

  // Go to Deep Sleep until the PDM ISR or other ISR wakes us.
  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

  interrupts();
}

//*****************************************************************************
//
// Analyze and print frequency data.
//
//*****************************************************************************
void printLoudest(void)
{
  float fMaxValue;
  uint32_t ui32MaxIndex;
  int16_t *pi16PDMData = (int16_t *) pdmDataBuffer;
  uint32_t ui32LoudestFrequency;

  //
  // Convert the PDM samples to floats, and arrange them in the format
  // required by the FFT function.
  //
  for (uint32_t i = 0; i < pdmDataBufferSize; i++)
  {
    if (PRINT_PDM_DATA)
    {
      Serial.printf("%d\n", pi16PDMData[i]);
    }

    g_fPDMTimeDomain[2 * i] = pi16PDMData[i] / 1.0;
    g_fPDMTimeDomain[2 * i + 1] = 0.0;
  }

  if (PRINT_PDM_DATA)
  {
    Serial.printf("END\n");
  }

  //
  // Perform the FFT.
  //
  arm_cfft_radix4_instance_f32 S;
  arm_cfft_radix4_init_f32(&S, pdmDataBufferSize, 0, 1);
  arm_cfft_radix4_f32(&S, g_fPDMTimeDomain);
  arm_cmplx_mag_f32(g_fPDMTimeDomain, g_fPDMMagnitudes, pdmDataBufferSize);

  if (PRINT_FFT_DATA)
  {
    for (uint32_t i = 0; i < pdmDataBufferSize / 2; i++)
    {
      Serial.printf("%f\n", g_fPDMMagnitudes[i]);
    }

    Serial.printf("END\n");
  }

  //
  // Find the frequency bin with the largest magnitude.
  //
  arm_max_f32(g_fPDMMagnitudes, pdmDataBufferSize / 2, &fMaxValue, &ui32MaxIndex);

  ui32LoudestFrequency = (sampleFreq * ui32MaxIndex) / pdmDataBufferSize;

  if (PRINT_FFT_DATA)
  {
    Serial.printf("Loudest frequency bin: %d\n", ui32MaxIndex);
  }

  Serial.printf("Loudest frequency: %d         \n", ui32LoudestFrequency);
}

//*****************************************************************************
//
// Print PDM configuration data.
//
//*****************************************************************************
void printPDMConfig(void)
{
  uint32_t PDMClk;
  uint32_t MClkDiv;
  float frequencyUnits;

  //
  // Read the config structure to figure out what our internal clock is set
  // to.
  //
  switch (myPDM.getClockDivider())
  {
    case AM_HAL_PDM_MCLKDIV_4: MClkDiv = 4; break;
    case AM_HAL_PDM_MCLKDIV_3: MClkDiv = 3; break;
    case AM_HAL_PDM_MCLKDIV_2: MClkDiv = 2; break;
    case AM_HAL_PDM_MCLKDIV_1: MClkDiv = 1; break;

    default:
      MClkDiv = 0;
  }

  switch (myPDM.getClockSpeed())
  {
    case AM_HAL_PDM_CLK_12MHZ:  PDMClk = 12000000; break;
    case AM_HAL_PDM_CLK_6MHZ:   PDMClk =  6000000; break;
    case AM_HAL_PDM_CLK_3MHZ:   PDMClk =  3000000; break;
    case AM_HAL_PDM_CLK_1_5MHZ: PDMClk =  1500000; break;
    case AM_HAL_PDM_CLK_750KHZ: PDMClk =   750000; break;
    case AM_HAL_PDM_CLK_375KHZ: PDMClk =   375000; break;
    case AM_HAL_PDM_CLK_187KHZ: PDMClk =   187000; break;

    default:
      PDMClk = 0;
  }

  //
  // Record the effective sample frequency. We'll need it later to print the
  // loudest frequency from the sample.
  //
  sampleFreq = (PDMClk / (MClkDiv * 2 * myPDM.getDecimationRate()));

  frequencyUnits = (float) sampleFreq / (float) pdmDataBufferSize;

  Serial.printf("Settings:\n");
  Serial.printf("PDM Clock (Hz):         %12d\n", PDMClk);
  Serial.printf("Decimation Rate:        %12d\n", myPDM.getDecimationRate());
  Serial.printf("Effective Sample Freq.: %12d\n", sampleFreq);
  Serial.printf("FFT Length:             %12d\n\n", pdmDataBufferSize);
  Serial.printf("FFT Resolution: %15.3f Hz\n", frequencyUnits);
}
