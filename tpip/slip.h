#ifndef SLIP_H
#define SLIP_H

int tpip_slip_getchar(unsigned char *cptr);
void tpip_slip_putchar(unsigned char c);
int tpip_slip_is_end(unsigned char c);
void tpip_slip_put_end();
int tpip_slip_is_esc(unsigned char c);
int tpip_slip_was_esc(unsigned char c);

int initserial();
int serialreceive(unsigned char *bufin, int max);
int serialsend(unsigned char *bufin, int cnt);

#endif //SLIP_H
