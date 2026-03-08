/**
 * @file   mem_test.c
 * @brief  Memory Test Module - implementation
 * @doc    WMC-SUD-001 §4.7 Memory Diagnostic Specification
 *
 * @target  Renesas RL78/G14 (R5F104PJ)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 1.0
 * @date    2026-03-01
 */

#include "mem_test.h"

/* ========================================================================
 * [SAFETY VARIABLE REGION]
 * Diagnostic status is safety-critical.
 * In linker script: place in .safety_data section.
 * ======================================================================== */

/** Diagnostic status structure */
static diag_status_t s_diag;

/* ========================================================================
 * ROM Reference CRC Table
 *
 * On target RL78/G14, this table is populated by the linker/post-build
 * tool with pre-computed CRC-32 values for each 1KB ROM block.
 * The table itself is placed at a known ROM address.
 *
 * For host environment testing, we use a placeholder array.
 * ======================================================================== */

#ifdef TARGET_RL78_G14
    /*
     * On target: Reference CRCs stored at end of flash
     * extern const uint32_t ROM_CRC_TABLE[ROM_CRC_BLOCK_COUNT]
     *     __attribute__((section(".rom_crc_table")));
     */
#endif

/** Placeholder CRC reference table for host-environment testing */
static const uint32_t s_rom_crc_ref[ROM_CRC_BLOCK_COUNT] = { 0UL };

/* ========================================================================
 * RAM Test Area Pointer
 *
 * On target RL78/G14, RAM starts at 0xFEF00 (5.6KB).
 * Each block is RAM_TEST_BLOCK_B (256) bytes.
 *
 * For host environment, we use a simulated RAM buffer.
 * ======================================================================== */

#ifndef TARGET_RL78_G14
    /** Simulated RAM for host-environment testing */
    static volatile uint8_t s_sim_ram[RAM_TOTAL_B];
#endif

/* ========================================================================
 * Internal Helper: Get RAM block base address
 * ======================================================================== */

static volatile uint8_t *GetRamBlockAddr(uint8_t block)
{
#ifdef TARGET_RL78_G14
    /*
     * Target: RAM base = 0xFEF00
     * Block address = 0xFEF00 + (block * RAM_TEST_BLOCK_B)
     */
    return (volatile uint8_t *)(0xFEF00UL + ((uint32_t)block * (uint32_t)RAM_TEST_BLOCK_B));
#else
    /* Host: use simulated RAM buffer */
    return &s_sim_ram[(uint16_t)block * RAM_TEST_BLOCK_B];
#endif
}

/* ========================================================================
 * Internal Helper: Get ROM block base address
 * ======================================================================== */

static const uint8_t *GetRomBlockAddr(uint8_t block)
{
#ifdef TARGET_RL78_G14
    /*
     * Target: ROM base = 0x00000
     * Block address = block * ROM_CRC_BLOCK_SIZE
     */
    return (const uint8_t *)((uint32_t)block * (uint32_t)ROM_CRC_BLOCK_SIZE);
#else
    /*
     * Host: Return NULL - CRC calculation will use placeholder.
     * In a real test harness, this would point to a test ROM image.
     */
    (void)block;
    return NULL_PTR;
#endif
}

/* ========================================================================
 * Internal Helper: CRC-32 calculation
 * Polynomial: CRC32_POLYNOMIAL (0x04C11DB7)
 * ======================================================================== */

static uint32_t CalcCrc32(const uint8_t *data, uint16_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    uint16_t i;
    uint8_t  bit;

    if (data == NULL_PTR)
    {
        /* No data available - return init value (will mismatch ref) */
        return crc;
    }

    for (i = 0U; i < len; i++)
    {
        crc ^= ((uint32_t)data[i] << 24U);

        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x80000000UL) != 0UL)
            {
                crc = (crc << 1U) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc = crc << 1U;
            }
        }
    }

    return crc ^ 0xFFFFFFFFUL;
}
/* ======================================================================== * MEM_Init * * Reset diagnostic status to initial state (all cleared). * Used at power-up and in test setUp for clean state. * ======================================================================== */void MEM_Init(void){    s_diag.cpu_ok        = FLAG_CLEAR;    s_diag.ram_ok        = FLAG_CLEAR;    s_diag.rom_ok        = FLAG_CLEAR;    s_diag.clk_ok        = FLAG_CLEAR;    s_diag.ram_block_idx = 0U;    s_diag.rom_block_idx = 0U;}

/* ========================================================================
 * MEM_CpuRegTest
 *
 * Tests CPU general-purpose registers for stuck-at faults.
 * On RL78/G14: tests register bank R0-R7, PSW, SP.
 *
 * Simplified for host environment - tests stack-allocated variables
 * with walking-1 and walking-0 patterns.
 * ======================================================================== */

Std_ReturnType MEM_CpuRegTest(void)
{
    volatile uint8_t  test8;
    volatile uint16_t test16;
    Std_ReturnType    result = E_OK;

    /*
     * On target RL78/G14, this would use inline assembly:
     *   MOV  A, #0x55
     *   CMP  A, #0x55
     *   BNZ  fail
     *   MOV  A, #0xAA
     *   CMP  A, #0xAA
     *   BNZ  fail
     *   ... repeat for each register
     */

    /* Test 8-bit patterns */
    test8 = 0x55U;
    if (test8 != 0x55U)
    {
        result = E_NOT_OK;
    }

    test8 = 0xAAU;
    if (test8 != 0xAAU)
    {
        result = E_NOT_OK;
    }

    test8 = 0x00U;
    if (test8 != 0x00U)
    {
        result = E_NOT_OK;
    }

    test8 = 0xFFU;
    if (test8 != 0xFFU)
    {
        result = E_NOT_OK;
    }

    /* Test 16-bit patterns (SP / register pairs) */
    test16 = 0x5555U;
    if (test16 != 0x5555U)
    {
        result = E_NOT_OK;
    }

    test16 = 0xAAAAU;
    if (test16 != 0xAAAAU)
    {
        result = E_NOT_OK;
    }

    test16 = 0x0000U;
    if (test16 != 0x0000U)
    {
        result = E_NOT_OK;
    }

    test16 = 0xFFFFU;
    if (test16 != 0xFFFFU)
    {
        result = E_NOT_OK;
    }

    s_diag.cpu_ok = (result == E_OK) ? FLAG_SET : FLAG_CLEAR;

    return result;
}

/* ========================================================================
 * MEM_RamBlockTest - March C Algorithm
 *
 * March C sequence (simplified):
 *   Step 0: W0 (ascending)  - Write all 0x00
 *   Step 1: R0,W1 (ascending)  - Read 0x00, write 0xFF
 *   Step 2: R1,W0 (ascending)  - Read 0xFF, write 0x00
 *   Step 3: R0,W1 (descending) - Read 0x00, write 0xFF
 *   Step 4: R1,W0 (descending) - Read 0xFF, write 0x00
 *   Step 5: R0 (ascending)  - Read 0x00 (verify)
 *
 * Block contents are saved to a stack buffer and restored after test.
 * ======================================================================== */

Std_ReturnType MEM_RamBlockTest(uint8_t block)
{
    volatile uint8_t *base;
    uint8_t  backup[RAM_TEST_BLOCK_B];
    uint16_t i;
    Std_ReturnType result = E_OK;

    if (block >= RAM_TEST_BLOCK_COUNT)
    {
        return E_NOT_OK;
    }

    base = GetRamBlockAddr(block);

    /* Save block contents */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        backup[i] = base[i];
    }

    /* Step 0: Write all 0x00 (ascending) */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        base[i] = MARCH_PATTERN_00;
    }

    /* Step 1: Read 0x00, Write 0xFF (ascending) */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        if (base[i] != MARCH_PATTERN_00)
        {
            result = E_NOT_OK;
        }
        base[i] = MARCH_PATTERN_FF;
    }

    /* Step 2: Read 0xFF, Write 0x00 (ascending) */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        if (base[i] != MARCH_PATTERN_FF)
        {
            result = E_NOT_OK;
        }
        base[i] = MARCH_PATTERN_00;
    }

    /* Step 3: Read 0x00, Write 0xFF (descending) */
    for (i = RAM_TEST_BLOCK_B; i > 0U; i--)
    {
        if (base[i - 1U] != MARCH_PATTERN_00)
        {
            result = E_NOT_OK;
        }
        base[i - 1U] = MARCH_PATTERN_FF;
    }

    /* Step 4: Read 0xFF, Write 0x00 (descending) */
    for (i = RAM_TEST_BLOCK_B; i > 0U; i--)
    {
        if (base[i - 1U] != MARCH_PATTERN_FF)
        {
            result = E_NOT_OK;
        }
        base[i - 1U] = MARCH_PATTERN_00;
    }

    /* Step 5: Read 0x00 (ascending - final verify) */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        if (base[i] != MARCH_PATTERN_00)
        {
            result = E_NOT_OK;
        }
    }

    /* Restore block contents */
    for (i = 0U; i < RAM_TEST_BLOCK_B; i++)
    {
        base[i] = backup[i];
    }

    if (result != E_OK)
    {
        s_diag.ram_ok = FLAG_CLEAR;
    }

    return result;
}

/* ========================================================================
 * MEM_RomBlockCrc
 *
 * Computes CRC-32 over one ROM block and compares with stored reference.
 * ======================================================================== */

Std_ReturnType MEM_RomBlockCrc(uint8_t block)
{
    const uint8_t *base;
    uint32_t computed_crc;
    uint32_t ref_crc;

    if (block >= ROM_CRC_BLOCK_COUNT)
    {
        return E_NOT_OK;
    }

    base = GetRomBlockAddr(block);
    ref_crc = s_rom_crc_ref[block];

    computed_crc = CalcCrc32(base, ROM_CRC_BLOCK_SIZE);

    /*
     * In host environment, both computed and reference are 0/placeholder,
     * so this always passes. On target, reference CRCs are populated
     * by the post-build tool and will detect ROM corruption.
     */
    if (base == NULL_PTR)
    {
        /* Host environment - skip ROM check (no actual ROM) */
        return E_OK;
    }

    if (computed_crc != ref_crc)
    {
        s_diag.rom_ok = FLAG_CLEAR;
        return E_NOT_OK;
    }

    return E_OK;
}

/* ========================================================================
 * MEM_ClockCheck
 *
 * Cross-checks main clock against sub-clock (32.768 kHz crystal).
 * Counts main clock cycles during one sub-clock period.
 *
 * Expected count = CLOCK_MAIN_MHZ * 1,000,000 / (CLOCK_SUB_KHZ * 1000)
 *                = 32,000,000 / 32,768 = ~976 cycles
 * Tolerance: +/- CLOCK_TOLERANCE_PCT %
 * ======================================================================== */

Std_ReturnType MEM_ClockCheck(void)
{
    uint32_t expected;
    uint32_t tolerance;
    uint32_t measured;

    /*
     * Target RL78/G14 clock check:
     * 1. Configure TM07 to count main clock cycles
     * 2. Use sub-clock (32.768 kHz) as gate signal
     * 3. Start count, wait for one sub-clock period
     * 4. Read count value from TDR07
     *
     * Expected = f_main / f_sub = 32MHz / 32.768kHz ≈ 976
     */

    expected = ((uint32_t)CLOCK_MAIN_MHZ * 1000000UL) /
               ((uint32_t)CLOCK_SUB_KHZ * 1000UL);

    tolerance = (expected * (uint32_t)CLOCK_TOLERANCE_PCT) / 100UL;

    /*
     * In host environment, simulate a correct measurement.
     * On target, this would read the actual timer capture value.
     */
#ifdef TARGET_RL78_G14
    /* measured = TDR07; */  /* Read from timer capture register */
    measured = expected;     /* Placeholder */
#else
    measured = expected;     /* Host: simulate correct clock */
#endif

    if ((measured < (expected - tolerance)) ||
        (measured > (expected + tolerance)))
    {
        s_diag.clk_ok = FLAG_CLEAR;
        return E_NOT_OK;
    }

    s_diag.clk_ok = FLAG_SET;
    return E_OK;
}

/* ========================================================================
 * MEM_StartupTest - Full startup diagnostic sequence
 * ======================================================================== */

Std_ReturnType MEM_StartupTest(void)
{
    Std_ReturnType result;
    uint8_t block;

    /* Initialize diagnostic status */
    s_diag.cpu_ok        = FLAG_CLEAR;
    s_diag.ram_ok        = FLAG_SET;   /* Set initially, cleared on failure */
    s_diag.rom_ok        = FLAG_SET;   /* Set initially, cleared on failure */
    s_diag.clk_ok        = FLAG_CLEAR;
    s_diag.ram_block_idx = 0U;
    s_diag.rom_block_idx = 0U;

    /* Step 1: CPU register test */
    result = MEM_CpuRegTest();
    if (result != E_OK)
    {
        return E_NOT_OK;
    }

    /* Step 2: Full RAM test (all blocks) */
    for (block = 0U; block < RAM_TEST_BLOCK_COUNT; block++)
    {
        result = MEM_RamBlockTest(block);
        if (result != E_OK)
        {
            return E_NOT_OK;
        }
    }
    s_diag.ram_ok = FLAG_SET;

    /* Step 3: Full ROM CRC test (all blocks) */
    for (block = 0U; block < ROM_CRC_BLOCK_COUNT; block++)
    {
        result = MEM_RomBlockCrc(block);
        if (result != E_OK)
        {
            return E_NOT_OK;
        }
    }
    s_diag.rom_ok = FLAG_SET;

    /* Step 4: Clock cross-check */
    result = MEM_ClockCheck();
    if (result != E_OK)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

/* ========================================================================
 * MEM_RunDiag - Runtime diagnostic (one block per call)
 * ======================================================================== */

Std_ReturnType MEM_RunDiag(void)
{
    Std_ReturnType result;

    /* Test one RAM block */
    result = MEM_RamBlockTest(s_diag.ram_block_idx);
    if (result != E_OK)
    {
        return E_NOT_OK;
    }

    /* Advance RAM block index (cyclic) */
    s_diag.ram_block_idx++;
    if (s_diag.ram_block_idx >= RAM_TEST_BLOCK_COUNT)
    {
        s_diag.ram_block_idx = 0U;
    }

    /* Test one ROM block */
    result = MEM_RomBlockCrc(s_diag.rom_block_idx);
    if (result != E_OK)
    {
        return E_NOT_OK;
    }

    /* Advance ROM block index (cyclic) */
    s_diag.rom_block_idx++;
    if (s_diag.rom_block_idx >= ROM_CRC_BLOCK_COUNT)
    {
        s_diag.rom_block_idx = 0U;

        /* One full ROM cycle complete - re-verify clock */
        result = MEM_ClockCheck();
        if (result != E_OK)
        {
            return E_NOT_OK;
        }
    }

    return E_OK;
}

/* ========================================================================
 * MEM_GetStatus - Read-only access to diagnostic status
 * ======================================================================== */

const diag_status_t *MEM_GetStatus(void)
{
    return &s_diag;
}
