# linux-v821

Self-contained Linux 5.4.220 V821 BSP for Avaota-F1. `bsp/` is a real source
directory, and the F1 DTS and kernel defconfig are included in this tree; no
Tina checkout is required.

The F1 configuration keeps the V821 storage, USB, audio, camera, remoteproc,
and FMAC support used by the board, without the unrelated GPU/DRM path. The
Buildroot external tree passes `BSP_TOP=bsp/` and `KERNEL_SRC_DIR=$(LINUX_DIR)`
to the legacy BSP makefiles.

Buildroot is the supported entry point. For a direct kernel build:

```sh
make ARCH=riscv CROSS_COMPILE=riscv32-buildroot-linux- \
  BSP_TOP=bsp/ avaota_f1_defconfig
make ARCH=riscv CROSS_COMPILE=riscv32-buildroot-linux- \
  BSP_TOP=bsp/ Image.gz avaota-f1.dtb
```
