/**
 * @file    diag.h
 * @brief   Self-diagnosis module (IEC 60730 Annex H)
 * @module  SA-007 DIAG
 * @safety  IEC 60730 Class B
 * @req     SR-009, SR-010, SR-011, SR-012, DR-001, DR-002, DR-003, DR-004
 */

#ifndef DIAG_H
#define DIAG_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Run startup self-diagnosis sequence
 * @module  SA-007 DIAG (UT-016)
 * @req     DR-001
 * @safety  IEC 60730 Class B
 *
 * Executes in order: CPU reg, RAM March C (full), ROM CRC, Clock check.
 * Total time must be <= STARTUP_DIAG_TIMEOUT_MS (700ms).
 *
 * @return  DIAG_PASS if all tests pass, DIAG_FAIL on any failure
 */
DiagResult_t Diag_RunStartup(void);

/**
 * @brief   Run runtime diagnosis step (called every 10ms)
 * @module  SA-007 DIAG (UT-017)
 * @req     DR-002
 * @safety  IEC 60730 Class B
 *
 * Performs incremental diagnosis:
 * - RAM March C: 1 block per cycle (256B, full cycle 2.2s)
 * - ROM CRC: incremental (full cycle <=5s)
 * - Clock check: every 100ms
 * - ADC/GPIO diagnosis: interleaved
 */
void Diag_RunCyclic10ms(void);

/**
 * @brief   CPU register self-test (Annex H.3.1)
 * @module  SA-007 DIAG (UT-018)
 * @req     SR-009
 * @safety  IEC 60730 Class B
 * @return  DIAG_PASS / DIAG_FAIL
 */
DiagResult_t Diag_CpuRegTest(void);

/**
 * @brief   RAM March C test on single block (Annex H.3.2)
 * @module  SA-007 DIAG (UT-018)
 * @req     SR-010
 * @safety  IEC 60730 Class B
 * @param   block_index  RAM block index (0 to RAM_TEST_BLOCK_COUNT-1)
 * @return  DIAG_PASS / DIAG_FAIL
 */
DiagResult_t Diag_RamTestBlock(uint8_t block_index);

/**
 * @brief   ROM CRC incremental step (Annex H.3.3)
 * @module  SA-007 DIAG (UT-018)
 * @req     SR-011
 * @safety  IEC 60730 Class B
 * @return  DIAG_PASS if full CRC verified, DIAG_RUNNING if in progress, DIAG_FAIL on mismatch
 */
DiagResult_t Diag_RomCrcStep(void);

/**
 * @brief   Clock frequency monitoring (Annex H.3.5)
 * @module  SA-007 DIAG (UT-018)
 * @req     SR-012
 * @safety  IEC 60730 Class B
 * @return  DIAG_PASS / DIAG_FAIL
 */
DiagResult_t Diag_ClockTest(void);

/**
 * @brief   ADC self-test (plausibility check)
 * @module  SA-007 DIAG (UT-019)
 * @req     DR-003
 * @safety  IEC 60730 Class B
 * @return  DIAG_PASS / DIAG_FAIL
 */
DiagResult_t Diag_AdcTest(void);

/**
 * @brief   GPIO self-test (open/short detection)
 * @module  SA-007 DIAG (UT-019)
 * @req     DR-004
 * @safety  IEC 60730 Class B
 * @return  DIAG_PASS / DIAG_FAIL
 */
DiagResult_t Diag_GpioTest(void);

#endif /* DIAG_H */
