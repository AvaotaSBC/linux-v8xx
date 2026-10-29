# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the usb host drivers.
#
# The driver includes the generic EHCI definitions and the legacy relative
# sunxi-hci.h header, so both source roots are needed in a standalone tree.
ccflags-y += -I $(srctree)/drivers/usb/host
ccflags-y += -I $(srctree)/bsp/drivers/usb/host

obj-$(CONFIG_USB_EHCI_HCD_SUNXI)	+= ehci-sunxi.o
obj-$(CONFIG_USB_OHCI_HCD_SUNXI)	+= ohci-sunxi.o
obj-$(CONFIG_USB_SUNXI_HCI)      	+= sunxi-hci.o
obj-$(CONFIG_USB_SUNXI_XHCI)     	+= xhci_sunxi.o

ccflags-y += -DDYNAMIC_DEBUG_MODULE
