# linux-v821

Linux 5.4.220 for the Allwinner V821 / Avaota-F1 board.

This tree is a self-contained export of the V821 BSP used by Tina Linux:

- the Linux 5.4.220 source tree is kept in this repository;
- `bsp/` is a real directory containing the V821 platform code, not Tina's
  `../../bsp` symlink;
- the Tina-generated `bsp/ramfs` directory is intentionally omitted because
  the root filesystem is produced by Buildroot;
- `arch/riscv/boot/dts/avaota-f1.dts` and the two V821 include files are
  included in the kernel tree, so the DTB does not depend on Tina;
- `configs/avaota_f1_defconfig` is the board kernel configuration exported
  from Tina.

The F1 configuration is RV32-specific. It keeps the V821 camera, storage,
USB, audio, remote-core, and FMAC support selected by the board DTS, but does
not select DRM or a board GPU. The unrelated ARM accelerator trees and their
legacy module makefiles have therefore been removed from this repository;
they were not part of the F1 build.

The kernel's BSP Kconfig expects `BSP_TOP` to point at the BSP directory. A
Buildroot external tree supplies `BSP_TOP=bsp/` through the Linux make flags.
For a direct kernel build, use for example:

```sh
make ARCH=riscv CROSS_COMPILE=riscv32-buildroot-linux- \
     BSP_TOP=bsp/ avaota_f1_defconfig
make ARCH=riscv CROSS_COMPILE=riscv32-buildroot-linux- \
     BSP_TOP=bsp/ Image.gz avaota-f1.dtb
```

The exact cross-compiler prefix is selected by the Buildroot toolchain; the
Buildroot external tree is the supported build entry point.
