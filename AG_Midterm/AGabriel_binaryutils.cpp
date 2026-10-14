/* Name: Annabel Gabriel
   Date: 9/1/2026
   Description: Function Descriptions
*/
#include <cstdint>
void setbit(uint32_t* addr, uint8_t whichbit){ // should set only the bit at position whichbit to 1
   *addr = *addr | ((uint32_t)1 << whichbit);
}

void clearbit(uint32_t* addr, uint8_t whichbit){ // should set only the bit at position whichbit to 0
    *addr = *addr & ~((uint32_t)1 << whichbit);
}

void setbits(uint32_t* addr, uint32_t bitmask){ //should only set the bits defined in the mask
    *addr = *addr | bitmask;

}

void clearbits(uint32_t* addr, uint32_t bitmask){//should only clear the bits defined in the mask

    *addr = *addr & ~bitmask;

}

char* display_binary(uint32_t num, char *p){
    uint32_t temp;
    int i;
    uint32_t temp_bitmask;

    for(i=31; i>=0; i--){
      //assign to temp int
      temp_bitmask = 0b1;
      temp = num;
      //bit shift so number you want is in the one's place
      temp = temp >> i;
      //set bitmask
      //bit mask everything else except number of interest
      temp = (temp & temp_bitmask)+48;
      p[31-i]=temp;

    }
    p[32] = '\0'; //null terminator for the string

    return p;

}