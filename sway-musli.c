#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BUF 100
#define OUTPUT_BUF_SIZE 100 

// status-line.c
// - Print a line containing the WIFI SSID, keyboard layout, battery capacity,
//   battery status, and time.
// - Made for Sway. 
// - Depends on swaymsg (for keyboard layout) and iwctl (SSID).
// - Example:
//    MyWiFi | Qwerty | 80% - Charging | Sun 2021-08-22 12:00:00

void read_file(const char *filename, char *buffer, size_t buffer_size) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        exit(1);
    }
    fgets(buffer, buffer_size - 1, fp);
    buffer[strcspn(buffer, "\n")] = 0; // remove newline
    fclose(fp);
}

char* get_time_string() {
    static char time_str[25]; // "aaa yyyy-mm-dd hh:mm:ss"
    time_t rawtime;
    struct tm *timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    strftime(time_str, sizeof(time_str), "%a %Y-%m-%d %H:%M:%S", timeinfo);
    return time_str;
}

void extract_wifi_ssid(char *buffer, size_t buffer_size) {
    const char *command = "sudo iwctl station wlan0 show";
    FILE *fp = popen(command, "r");
    char line[MAX_BUF];
    
    if (fp == NULL) {
        perror("popen");
        exit(1);
    }
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "Connected network")) {
           // extract SSID, which is the first non-whitespace string after "Connected network"
           char *start = strchr(line, 'k');
           start++; // skip 'k'
           // skip leading whitespace
           while (*start == ' ') {
               start++;
           }
           char *end = strchr(start, ' ');
           size_t len = end - start;
           // print to buffer and exit
           if (len < buffer_size) {
               strncpy(buffer, start, len);
               buffer[len] = 0; // Null-terminate buffer.
               pclose(fp);
               return;
           }
        }
    }
    
    // Set default string if SSID is not found.
    strncpy(buffer, "No WiFi", buffer_size);
    buffer[buffer_size - 1] = 0;
    pclose(fp);
}

// A simple function to extract the keyboard layout from the swaymsg command output.
void extract_keyboard_layout(char *buffer, size_t buffer_size) {
    const char *command = "swaymsg -t get_inputs";
    FILE *fp = popen(command, "r");
    char line[MAX_BUF];
    int found = 0;
    
    if (fp == NULL) {
        perror("popen");
        exit(1);
    }
    
    while (fgets(line, sizeof(line), fp) && !found) {
        char *start = strstr(line, "xkb_active_layout_name");
        if (start) {
            char *start_quote = strchr(start, '(');
            char *end_quote = strrchr(start, ')');
            if (start_quote && end_quote && start_quote < end_quote) {
                size_t len = end_quote - start_quote - 1;
                if (len < buffer_size) {
                    strncpy(buffer, start_quote + 1, len);
                    buffer[len] = 0; // Null-terminate buffer.
                                     
                    // if value is "US", change to "Qwerty"
                    if (strcmp(buffer, "US") == 0) {
                        strncpy(buffer, "Qwerty", buffer_size);
                        buffer[buffer_size - 1] = 0;
                    }

                    found = 1;
                }
            }
        }
    }
    
    if (!found) {
        strncpy(buffer, "Unknown", buffer_size);
        buffer[buffer_size - 1] = 0;
    }
    pclose(fp);
}

int main() {
    char wifi[MAX_BUF];
    char keyboard[MAX_BUF];
    char batcap[MAX_BUF];
    char batstat[MAX_BUF];
    
    extract_wifi_ssid(wifi, sizeof(wifi));
    extract_keyboard_layout(keyboard, sizeof(keyboard));
    read_file("/sys/class/power_supply/BAT0/capacity", batcap, sizeof(batcap));
    read_file("/sys/class/power_supply/BAT0/status", batstat, sizeof(batstat));

    // Print output
    printf("%s | %s | %s%% - %s | %s\n", wifi, keyboard, batcap, batstat, get_time_string());
    return 0;
}
