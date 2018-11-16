#include"indirect.h"
#include<errno.h>

VOID indirectInit(CHAR *pDev)
{
  //An array is initialized to hold a single block group based on the given block size
  gBlocksize = BLOCK_SIZE;
  gpBlockArray = (UINT1*)calloc(BLOCK_SIZE * BLOCK_GROUP, sizeof(UINT1));

  //specified device is opened to be read
  gpPartition = fopen(pDev, "r");

  if(gpPartition == NULL)
  {
    //if the device cannot be opened the used is alerted and the program exits
    printf("Cannot open specified device: %s\n", pDev);
    return;
  }

  //otherwise the program proceeds, starting with finding the totla number of bytes in the partition given by the primary superblock
  indirectUtilGetTotalBlocks();
  //The pointer is returned to the beginning
  fseek(gpPartition, 0, SEEK_SET);
  //The program proceeds to read through each block in the partition
  indirectUtilReadBlock();
}
