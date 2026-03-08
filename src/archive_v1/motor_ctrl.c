/**
 * @file motor_ctrl.c
 * @brief BLDC Motor Control Implementation
 * @details Speed calculation, overspeed detection, PWM control
 * @version 1.0
 * @date 2026-02-27
 * @author TORASAN Motor Control Project
 *
 * @req DR-007: BLDC motor speed control and monitoring
 * @req DR-008: Overspeed fault detection
 *
 * @safety_class IEC 60730-2-1 Class B / ASIL QM
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "motor_ctrl.h"
#include "safety_mgr.h"
#include "hal_timer.h"
#include "config.h"

/* ========================================================================
 * CONFIGURATION & CONSTANTS
 * ======================================================================== */

/**
 * @def MOTOR_RATED_RPM
 * @brief Motor rated speed (1200 rpm at nominal voltage)
 * @safety [SR-001][クラスB] Operating point reference
 */
#define MOTOR_RATED_RPM             (1200U)

/**
 * @def MOTOR_OVERSPEED_THRESHOLD_RPM
 * @brief Maximum allowable motor speed (1500 rpm = 125% rated)
 * @safety [SR-001][クラスB] Overspeed fault threshold
 */
#define MOTOR_OVERSPEED_THRESHOLD_RPM   (1500U)

/**
 * @def MOTOR_POLE_PAIRS
 * @brief Number of pole pairs in BLDC motor (affects speed calculation)
 * @note 4 poles = 2 pole pairs; 1500rpm / 60s = 25 Hz; 25Hz / 2 = 12.5 Hall edges/sec
 */
#define MOTOR_POLE_PAIRS            (2U)

/**
 * @def MOTOR_HALL_EDGES_PER_ROTATION
 * @brief Number of Hall sensor edges per motor rotation
 * @note BLDC with 2 pole pairs = 6 edges per rotation
 */
#define MOTOR_HALL_EDGES_PER_ROTATION  (6U)

/**
 * @def MOTOR_IIR_LPF_ALPHA
 * @brief Low-pass filter coefficient (0.2 = 80% old, 20% new)
 * @safety [SR-002][クラスB] Noise rejection filter constant
 * @details Provides stability against Hall noise while maintaining responsiveness
 */
#define MOTOR_IIR_LPF_ALPHA         (0.2f)

/**
 * @def MOTOR_OVERSPEED_DEBOUNCE_COUNT
 * @brief Number of consecutive samples above threshold to trigger fault
 * @safety [SR-001][クラスB] Debouncing to avoid transient faults
 */
#define MOTOR_OVERSPEED_DEBOUNCE_COUNT  (3U)

/**
 * @def MOTOR_FREQUENCY_TO_RPM_FACTOR
 * @brief Conversion: Hall frequency (Hz) → RPM
 * @calc 1500rpm = 25Hz (1500/60); 25Hz = 12.5 edges/sec * 2 pole pairs
 * @calc RPM = (Hz * 60) / MOTOR_HALL_EDGES_PER_ROTATION
 */
#define MOTOR_FREQUENCY_TO_RPM_FACTOR   (10.0f)  /* 60 / 6 edges per rotation */

/**
 * @def MOTOR_PWM_PERIOD_US
 * @brief PWM switching frequency period (microseconds)
 * @note 20kHz = 50us period (typical for BLDC)
 */
#define MOTOR_PWM_PERIOD_US         (50U)

/**
 * @def MOTOR_PWM_MAX_DUTY
 * @brief Maximum PWM duty cycle (0-100%)
 */
#define MOTOR_PWM_MAX_DUTY          (100U)

/**
 * @def MOTOR_STALL_RPM_THRESHOLD
 * @brief Below this RPM for extended period = stall condition
 * @safety [SR-002][クラスB] Motor stall detection threshold
 */
#define MOTOR_STALL_RPM_THRESHOLD   (100U)

/* ========================================================================
 * MODULE STATE
 * ======================================================================== */

/**
 * @struct MotorCtrl_State_t
 * @brief Motor control state tracker
 * @safety [SR-001][SR-002][SR-003][クラスB] Speed and control state
 */
typedef struct {
    float current_rpm;              /*!< Filtered motor speed (IIR LPF) */
    float target_rpm;               /*!< Desired motor speed setpoint */
    uint8_t pwm_duty;               /*!< Current PWM duty cycle (0-100%) */
    uint32_t last_hall_timestamp_us;/*!< Timestamp of last Hall edge (microseconds) */
    uint16_t hall_period_us;        /*!< Period between Hall edges (us) */
    uint8_t overspeed_counter;      /*!< Debounce counter for overspeed detection */
    uint8_t stall_counter;          /*!< Debounce counter for stall detection */
    bool motor_running;             /*!< Motor spin status */
} MotorCtrl_State_t;

/**
 * @var motor_ctrl_state
 * @brief Module-level motor control state
 * @safety [SR-001][SR-002][SR-003][クラスB] Non-volatile across calls
 */
static MotorCtrl_State_t motor_ctrl_state = {
    .current_rpm = 0.0f,
    .target_rpm = 0.0f,
    .pwm_duty = 0U,
    .last_hall_timestamp_us = 0UL,
    .hall_period_us = 0xFFFFU,
    .overspeed_counter = 0U,
    .stall_counter = 0U,
    .motor_running = false
};

/* ========================================================================
 * INITIALIZATION & CONFIGURATION
 * ======================================================================== */

/**
 * @brief Initialize Motor Control Module
 * @details Set up PWM, Hall sensor capture, and initial state
 *
 * @safety [SR-001][SR-002][SR-003][クラスB] Module initialization
 */
void MotorCtrl_Init(void)
{
    /* [SR-001][SR-002][SR-003][クラスB] Motor control module initialization */

    /* Initialize PWM channels (3-phase: U, V, W phases) */
    HAL_Timer_InitPwm(MOTOR_PWM_PERIOD_US, 0U);  /* Start at 0% duty */

    /* Initialize Hall sensor capture interrupt */
    HAL_Timer_InitHallCapture();

    /* Reset state */
    motor_ctrl_state.current_rpm = 0.0f;
    motor_ctrl_state.target_rpm = 0.0f;
    motor_ctrl_state.pwm_duty = 0U;
    motor_ctrl_state.hall_period_us = 0xFFFFU;
    motor_ctrl_state.overspeed_counter = 0U;
    motor_ctrl_state.stall_counter = 0U;
    motor_ctrl_state.motor_running = false;
}

/* ========================================================================
 * SPEED MONITORING
 * ======================================================================== */

/**
 * @brief Calculate Motor Speed from Hall Sensor Period
 * @details Convert Hall sensor edge period to RPM using 1st-order IIR LPF
 * @param hall_period_us [in] Time between Hall edges (microseconds)
 *
 * @req DR-007: Speed calculation from Hall period
 * @safety [SR-002][クラスB] Filtered speed calculation with noise rejection
 */
static void MotorCtrl_UpdateSpeed(uint16_t hall_period_us)
{
    /* [SR-002][クラスB] Speed calculation: Hall period → RPM with IIR LPF */

    /* Sanity check: reject unreasonable periods (avoid division by ~0) */
    if (hall_period_us < 10U) {
        return;  /* Unrealistic hall period, ignore */
    }

    /* Convert Hall period to frequency (Hz): f = 1e6 / period_us */
    float hall_frequency_hz = 1000000.0f / (float)hall_period_us;

    /* Convert frequency to RPM */
    float new_rpm = hall_frequency_hz * MOTOR_FREQUENCY_TO_RPM_FACTOR;

    /* 1st-order IIR Low-Pass Filter: new = (1-alpha)*old + alpha*measured */
    motor_ctrl_state.current_rpm =
        ((1.0f - MOTOR_IIR_LPF_ALPHA) * motor_ctrl_state.current_rpm) +
        (MOTOR_IIR_LPF_ALPHA * new_rpm);

    motor_ctrl_state.hall_period_us = hall_period_us;
    motor_ctrl_state.motor_running = true;
}

/**
 * @brief Get Current Motor Speed
 * @return Motor speed in RPM (filtered)
 *
 * @safety [SR-002][クラスB] Speed reading for control loop
 */
float MotorCtrl_GetRpm(void)
{
    /* [SR-002][クラスB] Query filtered motor speed */
    return motor_ctrl_state.current_rpm;
}

/**
 * @brief Hall Sensor Capture Callback (called from ISR)
 * @details Invoked on each Hall edge; updates speed calculation
 * @param timestamp_us [in] Timestamp of Hall edge (microseconds)
 *
 * @safety [SR-002][クラスB] Called from timer interrupt
 */
void MotorCtrl_HallEdgeISR(uint32_t timestamp_us)
{
    /* [SR-002][クラスB] Hall edge interrupt handler: update period */

    if (motor_ctrl_state.last_hall_timestamp_us != 0UL) {
        uint32_t period_us = timestamp_us - motor_ctrl_state.last_hall_timestamp_us;
        if (period_us < 65536U) {  /* Fits in uint16_t */
            MotorCtrl_UpdateSpeed((uint16_t)period_us);
        }
    }

    motor_ctrl_state.last_hall_timestamp_us = timestamp_us;
}

/* ========================================================================
 * OVERSPEED DETECTION & PROTECTION [SR-001]
 * ======================================================================== */

/**
 * @brief Check Motor Overspeed Condition
 * @details Compare current speed vs 1500rpm threshold with 3-count debounce.
 *          Triggers fault only if overspeed is sustained.
 * @return true if overspeed is detected and debounced, false otherwise
 *
 * @req DR-008: Overspeed fault detection
 * @safety [SR-001][クラスB] Overspeed detection with debouncing
 */
bool MotorCtrl_CheckOverspeed(void)
{
    /* [SR-001][クラスB] Overspeed check: 1500rpm threshold, 3-count debounce */

    if (motor_ctrl_state.current_rpm > (float)MOTOR_OVERSPEED_THRESHOLD_RPM) {
        motor_ctrl_state.overspeed_counter++;

        /* [SR-001] Trigger fault after debounce count exceeded */
        if (motor_ctrl_state.overspeed_counter >= MOTOR_OVERSPEED_DEBOUNCE_COUNT) {
            return true;
        }
    } else {
        /* [SR-001] Clear counter if below threshold */
        motor_ctrl_state.overspeed_counter = 0U;
    }

    return false;
}

/**
 * @brief Get Overspeed Counter (for diagnostics)
 * @return Current debounce counter value
 *
 * @safety [SR-001][クラスB] Debug support
 */
uint8_t MotorCtrl_GetOverspeedCounter(void)
{
    /* [SR-001][クラスB] Query overspeed debounce state */
    return motor_ctrl_state.overspeed_counter;
}

/* ========================================================================
 * MOTOR CONTROL
 * ======================================================================== */

/**
 * @brief Emergency Stop: Immediate PWM Shutdown
 * @details Force all PWM channels to inactive state (safe state)
 *
 * @req DR-007: Emergency motor stop
 * @safety [SR-003][クラスB] Emergency stop: PWM forced to 0% duty
 */
void MotorCtrl_EmergencyStop(void)
{
    /* [SR-003][クラスB] Emergency stop: Motor PWM kill */

    HAL_Timer_StopAllPwm();
    motor_ctrl_state.pwm_duty = 0U;
    motor_ctrl_state.target_rpm = 0.0f;
    motor_ctrl_state.motor_running = false;
}

/**
 * @brief Set Motor Target Speed (Speed Control Setpoint)
 * @param target_rpm [in] Desired speed (0-1500 RPM, clamped)
 *
 * @safety [SR-003][クラスB] Speed setpoint update
 */
void MotorCtrl_SetTargetRpm(uint16_t target_rpm)
{
    /* [SR-003][クラスB] Set motor speed setpoint (clamped to safe range) */

    /* Clamp to maximum safe speed */
    if (target_rpm > MOTOR_OVERSPEED_THRESHOLD_RPM) {
        target_rpm = MOTOR_OVERSPEED_THRESHOLD_RPM;
    }

    motor_ctrl_state.target_rpm = (float)target_rpm;
}

/**
 * @brief Set Motor PWM Duty Cycle
 * @param duty_percent [in] PWM duty (0-100%), clamped
 * @details In a real implementation, this would be called by a closed-loop
 *          speed controller (PI/PID). This is the low-level actuator interface.
 *
 * @req DR-007: PWM modulation
 * @safety [SR-003][クラスB] PWM duty cycle application
 */
void MotorCtrl_SetDuty(uint8_t duty_percent)
{
    /* [SR-003][クラスB] Apply PWM duty cycle to motor */

    /* Clamp duty to valid range */
    if (duty_percent > MOTOR_PWM_MAX_DUTY) {
        duty_percent = MOTOR_PWM_MAX_DUTY;
    }

    motor_ctrl_state.pwm_duty = duty_percent;

    /* Apply to hardware PWM */
    HAL_Timer_SetPwmDuty(duty_percent);
}

/**
 * @brief Get Current PWM Duty Cycle
 * @return Current duty cycle (0-100%)
 *
 * @safety [SR-003][クラスB] Duty status query
 */
uint8_t MotorCtrl_GetDuty(void)
{
    /* [SR-003][クラスB] Query current PWM duty */
    return motor_ctrl_state.pwm_duty;
}

/**
 * @brief Simple Open-Loop Speed Controller (Placeholder)
 * @details Maps target RPM to PWM duty linearly.
 *          In production: use closed-loop PI/PID with feedback.
 *
 * @safety [SR-003][クラスB] Speed control loop
 */
void MotorCtrl_SpeedControl(void)
{
    /* [SR-003][クラスB] Motor speed control (simple proportional) */

    /* Check for overspeed before proceeding */
    if (MotorCtrl_CheckOverspeed()) {
        SafetyMgr_FaultOverspeed();
        return;
    }

    /* [SR-003] Linear mapping: target_rpm → pwm_duty
     * At 1200 RPM rated: expect ~100% duty; scale proportionally */
    float duty_float = (motor_ctrl_state.target_rpm / (float)MOTOR_RATED_RPM) * 100.0f;

    /* Clamp and apply */
    if (duty_float < 0.0f) duty_float = 0.0f;
    if (duty_float > 100.0f) duty_float = 100.0f;

    MotorCtrl_SetDuty((uint8_t)duty_float);
}

/**
 * @brief Check Motor Stall Condition
 * @details Detect if motor is unable to spin despite PWM applied
 * @return true if stall detected, false otherwise
 *
 * @safety [SR-002][クラスB] Stall detection
 */
bool MotorCtrl_CheckStall(void)
{
    /* [SR-002][クラスB] Motor stall detection: low RPM despite PWM */

    if ((motor_ctrl_state.motor_running) &&
        (motor_ctrl_state.pwm_duty > 50U) &&
        (motor_ctrl_state.current_rpm < (float)MOTOR_STALL_RPM_THRESHOLD)) {

        motor_ctrl_state.stall_counter++;
        if (motor_ctrl_state.stall_counter >= 5U) {
            return true;
        }
    } else {
        motor_ctrl_state.stall_counter = 0U;
    }

    return false;
}

/**
 * @brief Get Motor Running Status
 * @return true if motor is spinning, false otherwise
 *
 * @safety [SR-002][クラスB] Motor state query
 */
bool MotorCtrl_IsRunning(void)
{
    /* [SR-002][クラスB] Query motor running status */
    return motor_ctrl_state.motor_running;
}

/* End of motor_ctrl.c */
