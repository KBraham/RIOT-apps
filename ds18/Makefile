APPLICATION = saul

BOARD ?= nucleo-f401
RIOTBASE ?= $(CURDIR)/../../RIOT
#QUIET ?= 1
APPLICATION := tests_$(APPLICATION)

# include and auto-initialize all available sensors
USEMODULE += saul_default
USEMODULE += ds18
USEMODULE += xtimer

include $(RIOTBASE)/Makefile.include
