/* FreeRTOS Real Time Stats Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define NUM_OF_SPIN_TASKS   6
#define SPIN_ITER           500000  //Actual CPU cycles used will depend on compiler optimization
#define SPIN_TASK_PRIO      2
#define STATS_TASK_PRIO     3
#define STATS_TICKS         pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

#define LED_GREEN 12
#define LED_RED 13
#define LED_CHEAT 14

#define BTN_CHEAT 27
#define BTN_FAIL 26
#define sensor_time 30
#define injector_time 29

static char task_names[NUM_OF_SPIN_TASKS][configMAX_TASK_NAME_LEN];
static SemaphoreHandle_t sync_spin_task;
static SemaphoreHandle_t sync_stats_task;

volatile int fuel_value = 100;

volatile bool cheat_enabled = false;
volatile bool fail_enabled = false;

// semaforos
SemaphoreHandle_t sensor_ready;

/**
 * @brief   Function to print the CPU usage of tasks over a given duration.
 *
 * This function will measure and print the CPU usage of tasks over a specified
 * number of ticks (i.e. real time stats). This is implemented by simply calling
 * uxTaskGetSystemState() twice separated by a delay, then calculating the
 * differences of task run times before and after the delay.
 *
 * @note    If any tasks are added or removed during the delay, the stats of
 *          those tasks will not be printed.
 * @note    This function should be called from a 1 priority task to minimize
 *          inaccuracies with delays.
 * @note    When running in dual core mode, each core will correspond to 50% of
 *          the run time.
 *
 * @param   xTicksToWait    Period of stats measurement
 *
 * @return
 *  - ESP_OK                Success
 *  - ESP_ERR_NO_MEM        Insufficient memory to allocated internal arrays
 *  - ESP_ERR_INVALID_SIZE  Insufficient array size for uxTaskGetSystemState. Trying increasing ARRAY_SIZE_OFFSET
 *  - ESP_ERR_INVALID_STATE Delay duration too short
 */


void SensorTask(void *pvParameters)
{
  TickType_t xLastWakeTime;
  const TickType_t period = pdMS_TO_TICKS(sensor_time);

  xLastWakeTime = xTaskGetTickCount();

  while (true)
  {
    // avisa que o sensor vai medir
    xSemaphoreGive(sensor_ready);

    // leitura
    if (fuel_value > 100)
    {
      gpio_set_level(LED_GREEN, 0);
      gpio_set_level(LED_RED, 1);
      //ESP_LOGW("SensorLog","Sensor OK");
    }

    ESP_LOGW("Sensor Log","combustivel: %d",fuel_value);

    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// ---------------- INJETOR ----------------
void InjectorTask(void *pvParameters)
{
  while (true)
  {
    // espera o sensor avisar que vai ler
  xSemaphoreTake(sensor_ready, portMAX_DELAY);

    // leitura dos botoes
  cheat_enabled = !gpio_get_level(BTN_CHEAT);
  fail_enabled  = !gpio_get_level(BTN_FAIL);

  //ESP_LOGW("Injector Log","cheat%d",cheat_enabled);


    // ----- MODO LEGAL -----
    if (!cheat_enabled)
    {
      gpio_set_level(LED_CHEAT, 0);
      fuel_value = 100;
    }

    // ----- MODO CHEAT -----
    else
    {
      fuel_value = 120;
      gpio_set_level(LED_CHEAT, 1);
      ESP_LOGW("Injector Log","combustivel: %d",fuel_value);
      vTaskDelay(pdMS_TO_TICKS(injector_time));
      if (!fail_enabled)
      {
        // engana o sensor
        fuel_value = 100;
      }
      else
      {
        // falha proposital
        vTaskDelay(pdMS_TO_TICKS(5));
        fuel_value = 120;
      }
    }

  }
}

void app_main(void)
{
  gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_CHEAT, GPIO_MODE_OUTPUT);

  gpio_set_direction(BTN_CHEAT, GPIO_MODE_INPUT);
  gpio_set_direction(BTN_FAIL, GPIO_MODE_INPUT);

  gpio_set_pull_mode(BTN_CHEAT, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(BTN_FAIL, GPIO_PULLUP_ONLY);

  gpio_set_intr_type(BTN_CHEAT, GPIO_INTR_NEGEDGE);
  gpio_set_intr_type(BTN_FAIL, GPIO_INTR_NEGEDGE);

  gpio_set_level(LED_GREEN,1);


  // cria semaforos
  sensor_ready = xSemaphoreCreateBinary();

  // cria tasks
  xTaskCreatePinnedToCore(
    SensorTask,
    "Sensor",
    2048,
    NULL,
    3,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    InjectorTask,
    "Injector",
    2048,
    NULL,
    2,
    NULL,
    1
  );

}
