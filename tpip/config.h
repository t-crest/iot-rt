/*
  Configuration of the tpIP stack.

  Using macros instead of function pointers to enable WCET analysis.
  Currently we have only SLIP as link layer.
*/

// The link layer used
#define LINKLAYER slip

// Helper macros
#define CONCAT_HELPER(a,b,c) a ## b ## c
#define CONCAT(a,b,c) CONCAT_HELPER(a, b, c)


// macros for the interface
#define LL_RUN CONCAT(tpip_, LINKLAYER, _run)
#define LL_RXFULL CONCAT(tpip_, LINKLAYER, _rxfull)
#define LL_RXREAD(x) CONCAT(tpip_, LINKLAYER, _rxread)(x)
#define LL_TXEMPTY  CONCAT(tpip_, LINKLAYER, _txempty)
#define LL_TXWRITE(x, y) CONCAT(tpip_, LINKLAYER, _txwrite)(x, y)

// declare all functions here
void LL_RUN();
int LL_RXFULL();
int LL_RXREAD(unsigned char buf[]);
int LL_TXEMPTY();
void LL_TXWRITE(unsigned char buf[], int len);




