/*
*  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L.
*  http://www.libelium.com
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Version 0.1
*  Author: Anartz Nuin Jiménez
*/


#include "arduPi.h"

struct bcm2835_peripheral {
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

struct ThreadArg{
    void (*func)();
    int pin;
    int mode;
};

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc0 = {BSC0_BASE};

void *spi0 = MAP_FAILED;
static  uint8_t *spi0Mem = NULL;


pthread_t idThread2;
pthread_t idThread3;
pthread_t idThread4;
pthread_t idThread5;
pthread_t idThread6;
pthread_t idThread7;
pthread_t idThread8;
pthread_t idThread9;


/*********************************
 *                               *
 * SerialPi Class implementation *
 * ----------------------------- *
 *********************************/

/******************
 * Public methods *
 ******************/

//Constructor
SerialPi::SerialPi(){
    serialPort="/dev/ttyAMA0";
    timeOut = 1000;
}

//Sets the data rate in bits per second (baud) for serial data transmission
void SerialPi::begin(int serialSpeed){

	switch(serialSpeed){
		case     50:	speed =     B50 ; break ;
		case     75:	speed =     B75 ; break ;
		case    110:	speed =    B110 ; break ;
		case    134:	speed =    B134 ; break ;
		case    150:	speed =    B150 ; break ;
		case    200:	speed =    B200 ; break ;
		case    300:	speed =    B300 ; break ;
		case    600:	speed =    B600 ; break ;
		case   1200:	speed =   B1200 ; break ;
		case   1800:	speed =   B1800 ; break ;
		case   2400:	speed =   B2400 ; break ;
		case   9600:	speed =   B9600 ; break ;
		case  19200:	speed =  B19200 ; break ;
		case  38400:	speed =  B38400 ; break ;
		case  57600:	speed =  B57600 ; break ;
		case 115200:	speed = B115200 ; break ;
		default:	speed = B230400 ; break ;
			
	}


	if ((sd = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1){
		fprintf(stderr,"Unable to open the serial port %s - \n", serialPort);
		exit(-1);
	}
    
	fcntl (sd, F_SETFL, O_RDWR) ;
    
	tcgetattr(sd, &options);
	cfmakeraw(&options);
	cfsetispeed (&options, speed);
	cfsetospeed (&options, speed);

	options.c_cflag |= (CLOCAL | CREAD) ;
	options.c_cflag &= ~PARENB ;
	options.c_cflag &= ~CSTOPB ;
	options.c_cflag &= ~CSIZE ;
	options.c_cflag |= CS8 ;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
	options.c_oflag &= ~OPOST ;

	options.c_cc [VMIN] = 0;
	options.c_cc [VTIME] = 100;

	tcsetattr (sd, TCSANOW, &options);

	ioctl (sd, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl (sd, TIOCMSET, &status);
	
	unistd::usleep (10000);
    
}

//Prints data to the serial port as human-readable ASCII text.
void SerialPi::print(const char *message){
    unistd::write(sd,message,strlen(message));
}

//Prints data to the serial port as human-readable ASCII text.
void SerialPi::print (char message){
	unistd::write(sd,&message,1);
}

/*Prints data to the serial port as human-readable ASCII text.
 * It can print the message in many format representations such as:
 * Binary, Octal, Decimal, Hexadecimal and as a BYTE. */
void SerialPi::print(unsigned char i,Representation rep){
    char * message;
    switch(rep){

        case BIN:
            message = int2bin(i);
            unistd::write(sd,message,strlen(message));
            break;
        case OCT:
            message = int2oct(i);
            unistd::write(sd,message,strlen(message));
            break;
        case DEC:
            sprintf(message,"%d",i);
            unistd::write(sd,message,strlen(message));
            break;
        case HEX:
            message = int2hex(i);
            unistd::write(sd,message,strlen(message));
            break;
        case BYTE:
            unistd::write(sd,&i,1);
            break;

    }
}

/* Prints data to the serial port as human-readable ASCII text.
 * precission is used to limit the number of decimals.
 */
void SerialPi::print(float f, int precission){
    const char *str1="%.";
    char * str2;
    char * str3;
    char * message;
    sprintf(str2,"%df",precission);
    asprintf(&str3,"%s%s",str1,str2);
    sprintf(message,str3,f);
    unistd::write(sd,message,strlen(message));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(const char *message){
	const char *newline="\r\n";
	char * msg = NULL;
	asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(char message){
	const char *newline="\r\n";
	char * msg = NULL;
	asprintf(&msg,"%s%s",&message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(int i, Representation rep){
    char * message;
    switch(rep){

        case BIN:
            message = int2bin(i);
            break;
        case OCT:
            message = int2oct(i);
            break;
        case DEC:
            sprintf(message,"%d",i);
            break;
        case HEX:
            message = int2hex(i);
            break;

    }

    const char *newline="\r\n";
    char * msg = NULL;
    asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Prints data to the serial port as human-readable ASCII text followed
 * by a carriage retrun character '\r' and a newline character '\n' */
void SerialPi::println(float f, int precission){
    const char *str1="%.";
    char * str2;
    char * str3;
    char * message;
    sprintf(str2,"%df",precission);
    asprintf(&str3,"%s%s",str1,str2);
    sprintf(message,str3,f);

    const char *newline="\r\n";
    char * msg = NULL;
    asprintf(&msg,"%s%s",message,newline);
    unistd::write(sd,msg,strlen(msg));
}

/* Writes binary data to the serial port. This data is sent as a byte 
 * Returns: number of bytes written */
int SerialPi::write(unsigned char message){
	unistd::write(sd,&message,1);
	return 1;
}

/* Writes binary data to the serial port. This data is sent as a series
 * of bytes
 * Returns: number of bytes written */
int SerialPi::write(const char *message){
	int len = strlen(message);
	unistd::write(sd,&message,len);
	return len;
}

/* Writes binary data to the serial port. This data is sent as a series
 * of bytes placed in an buffer. It needs the length of the buffer
 * Returns: number of bytes written */
int SerialPi::write(char *message, int size){
	unistd::write(sd,message,size);
	return size;
}

/* Get the numberof bytes (characters) available for reading from 
 * the serial port.
 * Return: number of bytes avalable to read */
int SerialPi::available(){
    int nbytes = 0;
    if (ioctl(sd, FIONREAD, &nbytes) < 0)  {
		fprintf(stderr, "Failed to get byte count on serial.\n");
        exit(-1);
    }
    return nbytes;
}

/* Reads 1 byte of incoming serial data
 * Returns: first byte of incoming serial data available */
char SerialPi::read() {
	unistd::read(sd,&c,1);
    return c;
}

/* Reads characters from th serial port into a buffer. The function 
 * terminates if the determined length has been read, or it times out
 * Returns: number of bytes readed */
int SerialPi::readBytes(char message[], int size){
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
		int count;
		for (count=0;count<size;count++){
			if(available()) unistd::read(sd,&message[count],1);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
			timespec t = timeDiff(time1,time2);
			if((t.tv_nsec/1000)>timeOut) break;
		}
		return count;
}

/* Reads characters from the serial buffer into an array. 
 * The function terminates if the terminator character is detected,
 * the determined length has been read, or it times out.
 * Returns: number of characters read into the buffer. */
int SerialPi::readBytesUntil(char character,char buffer[],int length){
    char lastReaded = character +1; //Just to make lastReaded != character
    int count=0;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    while(count != length && lastReaded != character){
        if(available()) unistd::read(sd,&buffer[count],1);
        lastReaded = buffer[count];
        count ++;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        timespec t = timeDiff(time1,time2);
        if((t.tv_nsec/1000)>timeOut) break;
    }

    return count;
}


bool SerialPi::find(const char *target){
    findUntil(target,NULL);
}

/* Reads data from the serial buffer until a target string of given length
 * or terminator string is found.
 * Returns: true if the target string is found, false if it times out */
bool SerialPi::findUntil(const char *target, const char *terminal){
    int index = 0;
    int termIndex = 0;
    int targetLen = strlen(target);
    int termLen = strlen(terminal);
    char readed;
    timespec t;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

    if( *target == 0)
        return true;   // return true if target is a null string

    do{
        if(available()){
            unistd::read(sd,&readed,1);
            if(readed != target[index])
            index = 0; // reset index if any char does not match

            if( readed == target[index]){
                if(++index >= targetLen){ // return true if all chars in the target match
                    return true;
                }
            }

            if(termLen > 0 && c == terminal[termIndex]){
                if(++termIndex >= termLen) return false; // return false if terminate string found before target string
            }else{ 
                termIndex = 0;
            }
        }

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        t = timeDiff(time1,time2);

    }while((t.tv_nsec/1000)<=timeOut);

    return false;
}

/* returns the first valid (long) integer value from the current position.
 * initial characters that are not digits (or the minus sign) are skipped
 * function is terminated by the first character that is not a digit. */
long SerialPi::parseInt(){
    bool isNegative = false;
    long value = 0;
    char c;

    //Skip characters until a number or - sign found
    do{
        c = peek();
        if (c == '-') break;
        if (c >= '0' && c <= '9') break;
        unistd::read(sd,&c,1);  // discard non-numeric
    }while(1);

    do{
        if(c == '-')
            isNegative = true;
        else if(c >= '0' && c <= '9')// is c a digit?
            value = value * 10 + c - '0';
        unistd::read(sd,&c,1);  // consume the character we got with peek
        c = peek();

    }while(c >= '0' && c <= '9');

    if(isNegative)
        value = -value;
    return value;
}

float SerialPi::parseFloat(){
    boolean isNegative = false;
    boolean isFraction = false;
    long value = 0;
    char c;
    float fraction = 1.0;

    //Skip characters until a number or - sign found
    do{
        c = peek();
        if (c == '-') break;
        if (c >= '0' && c <= '9') break;
        unistd::read(sd,&c,1);  // discard non-numeric
    }while(1);

    do{
        if(c == '-')
            isNegative = true;
        else if (c == '.')
            isFraction = true;
        else if(c >= '0' && c <= '9')  {      // is c a digit?
            value = value * 10 + c - '0';
            if(isFraction)
                fraction *= 0.1;
        }
        unistd::read(sd,&c,1);  // consume the character we got with peek
        c = peek();
    }while( (c >= '0' && c <= '9')  || (c == '.' && isFraction==false));

    if(isNegative)
        value = -value;
    if(isFraction)
        return value * fraction;
    else
        return value;


}

// Returns the next byte (character) of incoming serial data without removing it from the internal serial buffer.
char SerialPi::peek(){
    //We obtain a pointer to FILE structure from the file descriptor sd
    FILE * f = fdopen(sd,"r+");
    //With a pointer to FILE we can do getc and ungetc
    c = getc(f);
    ungetc(c, f);
    return c;
}

// Remove any data remaining on the serial buffer
void SerialPi::flush(){
    while(available()){
        unistd::read(sd,&c,1);
    }
}

/* Sets the maximum milliseconds to wait for serial data when using SerialPi::readBytes()
 * The default value is set to 1000 */
void SerialPi::setTimeout(long millis){
	timeOut = millis;
}

//Disables serial communication
void SerialPi::end(){
	unistd::close(sd);
}

/*******************
 * Private methods *
 *******************/

//Returns a timespec struct with the time elapsed between start and end timespecs
timespec SerialPi::timeDiff(timespec start, timespec end){
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

//Returns a binary representation of the integer passed as argument
char * SerialPi::int2bin(int i){
	size_t bits = sizeof(int) * CHAR_BIT;
    char * str = (char *)malloc(bits + 1);
    int firstCeros = 0;
    int size = bits;
    if(!str) return NULL;
    str[bits] = 0;

    // type punning because signed shift is implementation-defined
    unsigned u = *(unsigned *)&i;
    for(; bits--; u >>= 1)
        str[bits] = u & 1 ? '1' : '0';

    //Delete first 0's
    for (int i=0; i<bits; i++){
        if(str[i] == '0'){
            firstCeros++;
        }else{
            break;
        }
    }
    char * str_noceros = (char *)malloc(size-firstCeros+1);
    for (int i=0; i<(size-firstCeros);i++){
        str_noceros[i]=str[firstCeros+i];
    }

    return str_noceros;
}

//Returns an hexadecimal representation of the integer passed as argument
char * SerialPi::int2hex(int i){
	char buffer[32];
    sprintf(buffer,"%x",i);
    char * hex = (char *)malloc(strlen(buffer)+1);
    strcpy(hex,buffer);
    return hex;
}

//Returns an octal representation of the integer passed as argument
char * SerialPi::int2oct(int i){
    char buffer[32];
    sprintf(buffer,"%o",i);
    char * oct = (char *)malloc(strlen(buffer)+1);
    strcpy(oct,buffer);
    return oct;	
}






/*******************************
 *                             *
 * WirePi Class implementation *
 * --------------------------- *
 *******************************/

/******************
 * Public methods *
 ******************/

//Constructor
WirePi::WirePi(){
	if(map_peripheral(&gpio) == -1) {
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
	}
}

//Initiate the Wire library and join the I2C bus.
void WirePi::begin(){

	if(map_peripheral(&bsc0) == -1) {
		printf("Failed to map the physical BSC0 (I2C) registers into the virtual memory space.\n");
	}

	/* BSC0 is on GPIO 0 & 1 */
	*gpio.addr &= ~0x3f; // Mask out bits 0-5 of FSEL0 (i.e. force to zero)
	*gpio.addr |= 0x24;  // Set bits 0-5 of FSEL0 to binary '100100'
}

//Begin a transmission to the I2C slave device with the given address
void WirePi::beginTransmission(unsigned char address){
	BSC0_A = address;
}

//Writes data to the I2C.
void WirePi::write(unsigned char data){
	BSC0_DLEN = 1;

	BSC0_FIFO = data;

	BSC0_S = CLEAR_STATUS;
	BSC0_C = START_WRITE; 
	wait_i2c_done(); 
	
}

//Writes data to the I2C.
void WirePi::write(const char *data){
	//We can write data in 16bytes fragments or less
	int length = strlen(data);
	int times;
	int residual;

	if(length <= 16){
		BSC0_DLEN = length;
		for(int i=0;i<length;i++){
			BSC0_FIFO = data[i];
		}
		BSC0_S = CLEAR_STATUS;
		BSC0_C = START_WRITE;  
		wait_i2c_done();
	}else{
		times = length/16;
		residual = length%16;
		for(int i=0;i<times;i++){
			BSC0_DLEN = 16;
			for(int j=0;j<16;j++){
				BSC0_FIFO = data[i*16+j];
			}
			BSC0_S = CLEAR_STATUS;
			BSC0_C = START_WRITE;  
			wait_i2c_done();
		}
		for(int i=0;i<residual;i++){
            BSC0_DLEN = residual;
			BSC0_FIFO = data[times*16+i];
		}
		BSC0_S = CLEAR_STATUS;
		BSC0_C = START_WRITE;  
		

	}
}

//Writes data to the I2C.
void WirePi::write(unsigned char*data,int length){
	//We can write data in 16bytes fragments or less
	int times;
	int residual;
	if(length <= 16){
		BSC0_DLEN = length;
		for(int i=0;i<length;i++){
			BSC0_FIFO = data[i];
		}
		BSC0_S = CLEAR_STATUS;
		BSC0_C = START_WRITE;  
		
	}else{
		times = length/16;
		residual = length%16;
		for(int i=0;i<times;i++){
			BSC0_DLEN = 16;
			for(int j=0;j<16;j++){
				BSC0_FIFO = data[i*16+j];
			}
			BSC0_S = CLEAR_STATUS;
			BSC0_C = START_WRITE;  
			wait_i2c_done();
		}
		for(int i=0;i<residual;i++){
            BSC0_DLEN = residual;
			BSC0_FIFO = data[times*16+i];
		}
		BSC0_S = CLEAR_STATUS;
		BSC0_C = START_WRITE;  
		

	}
}


void WirePi::endTransmission(){
	wait_i2c_done();
}

//Used by the master to request bytes from a slave device
void WirePi::requestFrom(unsigned char address,int quantity){

	BSC0_A = address;

	BSC0_DLEN = quantity;

	BSC0_S = CLEAR_STATUS;

	BSC0_C = START_READ;
	wait_i2c_done();

}

//Reads a byte that was transmitted from a slave device to a master after a call to WirePi::requestFrom()
unsigned char WirePi::read(){
	return BSC0_FIFO;
}


/*******************
 * Private methods *
 *******************/

// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int WirePi::map_peripheral(struct bcm2835_peripheral *p)
{
   // Open /dev/mem
   if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("Failed to open /dev/mem, try checking permissions.\n");
      return -1;
   }

   p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      p->mem_fd,  // File descriptor to physical memory virtual file '/dev/mem'
      p->addr_p      // Address in physical map that we want this memory block to expose
   );

   if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
   }

   p->addr = (volatile unsigned int *)p->map;

   return 0;
}

void WirePi::unmap_peripheral(struct bcm2835_peripheral *p) {

    munmap(p->map, BLOCK_SIZE);
    unistd::close(p->mem_fd);
}

void WirePi::wait_i2c_done() {
        //Wait till done, let's use a timeout just in case
        int timeout = 50;
        while((!((BSC0_S) & BSC_S_DONE)) && --timeout) {
            unistd::usleep(1000);
        }
        if(timeout == 0)
            printf("wait_i2c_done() timeout. Something went wrong.\n");
}





/*******************************
 *                             *
 * SPIPi Class implementation *
 * --------------------------- *
 *******************************/

/******************
 * Public methods *
 ******************/

 SPIPi::SPIPi(){

    uint8_t *mapaddr;

    if ((spi0Mem = (uint8_t*)malloc(BLOCK_SIZE + (PAGESIZE - 1))) == NULL){
        fprintf(stderr, "bcm2835_init: spi0Mem malloc failed: %s\n", strerror(errno));
        exit(1);
    }
    
    mapaddr = spi0Mem;
    if (((uint32_t)mapaddr % PAGESIZE) != 0)
        mapaddr += PAGESIZE - ((uint32_t)mapaddr % PAGESIZE) ;
    
    spi0 = (uint32_t *)mmap(mapaddr, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, gpio.mem_fd, BCM2835_SPI0_BASE) ;
    
    if ((int32_t)spi0 < 0){
        fprintf(stderr, "bcm2835_init: mmap failed (spi0): %s\n", strerror(errno)) ;
        exit(1);
    }
 }

void SPIPi::begin(){
    // Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
    bcm2835_gpio_fsel(7, BCM2835_GPIO_FSEL_ALT0); // CE1
    bcm2835_gpio_fsel(8, BCM2835_GPIO_FSEL_ALT0); // CE0
    bcm2835_gpio_fsel(9, BCM2835_GPIO_FSEL_ALT0); // MISO
    bcm2835_gpio_fsel(10, BCM2835_GPIO_FSEL_ALT0); // MOSI
    bcm2835_gpio_fsel(11, BCM2835_GPIO_FSEL_ALT0); // CLK
    
    // Set the SPI CS register to the some sensible defaults
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    bcm2835_peri_write(paddr, 0); // All 0s
    
    // Clear TX and RX fifos
    bcm2835_peri_write_nb(paddr, BCM2835_SPI0_CS_CLEAR);
}

void SPIPi::end(){  
    // Set all the SPI0 pins back to input
    bcm2835_gpio_fsel(7, BCM2835_GPIO_FSEL_INPT); // CE1
    bcm2835_gpio_fsel(8, BCM2835_GPIO_FSEL_INPT); // CE0
    bcm2835_gpio_fsel(9, BCM2835_GPIO_FSEL_INPT); // MISO
    bcm2835_gpio_fsel(10, BCM2835_GPIO_FSEL_INPT); // MOSI
    bcm2835_gpio_fsel(11, BCM2835_GPIO_FSEL_INPT); // CLK
}

void SPIPi::setBitOrder(uint8_t order){
    // BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one suported by SPI0
}

// defaults to 0, which means a divider of 65536.
// The divisor must be a power of 2. Odd numbers
// rounded down. The maximum SPI clock rate is
// of the APB clock
void SPIPi::setClockDivider(uint16_t divider){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CLK/4;
    bcm2835_peri_write(paddr, divider);
}

void SPIPi::setDataMode(uint8_t mode){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    // Mask in the CPO and CPHA bits of CS
    bcm2835_peri_set_bits(paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

// Writes (and reads) a single byte to SPI
uint8_t SPIPi::transfer(uint8_t value){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = (volatile uint32_t*)spi0 + BCM2835_SPI0_FIFO/4;

    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
    delayMicroseconds(10);

    bcm2835_peri_write_nb(fifo, value);

    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
    delayMicroseconds(10);

    uint32_t ret = bcm2835_peri_read_nb(fifo);

    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);

    return ret;
}

// Writes (and reads) a number of bytes to SPI
void SPIPi::transfernb(char* tbuf, char* rbuf, uint32_t len){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = (volatile uint32_t*)spi0 + BCM2835_SPI0_FIFO/4;

    // This is Polled transfer as per section 10.6.1
    // BUG ALERT: what happens if we get interupted in this section, and someone else
    // accesses a different peripheral? 

    // Clear TX and RX fifos
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
    bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    uint32_t i;
    for (i = 0; i < len; i++)
    {
    // Maybe wait for TXD
    while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
        delayMicroseconds(10);

    // Write to FIFO, no barrier
    bcm2835_peri_write_nb(fifo, tbuf[i]);

    // Wait for RXD
    while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_RXD))
        delayMicroseconds(10);

    // then read the data byte
    rbuf[i] = bcm2835_peri_read_nb(fifo);
    }
    // Wait for DONE to be set
    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
    delayMicroseconds(10);

    // Set TA = 0, and also set the barrier
    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

void SPIPi::chipSelect(uint8_t cs){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    // Mask in the CS bits of CS
    bcm2835_peri_set_bits(paddr, cs, BCM2835_SPI0_CS_CS);
}

void SPIPi::setChipSelectPolarity(uint8_t cs, uint8_t active){
    volatile uint32_t* paddr = (volatile uint32_t*)spi0 + BCM2835_SPI0_CS/4;
    uint8_t shift = 21 + cs;
    // Mask in the appropriate CSPOLn bit
    bcm2835_peri_set_bits(paddr, active << shift, 1 << shift);
}




/********** FUNCTIONS OUTSIDE CLASSES **********/

// Sleep the specified milliseconds
void delay(long millis){
	unistd::usleep(millis*1000);
}

void delayMicroseconds(long micros){
	unistd::usleep(micros);
}

uint8_t shiftIn(uint8_t dPin, uint8_t cPin, bcm2835SPIBitOrder order){
	uint8_t value = 0 ;
	int8_t  i ;

	if (order == MSBFIRST)
		for (i = 7 ; i >= 0 ; --i){
			digitalWrite (cPin, HIGH);
			value |= digitalRead (dPin) << i;
			digitalWrite (cPin, LOW);
		}
	else
		for (i = 0 ; i < 8 ; ++i){
		  digitalWrite (cPin, HIGH);
		  value |= digitalRead (dPin) << i;
		  digitalWrite (cPin, LOW);
		}

	return value;
}

void shiftOut(uint8_t dPin, uint8_t cPin, bcm2835SPIBitOrder order, uint8_t val){
	int8_t i;

	if (order == MSBFIRST)
		for (i = 7 ; i >= 0 ; --i){	
			digitalWrite (dPin, val & (1 << i)) ;
			digitalWrite (cPin, HIGH) ;
			digitalWrite (cPin, LOW) ;
		}
	else
		for (i = 0 ; i < 8 ; ++i){
			digitalWrite (dPin, val & (1 << i)) ;
			digitalWrite (cPin, HIGH) ;
			digitalWrite (cPin, LOW) ;
		}
}

// Configures the specified pin to behave either as an input or an output
void pinMode(int pin, Pinmode mode){
	pin = raspberryPinNumber(pin);
	if(mode == OUTPUT){
		switch(pin){
			case 4:  GPFSEL0 &= ~(7 << 12); GPFSEL0 |= (1 << 12); break;
			case 17: GPFSEL1 &= ~(7 << 21); GPFSEL1 |= (1 << 21); break;
			case 18: GPFSEL1 &= ~(7 << 24); GPFSEL1 |= (1 << 24); break;
			case 21: GPFSEL2 &= ~(7 << 3);  GPFSEL2 |= (1 << 3);  break;
			case 22: GPFSEL2 &= ~(7 << 6);  GPFSEL2 |= (1 << 6);  break;
			case 23: GPFSEL2 &= ~(7 << 9);  GPFSEL2 |= (1 << 9);  break;
			case 24: GPFSEL2 &= ~(7 << 12); GPFSEL2 |= (1 << 12); break;
			case 25: GPFSEL2 &= ~(7 << 15); GPFSEL2 |= (1 << 15); break;
		}

	}else if (mode == INPUT){
		switch(pin){
			case 4:  GPFSEL0 &= ~(7 << 12); break;
            		case 17: GPFSEL1 &= ~(7 << 21); break;
			case 18: GPFSEL1 &= ~(7 << 24); break;
			case 21: GPFSEL2 &= ~(7 << 3);  break;
			case 22: GPFSEL2 &= ~(7 << 6);  break;
			case 23: GPFSEL2 &= ~(7 << 9);  break;
			case 24: GPFSEL2 &= ~(7 << 12); break;
			case 25: GPFSEL2 &= ~(7 << 15); break;
		}
	}
}

// Write a HIGH or a LOW value to a digital pin
void digitalWrite(int pin, int value){
	pin = raspberryPinNumber(pin);
	if (value == HIGH){
		switch(pin){
			case  4:GPSET0 =  BIT_4;break;
			case 17:GPSET0 = BIT_17;break;
			case 18:GPSET0 = BIT_18;break;
			case 21:GPSET0 = BIT_21;break;
			case 22:GPSET0 = BIT_22;break;
			case 23:GPSET0 = BIT_23;break;
			case 24:GPSET0 = BIT_24;break;
			case 25:GPSET0 = BIT_25;break;
		}
	}else if(value == LOW){
		switch(pin){
			case  4:GPCLR0 =  BIT_4;break;
			case 17:GPCLR0 = BIT_17;break;
			case 18:GPCLR0 = BIT_18;break;
			case 21:GPCLR0 = BIT_21;break;
			case 22:GPCLR0 = BIT_22;break;
			case 23:GPCLR0 = BIT_23;break;
			case 24:GPCLR0 = BIT_24;break;
			case 25:GPCLR0 = BIT_25;break;
		}
	}
    	delayMicroseconds(1);    // Delay to allow any change in state to be reflected in the LEVn, register bit.
}



// Reads the value from a specified digital pin, either HIGH or LOW.
int digitalRead(int pin){
	Digivalue value;
	pin = raspberryPinNumber(pin);
	switch(pin){
		case 4: if(GPLEV0 & BIT_4){value = HIGH;} else{value = LOW;};break;
		case 17:if(GPLEV0 & BIT_17){value = HIGH;}else{value = LOW;};break;
		case 18:if(GPLEV0 & BIT_18){value = HIGH;}else{value = LOW;};break;
		case 21:if(GPLEV0 & BIT_21){value = HIGH;}else{value = LOW;};break;
		case 22:if(GPLEV0 & BIT_22){value = HIGH;}else{value = LOW;};break;
		case 23:if(GPLEV0 & BIT_23){value = HIGH;}else{value = LOW;};break;
		case 24:if(GPLEV0 & BIT_24){value = HIGH;}else{value = LOW;};break;
		case 25:if(GPLEV0 & BIT_25){value = HIGH;}else{value = LOW;};break;
	}
	return value;
}

void attachInterrupt(int p,void (*f)(), Digivalue m){
	pthread_t *threadId = getThreadIdFromPin(p);
	struct ThreadArg *threadArgs = (ThreadArg *)malloc(sizeof(ThreadArg));
	threadArgs->func = f;
	threadArgs->pin = p;
	threadArgs->mode = m;
	
	//Clear the event detection flag
	bcm2835_gpio_set_eds(p);
	
	if(*threadId == 0){
		//Create a thread passing the pin, function and mode
		pthread_create (threadId, NULL, threadFunction, (void *)threadArgs);
	}else{
		//First cancel the existing thread for that pin
		pthread_cancel(*threadId);
		//Create a thread passing the pin, function and mode
		pthread_create (threadId, NULL, threadFunction, (void *)threadArgs);
	}
}

void detachInterrupt(int p){
	pthread_t *threadId = getThreadIdFromPin(p);
	pthread_cancel(*threadId);
}

/* Some helper functions */

int raspberryPinNumber(int arduinoPin){
	switch(arduinoPin){
		case 2: return 18; break;
		case 3: return 23; break;
		case 4: return 24; break;
		case 5: return 25; break;
		case 6: return  4; break;
		case 7: return 17; break;
		case 8: return 21; break;
		case 9: return 22; break;
	}
}

// safe read from peripheral
uint32_t bcm2835_peri_read(volatile uint32_t* paddr){
    uint32_t ret = *paddr;
    ret = *paddr;
    return ret;

}

// read from peripheral without the read barrier
uint32_t bcm2835_peri_read_nb(volatile uint32_t* paddr){
    return *paddr;
}

// safe write to peripheral
void bcm2835_peri_write(volatile uint32_t* paddr, uint32_t value){
    *paddr = value;
    *paddr = value;
}

// write to peripheral without the write barrier
void bcm2835_peri_write_nb(volatile uint32_t* paddr, uint32_t value){
    *paddr = value;
}

// Set/clear only the bits in value covered by the mask
void bcm2835_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask){
    uint32_t v = bcm2835_peri_read(paddr);
    v = (v & ~mask) | (value & mask);
    bcm2835_peri_write(paddr, v);
}

//
// Low level convenience functions
//

// Function select
// pin is a BCM2835 GPIO pin number NOT RPi pin number
//      There are 6 control registers, each control the functions of a block
//      of 10 pins.
//      Each control register has 10 sets of 3 bits per GPIO pin:
//
//      000 = GPIO Pin X is an input
//      001 = GPIO Pin X is an output
//      100 = GPIO Pin X takes alternate function 0
//      101 = GPIO Pin X takes alternate function 1
//      110 = GPIO Pin X takes alternate function 2
//      111 = GPIO Pin X takes alternate function 3
//      011 = GPIO Pin X takes alternate function 4
//      010 = GPIO Pin X takes alternate function 5
//
// So the 3 bits for port X are:
//      X / 10 + ((X % 10) * 3)
void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode){
    // Function selects are 10 pins per 32 bit word, 3 bits per pin
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPFSEL0/4 + (pin/10);
    uint8_t   shift = (pin % 10) * 3;
    uint32_t  mask = BCM2835_GPIO_FSEL_MASK << shift;
    uint32_t  value = mode << shift;
    bcm2835_peri_set_bits(paddr, value, mask);
}

// See if an event detection bit is set
uint8_t bcm2835_gpio_eds(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPEDS0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = bcm2835_peri_read(paddr);
    return (value & (1 << shift)) ? HIGH : LOW;
}

// Write a 1 to clear the bit in EDS
void bcm2835_gpio_set_eds(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPEDS0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_write(paddr, value);
}

// Rising edge detect enable
void bcm2835_gpio_ren(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPREN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, value, value);
}

// Rising edge detect disable
void bcm2835_gpio_clr_ren(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPREN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, 0, value);
}

// Falling edge detect enable
void bcm2835_gpio_fen(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPFEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, value, value);
}

// Falling edge detect disable
void bcm2835_gpio_clr_fen(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPFEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, 0, value);
}

// High detect enable
void bcm2835_gpio_hen(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPHEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, value, value);
}
// High detect disable
void bcm2835_gpio_clr_hen(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPHEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, 0, value);
}

// Low detect enable
void bcm2835_gpio_len(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPLEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, value, value);
}

// Low detect disable
void bcm2835_gpio_clr_len(uint8_t pin){
	pin = raspberryPinNumber(pin);
    volatile uint32_t* paddr = (volatile uint32_t*)gpio.map + BCM2835_GPLEN0/4 + pin/32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    bcm2835_peri_set_bits(paddr, 0, value);
}

pthread_t *getThreadIdFromPin(int pin){
	switch(pin){
		case 2: return &idThread2; break;
		case 3: return &idThread3; break;
		case 4: return &idThread4; break;
		case 5: return &idThread5; break;
		case 6: return &idThread6; break;
		case 7: return &idThread7; break;
		case 8: return &idThread8; break;
		case 9: return &idThread9; break;
	}
}

/* This is the function that will be running in a thread if
 * attachInterrupt() is called */
void * threadFunction(void *args){
	ThreadArg *arguments = (ThreadArg *)args;
	int pin = arguments->pin;
	int mode = arguments->mode;

	switch(mode){
			case RISING: bcm2835_gpio_ren(pin);break;
			case FALLING: bcm2835_gpio_fen(pin);break;
			case HIGH: bcm2835_gpio_hen(pin);break;
			case LOW: bcm2835_gpio_len(pin);break;
	}
	
	while(1){
		if(bcm2835_gpio_eds(pin)){
			//Clear flag
			bcm2835_gpio_set_eds(pin);
			//Call to the user function
			arguments->func();
		}
		delay(1);
	}
}
