#include "DAC7554.h"
#include "daisy_seed.h"
#include "../daisy_core.h"
#include "../per/spi.h"
#include "../per/gpio.h"
#include "../sys/system.h"

// Driver for DAC7554 based on code from Making Sound Machines 
// Based on Code from Westlicht Performer   - https://westlicht.github.io/performer/
// Port for Daisy by Making Sound Machines  - https://github.com/makingsoundmachines

// Pin Names in Schematics
//
// Patch    Seed        STM32H7   DAC7554
//
// SCK      SPI1_SCK    PG11      SCLK - spi.init GPIO_InitStruct.Pin = GPIO_PIN_11;
// DATA     SPI1_MOSI   PB5       DIN  - spi.init GPIO_InitStruct.Pin = GPIO_PIN_5
// RESET    USB_HS_D+   PB15      n.c.
// CMD      SPI1_MISO   PB4       SYNC
// CS       SPI1_NSS    PG10      n.c.

// Commands

#define WRITE_INPUT_REGISTER 0
#define UPDATE_OUTPUT_REGISTER 1
#define WRITE_INPUT_REGISTER_UPDATE_ALL 2
#define WRITE_INPUT_REGISTER_UPDATE_N 3
#define POWER_DOWN_UP_DAC 4
#define LOAD_CLEAR_CODE_REGISTER 5
#define LOAD_LDAC_REGISTER 6
#define RESET_POWER_ON 7
#define SETUP_INTERNAL_REF 8


using namespace daisy;

typedef struct
{
    uint8_t Initialized;
} Dac7554_t;

static SpiHandle         h_spi;
static dsy_gpio          pin_sync;
static Dac7554_t         Dac7554_;
static SpiHandle::Config spi_config;

void Dac7554::Init(/*dsy_gpio_pin* pin_cfg*/)
{
    // Initialize GPIO
    pin_sync.mode = DSY_GPIO_MODE_OUTPUT_PP;
    pin_sync.pin  = seed::D7;
    dsy_gpio_init(&pin_sync);

    // Initialize SPI

    spi_config.periph         = SpiHandle::Config::Peripheral::SPI_1;
    spi_config.mode           = SpiHandle::Config::Mode::MASTER;
    spi_config.direction      = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
    spi_config.datasize       = 8;
    spi_config.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
    spi_config.clock_phase    = SpiHandle::Config::ClockPhase::ONE_EDGE;
    spi_config.nss            = SpiHandle::Config::NSS::SOFT;
    spi_config.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_2;

    spi_config.pin_config.sclk = seed::D8; //{DSY_GPIOB, 8};
    spi_config.pin_config.miso = seed::D9; //{DSY_GPIOB, 9};
    spi_config.pin_config.mosi = seed::D10; //{DSY_GPIOB, 10};
    spi_config.pin_config.nss  = {DSY_GPIOX, 0}; //{DSY_GPIOX, 0};

    h_spi.Init(spi_config);

    //Reset();
    //SetClearCode(ClearIgnore);
    //SetInternalRef(false);

    Dac7554_.Initialized = 1;
}

void Dac7554::Write(int channel)
{
    WriteDac7554(channel, _values[channel]);
}

void Dac7554::Write()
{
    for(int channel = 0; channel < Channels; ++channel)
    {
        WriteDac7554(channel, _values[channel]);
    }
}

void Dac7554::WriteDac7554(uint16_t  address, uint16_t data)
{
    uint16_t cmd = (2 << 14) | (address << 12) | data;

    dsy_gpio_write(&pin_sync, 0);

    uint8_t west_sid3 = (cmd >> 8) & 0xff;
    uint8_t east_sid3 = (cmd >> 0) & 0xff;

    h_spi.BlockingTransmit(&west_sid3, 4);
    h_spi.BlockingTransmit(&east_sid3, 4);

    dsy_gpio_write(&pin_sync, 1);
}

void Dac7554::Reset()
{
}

void Dac7554::SetInternalRef(bool enabled)
{
}

void Dac7554::SetClearCode(ClearCode code)
{
}