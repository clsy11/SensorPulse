#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


struct ap3216c_data {
    uint16_t ir;
    uint16_t als;
    uint16_t ps;
};

int main(int argc, char *argv[]) {
    int fd;
    struct ap3216c_data data;
    const char *filename = "/dev/ap3216c";

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Can't open file %s\n", filename);
        return -1;
    }

    printf("--- AP3216C Driver Test Start ---\n");
    while (1) {
        if (read(fd, &data, sizeof(data)) == sizeof(data)) {
            printf("IR: %d, ALS: %d, PS: %d\n", data.ir, data.als, data.ps);
        }
        usleep(500000); // 0.5秒刷新一次
    }

    close(fd);
    return 0;
}