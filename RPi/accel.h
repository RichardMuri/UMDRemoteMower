#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>

#define BUS 1 // Default i2c unix bus is 1
#define ADXL345_I2C_ADDR 0x53

int writeToDevice(int fd, int reg, int val)
{
   int s;
   char buf[2];

   buf[0]=reg; buf[1]=val;

   s = write(fd, buf, 2);

   if (s == -1)
   {
      perror("writeToDevice");
   }
   else if (s != 2)
   {
      fprintf(stderr, "short write to device\n");
   }
   return 0;
}

int selectDevice(int fd, int addr, char *name)
{
   int s;
   char str[128];

    s = ioctl(fd, I2C_SLAVE, addr);

    if (s == -1)
    {
       sprintf(str, "selectDevice for %s", name);
       perror(str);
    }

    return s;
}

int initAccelerometer()
{
   char buf[50];
   sprintf(buf, "/dev/i2c-%d", BUS);
   int fd; // file descriptor for accelerometer
   
   if ((fd = open(buf, O_RDWR)) < 0)
   {
      // Open port for reading and writing

      printf("Failed to open i2c bus /dev/i2c-%d\n", BUS);

      exit(1);
   }
   //printf("fd is %d\n", fd);
   /* initialise ADXL345 */

   selectDevice(fd, ADXL345_I2C_ADDR, "ADXL345");

   writeToDevice(fd, 0x2d, 0);  /* POWER_CTL reset */
   writeToDevice(fd, 0x2d, 8);  /* POWER_CTL measure */
   writeToDevice(fd, 0x31, 0);  /* DATA_FORMAT reset */
   writeToDevice(fd, 0x31, 11); /* DATA_FORMAT full res +/- 16g */
   
   return fd;
}

void readAccelerometer(int fd, char *buf, float *xa, float *ya, float *za)
{
	short x, y, z;
	buf[0] = 0x32;
	//printf("Writing to accelerometer\n");
    if ((write(fd, buf, 1)) != 1)
    {
     // Send the register to read from
      printf("Error writing to ADXL345\n");
    }
    
	//printf("Reading...\n");
    if (read(fd, buf, 6) != 6)
    {
      //  X, Y, Z accelerations
      printf("Error reading from ADXL345\n");
    }
    else
    {
		//printf("Calculating angles\n");
		// Raw readings
         x = buf[1]<<8| buf[0];
         y = buf[3]<<8| buf[2];
         z = buf[5]<<8| buf[4];
		// Angles of x, y, and z
         *xa = (90.0 / 256.0) * (float) x;
         *ya = (90.0 / 256.0) * (float) y;
         *za = (90.0 / 256.0) * (float) z;
    }
	printf("Finished reading accelerometer\n");
}