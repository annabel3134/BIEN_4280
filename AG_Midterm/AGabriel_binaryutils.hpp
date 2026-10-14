/* Name: Annabel Gabriel
   Date: 9/1/2026
   Description: Header File - Function Declarations
   Full project demonstrates the use of binary utility functions to manipulate bits in a 32-bit unsigned integer. 
   The functions include setting and clearing individual bits, setting and clearing multiple bits using a bitmask, 
   and displaying the binary representation of the number in a string format. This is connected to a USB serial 
   interface for output, allowing the user to see the results of the bit manipulations in both binary and decimal formats.
*/

//function declarations for future use
#include <cstdint>
void setbit(uint32_t* addr, uint8_t whichbit); //sets the bit at the specified position to 1
void clearbit(uint32_t* addr, uint8_t whichbit); //clears the bit at the specified position to 0
void setbits(uint32_t* addr, uint32_t bitmask); //sets the bits specified in the bitmask to 1
void clearbits(uint32_t* addr, uint32_t bitmask); //clears the bits specified in the bitmask to 0
char* display_binary(uint32_t num, char *p); //displays the binary representation of the number in a string format