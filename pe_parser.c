/* ============================================================
 * pe_parser.c  --  PE 解析器 v4
 * ============================================================
 *  用法 1:  pe_parser.exe 某.exe                    打印节表 + 导入表
 *  用法 2:  pe_parser.exe 某.exe --rva 0xA011       换算 RVA
 *
 * 编译:  gcc -O0 -m64 -o pe_parser.exe pe_parser.c
 * ============================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char buf[4 * 1024 * 1024];   /* 4 MB，放全局（栈太小放不下） */
static size_t n = 0;                          /* 实际读了多少字节 */

/* ---------- 按小端序读整数 ---------- */
static unsigned int rd16(const unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}
static unsigned int rd32(const unsigned char *p)
{
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8)
         | ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}
static unsigned long long rd64(const unsigned char *p)
{
    return (unsigned long long)rd32(p) | ((unsigned long long)rd32(p + 4) << 32);
}

/* ============================================================
 * RVA → 文件偏移（昨天学的公式）
 * ============================================================ */
static int rva_to_off(const unsigned char *b, unsigned int secOff,
                      unsigned int numSec, unsigned int rva,
                      unsigned int *outOff)
{
    unsigned int i;
    for (i = 0; i < numSec; ++i) {
        const unsigned char *p = b + secOff + i * 40;
        unsigned int vsize = rd32(p + 8);
        unsigned int vaddr = rd32(p + 12);
        unsigned int raw   = rd32(p + 20);
        if (rva >= vaddr && rva < vaddr + vsize) {
            *outOff = raw + (rva - vaddr);
            return 1;
        }
    }
    return 0;
}

/* 读以 \0 结尾的字符串，最多读 200 个字符 */
static void read_cstr(unsigned int off, char *out, int maxlen)
{
    int i = 0;
    while (i < maxlen - 1 && off + i < n && buf[off + i] != 0) {
        out[i] = (char)buf[off + i];
        i++;
    }
    out[i] = '\0';
}

int main(int argc, char **argv)
{
    FILE *fp;
    unsigned int e_lfanew, numSec, optSize, secOff;
    unsigned int optStart, magic, ddOff, impRva, impOff;
    unsigned int i, k;
    int is64;

    if (argc < 2) {
        printf("用法:\n");
        printf("  pe_parser.exe <某个.exe>               打印节表 + 导入表\n");
        printf("  pe_parser.exe <某个.exe> --rva 0xA011  换算 RVA\n");
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) { printf("打不开文件: %s\n", argv[1]); return 1; }
    n = fread(buf, 1, sizeof(buf), fp);
    fclose(fp);

    printf("文件: %s\n", argv[1]);
    printf("读入 %u 字节\n\n", (unsigned int)n);

    if (n < 0x40 || buf[0] != 'M' || buf[1] != 'Z') { printf("X 不是 PE 文件\n"); return 1; }

    e_lfanew = rd32(buf + 0x3C);
    if (buf[e_lfanew] != 'P' || buf[e_lfanew+1] != 'E') { printf("X NT 头签名不对\n"); return 1; }

    numSec  = rd16(buf + e_lfanew + 6);
    optSize = rd16(buf + e_lfanew + 20);
    secOff  = e_lfanew + 4 + 20 + optSize;      /* 节表位置 */

    if (secOff + numSec * 40 > n) { printf("X 文件没读够\n"); return 1; }

    /* ============================================================
     * 分支 A：--rva 换算
     * ============================================================ */
    if (argc >= 4 && strcmp(argv[2], "--rva") == 0) {
        unsigned int rva = (unsigned int)strtoul(argv[3], NULL, 16);
        unsigned int off = 0;
        printf("目标 RVA = 0x%X\n\n", rva);
        if (!rva_to_off(buf, secOff, numSec, rva, &off)) {
            printf("X 这个 RVA 不在任何节里\n");
            return 0;
        }
        printf("=> 文件偏移 = 0x%X  (%u)\n", off, off);
        if (off + 4 <= n)
            printf("\n那一处的 4 个字节: %02X %02X %02X %02X\n",
                   buf[off], buf[off+1], buf[off+2], buf[off+3]);
        return 0;
    }

    /* ============================================================
     * 分支 B：打印节表 + 导入表
     * ============================================================ */

    /* ---------- 节表 ---------- */
    printf("========== 节表 ==========\n");
    printf("%-9s %-11s %-11s %-11s %-11s %s\n",
           "名字","VirtualSize","RVA","RawSize","文件偏移","属性");
    printf("--------  ----------- ----------- ----------- ----------- --------\n");
    for (i = 0; i < numSec; ++i) {
        const unsigned char *p = buf + secOff + i * 40;
        char nm[9]; int j;
        for (j = 0; j < 8; ++j) nm[j] = (char)p[j];
        nm[8] = '\0';
        printf("%-9s 0x%08X  0x%08X  0x%08X  0x%08X  0x%08X\n",
               nm, rd32(p+8), rd32(p+12), rd32(p+16), rd32(p+20), rd32(p+36));
    }
    printf("\n");

    /* ---------- 找导入表 ---------- */
    /* OptionalHeader 从 e_lfanew + 24 开始 */
    optStart = e_lfanew + 4 + 20;
    magic    = rd16(buf + optStart);          /* 0x20B = 64位, 0x10B = 32位 */
    is64     = (magic == 0x20B);

    /* DataDirectory 在 OptionalHeader 里的偏移：64位是 112，32位是 96 */
    ddOff = optStart + (is64 ? 112 : 96);

    /* DataDirectory[1] 就是导入表：8 字节 = RVA(4) + Size(4) */
    impRva = rd32(buf + ddOff + 1 * 8);

    printf("========== 导入表 ==========\n");
    printf("OptionalHeader 类型: %s\n", is64 ? "PE32+ (64位)" : "PE32 (32位)");
    printf("导入表 RVA = 0x%X\n\n", impRva);

    if (impRva == 0) { printf("（这个程序没有导入表）\n"); return 0; }

    if (!rva_to_off(buf, secOff, numSec, impRva, &impOff)) {
        printf("X 导入表 RVA 换算失败\n");
        return 0;
    }
    printf("导入表在文件偏移 0x%X\n\n", impOff);

    /* ---------- 遍历每个 DLL ---------- */
    /* 导入表是一个数组，每项 20 字节（IMAGE_IMPORT_DESCRIPTOR）
     *   +0x00  OriginalFirstThunk   → 函数名表（INT）的 RVA
     *   +0x0C  Name                 → DLL 名字的 RVA
     *   +0x10  FirstThunk           → 地址表（IAT）的 RVA
     * 数组最后用一项"全零"表示结束。 */
    for (k = 0; ; ++k) {
        const unsigned char *p = buf + impOff + k * 20;
        unsigned int oft     = rd32(p + 0);
        unsigned int nameRva = rd32(p + 12);
        unsigned int ft      = rd32(p + 16);
        unsigned int nameOff, thunk, idx;
        char dllname[256];

        if (oft == 0 && nameRva == 0 && ft == 0) break;   /* 全零 → 结束 */

        if (!rva_to_off(buf, secOff, numSec, nameRva, &nameOff)) continue;
        read_cstr(nameOff, dllname, sizeof(dllname));

        printf("--- %s ---\n", dllname);

        /* 用 OriginalFirstThunk 找函数名；如果它是 0，退回用 FirstThunk */
        thunk = (oft != 0) ? oft : ft;
        if (!rva_to_off(buf, secOff, numSec, thunk, &idx)) { printf("\n"); continue; }

        /* 遍历函数表：64 位每项 8 字节，32 位每项 4 字节，用 0 结尾 */
        for (i = 0; ; ++i) {
            unsigned long long v;
            unsigned int entOff, nmOff;
            char fname[256];

            if (is64) {
                if (idx + i * 8 + 8 > n) break;
                v = rd64(buf + idx + i * 8);
                if (v == 0) break;
                if ((v & 0x8000000000000000ULL) != 0) {
                    printf("    (按序号) #%u\n", (unsigned)(v & 0xFFFF));
                    continue;
                }
                entOff = (unsigned int)v;
            } else {
                unsigned int v32;
                if (idx + i * 4 + 4 > n) break;
                v32 = rd32(buf + idx + i * 4);
                if (v32 == 0) break;
                if ((v32 & 0x80000000U) != 0) {
                    printf("    (按序号) #%u\n", v32 & 0xFFFF);
                    continue;
                }
                entOff = v32;
            }

            /* entOff 指向 IMAGE_IMPORT_BY_NAME：2 字节 hint + 名字 */
            if (!rva_to_off(buf, secOff, numSec, entOff, &nmOff)) continue;
            read_cstr(nmOff + 2, fname, sizeof(fname));
            printf("    %s\n", fname);
        }
        printf("\n");
    }

    return 0;
}