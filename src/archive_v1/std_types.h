/**
 * @file   std_types.h
 * @brief  プロジェクト共通型定義
 * @detail MISRA-C:2012準拠。全ソースファイルが最初にincludeすること。
 *         bool/true/false は使用禁止。本ファイルの型定義を使用する。
 *
 * @project WMC (Washing Machine Motor Control)
 * @standard IEC 60730 Class B / MISRA-C:2012
 * @version 2.0
 * @date    2026-02-28
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/* ========================================================================
 * 固定幅整数型（ISO C99 stdint.h）
 * MISRA-C:2012 Dir 4.6: 基本型の直接使用禁止。固定幅型のみ使用可
 * ======================================================================== */
#include <stdint.h>

/* ========================================================================
 * 論理値型（bool代替）
 * MISRA-C:2012 Rule 10.1: stdbool.hのboolは処理系依存のため使用禁止
 * uint8_t ベースで定義する
 * ======================================================================== */

/** @brief 論理値型（0 or 1 のみ） */
typedef uint8_t     Std_BoolType;

/** @brief 論理値: 真 */
#define STD_TRUE    ((Std_BoolType)1U)

/** @brief 論理値: 偽 */
#define STD_FALSE   ((Std_BoolType)0U)

/* ========================================================================
 * 関数戻り値型
 * 全モジュール共通の標準戻り値型を定義する
 * ======================================================================== */

/** @brief 標準戻り値型 */
typedef uint8_t     Std_ReturnType;

/** @brief 戻り値: 正常完了 */
#define E_OK        ((Std_ReturnType)0U)

/** @brief 戻り値: 異常検出 */
#define E_NOT_OK    ((Std_ReturnType)1U)

/** @brief 戻り値: 未準備 / リソース使用中 */
#define E_PENDING   ((Std_ReturnType)2U)

/* ========================================================================
 * NULL ポインタ
 * ======================================================================== */
#ifndef NULL_PTR
#define NULL_PTR    ((void *)0)
#endif

/* ========================================================================
 * ビット操作マクロ
 * ======================================================================== */

/** @brief ビットセット */
#define STD_BIT_SET(reg, bit)    ((reg) |= (uint8_t)(1U << (bit)))

/** @brief ビットクリア */
#define STD_BIT_CLR(reg, bit)    ((reg) &= (uint8_t)(~(1U << (bit))))

/** @brief ビットテスト (0 or 非0) */
#define STD_BIT_TST(reg, bit)    (((reg) & (uint8_t)(1U << (bit))) != 0U)

/* ========================================================================
 * float 安全マクロ
 * MISRA-C:2012 Rule 13.3/10.1: float同士の == != 比較禁止
 * 許容誤差比較のみ使用可
 * ======================================================================== */

/** @brief float許容誤差比較（|a - b| < eps ならば真） */
#define STD_FLOAT_NEAR(a, b, eps) \
    ((((a) - (b)) < (eps)) && (((b) - (a)) < (eps)))

/** @brief float ゼロ判定用イプシロン */
#define STD_FLOAT_EPSILON       (0.001f)

/* ========================================================================
 * ターゲット分岐
 * ======================================================================== */

/* RL78/G14 ターゲットかホストコンパイルかを区別 */
#if defined(__CCRL__) || defined(__RL78__)
    #define TARGET_RL78_G14
#endif

/* ========================================================================
 * コンパイラ固有属性の抽象化
 * ======================================================================== */

#ifdef TARGET_RL78_G14
    #define STD_SECTION(name)   __attribute__((section(name)))
    #define STD_INLINE          __inline
    #define STD_NORETURN        __attribute__((noreturn))
#else
    /* ホストGCC用 */
    #define STD_SECTION(name)   __attribute__((section(name)))
    #define STD_INLINE          static inline
    #define STD_NORETURN        __attribute__((noreturn))
#endif

#endif /* STD_TYPES_H */
