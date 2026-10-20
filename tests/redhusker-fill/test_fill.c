#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
struct osd_bitmap { int depth,width,height; void *line[512]; };
struct fake_screen { int seg; } fake, *screen = &fake;
#define GFX_VESA2L 1
#define GFX_VESA3 2
#define VIDEO_RGB_DIRECT 1
#define VIDEO_DUAL_MONITOR 2
static int filltest=1,use_vesa=1,vector_game,scanrate15KHz,use_triplebuf;
static int scanlines,gfx_mode=2,gfx_width=640,gfx_height=480,video_attributes;
static int vis_min_x=0,vis_max_x=255,vis_min_y=16,vis_max_y=239;
static UINT32 palette[65536], *palette_16bit_lookup=palette;
static unsigned char output[480][2048];
static int output_depth=8;
static int bitmap_color_depth(struct fake_screen *s) { (void)s; return output_depth; }
static unsigned long bmp_write_line(struct fake_screen *s,int y) { (void)s; return (unsigned long)output[y]; }
static void _farsetsel(int s) { (void)s; }
static void _farnspokel(unsigned long p,UINT32 v) { memcpy((void *)p,&v,4); }
static void _farnspokeb(unsigned long p,unsigned char v) { *(unsigned char *)p=v; }
static void bmp_unwrite_line(struct fake_screen *s) { (void)s; }
#include "filltest.c"
int main(void)
{
    struct osd_bitmap b;
    static UINT8 pixels[256][256];
    static UINT16 pixels16[256][256];
    int x,y;
    b.depth=8; b.width=b.height=256;
    for(y=0;y<256;y++) { b.line[y]=pixels[y]; for(x=0;x<256;x++) pixels[y][x]=(x+3*y)%256; }
    assert(redhusker_fill_screen(&b)==1);
    for(y=0;y<480;y++) for(x=0;x<640;x++) assert(output[y][x]==pixels[16+y*224/480][x*256/640]);
    filltest=0; assert(redhusker_fill_screen(&b)==0); filltest=1;
    use_triplebuf=1; assert(redhusker_fill_screen(&b)==0); use_triplebuf=0;
    scanrate15KHz=1; assert(redhusker_fill_screen(&b)==0); scanrate15KHz=0;
    b.depth=16; output_depth=16;
    for(x=0;x<65536;x++) palette[x]=(UINT32)((x*37)&65535)*65537U;
    for(y=0;y<256;y++) { b.line[y]=pixels16[y]; for(x=0;x<256;x++) pixels16[y][x]=x+256*y; }
    assert(redhusker_fill_screen(&b)==1);
    for(y=0;y<480;y++) for(x=0;x<640;x++) { UINT16 value; memcpy(&value,output[y]+2*x,2); assert(value==(UINT16)palette[pixels16[16+y*224/480][x*256/640]]); }
    gfx_width=639; assert(redhusker_fill_screen(&b)==1);
    gfx_width=1025; assert(redhusker_fill_screen(&b)==0);
    puts("PASS: full-frame 8/16-bit Rygar mapping, palette conversion, odd width, opt-out and safety guards.");
    return 0;
}
