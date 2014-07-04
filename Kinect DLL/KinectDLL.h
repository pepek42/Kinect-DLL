#ifdef KINECTDLL_EXPORTS
#define KINECTDLL_API __declspec(dllexport) 
#else
#define KINECTDLL_API __declspec(dllimport) 
#endif