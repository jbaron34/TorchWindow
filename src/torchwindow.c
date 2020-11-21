#include "twcommon.h"
#include "twvulkan.h"

int main(int argc, char *argv[]){

    const char* name = argv[argc-1];

    printf("begin torchwindow\n");
    //getchar();

    TwWindow window = twCreateWindow(name);

    twRunWindow(window);

    twDestroyWindow(window);

    return 0;
}