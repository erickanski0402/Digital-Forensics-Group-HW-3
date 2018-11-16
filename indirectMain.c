#include "indirect.h"

VOID main(INT2 argc, CHAR** argv)
{
  if(argc < 2)
  {
    //if the user has not provided a device to read, they are alerted and the program exits
    printf("Please include desired device as an argument\n");
    return;
  }

  //otherwise the program proceeds to attempt to open the specified device
  CHAR *pDev = argv[1];
  indirectInit(pDev);
}
