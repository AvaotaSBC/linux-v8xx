# SPDX-License-Identifier: GPL-2.0
CONFIG_MODULE_NAME:=aglink

aglink-y := \
	hwio.o \
	main.o \
	queue.o \
	txrx.o \
	debug.o \
	os_dep/os_intf.o \
	app/app.o \
	app/up_user.o \
	app/take_photo.o

ifeq ($(CONFIG_AG_USE_CHECKSUM), y)
aglink-$(CONFIG_AG_USE_CHECKSUM) += checksum.o
EXTRA_CFLAGS += -DAGLINK_CHECKSUM
endif

ifeq ($(CONFIG_AG_LOOPBACK), y)
aglink-y += platform/loop_dev.o
EXTRA_CFLAGS += -DCONFIG_AG_LOOPBACK
endif

ifeq ($(CONFIG_AG_LOOPBACK), m)
aglink-m += platform/loop_dev.o
EXTRA_CFLAGS += -DCONFIG_AG_LOOPBACK
endif

ifeq ($(CONFIG_AG_UART), y)
aglink-y += platform/uart_dev.o
EXTRA_CFLAGS += -DCONFIG_AG_UART
endif

ifeq ($(CONFIG_AG_UART), m)
aglink-m += platform/uart_dev.o
EXTRA_CFLAGS += -DCONFIG_AG_UART
endif

ifeq ($(CONFIG_AG_SAVE_THUMBNAIL), y)
EXTRA_CFLAGS += -DAG_SAVE_THUMBNAIL
endif

ifeq ($(CONFIG_AG_SAVE_THUMBNAIL), m)
EXTRA_CFLAGS += -DAG_SAVE_THUMBNAIL
endif

ifeq ($(CONFIG_AG_UART_KERNEL), y)
aglink-y += platform/uart_kernel_dev.o
EXTRA_CFLAGS += -DTTY_NAME=\"$(CONFIG_AG_UART_KERNEL_PORT)\"
EXTRA_CFLAGS += -DUART_RATE=$(CONFIG_AG_UART_KERNEL_RATE)
endif

ifeq ($(CONFIG_HW_UART_KERNEL), m)
aglink-m += platform/uart_kernel_dev.o
EXTRA_CFLAGS += -DTTY_NAME=\"$(CONFIG_AG_UART_KERNEL_PORT)\"
EXTRA_CFLAGS += -DUART_RATE=$(CONFIG_AG_UART_KERNEL_RATE)
endif

ifeq ($(CONFIG_AG_LED_CTRL), m)
CONFIG_MODULE_NAME_LED:=aglink_led
aglink_led-m := app/led.o
endif

ifeq ($(CONFIG_AG_LED_CTRL), y)
CONFIG_MODULE_NAME_LED:=aglink_led
aglink_led-y := app/led.o
endif

ifeq ($(CONFIG_AG_GYRO_USE), m)
CONFIG_MODULE_NAME_GYRO:=aglink_gyro
aglink_gyro-m := \
	app/gyro/aglink_gyro_hwio.o \
	app/gyro/aglink_take_gyro.o \
	app/gyro/lsm6dsox/lsm6dsox.o \
	app/gyro/lsm6dsox/lsm6dsox_reg.o \
	app/gyro/aglink_gyro.o
endif

ifeq ($(CONFIG_AG_GYRO_USE), y)
CONFIG_MODULE_NAME_GYRO:=aglink_gyro
aglink_gyro-y := \
	app/gyro/aglink_gyro_hwio.o \
	app/gyro/aglink_take_gyro.o \
	app/gyro/lsm6dsox/lsm6dsox.o \
	app/gyro/lsm6dsox/lsm6dsox_reg.o \
	app/gyro/aglink_gyro.o
endif

EXTRA_CFLAGS += \
			-I$(LICHEE_BSP_DIR)/include/uapi \
			-I$(LICHEE_BSP_DIR)/include/uapi/media

EXTRA_CFLAGS += -Wno-error=date-time -Wno-date-time
EXTRA_CFLAGS += -I$(srctree)/$(src)/include -I$(srctree)/$(src)/os_dep

#ccflags-y += $(DRV_FLAGS)
obj-$(CONFIG_AG_GYRO_USE) += $(CONFIG_MODULE_NAME_GYRO).o
obj-$(CONFIG_AG_LINK) += $(CONFIG_MODULE_NAME).o
obj-$(CONFIG_AG_LED_CTRL) += $(CONFIG_MODULE_NAME_LED).o
