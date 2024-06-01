# SPDX-License-Identifier: Apache-2.0

board_runner_args(jlink "--device=STM32L496RG" "--speed=4000")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)

set(BOARD_FLASH_RUNNER jlink)
set(BOARD_DEBUG_RUNNER jlink)
