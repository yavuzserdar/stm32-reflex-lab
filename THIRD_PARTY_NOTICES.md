# Third-party notices

The original application, scripts, tests, and documentation are MIT licensed.
The repository also includes files copied from the existing STM32Cube project:

- `Drivers/STM32F4xx_HAL_Driver/`: STMicroelectronics HAL; retain file headers
  and `LICENSE.txt`, which specifies BSD-3-Clause when no package terms are supplied.
- `Drivers/CMSIS/`: Arm CMSIS; see `Drivers/CMSIS/LICENSE.txt` (Apache-2.0).
- `Drivers/CMSIS/Device/ST/STM32F4xx/`: ST device files; retain their headers
  and the license in that directory.
- `Drivers/BSP/STM32F4xx-Nucleo/`: ST board support files, retained from the source
  project but not compiled by this project's build script; see their file headers.
- `Core/Startup/startup_stm32f446retx.s`, `Core/Src/system_stm32f4xx.c`,
  `Core/Src/syscalls.c`, `Core/Src/sysmem.c`, `Core/Inc/stm32f4xx_hal_conf.h`, and
  `STM32F446RETX_FLASH.ld`: ST-supplied/generated support files; original notices
  remain in place. The project MIT grant does not replace those notices.

NUCLEO, STM32, and ST-LINK are names used to identify supported hardware and tools.
