#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
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

void send_sway_command(int sockfd, uint32_t command_type, const char *command) {
    const char *magic = "i3-ipc"; // Sway IPC magic string
    uint32_t magic_length = strlen(magic);
    uint32_t command_size = strlen(command);
    size_t message_size = magic_length + sizeof(command_size) + sizeof(command_type) + command_size;

    // Allocate buffer for the entire message
    char *message = malloc(message_size);
    if (message == NULL) {
        perror("malloc failed");
        exit(1);
    }

    // Copy the magic string, command size, command type, and command into the buffer
    char *buffer_ptr = message;
    memcpy(buffer_ptr, magic, magic_length);
    buffer_ptr += magic_length;
    memcpy(buffer_ptr, &command_size, sizeof(command_size));
    buffer_ptr += sizeof(command_size);
    memcpy(buffer_ptr, &command_type, sizeof(command_type));
    buffer_ptr += sizeof(command_type);
    memcpy(buffer_ptr, command, command_size);

    if (write(sockfd, message, message_size) != message_size) {
        perror("write failed");
        exit(1);
    }

    // Free the allocated buffer
    free(message);
}

// A simple function to extract the keyboard layout from the swaymsg command output.
void extract_keyboard_layout(char *buffer, size_t buffer_size) {
    static int sockfd = -1;
    struct sockaddr_un addr;
    static const char *socket_path;
    const char *get_inputs_command = "get_inputs";
    char read_buffer[1024];
    int found = 0;

    // get socket path on first run
    if (socket_path == NULL) {
        socket_path = getenv("SWAYSOCK");
    }

    if (sockfd == -1) {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("socket");
            exit(1);
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            close(sockfd);
            exit(1);
        }
    }

    // Send "get_inputs" command to Sway
   const uint32_t get_inputs_command_type = 100; // IPC command type for "get_inputs"
   send_sway_command(sockfd, get_inputs_command_type, get_inputs_command);

    // Read Sway's response header
    size_t prefix_size = strlen("i3-ipc") + sizeof(uint32_t) + sizeof(uint32_t);
    ssize_t num_read = read(sockfd, read_buffer, prefix_size);
    if (num_read == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }

    // Read Sway's response
    num_read = read(sockfd, read_buffer, sizeof(read_buffer));
    if (num_read == -1) {
        perror("read");
        close(sockfd);
        exit(1);
    }
    
    char *start = strstr(read_buffer, "xkb_active_layout_name");
    if (start) {
        char *start_quote = strchr(start, '(');
        char *end_quote = strchr(start_quote, ')');
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

    // flush rest of response
    while (num_read == sizeof(read_buffer)) {
        num_read = read(sockfd, read_buffer, sizeof(read_buffer));
    }
    
    if (!found) {
        strncpy(buffer, "Unknown", buffer_size);
        buffer[buffer_size - 1] = 0;
    }
}

void print() {
    char wifi[MAX_BUF];
    char keyboard[MAX_BUF];
    char batcap[MAX_BUF];
    char batstat[MAX_BUF];
    static char last_message[MAX_BUF];
    char message[MAX_BUF];
    
    extract_wifi_ssid(wifi, sizeof(wifi));
    extract_keyboard_layout(keyboard, sizeof(keyboard));
    read_file("/sys/class/power_supply/BAT0/capacity", batcap, sizeof(batcap));
    read_file("/sys/class/power_supply/BAT0/status", batstat, sizeof(batstat));

    // only print if it changed since last time
    sprintf(message, "%s | %s | %s%% - %s | %s", wifi, keyboard, batcap, batstat, get_time_string());
    if (strcmp(message, last_message) == 0) {
        return;
    }
    strncpy(last_message, message, sizeof(last_message));
    last_message[sizeof(last_message) - 1] = 0;
    printf("%s\n", message);

    fflush(stdout);
}

void usage() {
    printf("Usage: sway-musli [-1|--once] [-f|--fps <FPS>]\n");
    printf(" - Print a stream of status lines to be used with swaybar.\n");
    printf(" - If passed -1 or --once, print once and exit.\n");
    printf(" - If passed -f or --fps, print at most <FPS> times per second.\n");
    printf("   - Default is 30.\n");
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

  // parse fps
  int fps = 30;
  if (argc == 3 && (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--fps") == 0)) {
    fps = atoi(argv[2]);
    argc = 1;
  }

  // on any other argument, print usage and exit
  if (argc != 1) {
    usage();
    return 1;
  }

  while (1) {
    print();
    // run at 1second/fps
    usleep(1000000 / fps);
  }
}
