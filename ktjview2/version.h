// KTJIDE Version
#define VERSION "0.5"
// Comment following line out on Release!
#define CVS_BUILD __DATE__

#ifdef CVS_BUILD
#define KTJIDE_VERSION VERSION " (" CVS_BUILD ")"
#else
#define KTJIDE_VERSION VERSION
#endif
