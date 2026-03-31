#include <stdio.h>
#include <windows.h>

int main() {
    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    printf("Pagesize:\n B: %lu\n KB: %lu\n MB: %lu\n\n", 
        info.dwPageSize,
        info.dwPageSize >> 10,
        info.dwPageSize >> 20);

    printf("Pages in MB = %lu\n\n", 1048576 / info.dwPageSize);

    return 0;
}