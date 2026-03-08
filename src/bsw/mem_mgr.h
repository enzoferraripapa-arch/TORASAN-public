/**
 * @file    mem_mgr.h
 * @brief   Memory management (NVM error log persistence)
 * @module  SA-006 MEM_MGR
 * @safety  IEC 60730 Class B
 * @req     SR-006
 */

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <stdint.h>
#include "../types/std_types.h"
#include "../types/safety_types.h"

/**
 * @brief   Initialize memory manager (read NVM header)
 * @module  SA-006 MEM_MGR
 */
void MemMgr_Init(void);

/**
 * @brief   Write error log entry to NVM (circular buffer)
 * @module  SA-006 MEM_MGR
 * @req     SR-006
 * @safety  IEC 60730 Class B
 * @param   p_entry  Pointer to log entry to write
 * @return  STD_OK on success, STD_NOT_OK on failure
 */
uint8_t MemMgr_WriteLog(const LogEntry_t *p_entry);

/**
 * @brief   Read error log entry from NVM
 * @module  SA-006 MEM_MGR
 * @param   index    Log index (0 = most recent)
 * @param   p_entry  Pointer to receive log entry
 * @return  STD_OK on success, STD_NOT_OK if index out of range
 */
uint8_t MemMgr_ReadLog(uint8_t index, LogEntry_t *p_entry);

/**
 * @brief   Get number of stored log entries
 * @module  SA-006 MEM_MGR
 * @return  Number of valid log entries
 */
uint8_t MemMgr_GetLogCount(void);

#endif /* MEM_MGR_H */
