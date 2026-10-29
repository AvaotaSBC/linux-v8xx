# SPDX-License-Identifier: GPL-2.0-only
lib-y	+= delay.o
lib-y	+= memset.o
lib-y	+= memmove.o
lib-y	+= string.o
lib-y	+= uaccess.o
lib-y	+= driver_access.o

lib-$(CONFIG_64BIT) += tishift.o

CFLAGS_string.o := -ffreestanding
