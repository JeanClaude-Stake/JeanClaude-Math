#ifdef _WIN32

extern "C" void __cdecl __chkstk(void);

extern "C" void __cdecl ___chkstk_ms(void) {
    __chkstk();
}

#endif
