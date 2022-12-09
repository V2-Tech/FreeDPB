#include "main.h"

/**********************
 *   APPLICATION MAIN
 **********************/
void app_main() 
{
    /* Creating tasks
    * ! GUI task: 
    */
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);

    /* Creating tasks
    * ! Sensor task: 
    */
    xTaskCreatePinnedToCore(accelTask, "accel", 4096*2, NULL, 1, NULL, 0);

    /* Creating tasks
    * ! Filter task: 
    */
    //xTaskCreatePinnedToCore(filterTask, "filter", 4096*2, NULL, 1, NULL, 0);

    /* Creating tasks
    * ! ESC task: 
    */
    xTaskCreatePinnedToCore(motorTask, "motor", 4096*2, NULL, 1, NULL, 0);
}