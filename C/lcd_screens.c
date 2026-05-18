/* lcd_screens.c — shared display logic for the SSD1306 128x32 OLED.
 *
 * All high-level display functions that only call the low-level primitives
 * (OLED_Set_Pos, OLED_WR_Byte, OLED_Clear) live here so that changes to
 * display logic need to be made in exactly one place.
 *
 * Compiled into both the hardware binary (with ssd1306_i2c.c) and the
 * terminal simulator binary (with ssd1306_sim.c).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#include "ssd1306_i2c.h"
#include "oled_fonts.h"

#ifdef __linux__
#  include <sys/sysinfo.h>
#  include <sys/vfs.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/ioctl.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <arpa/inet.h>
#else
#  include <sys/param.h>
#  include <sys/mount.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/ioctl.h>
#  include <netinet/in.h>
#  include <net/if.h>
#  include <arpa/inet.h>
#endif

/* ------------------------------------------------------------------ */
/* Global state                                                         */
/* ------------------------------------------------------------------ */

char IPSource[20] = {0};

/* ------------------------------------------------------------------ */
/* Font / rendering primitives                                          */
/* ------------------------------------------------------------------ */

unsigned int oled_pow(unsigned char m, unsigned char n)
{
    unsigned int result = 1;
    while (n--) result *= m;
    return result;
}

void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char Char_Size)
{
    unsigned char c = 0, i = 0;
    c = chr - ' ';
    if (x > SSD1306_LCDWIDTH - 1) { x = 0; y = y + 2; }
    if (Char_Size == 16) {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++) OLED_WR_Byte(F8X16[c * 16 + i],     OLED_DATA);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++) OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
    } else {
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++) OLED_WR_Byte(F6x8[c][i], OLED_DATA);
    }
}

void OLED_ShowString(unsigned char x, unsigned char y, unsigned char *chr, unsigned char Char_Size)
{
    unsigned char j = 0;
    while (chr[j] != '\0') {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > 120) { x = 0; y += 2; }
        j++;
    }
}

void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num,
                  unsigned char len, unsigned char size2)
{
    unsigned char t, temp;
    unsigned char enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) { OLED_ShowChar(x + (size2/2)*t, y, ' ', size2); continue; }
            else enshow = 1;
        }
        OLED_ShowChar(x + (size2/2)*t, y, temp + '0', size2);
    }
}

void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,
                  unsigned char BMP[][512], unsigned char symbol)
{
    unsigned int j = 0;
    unsigned char x, y;
    if (y1 % 8 == 0) y = y1 / 8;
    else              y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++)
            OLED_WR_Byte(BMP[symbol][j++], OLED_DATA);
    }
}

void OLED_DrawPartBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,
                      unsigned char BMP[][512], unsigned char symbol)
{
    unsigned int j = x1 * y0;
    unsigned char x, y;
    if (y1 % 8 == 0) y = y1 / 8;
    else              y = y1 / 8 + 1;
    for (y = y0; y < y1; y++) {
        OLED_Set_Pos(x0, y);
        for (x = x0; x < x1; x++)
            OLED_WR_Byte(BMP[symbol][j++], OLED_DATA);
    }
}

/* ------------------------------------------------------------------ */
/* System info — real on Linux, stubs elsewhere                        */
/* ------------------------------------------------------------------ */

unsigned char Obaintemperature(void)
{
#ifdef __linux__
    FILE *fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fd) {
        unsigned int temp = 0;
        char buff[32] = {0};
        fgets(buff, sizeof(buff), fd);
        fclose(fd);
        sscanf(buff, "%u", &temp);
        return g_config.temp_fahrenheit
               ? (unsigned char)(temp / 1000 * 1.8 + 32)
               : (unsigned char)(temp / 1000);
    }
#endif
    return g_config.temp_fahrenheit ? 77 : 25;
}

char *GetIpAddress(void)
{
    static char addr[16] = "127.0.0.1";
    /* Use first interface from network_interfaces list */
    char iface[IFNAMSIZ] = {0};
    const char *p = g_config.network_interfaces;
    const char *end = p;
    while (*end && *end != ',') end++;
    size_t len = (size_t)(end - p);
    if (len >= IFNAMSIZ) len = IFNAMSIZ - 1;
    strncpy(iface, p, len);
    if (!iface[0]) return addr;

    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return addr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
        strncpy(addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                sizeof(addr) - 1);
    close(fd);
    return addr;
}

void FirstGetIpAddress(void)
{
    strcpy(IPSource, GetIpAddress());
}

/* ------------------------------------------------------------------ */
/* Internal layout helpers (static — internal to this translation unit) */
/* ------------------------------------------------------------------ */

static void draw_header(const char *fallback)
{
    char buf[17] = {0};
    if (g_config.top_line == TOP_LINE_HOSTNAME) {
        char hostname[64] = {0};
        gethostname(hostname, sizeof(hostname) - 1);
        strncpy(buf, hostname, 16);
    } else if (g_config.top_line == TOP_LINE_IP) {
        strcpy(IPSource, GetIpAddress());
        strncpy(buf, IPSource, 16);
    } else if (g_config.top_line == TOP_LINE_CUSTOM) {
        strncpy(buf, g_config.custom_text, 16);
    } else {
        strncpy(buf, fallback, 16);
    }
    /* Page 0: label text with rule baked into bit 7 (bottom of page).
     * Uppercase F6x8 glyphs never set bit 7, so ORing 0x80 adds a clean rule
     * with a natural 1px gap below it (font bit 0 = 0 for all labels used). */
    OLED_Set_Pos(0, 0);
    int col = 0;
    for (int j = 0; buf[j] && col + 6 <= 128; j++) {
        unsigned char idx = (unsigned char)buf[j] - ' ';
        for (int i = 0; i < 6; i++, col++)
            OLED_WR_Byte(F6x8[idx][i], OLED_DATA);
    }
    while (col < 128) { OLED_WR_Byte(0x00, OLED_DATA); col++; }
}

/* Render a string to the pixel page buffers for a given data line.
 * line_idx 0 = line1 (rows 12-18), 1 = line2 (rows 21-27). */
static void render_to_pages(unsigned char *p1, unsigned char *p2, unsigned char *p3,
                              int x, const char *s, int line_idx)
{
    int j, i;
    for (j = 0; s[j]; j++) {
        int base = x + j * 8;
        if (base >= 128) break;
        unsigned char c = (unsigned char)s[j] - ' ';
        for (i = 0; i < 6; i++) {
            int col = base + i;
            if (col < 0 || col >= 128) continue;
            unsigned char fb = F6x8[c][i];
            if (line_idx == 0) {
                p1[col] |= (fb & 0x0F) << 4;   /* font bits 0-3 -> page1 bits 4-7 */
                p2[col] |= (fb >> 4) & 0x07;   /* font bits 4-6 -> page2 bits 0-2 */
            } else {
                p2[col] |= (fb & 0x07) << 5;   /* font bits 0-2 -> page2 bits 5-7 */
                p3[col] |= (fb >> 3) & 0x0F;   /* font bits 3-6 -> page3 bits 0-3 */
            }
        }
    }
}

/* Parse a numeric value string: [-+]? [0-9]+ ('.' [0-9]+)? [A-Za-z%/]*
 * No spaces or secondary decimals allowed — excludes "2h 30m", "14:35:22", etc.
 * Returns 1 if numeric; sets int_digits, frac_digits, suffix_len, sign_len. */
static int parse_numeric_val(const char *val,
                              int *int_digits, int *frac_digits,
                              int *suffix_len, int *sign_len)
{
    const char *p = val;
    const char *suf_start;
    int id = 0, fd = 0;
    *sign_len = 0;
    if (!val || !val[0]) return 0;
    if (*p == '-' || *p == '+') { *sign_len = 1; p++; }
    while (isdigit((unsigned char)*p)) { id++; p++; }
    if (id == 0) return 0;
    if (*p == '.') {
        p++;
        while (isdigit((unsigned char)*p)) { fd++; p++; }
        if (fd == 0) return 0;
    }
    suf_start = p;
    while (*p && (isalpha((unsigned char)*p) || *p == '%' || *p == '/')) p++;
    if (*p != '\0') return 0;
    *int_digits  = id;
    *frac_digits = fd;
    *suffix_len  = (int)(p - suf_start);
    return 1;
}

/* Render two data lines with coordinated alignment.
 *
 * Numeric mode (both values parse as numbers):
 *   Decimal points aligned in the same column; value with most fractional
 *   digits (and suffix) is flush right; label colons one space left of the
 *   widest integer part.
 *
 * Text mode:
 *   Longest value flush right; shorter value left-aligned to same start col;
 *   label colons one space left of the data start column.
 *
 * Pixel layout (rows 0-31):
 *   0-7   page 0: header label (drawn by draw_header)
 *     7   blank  - natural font gap above rule
 *     8   blank  - 2nd gap above rule        (page 1 bit 0)
 *     9   RULE                               (page 1 bit 1)
 *  10-11  blank  - 2px gap below rule        (page 1 bits 2-3)
 *  12-18  line1  - full 7px font             (page 1 bits 4-7, page 2 bits 0-2)
 *  19-20  blank  - 2px gap between lines     (page 2 bits 3-4)
 *  21-27  line2  - full 7px font             (page 2 bits 5-7, page 3 bits 0-3)
 *  28-31  blank
 */
static void draw_data_lines(const char *line1, const char *line2)
{
    unsigned char p1[128], p2[128], p3[128];
    int i, k;
    for (i = 0; i < 128; i++) { p1[i] = 0x02; p2[i] = 0; p3[i] = 0; }

    const char *lines[2] = { line1 ? line1 : "", line2 ? line2 : "" };
    char lbuf[2][32];
    const char *labels[2] = {"", ""};
    const char *vals[2]   = {"", ""};
    memset(lbuf, 0, sizeof(lbuf));

    for (k = 0; k < 2; k++) {
        const char *s = lines[k];
        if (!s[0]) continue;
        const char *c = strchr(s, ':');
        if (c && c[1] == ' ') {
            int ll = (int)(c - s) + 1;
            if (ll > 31) ll = 31;
            strncpy(lbuf[k], s, ll);
            lbuf[k][ll] = '\0';
            labels[k] = lbuf[k];
            const char *v = c + 2;
            while (*v == ' ') v++;
            vals[k] = v;
        } else {
            vals[k] = s;
        }
    }

    int int_d[2]  = {0,0};
    int frac_d[2] = {0,0};
    int suf_d[2]  = {0,0};
    int sgn_d[2]  = {0,0};
    int is_num[2] = {0,0};
    for (k = 0; k < 2; k++)
        is_num[k] = vals[k][0] ?
            parse_numeric_val(vals[k], &int_d[k], &frac_d[k], &suf_d[k], &sgn_d[k]) : 0;

    /* Use decimal alignment only when both values are numeric and neither has a
     * unit suffix.  As soon as any value carries a unit (F, C, %, G, …) all
     * values are right-aligned independently so every last character lands at
     * the same column — effectively treating that column as "the unit column". */
    int any_suffix   = (vals[0][0] && is_num[0] && suf_d[0] > 0) ||
                       (vals[1][0] && is_num[1] && suf_d[1] > 0);
    int both_numeric = !any_suffix && vals[0][0] && vals[1][0] && is_num[0] && is_num[1];

    int val_x[2] = {0, 0};
    int lbl_x[2] = {0, 0};

    if (both_numeric) {
        /* cfD = chars from decimal column D to end of value string:
         *   has fraction: '.' + frac_digits + suffix_len
         *   no  fraction: suffix_len only (suffix starts at D) */
        int cfD[2], max_cfD, D, max_int, colon_x;
        for (k = 0; k < 2; k++)
            cfD[k] = suf_d[k] + (frac_d[k] > 0 ? 1 + frac_d[k] : 0);
        max_cfD  = cfD[0] > cfD[1] ? cfD[0] : cfD[1];
        D        = 130 - max_cfD * 8;
        max_int  = int_d[0] > int_d[1] ? int_d[0] : int_d[1];
        colon_x  = D - max_int * 8 - 16;

        for (k = 0; k < 2; k++) {
            val_x[k] = D - (sgn_d[k] + int_d[k]) * 8;
            if (val_x[k] < 0) val_x[k] = 0;
            if (labels[k][0]) {
                int llen = (int)strlen(labels[k]);
                lbl_x[k] = colon_x - (llen - 1) * 8;
                if (lbl_x[k] < 0) lbl_x[k] = 0;
            }
        }
    } else {
        /* Text mode: align all values at a shared "unit column".
         * Any value with a suffix (F, %, G …) places its first unit char at
         * unit_col.  Values without a suffix are shifted left by one slot so
         * their last digit is in unit_col-8 and the phantom unit slot is blank —
         * exactly as if they had a trailing space unit. */
        int max_suf_len = 0;
        for (k = 0; k < 2; k++)
            if (vals[k][0] && is_num[k] && suf_d[k] > max_suf_len)
                max_suf_len = suf_d[k];

        int unit_col = 130 - max_suf_len * 8;
        for (k = 0; k < 2; k++) {
            if (!vals[k][0]) { val_x[k] = 0; continue; }
            int pre = (int)strlen(vals[k]) - (is_num[k] ? suf_d[k] : 0);
            val_x[k] = unit_col - pre * 8;
            if (val_x[k] < 0) val_x[k] = 0;
        }

        /* Shared colon column: one gap left of the leftmost value start. */
        int min_val_x = 130;
        for (k = 0; k < 2; k++)
            if (vals[k][0] && val_x[k] < min_val_x) min_val_x = val_x[k];
        int colon_x_t = min_val_x - 16;
        for (k = 0; k < 2; k++) {
            if (labels[k][0]) {
                int llen = (int)strlen(labels[k]);
                lbl_x[k] = colon_x_t - (llen - 1) * 8;
                if (lbl_x[k] < 0) lbl_x[k] = 0;
            }
        }
    }

    for (k = 0; k < 2; k++) {
        if (!lines[k][0] || !vals[k][0]) continue;
        if (labels[k][0])
            render_to_pages(p1, p2, p3, lbl_x[k], labels[k], k);
        render_to_pages(p1, p2, p3, val_x[k], vals[k], k);
    }

    OLED_Set_Pos(0, 1);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p1[i], OLED_DATA);
    OLED_Set_Pos(0, 2);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p2[i], OLED_DATA);
    OLED_Set_Pos(0, 3);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p3[i], OLED_DATA);
}

static void draw_centered_lines(const char *line1, const char *line2)
{
    unsigned char p1[128], p2[128], p3[128];
    int i;
    for (i = 0; i < 128; i++) { p1[i] = 0x02; p2[i] = 0; p3[i] = 0; }

    if (line1 && line1[0]) {
        int x = (128 - (int)strlen(line1) * 8) / 2;
        if (x < 0) x = 0;
        render_to_pages(p1, p2, p3, x, line1, 0);
    }
    if (line2 && line2[0]) {
        int x = (128 - (int)strlen(line2) * 8) / 2;
        if (x < 0) x = 0;
        render_to_pages(p1, p2, p3, x, line2, 1);
    }

    OLED_Set_Pos(0, 1);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p1[i], OLED_DATA);
    OLED_Set_Pos(0, 2);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p2[i], OLED_DATA);
    OLED_Set_Pos(0, 3);
    for (i = 0; i < 128; i++) OLED_WR_Byte(p3[i], OLED_DATA);
}

static const char *throttle_str(unsigned int flags)
{
    static char buf[20];
    if ((flags & 0xF) == 0) return "OK";
    buf[0] = '\0';
    if (flags & 0x1) strcat(buf, "UV ");  /* under-voltage */
    if (flags & 0x2) strcat(buf, "FC ");  /* freq capped */
    if (flags & 0x4) strcat(buf, "TH ");  /* throttled */
    if (flags & 0x8) strcat(buf, "TL");   /* soft temp limit */
    int len = strlen(buf);
    if (len > 0 && buf[len - 1] == ' ') buf[len - 1] = '\0';
    return buf;
}

#ifdef __linux__
static void get_iface_ip(const char *iface, char *out, size_t outlen)
{
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { strncpy(out, "0.0.0.0", outlen - 1); return; }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
        strncpy(out, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), outlen - 1);
    else
        strncpy(out, "0.0.0.0", outlen - 1);
    close(fd);
}

/* Return the nth comma-delimited interface name from g_config.network_interfaces. */
static void get_nth_iface(int n, char *out, size_t outlen)
{
    const char *p = g_config.network_interfaces;
    int cur = 0;
    while (*p) {
        const char *start = p;
        while (*p && *p != ',') p++;
        if (cur == n) {
            size_t len = (size_t)(p - start);
            if (len >= outlen) len = outlen - 1;
            strncpy(out, start, len);
            out[len] = '\0';
            return;
        }
        if (*p == ',') p++;
        cur++;
    }
    out[0] = '\0';
}
#endif /* __linux__ */

/* ------------------------------------------------------------------ */
/* Screen display functions                                             */
/* ------------------------------------------------------------------ */

void LCD_DisplayTemperature(void)
{
    unsigned char buffer[80] = {0};
    unsigned int temp = Obaintemperature();
#ifdef __linux__
    FILE *fp = popen("top -bn1 | grep load | awk '{printf \"%.2f\", $(NF-2)}'", "r");
    if (fp) { fgets((char *)buffer, sizeof(buffer), fp); pclose(fp); }
    buffer[5] = '\0';
#else
    strncpy((char *)buffer, "0.15", sizeof(buffer) - 1);
#endif

    char tempbuf[16] = {0};
    char loadbuf[16] = {0};
    char unit = g_config.temp_fahrenheit ? 'F' : 'C';
    float load_val = atof((char *)buffer);
    snprintf(tempbuf, sizeof(tempbuf), "TEMP: %u%c", temp, unit);
    if (g_config.load_cores)
        snprintf(loadbuf, sizeof(loadbuf), "LOAD: %.2f", load_val);
    else
        snprintf(loadbuf, sizeof(loadbuf), "LOAD: %d%%", (int)(load_val * 100.0f + 0.5f));

    OLED_Clear();
    draw_header("CPU TEMP");
    draw_data_lines(tempbuf, loadbuf);
}

void LCD_DisPlayCpuMemory(void)
{
    float Totalram = 0.0f, freeram = 0.0f;
    unsigned int value = 0;
    unsigned char buffer[100] = {0}, famer[100] = {0};

    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp) {
        while (fgets((char *)buffer, sizeof(buffer), fp)) {
            if (sscanf((char *)buffer, "%s%u", famer, &value) != 2) continue;
            if      (strcmp((char *)famer, "MemTotal:") == 0) Totalram = value / 1024.0f / 1024.0f;
            else if (strcmp((char *)famer, "MemFree:")  == 0) freeram  = value / 1024.0f / 1024.0f;
        }
        fclose(fp);
    }

    char freebuf[16] = {0}, totalbuf[16] = {0};
    snprintf(freebuf,  sizeof(freebuf),  "M FREE: %.1fG", freeram);
    snprintf(totalbuf, sizeof(totalbuf), "TOTAL: %.1fG", Totalram);

    OLED_Clear();
    draw_header("MEMORY");
    draw_data_lines(freebuf, totalbuf);
}

void LCD_DisplaySdMemory(void)
{
    struct statfs diskInfo;
    statfs("/", &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;
    unsigned int total_gb = (unsigned int)((blocksize * diskInfo.f_blocks) >> 30);
    unsigned int free_gb  = (unsigned int)((blocksize * diskInfo.f_bfree)  >> 30);

    char freebuf[16] = {0}, totalbuf[16] = {0};
    snprintf(freebuf,  sizeof(freebuf),  "D FREE: %uG", free_gb);
    snprintf(totalbuf, sizeof(totalbuf), "TOTAL: %uG", total_gb);

    OLED_Clear();
    draw_header("DISK");
    draw_data_lines(freebuf, totalbuf);
}

void LCD_DisplayClock(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[16] = {0};
    char datebuf[20] = {0};
    static const char *wdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

    snprintf(datebuf, sizeof(datebuf), "%s %04d-%02d-%02d",
             wdays[t->tm_wday], t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d",
             t->tm_hour, t->tm_min, t->tm_sec);

    OLED_Clear();
    draw_header("CLOCK");
    draw_centered_lines(datebuf, timebuf);
}

void LCD_DisplayUptime(void)
{
    char uptimebuf[32] = {0}, procsbuf[12] = {0};
#ifdef __linux__
    struct sysinfo s_info;
    if (sysinfo(&s_info) != 0) return;
    long up   = s_info.uptime;
    int days  = (int)(up / 86400);
    int hours = (int)((up % 86400) / 3600);
    int mins  = (int)((up % 3600) / 60);
    if (days > 0)       snprintf(uptimebuf, sizeof(uptimebuf), "%dd %dh %dm", days, hours, mins);
    else if (hours > 0) snprintf(uptimebuf, sizeof(uptimebuf), "%dh %dm", hours, mins);
    else                snprintf(uptimebuf, sizeof(uptimebuf), "%dm", mins);
    snprintf(procsbuf, sizeof(procsbuf), "%d", (int)s_info.procs);
#else
    snprintf(uptimebuf, sizeof(uptimebuf), "N/A");
    snprintf(procsbuf,  sizeof(procsbuf),  "N/A");
#endif
    char procline[16] = {0};
    snprintf(procline, sizeof(procline), "PR: %s", procsbuf);

    OLED_Clear();
    draw_header("UPTIME");
    draw_data_lines(uptimebuf, procline);
}

void LCD_DisplayCpuFreq(void)
{
    unsigned int freq_khz = 0, throttle_val = 0;
    char freqline[24] = {0};
    FILE *fp;

#ifdef __linux__
    fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (fp) { fscanf(fp, "%u", &freq_khz); fclose(fp); }
#endif

    fp = popen("vcgencmd get_throttled 2>/dev/null", "r");
    if (fp) {
        char buf[32] = {0};
        if (fgets(buf, sizeof(buf), fp))
            sscanf(buf, "throttled=0x%x", &throttle_val);
        pclose(fp);
    }

    snprintf(freqline, sizeof(freqline), "frq: %u %s",
             freq_khz / 1000, throttle_str(throttle_val));

    OLED_Clear();
    draw_header("CPU FREQ");
    draw_data_lines(freqline, "");
}

void LCD_DisplayGpuTemp(void)
{
    unsigned char cpu_temp = Obaintemperature();
    char cpubuf[16] = {0}, gpubuf[16] = {0};
    char unit = g_config.temp_fahrenheit ? 'F' : 'C';
    FILE *fp;

    snprintf(cpubuf, sizeof(cpubuf), "CPU: %d%c", (int)cpu_temp, unit);
    strncpy(gpubuf, "GPU: N/A", sizeof(gpubuf) - 1);

    fp = popen("vcgencmd measure_temp 2>/dev/null", "r");
    if (fp) {
        char buf[32] = {0};
        float gpu_raw = 0.0f;
        if (fgets(buf, sizeof(buf), fp) && sscanf(buf, "temp=%f", &gpu_raw) == 1) {
            float gpu = g_config.temp_fahrenheit ? gpu_raw * 1.8f + 32.0f : gpu_raw;
            snprintf(gpubuf, sizeof(gpubuf), "GPU: %.1f%c", gpu, unit);
        }
        pclose(fp);
    }

    OLED_Clear();
    draw_header("TEMPS");
    draw_data_lines(cpubuf, gpubuf);
}

void LCD_DisplayNetwork(void)
{
    char iface_name[2][IFNAMSIZ] = {{0}, {0}};
    char rate_str[2][16]         = {"---", "---"};
    int nifaces = 0;

#ifdef __linux__
    static unsigned long long prev_rx[2]    = {0, 0};
    static char prev_iface[2][IFNAMSIZ]     = {{0}, {0}};
    static time_t prev_time                 = 0;

    unsigned long long cur_rx[2] = {0, 0};
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        while (fgets(line, sizeof(line), fp) && nifaces < 2) {
            char name[IFNAMSIZ] = {0};
            unsigned long long r;
            if (sscanf(line, " %15[^:]:%llu", name, &r) != 2) continue;
            if (strcmp(name, "lo") == 0) continue;
            char ip[16] = {0};
            get_iface_ip(name, ip, sizeof(ip));
            if (strcmp(ip, "0.0.0.0") == 0) continue;
            strncpy(iface_name[nifaces], name, IFNAMSIZ - 1);
            cur_rx[nifaces] = r;
            nifaces++;
        }
        fclose(fp);
    }

    time_t now = time(NULL);
    if (prev_time > 0 && now > prev_time) {
        double elapsed = difftime(now, prev_time);
        for (int k = 0; k < nifaces; k++) {
            if (strcmp(iface_name[k], prev_iface[k]) == 0 && cur_rx[k] >= prev_rx[k]) {
                double rx_rate = (double)(cur_rx[k] - prev_rx[k]) / elapsed;
                if      (rx_rate >= 1048576) snprintf(rate_str[k], sizeof(rate_str[k]), "%.1f MB/s", rx_rate / 1048576);
                else if (rx_rate >= 1024)    snprintf(rate_str[k], sizeof(rate_str[k]), "%.1f KB/s", rx_rate / 1024);
                else                         snprintf(rate_str[k], sizeof(rate_str[k]), "%.0f B/s",  rx_rate);
            }
        }
    }
    for (int k = 0; k < 2; k++) {
        strncpy(prev_iface[k], k < nifaces ? iface_name[k] : "", IFNAMSIZ - 1);
        prev_rx[k] = k < nifaces ? cur_rx[k] : 0;
    }
    prev_time = now;

#else
    strncpy(iface_name[0], "eth0",  IFNAMSIZ - 1);
    strncpy(iface_name[1], "wlan0", IFNAMSIZ - 1);
    strncpy(rate_str[0], "1.0 MB/s", sizeof(rate_str[0]) - 1);
    strncpy(rate_str[1], "500 KB/s", sizeof(rate_str[1]) - 1);
    nifaces = 2;
#endif

    if (nifaces == 0) { OLED_Clear(); return; }

    char line1[36] = {0}, line2[36] = {0};
    snprintf(line1, sizeof(line1), "%s: %s", iface_name[0], rate_str[0]);
    if (nifaces > 1)
        snprintf(line2, sizeof(line2), "%s: %s", iface_name[1], rate_str[1]);

    OLED_Clear();
    draw_header("NETWORK");
    draw_data_lines(line1, line2);
}

void LCD_DisplayWifi(void)
{
    char sigbuf[16] = "N/A", linkbuf[16] = "N/A";
#ifdef __linux__
    FILE *fp = fopen("/proc/net/wireless", "r");
    if (fp) {
        char line[256];
        fgets(line, sizeof(line), fp);
        fgets(line, sizeof(line), fp);
        while (fgets(line, sizeof(line), fp)) {
            char name[16] = {0};
            int link = 0, level = 0;
            if (sscanf(line, " %15[^:]: %*s %d. %d.", name, &link, &level) == 3
                    && strcmp(name, "wlan0") == 0) {
                snprintf(sigbuf,  sizeof(sigbuf),  "%d dBm",    level);
                snprintf(linkbuf, sizeof(linkbuf), "Lnk:%d/70", link);
                break;
            }
        }
        fclose(fp);
    }
#endif
    OLED_Clear();
    draw_header("WIFI");
    draw_data_lines(sigbuf, linkbuf);
}

void LCD_DisplayDocker(void)
{
    int running = -1, total = -1;
    char runbuf[16] = "RUN: N/A", stpbuf[16] = "STP: N/A";
    FILE *fp;

    fp = popen("docker ps -q 2>/dev/null | wc -l", "r");
    if (fp) { fscanf(fp, "%d", &running); pclose(fp); }
    fp = popen("docker ps -aq 2>/dev/null | wc -l", "r");
    if (fp) { fscanf(fp, "%d", &total);   pclose(fp); }

    if (running >= 0) snprintf(runbuf, sizeof(runbuf), "RUN: %d", running);
    if (running >= 0 && total >= 0)
        snprintf(stpbuf, sizeof(stpbuf), "STP: %d", total - running);

    OLED_Clear();
    draw_header("DOCKER");
    draw_data_lines(runbuf, stpbuf);
}

void LCD_DisplayHostname(void)
{
    char hostname[64] = {0};
    gethostname(hostname, sizeof(hostname) - 1);

    OLED_Clear();
    draw_header("HOSTNAME");
    draw_data_lines(hostname, "");
}

void LCD_DisplayIp(void)
{
    char ip[2][16] = {{0}, {0}};
    int nfound = 0;

#ifdef __linux__
    char iface[IFNAMSIZ];
    for (int n = 0; n < 2; n++) {
        get_nth_iface(n, iface, sizeof(iface));
        if (!iface[0]) break;
        get_iface_ip(iface, ip[nfound], sizeof(ip[nfound]));
        if (strcmp(ip[nfound], "0.0.0.0") != 0)
            nfound++;
    }
    if (nfound == 0) { OLED_Clear(); return; }
#else
    strncpy(ip[0], "192.168.1.100", sizeof(ip[0]) - 1);
    strncpy(ip[1], "10.0.0.42",     sizeof(ip[1]) - 1);
    nfound = 2;
#endif

    OLED_Clear();
    draw_header("IP ADDR");
    draw_data_lines(ip[0], nfound > 1 ? ip[1] : "");
}

void LCD_Display(unsigned char symbol)
{
    switch (symbol) {
        case 0:  LCD_DisplayTemperature(); break;
        case 1:  LCD_DisPlayCpuMemory();   break;
        case 2:  LCD_DisplaySdMemory();    break;
        case 3:  LCD_DisplayClock();       break;
        case 4:  LCD_DisplayUptime();      break;
        case 5:  LCD_DisplayCpuFreq();     break;
        case 6:  LCD_DisplayGpuTemp();     break;
        case 7:  LCD_DisplayNetwork();     break;
        case 8:  LCD_DisplayWifi();        break;
        case 9:  LCD_DisplayDocker();      break;
        case 10: LCD_DisplayHostname();    break;
        case 11: LCD_DisplayIp();          break;
        default: break;
    }
}
