/**
 * @file   mem_test.h
 * @brief  Memory Test Module - CPU, RAM, ROM, and Clock diagnostics
 * @doc    WMC-SUD-001 §4.7 Memory Diagnostic Specification
 *
 * Provides IEC 60730 Class B required self-tests:
 *   - CPU register test (stuck-at-fault detection)
 *   - RAM March C test (block-based for runtime)
 *   - ROM CRC-32 verification (block-based for runtime)
 *   - Clock cross-check (main vs. sub-clock comparison)
 *
 * Two modes of operation:
 *   - Startup: Full test of all memory (blocking)
 *   - Runtime: One block per call (non-blocking, cyclic)
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.1
 * @date    2026-03-01
 */

#ifndef MEM_TEST_H
#define MEM_TEST_H

#include "std_types.h"
#include "product_config.h"

/* ========================================================================
 * ROM CRC Block Parameters
 * ======================================================================== */

/** ROM CRC block size (1KB per block) */
#define ROM_CRC_BLOCK_SIZE      ((uint16_t)1024U)

/** Total ROM blocks (64KB / 1KB = 64 blocks) */
#define ROM_CRC_BLOCK_COUNT     ((uint8_t)64U)

/* ========================================================================
 * March C Test Patterns
 * ======================================================================== */

#define MARCH_PATTERN_00        ((uint8_t)0x00U)
#define MARCH_PATTERN_FF        ((uint8_t)0xFFU)
#define MARCH_PATTERN_55        ((uint8_t)0x55U)
#define MARCH_PATTERN_AA        ((uint8_t)0xAAU)

/* ========================================================================
 * Function Prototypes
 * ======================================================================== */

/**
 * @brief  Initialize diagnostic status to default (all cleared)
 * @detail Resets all diagnostic flags and block indices to initial state.
 *         Called at power-up or from test setUp to ensure clean state.
 */
void MEM_Init(void);

/**
 * @brief  Execute full startup diagnostic sequence
 * @return E_OK if all tests pass, E_NOT_OK if any test fails
 * @detail Runs complete CPU + RAM + ROM + Clock test (blocking).
 *         Must be called once at power-up before entering main loop.
 *         Typical execution time: ~200ms for 5.6KB RAM + 64KB ROM.
 */
Std_ReturnType MEM_StartupTest(void);

/**
 * @brief  Execute one block of runtime diagnostic
 * @return E_OK if block test passes, E_NOT_OK if block test fails
 * @detail Tests one RAM block and one ROM block per call.
 *         Block index advances automatically (cyclic).
 *         Called every CONTROL_LOOP_MS from scheduler.
 *         Full RAM cycle: RAM_TEST_BLOCK_COUNT * 10ms = 220ms
 *         Full ROM cycle: ROM_CRC_BLOCK_COUNT * 10ms = 640ms
 */
Std_ReturnType MEM_RunDiag(void);

/**
 * @brief  Test CPU general-purpose registers for stuck-at faults
 * @return E_OK if all registers pass, E_NOT_OK if fault detected
 * @detail Writes test patterns to CPU registers and verifies readback.
 *         Tests: R0-R7 (RL78 register bank), PSW, SP
 */
Std_ReturnType MEM_CpuRegTest(void);

/**
 * @brief  Test one RAM block using March C algorithm
 * @param  block  Block index (0 to RAM_TEST_BLOCK_COUNT-1)
 * @return E_OK if block passes, E_NOT_OK if fault detected
 * @detail March C sequence: {W0; R0W1; R1W0; R0W1; R1W0; R0}
 *         Block contents are saved and restored.
 */
Std_ReturnType MEM_RamBlockTest(uint8_t block);

/**
 * @brief  Verify CRC-32 of one ROM block
 * @param  block  Block index (0 to ROM_CRC_BLOCK_COUNT-1)
 * @return E_OK if CRC matches, E_NOT_OK if CRC mismatch
 * @detail Computes CRC-32 over ROM_CRC_BLOCK_SIZE bytes and
 *         compares with stored reference CRC.
 */
Std_ReturnType MEM_RomBlockCrc(uint8_t block);

/**
 * @brief  Verify main clock frequency via sub-clock cross-check
 * @return E_OK if within tolerance, E_NOT_OK if out of range
 * @detail Counts main clock cycles during one sub-clock period.
 *         Expected: CLOCK_MAIN_MHZ * 1000 / CLOCK_SUB_KHZ
 *         Tolerance: +/- CLOCK_TOLERANCE_PCT percent
 */
Std_ReturnType MEM_ClockCheck(void);

/**
 * @brief  Get current diagnostic status
 * @return Const pointer to diag_status_t structure
 */
const diag_status_t *MEM_GetStatus(void);

#endif /* MEM_TEST_H */
