#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    
    // Đặt UID và EUID thành root
    setuid(1000);
    seteuid(0);
    
    // Gọi bash với quyền root
    system("/bin/bash");
    
    return 0;
}
