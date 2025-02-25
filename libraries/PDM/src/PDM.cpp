/*
Copyright (c) 2019 SparkFun Electronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "PDM.h"

AP3_PDM *ap3_pdm_handle = 0;

bool AP3_PDM::begin(ap3_gpio_pin_t pinPDMData, ap3_gpio_pin_t pinPDMClock)
{
    _PDMhandle = NULL;
    _PDMconfig = ap3_pdm_config_default;
    _pinPDMData = pinPDMData;
    _pinPDMClock = pinPDMClock;

    if (_begin() != AP3_OK)
    {
        return (false);
    }
    return (true);
}

bool AP3_PDM::available(void)
{
    return (_PDMdataReady);
}

ap3_err_t AP3_PDM::_begin(void)
{
    ap3_err_t retval = AP3_OK;
    am_hal_gpio_pincfg_t pincfg = AP3_GPIO_DEFAULT_PINCFG;
    uint8_t funcsel = 0;

    //Configure PDM pins
    retval = ap3_pdm_pad_funcsel(AP3_PDM_DATA, ap3_gpio_pin2pad(_pinPDMData), &funcsel);
    if (retval != AP3_OK)
    {
        return retval;
    }
    pincfg.uFuncSel = funcsel; // set the proper function select option for this instance/pin/type combination
    pinMode(_pinPDMData, pincfg, &retval);
    if (retval != AP3_OK)
    {
        return ap3_return(retval);
    }

    retval = ap3_pdm_pad_funcsel(AP3_PDM_CLOCK, ap3_gpio_pin2pad(_pinPDMClock), &funcsel);
    if (retval != AP3_OK)
    {
        return retval;
    }
    pincfg.uFuncSel = funcsel; // set the proper function select option for this instance/pin/type combination
    pinMode(_pinPDMClock, pincfg, &retval);
    if (retval != AP3_OK)
    {
        return ap3_return(retval);
    }

    // Initialize, power-up, and configure the PDM.

    // //User may want to change settings mid-sketch. Only init PDM if it's new.
    if (_PDMhandle == NULL)
    {
        // Now that pins are initialized start the actual driver
        retval = (ap3_err_t)am_hal_pdm_initialize(0, &_PDMhandle);
        if (retval != AP3_OK)
        {
            return ap3_return(retval);
        }
    }
    retval = (ap3_err_t)am_hal_pdm_power_control(_PDMhandle, AM_HAL_PDM_POWER_ON, false);
    if (retval != AP3_OK)
    {
        return ap3_return(retval);
    }
    retval = (ap3_err_t)am_hal_pdm_configure(_PDMhandle, &_PDMconfig);
    if (retval != AP3_OK)
    {
        return ap3_return(retval);
    }
    retval = (ap3_err_t)am_hal_pdm_enable(_PDMhandle);
    if (retval != AP3_OK)
    {
        return ap3_return(retval);
    }

    //
    // Configure and enable PDM interrupts (set up to trigger on DMA
    // completion).
    //
    am_hal_pdm_interrupt_enable(_PDMhandle, (AM_HAL_PDM_INT_DERR | AM_HAL_PDM_INT_DCMP | AM_HAL_PDM_INT_UNDFL | AM_HAL_PDM_INT_OVF));
    am_hal_interrupt_master_enable();
    NVIC_EnableIRQ(PDM_IRQn);

    // Register the class into the local list
    ap3_pdm_handle = this;

    return retval;
}

bool AP3_PDM::setClockSpeed(am_hal_pdm_clkspd_e clockSpeed)
{
    _PDMconfig.ePDMClkSpeed = clockSpeed;

    return (updateConfig(_PDMconfig));
}

am_hal_pdm_clkspd_e AP3_PDM::getClockSpeed()
{
    return (_PDMconfig.ePDMClkSpeed);
}

bool AP3_PDM::setClockDivider(am_hal_pdm_mclkdiv_e clockDivider)
{
    _PDMconfig.eClkDivider = clockDivider;

    return (updateConfig(_PDMconfig));
}

am_hal_pdm_mclkdiv_e AP3_PDM::getClockDivider()
{
    return (_PDMconfig.eClkDivider);
}

bool AP3_PDM::setLeftGain(am_hal_pdm_gain_e gain)
{
    _PDMconfig.eLeftGain = gain;

    return (updateConfig(_PDMconfig));
}

bool AP3_PDM::setRightGain(am_hal_pdm_gain_e gain)
{
    _PDMconfig.eRightGain = gain;

    return (updateConfig(_PDMconfig));
}

bool AP3_PDM::setGain(am_hal_pdm_gain_e gain)
{
    _PDMconfig.eLeftGain = gain;
    _PDMconfig.eRightGain = gain;

    return (updateConfig(_PDMconfig));
}

am_hal_pdm_gain_e AP3_PDM::getLeftGain()
{
    return (_PDMconfig.eLeftGain);
}
am_hal_pdm_gain_e AP3_PDM::getRightGain()
{
    return (_PDMconfig.eRightGain);
}

bool AP3_PDM::setChannel(am_hal_pdm_chset_e channel)
{
    _PDMconfig.ePCMChannels = channel;

    return (updateConfig(_PDMconfig));
}

am_hal_pdm_chset_e AP3_PDM::getChannel()
{
    return (_PDMconfig.ePCMChannels);
}

bool AP3_PDM::setDecimationRate(uint32_t deciRate)
{
    _PDMconfig.ui32DecimationRate = deciRate;

    return (updateConfig(_PDMconfig));
}

uint32_t AP3_PDM::getDecimationRate()
{
    return (_PDMconfig.ui32DecimationRate);
}

//Send a given configuration struct to PDM
bool AP3_PDM::updateConfig(am_hal_pdm_config_t newConfiguration)
{
    ap3_err_t retval = (ap3_err_t)am_hal_pdm_configure(_PDMhandle, &newConfiguration);
    if (retval != AP3_OK)
    {
        return false;
    }
    return true;
}

ap3_err_t ap3_pdm_pad_funcsel(ap3_pdm_pad_type_e type, ap3_gpio_pad_t pad, uint8_t *funcsel)
{
    ap3_err_t retval = AP3_ERR;

    const ap3_pdm_pad_map_elem_t *map = NULL;
    uint8_t map_len = 0;

    switch (type)
    {
    case AP3_PDM_CLOCK:
        map = ap3_pdm_clock_map;
        map_len = AP3_PDM_NUM_CLOCK_PADS;
        break;
    case AP3_PDM_DATA:
        map = ap3_pdm_data_map;
        map_len = AP3_PDM_NUM_DATA_PADS;
        break;
    default:
        goto invalid_args;
        break;
    }

    if ((map == NULL) || (map_len == 0))
    {
        goto invalid_args;
    }

    for (uint16_t indi = 0; indi < map_len; indi++)
    {
        if (map[indi].pad == pad)
        {
            *funcsel = map[indi].funcsel;
            return AP3_OK;
        }
    }
    return retval;

invalid_args:
    retval = AP3_INVALID_ARG;
    *funcsel = 0; // do not use
    return retval;
}

//*****************************************************************************
//
// Start a transaction to get some number of bytes from the PDM interface.
//
//*****************************************************************************
void AP3_PDM::getData(uint32_t *PDMDataBuffer, uint32_t bufferSize)
{
    //
    // Configure DMA and target address.
    //
    am_hal_pdm_transfer_t sTransfer;
    sTransfer.ui32TargetAddr = (uint32_t)PDMDataBuffer;
    sTransfer.ui32TotalCount = bufferSize * 2; //PDM_FFT_BYTES;

    //
    // Start the data transfer.
    //
    am_hal_pdm_enable(_PDMhandle);
    am_util_delay_ms(100);
    am_hal_pdm_fifo_flush(_PDMhandle);
    am_hal_pdm_dma_start(_PDMhandle, &sTransfer);

    _PDMdataReady = false;
}

inline void AP3_PDM::pdm_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(_PDMhandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(_PDMhandle, ui32Status);

    //
    // Once our DMA transaction completes, we will disable the PDM and send a
    // flag back down to the main routine. Disabling the PDM is only necessary
    // because this example only implemented a single buffer for storing FFT
    // data. More complex programs could use a system of multiple buffers to
    // allow the CPU to run the FFT in one buffer while the DMA pulls PCM data
    // into another buffer.
    //
    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        am_hal_pdm_disable(_PDMhandle);
        _PDMdataReady = true;
    }
}

//*****************************************************************************
//
// PDM interrupt handler.
//
//*****************************************************************************
extern "C" void am_pdm_isr(void)
{
    ap3_pdm_handle->pdm_isr();
}
