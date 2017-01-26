# Default Makefile, for host native GNRC-based networking

# name of your application
APPLICATION = gcoap

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f446

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

BOARD_INSUFFICIENT_MEMORY := chronos msb-430 msb-430h nucleo-f030 nucleo-f334 \
                             stm32f0discovery telosb weio wsn430-v1_3b wsn430-v1_4 \
                             z1 nucleo32-f042

# Must read nordic_softdevice_ble package before nanocoap package. However,
# can't read it explicitly here because it is read later, as a dependency for
# the nrf52dk board.
BOARD_BLACKLIST := nrf52dk

## Uncomment to redefine port, for example use 61616 for RFC 6282 UDP compression.
#GCOAP_PORT = 5683
#CFLAGS += -DGCOAP_PORT=$(GCOAP_PORT)

## Uncomment to redefine request token length, max 8.
#GCOAP_TOKENLEN = 2
#CFLAGS += -DGCOAP_TOKENLEN=$(GCOAP_TOKENLEN)

USEPKG += nanocoap
# Required by nanocoap, but only due to issue #5959.
USEMODULE += posix

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_sock_udp
USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add a routing protocol
USEMODULE += gnrc_rpl
USEMODULE += auto_init_gnrc_rpl
# Required by gcoap example
USEMODULE += od
USEMODULE += fmt
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# mrf24j40
USEMODULE += mrf24j40

# saul
USEMODULE += saul_reg
USEMODULE += auto_init_saul

# bme280
USEMODULE += bme280

# set default device parameters in case they are undefined
TEST_I2C                  ?= I2C_0
TEST_I2C_ADDR             ?= 0x76

# Set PAN_ID
DEFAULT_PAN_ID ?= 0xa9f7U
CFLAGS += -DIEEE802154_DEFAULT_PANID=$(DEFAULT_PAN_ID)

# export parameters
CFLAGS += -DBME280_PARAM_I2C_DEV=$(TEST_I2C)
CFLAGS += -DBME280_PARAM_I2C_ADDR=$(TEST_I2C_ADDR)

# Comment this out to join RPL DODAGs even if DIOs do not contain
# DODAG Configuration Options (see the doc for more info)
CFLAGS += -DGNRC_RPL_DODAG_CONF_OPTIONAL_ON_JOIN

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
