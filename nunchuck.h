#ifndef NUNCHUCK_H_INCLUDED
#define NUNCHUCK_H_INCLUDED
void nunchuck_init(int sda,int scl);
uint8_t nunchuck_read(void);
void nunchuck_disable(byte n);
#endif
