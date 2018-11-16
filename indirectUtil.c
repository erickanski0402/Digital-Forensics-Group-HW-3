#include"indirect.h"

VOID indirectUtilReadBlock()
{
  UINT2 currentGroup;
  for(currentGroup = 0; currentGroup < (gNumBlocks / BLOCK_GROUP); currentGroup++)
  {
    //reads a full block group at a time for quick access
    fread(gpBlockArray, BLOCK_SIZE * BLOCK_GROUP, 1, gpPartition);
    //sends the array containing the block group to be checked for indirects
    indirectUtilCheckForAddr(currentGroup);
    //once here it is assumed no blocks remain to be checked and the user is updated
    printf("No other indirect blocks found in group %d\n", currentGroup);
  }
}

INT1 indirectUtilCheckForAddr(UINT4 currentGroup)
{
  UINT4 currentBlock, currentByte, *block;
  UINT2 blockIterator;

  for(currentBlock = 0; currentBlock < BLOCK_GROUP; currentBlock++)
  {
    //current byte within the block group is kept up to date
    currentByte = BLOCK_SIZE * currentBlock;
    //an array containing a single block is allocated and zeroed out
    block = calloc(BLOCK_SIZE / BYTES_IN_ADDR, sizeof(UINT4));

    //iterates through every 4 bytes of the block
    for(blockIterator = 0; blockIterator < (BLOCK_SIZE / BYTES_IN_ADDR); blockIterator ++)
    {
      //converts the values from little endian
      block[blockIterator] = (gpBlockArray[(BYTES_IN_ADDR * blockIterator) + 3 + currentByte] << 24) |
                             (gpBlockArray[(BYTES_IN_ADDR * blockIterator) + 2 + currentByte] << 16) |
                             (gpBlockArray[(BYTES_IN_ADDR * blockIterator) + 1 + currentByte] << 8) |
                             (gpBlockArray[(BYTES_IN_ADDR * blockIterator) + currentByte] << 0);
    }

    //sends the single block to have the values  checked to determine if it may be an indirect or not
    if(indirectUtilCheckIfIndirect(block) > 0)
    {
      //if the function returns valid, it can be assumed an indirect block is found, the address of the block is printed
      printf("Indirect block found at block number: %d\n", (BLOCK_GROUP * BLOCK_SIZE * currentGroup + currentByte) / 4096);
      //followed by the first and last (non-zero) value in the array
      indirectUtilPrintFirstLast(block);
    }
  }
  return 0;
}

INT1 indirectUtilCheckIfIndirect(UINT4 *block)
{
  UINT2 blockIterator;
  UINT4 prevVal;

  if(block[0] <= 0 || block[0] > gNumBlocks)
  {
    //if the first block is an invalid value an invalid value is returned
    return -1;
  }
  else
  {
    //otherwise the previous value is instantiated to be the first value
    prevVal = block[0];
  }

  for(blockIterator = 1; blockIterator < (BLOCK_SIZE / BYTES_IN_ADDR); blockIterator++)
  {
    if(block[blockIterator] > gNumBlocks)
    {
      //if any given value is greater than the number of blocks in the partition it is assumed invalid
      return -1;
    }

    if(block[blockIterator] != 0 && prevVal == 0)
    {
      //if there is a zero between values it can be assumed that this is not an indirect
      return -1;
    }

    if(block[blockIterator] <= prevVal && block[blockIterator] != 0){
      //if the curent value is less than the previous and is non-zero it can be assumed that this is not an indirect
      //except for some odd exceptions of non-contiguous addresses
      if(block[blockIterator] >= block[blockIterator + 1] && block[blockIterator + 1] >= block[blockIterator + 2])
      {
        return -1;
      }
      else
      {
        prevVal = block[blockIterator];
      }
    }
    else
    {
      //otherwise previous value is reset to the current value
      prevVal = block[blockIterator];
    }

    if(block[blockIterator] == 0)
    {
      //if a zero is found then we must check to make sure there are no outlyers
      prevVal = 0;
    }
  }

  return 1;
}

VOID indirectUtilPrintFirstLast(UINT4 *block)
{
  UINT4 blockIterator = 0;

  //Prints the first value in the block, the first address
  printf("    First block address: %d\n", block[blockIterator]);

  while(block[blockIterator + 1] != 0)
  {
    //iterates through until it finds the last (non-zero) value
    blockIterator++;
  }

  if(blockIterator >= (BLOCK_SIZE / BYTES_IN_ADDR))
  {
    //redundancy to avoid out of bounds exceptions
    blockIterator = (BLOCK_SIZE / BYTES_IN_ADDR) - 1;
  }

  //prints the last value in the array, the last block address
  printf("    Last block address:  %d\n\n", block[blockIterator]);
}

VOID indirectUtilGetTotalBlocks()
{
  //Seeks the position in the primary superblock where the value of the total blocks is stored
  fseek(gpPartition, 1028, SEEK_SET);
  //Reads the 4 byte value
  fread(&gNumBlocks, 1, 4, gpPartition);
}
