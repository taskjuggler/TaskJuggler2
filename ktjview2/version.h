// KTJIDE Version
#define MY_VERSION "0.6"
// Comment following line out on Release!
#define CVS_BUILD __DATE__

#ifdef CVS_BUILD
#define KTJIDE_VERSION MY_VERSION " (" CVS_BUILD ")"
#else
#define KTJIDE_VERSION MY_VERSION
#endif
