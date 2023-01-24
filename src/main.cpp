#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "./tasks/tasks.h"
#include "./sanity/sanity_check.h"

//************************/
//*      DEFINES         */
//************************/

//************************/
//*      VARIABLES       */
//************************/
TaskHandle_t guiTaskHandle;
TaskHandle_t accelTaskHandle;
TaskHandle_t senseTaskHandle;
TaskHandle_t appTaskHandle;

//*******************************/
//*     FUNCTIONS DECLARATION   */
//*******************************/
extern "C"
{
    void app_main(void);
}

extern void guiTask(void *pvParameter);
extern void accelTask(void *pvParameter);
extern void senseTask(void *pvParameter);
extern void application(void *pvParameter);


//?^^^^^^^^^^^^^^^^^^^^^^^^^/
//?     APPLICATION MAIN    /
//?^^^^^^^^^^^^^^^^^^^^^^^^^/
void app_main()
{
    /* Creating tasks
     * ! GUI task:
     */
    xTaskCreatePinnedToCore(guiTask, "gui", 1024 * 8, NULL, 0, &guiTaskHandle, 1);

    /* Creating tasks
     * ! Accelerometer task:
     */
    xTaskCreatePinnedToCore(accelTask, "accel", 1024 * 4, NULL, 1, &accelTaskHandle, 0);

    /* Creating tasks
     * ! Position sensor task:
     */
    xTaskCreatePinnedToCore(senseTask, "sense", 1024 * 4, NULL, 1, &senseTaskHandle, 1);

    /* Creating tasks
     * ! DPB app task:
     */
    xTaskCreatePinnedToCore(application, "app", 1024 * 8, NULL, 0, &appTaskHandle, 0);
}