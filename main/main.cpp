#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tasks.hpp"
#include "sanity/sanity_check.h"

//************************/
//*      DEFINES         */
//************************/

//************************/
//*      VARIABLES       */
//************************/
TaskHandle_t guiTaskHandle;
TaskHandle_t accelTaskHandle;
TaskHandle_t sensorTaskHandle;
TaskHandle_t appTaskHandle;

//*******************************/
//*     FUNCTIONS DECLARATION   */
//*******************************/
extern void guiTask(void *pvParameter);
extern void accelTask(void *pvParameter);
extern void sensorTask(void *pvParameter);
extern void application(void *pvParameter);

extern "C"
{
    void app_main();
}

//?^^^^^^^^^^^^^^^^^^^^^^^^^/
//?     APPLICATION MAIN    /
//?^^^^^^^^^^^^^^^^^^^^^^^^^/
extern "C"
{
    void app_main()
    {
        /* Creating tasks
         * ! GUI task:
         */
        xTaskCreatePinnedToCore(guiTask, "gui", 1024 * 6, NULL, 0, &guiTaskHandle, 1);

        /* Creating tasks
         * ! Accelerometer task:
         */
        xTaskCreatePinnedToCore(accelTask, "accel", 1024 * 2, NULL, 1, &accelTaskHandle, 0);

        /* Creating tasks
         * ! RPM sensor task:
         */
        xTaskCreatePinnedToCore(sensorTask, "sensor", 1024 * 2, NULL, 1, &sensorTaskHandle, 1);

        /* Creating tasks
         * ! DPB app task:
         */
        xTaskCreatePinnedToCore(application, "app", 1024 * 7, NULL, 0, &appTaskHandle, 0);
    }
}