#ifndef HAVE_ATMELOPS_H_
#define HAVE_ATMELOPS_H_

int atmel_eraseAll(void);
int atmel_programPage(unsigned int,unsigned char[]);
int atmel_readPage(unsigned int,unsigned char[]);
void atmel_endSPIFlash(void);
int atmel_getBytesInPage();
int atmel_startWrite(void);
int atmel_endWrite(void);

int atmel_eraseOne(int i);
#endif /* HAVE_ATMELOPS_H_ */
