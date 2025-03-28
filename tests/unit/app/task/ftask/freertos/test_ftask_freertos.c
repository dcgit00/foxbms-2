/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    test_ftask_freertos.c
 * @author  foxBMS Team
 * @date    2021-11-26 (date of creation)
 * @updated 2024-12-20 (date of last update)
 * @version v1.8.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ftask implementation for FreeRTOS
 * @details Tests Create Task
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockftask_cfg.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"
#include "Mocksys_mon.h"

#include "ftask.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ftask_freertos.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/fram")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/engine/sys_mon")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
OS_TASK_HANDLE ftsk_taskHandleAfe;
OS_TASK_HANDLE ftsk_taskHandleI2c;

OS_TASK_DEFINITION_s ftsk_taskDefinitionEngine = {
    OS_PRIORITY_REAL_TIME,
    FTSK_TASK_ENGINE_PHASE,
    FTSK_TASK_ENGINE_CYCLE_TIME,
    FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_ENGINE_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic1ms = {
    OS_PRIORITY_ABOVE_HIGH,
    FTSK_TASK_CYCLIC_1MS_PHASE,
    FTSK_TASK_CYCLIC_1MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_1MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic10ms = {
    OS_PRIORITY_HIGH,
    FTSK_TASK_CYCLIC_10MS_PHASE,
    FTSK_TASK_CYCLIC_10MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_10MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclic100ms = {
    OS_PRIORITY_ABOVE_NORMAL,
    FTSK_TASK_CYCLIC_100MS_PHASE,
    FTSK_TASK_CYCLIC_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_100MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionCyclicAlgorithm100ms = {
    OS_PRIORITY_NORMAL,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PHASE,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_CYCLE_TIME,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES,
    FTSK_TASK_CYCLIC_ALGORITHM_100MS_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionI2c = {
    FTSK_TASK_I2C_PRIORITY,
    FTSK_TASK_I2C_PHASE,
    FTSK_TASK_I2C_CYCLE_TIME,
    FTSK_TASK_I2C_STACK_SIZE_IN_BYTES,
    FTSK_TASK_I2C_PV_PARAMETERS};
OS_TASK_DEFINITION_s ftsk_taskDefinitionAfe = {
    FTSK_TASK_AFE_PRIORITY,
    FTSK_TASK_AFE_PHASE,
    FTSK_TASK_AFE_CYCLE_TIME,
    FTSK_TASK_AFE_STACK_SIZE_IN_BYTES,
    FTSK_TASK_AFE_PV_PARAMETERS};

/** boot state of the OS */
volatile OS_BOOT_STATE_e os_boot = OS_OFF;
/** timestamp of the scheduler start */
uint32_t os_schedulerStartTime = 0u;

/** helper function that sets up everything for the test in #testFTSK_CreateTasks() */
void helperCreateStatic(const char *pTaskName, OS_TASK_HANDLE handleToBeReturned) {
    MPU_xTaskCreateStatic_ExpectAndReturn(NULL_PTR, pTaskName, 0u, 0u, 0u, NULL_PTR, NULL_PTR, handleToBeReturned);
    MPU_xTaskCreateStatic_IgnoreArg_pxTaskCode();
    MPU_xTaskCreateStatic_IgnoreArg_ulStackDepth();
    MPU_xTaskCreateStatic_IgnoreArg_pvParameters();
    MPU_xTaskCreateStatic_IgnoreArg_uxPriority();
    MPU_xTaskCreateStatic_IgnoreArg_puxStackBuffer();
    MPU_xTaskCreateStatic_IgnoreArg_pxTaskBuffer();
}

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** test correct task creation */
void testFTSK_CreateTasks(void) {
    OS_TASK_HANDLE dummyHandleSuccess = (OS_TASK_HANDLE)1u;
    helperCreateStatic("TaskEngine", dummyHandleSuccess);
    helperCreateStatic("TaskCyclic1ms", dummyHandleSuccess);
    helperCreateStatic("TaskCyclic10ms", dummyHandleSuccess);
    helperCreateStatic("TaskCyclic100ms", dummyHandleSuccess);
    helperCreateStatic("TaskCyclicAlgorithm100ms", dummyHandleSuccess);
    helperCreateStatic("TaskI2c", dummyHandleSuccess);
    helperCreateStatic("TaskAfe", dummyHandleSuccess);
    FTSK_CreateTasks();
}
