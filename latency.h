uint64_t nstore_64byte_fence(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /*
     * vmovntpd: 32-byte non-temporal store (check below)
     * https://software.intel.com/en-us/node/524246
     */
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        FLUSH_CACHE_LINE
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}
