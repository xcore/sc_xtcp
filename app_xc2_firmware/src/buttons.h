#ifdef __XC__
void button_monitor(chanend client, chanend startup, port keyA, port keyB);
#endif

unsigned int getButtonCount(chanend, int);

#define getButtonCountA(c) getButtonCount(c, 0)
#define getButtonCountB(c) getButtonCount(c, 1)

int get_button_startup(chanend startup);
