

// Retrieves Serial Number from OTP
// - serialNum - destination of serialNum
// - returns 0 for success
#ifdef __XC__
int getSerialNum(unsigned &serialNum);
#else
int getSerialNum(unsigned *serialNum);
#endif

