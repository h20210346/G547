#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>              
#include <unistd.h>            
#include <stdint.h>
#include <sys/ioctl.h>          
#include "adcdev.h"


uint16_t data;


int ioctl_sel_channel(int file_desc, int channel)
{
    int ret_val;
    ret_val = ioctl(file_desc, SEL_CHANNEL, channel);
    if (ret_val < 0) {
        printf("ioctl_sel_channel failed:%d\n", ret_val);
        exit(-1);
    }
    return 0;
}

int ioctl_sel_alignment(int file_desc, char align)
{
    int ret_val;
    ret_val = ioctl(file_desc, SEL_ALIGNMENT, align);
    if (ret_val < 0) {
        printf("ioctl_sel_alignment failed:%d\n", ret_val);
        exit(-1);
    }
    return 0;
}

int ioctl_sel_conv(int file_desc, int conv)
{
    int ret_val;
    ret_val = ioctl(file_desc, SEL_CONV, conv );
    if (ret_val < 0)
 {
        printf("IOCTL allignment failed:%d\n", ret_val);
        exit(-1);
    }
    return 0;
}


void decimal_to_binary(uint16_t n){
  int binaryNum[16];
  int i = 0;
  while (n > 0) {
      binaryNum[i] = n % 2;
      n = n / 2;
      i++;
  }
  printf("This number in binary is:");
  for (int j = i - 1; j >= 0; j--){
      printf("%d", binaryNum[j]);
     }
    printf("\n");
}


/*
 * Main - Call the ioctl functions
 */
int main()
{
    int file_desc, ret_val;
    int channel;
    char align;
    int conv;
    file_desc = open(DEVICE_FILE_NAME, 0);
    if (file_desc < 0) {
        printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
        exit(-1);
    }
    printf("Enter the Channel(0-7):");
    scanf("%d", &channel);
    printf("Enter the Alignment(r or l):");
    scanf(" %c", &align);
    if(channel <0 || channel>7 || (align != 'r' && align != 'l'))
{
      printf("Invalid Selection of Channel or Alignment\n");
      exit(-1);
    }

    printf("Enter type of covesrion( 0-1):");
    scanf("%d", &conv);

    ioctl_sel_channel(file_desc,channel);
    ioctl_sel_alignment(file_desc,align);
    ioctl_sel_conv(file_desc,conv);
    if(read(file_desc,&data,sizeof(data)))
{
      if(align == 'l'){
          printf("The alignment selected is left so the random number aligned to left side is:- %u --", data);
          decimal_to_binary(data);
        data = data/16;
          printf("The actual decimal number is: %u --", data);
          decimal_to_binary(data);
      }
      else{
        printf("DATA read by the user is:%u --", data);
        decimal_to_binary(data);

      }
 if (conv ==0)
{
printf("one shot execution");
}

else
{
 for(int i=0;i<100;i++)
 { printf("\nDATA read by the user is:%u --", data);
}
}

}
    close(file_desc);
    return 0;
}
