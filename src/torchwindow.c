#include "twcommon.h"
#include "twvulkan.h"

int main(int argc, char *argv[]){

    const char* name = argv[argc-1];

    TwWindow window = twCreateWindow(name);

    twRunWindow(window);

    twDestroyWindow(window);
}