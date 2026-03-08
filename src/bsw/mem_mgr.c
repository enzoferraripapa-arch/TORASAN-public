/**
 * @file    mem_mgr.c
 * @brief   Memory management implementation (NVM error log)
 * @module  SA-006 MEM_MGR (UT-015)
 * @safety  IEC 60730 Class B
 * @req     SR-006
 *
 * Uses RL78/G14 data flash (4KB) as NVM.
 * Circular buffer for error log entries.
 */

#include "mem_mgr.h"
#include "../config/safety_config.h"

/* ============================================================
 * NVM Layout (simulated in RAM for host build)
 * ============================================================ */

/** Error log circular buffer in NVM */
static LogEntry_t s_log_buffer[ERROR_LOG_MAX_ENTRIES];

/** Write index (next write position) */
static uint8_t s_write_index = 0U;

/** Number of valid entries (max = ERROR_LOG_MAX_ENTRIES) */
static uint8_t s_log_count = 0U;

/* ============================================================
 * MemMgr_Init
 * ============================================================ */
void MemMgr_Init(void)
{
    uint8_t i;

    /* Clear log buffer */
    for (i = 0U; i < ERROR_LOG_MAX_ENTRIES; i++)
    {
        s_log_buffer[i].fault_code    = FAULT_NONE;
        s_log_buffer[i].timestamp_ms  = 0UL;
    }

    s_write_index = 0U;
    s_log_count   = 0U;
}

/* ============================================================
 * MemMgr_WriteLog
 * ============================================================ */
uint8_t MemMgr_WriteLog(const LogEntry_t *p_entry)
{
    uint8_t result;

    if (p_entry != NULL_PTR)
    {
        /* Write to current position (circular overwrite) */
        s_log_buffer[s_write_index].fault_code   = p_entry->fault_code;
        s_log_buffer[s_write_index].timestamp_ms = p_entry->timestamp_ms;

        /* Advance write index */
        s_write_index++;
        if (s_write_index >= ERROR_LOG_MAX_ENTRIES)
        {
            s_write_index = 0U;
        }

        /* Update count (saturate at max) */
        if (s_log_count < ERROR_LOG_MAX_ENTRIES)
        {
            s_log_count++;
        }

        result = STD_OK;
    }
    else
    {
        result = STD_NOT_OK;
    }

    return result;
}

/* ============================================================
 * MemMgr_ReadLog
 * ============================================================ */
uint8_t MemMgr_ReadLog(uint8_t index, LogEntry_t *p_entry)
{
    uint8_t result;
    uint8_t read_pos;

    if ((p_entry != NULL_PTR) && (index < s_log_count))
    {
        /* Index 0 = most recent, index 1 = second most recent, etc.
         * read_pos = (write_index - 1 - index) mod MAX */
        if (s_write_index > index)
        {
            read_pos = (uint8_t)(s_write_index - 1U - index);
        }
        else
        {
            read_pos = (uint8_t)(ERROR_LOG_MAX_ENTRIES - 1U - index + s_write_index);
        }

        p_entry->fault_code   = s_log_buffer[read_pos].fault_code;
        p_entry->timestamp_ms = s_log_buffer[read_pos].timestamp_ms;

        result = STD_OK;
    }
    else
    {
        result = STD_NOT_OK;
    }

    return result;
}

/* ============================================================
 * MemMgr_GetLogCount
 * ============================================================ */
uint8_t MemMgr_GetLogCount(void)
{
    return s_log_count;
}
