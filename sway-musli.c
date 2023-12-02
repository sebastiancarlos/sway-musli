#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

#define IW_INTERFACE "wlan0"
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

// adapted from https://stackoverflow.com/a/19733653/21567639
void extract_wifi_ssid(char *buffer, size_t buffer_size) {
    static int sockfd = -1;
    char * id;
    id = (char *)malloc(sizeof(char)*IW_ESSID_MAX_SIZE+1);

    struct iwreq wreq;
    memset(&wreq, 0, sizeof(struct iwreq));
    wreq.u.essid.length = IW_ESSID_MAX_SIZE+1;

    sprintf(wreq.ifr_name, IW_INTERFACE);

    // initialize socket if it hasn't been initialized yet
    if (sockfd == -1) {
      if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
          exit(1);
      }
    }

    wreq.u.essid.pointer = id;
    if (ioctl(sockfd,SIOCGIWESSID, &wreq) == -1) {
        fprintf(stderr, "Get ESSID ioctl failed \n");
        exit(2);
    }

    strncpy(buffer, (char *)wreq.u.essid.pointer, buffer_size);
    buffer[buffer_size - 1] = 0;

    // if buffer is empty, set "Not Connected" instead.
    if (strlen(buffer) == 0) {
        strncpy(buffer, "Not Connected", buffer_size);
        buffer[buffer_size - 1] = 0;
    }

    free(id);
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

void print() {
    char wifi[MAX_BUF];
    char keyboard[MAX_BUF];
    char batcap[MAX_BUF];
    char batstat[MAX_BUF];
    
    extract_wifi_ssid(wifi, sizeof(wifi));
    extract_keyboard_layout(keyboard, sizeof(keyboard));
    read_file("/sys/class/power_supply/BAT0/capacity", batcap, sizeof(batcap));
    read_file("/sys/class/power_supply/BAT0/status", batstat, sizeof(batstat));

    printf("%s | %s | %s%% - %s | %s\n", wifi, keyboard, batcap, batstat, get_time_string());
    fflush(stdout);
}

void usage() {
    printf("Usage: sway-musli [-1|--once]\n");
    printf(" - Print a stream of status lines to be used with swaybar.\n");
    printf(" - If passed -1 or --once, print once and exit.\n");
    printf(" - Example sway config:\n");
    printf("    ...\n");
    printf("    bar {\n");
    printf("        status_command sway-musli\n");
    printf("    }\n");
    printf("    ...\n");
    printf("    # Note: Make sure sway-musli is in your PATH.\n");
}

int main(int argc, char *argv[]) {
  // print usage on -h or --help
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
    usage();
    return 0;
  }

  // if passed argument -1 or --once, print once and exit
  if (argc == 2 && (strcmp(argv[1], "-1") == 0 || strcmp(argv[1], "--once") == 0)) {
    print();
    return 0;
  }

  // on any other argument, print usage and exit
  if (argc != 1) {
    usage();
    return 1;
  }


  while (1) {
    print();
    sleep(1);
  }
}
