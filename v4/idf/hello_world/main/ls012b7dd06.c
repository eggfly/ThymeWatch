#include "ls012b7dd06.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// This function will be called by the ESP log library every time ESP_LOG needs to be performed.
//      @important Do NOT use the ESP_LOG* macro's in this function ELSE recursive loop and stack overflow! So use printf() instead for debug messages.
/* int _log_vprintf(const char* fmt, va_list args) {
    static bool static_fatal_error = false;
    static const uint32_t WRITE_CACHE_CYCLE = 5;
    static uint32_t counter_write = 0;
    int iresult;

    // #1 Write to SPIFFS
    if (_log_remote_fp == NULL) {
        printf("%s() ABORT. file handle _log_remote_fp is NULL\n", __FUNCTION__);
        return -1;
    }
    if (static_fatal_error == false) {
        iresult = vfprintf(_log_remote_fp, fmt, args);
        if (iresult < 0) {
            printf("%s() ABORT. failed vfprintf() -> disable future vfprintf(_log_remote_fp) \n", __FUNCTION__);
            // MARK FATAL
            static_fatal_error = true;
            return iresult;
        }

        // #2 Smart commit after x writes
        counter_write++;
        if (counter_write % WRITE_CACHE_CYCLE == 0) {
            /////printf("%s() fsync'ing log file on SPIFFS (WRITE_CACHE_CYCLE=%u)\n", WRITE_CACHE_CYCLE);
            fsync(fileno(_log_remote_fp));
        }
    }

    // #3 ALWAYS Write to stdout!
    return vprintf(fmt, args);
}
*/

/* void LOG_TO_STRING(const char *format,...)
{
    ESP_LOGI("NO TAG","Call Accepted ");

        char *string;//printf result will be stored in this     
        va_list arguments_list;
        va_start(arguments_list,format);//Initialiasing the List 
        
                    //Calculating & Allocating Size 
        
                    size_t size_string=snprintf(NULL,0,format,arguments_list);//Calculating the size of the formed string 
                    string=(char *)malloc(size_string+4);//Initialising the string 
                   

                    vsnprintf(string,size_string,format,arguments_list);//Storing the outptut into the string 
                     va_end(arguments_list);//Deinitializing the List 

                     ESP_LOGI("NO TAG","Converted String is :  %s",string);

    free(string);
}
*/

/*
char log_print_buffer[RLCD_DISP_W/5] = {0};

void task_LCDSendFrame( void* pvParameters ){
    vTaskDelay( 10 / portTICK_PERIOD_MS );
    rlcd_sendFrame();

	vTaskDelete(NULL);
}

int rlcd_vprintf_func( const char *szFormat, va_list args ){
    //write evaluated format string into buffer
    int ret = vsnprintf(log_print_buffer, sizeof(log_print_buffer), szFormat, args);

    if(ret >= 0) {
        rlcd_putStr( 50, 50, log_print_buffer, COLOUR_WHITE, COLOUR_BLACK, 1 );
        xTaskCreate( task_LCDSendFrame, "RLCD_rlcd_display_task", 2048, NULL, 20, NULL );
    }

    
    //output is now in buffer. write to file.
    if (ret >= 0){
        if (!SPIFFS.exists(filePath)){
            File writeLog = SPIFFS.open(filePath, FILE_WRITE);
            if (!writeLog){
                Serial.print("Couldn't open ");
                Serial.println(filePath);
            }
            delay(50);
            writeLog.close();
        }

        File spiffsLogFile = SPIFFS.open(filePath, FILE_APPEND);
        //debug output
        printf("[Writing to SPIFFS] %.*s", ret, log_print_buffer);
        spiffsLogFile.write((uint8_t *)log_print_buffer, (size_t)ret);
        //to be safe in case of crashes: flush the output
        spiffsLogFile.flush();
        spiffsLogFile.close();
    }
    
    return ret;
}

*/