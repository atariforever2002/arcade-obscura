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
#include "fill52.c"
int main(void)
{
    struct osd_bitmap b;
    static UINT16 data[256][256];
    int widths[]={1,2,3,4,7,255,256,319,320,639,640,800,1023,1024};
    int heights[]={1,17,223,224,240,480};
    int depths[]={8,15,16};
    int di,wi,hi,frame,x,y,n=0;
    b.width=b.height=256;
    for(y=0;y<256;y++) b.line[y]=data[y];
    for(di=0;di<3;di++) for(wi=0;wi<14;wi++) for(hi=0;hi<6;hi++)
    for(frame=0;frame<6;frame++)
    {
        b.depth=output_depth=depths[di];
        gfx_width=widths[wi]; gfx_height=heights[hi];
        vis_min_x=frame<3?frame*3:0; vis_max_x=frame<3?255-frame:255;
        vis_min_y=16+frame; vis_max_y=239-frame;
        for(x=0;x<65536;x++) palette[x]=((x*37+frame*111)&65535U)*65537U;
        for(y=0;y<256;y++) for(x=0;x<256;x++)
            if(b.depth==8) ((UINT8 *)data[y])[x]=(x+3*y+frame)%256;
            else data[y][x]=(x+256*y+frame)&65535;
        memset(output,0xa5,sizeof(output));
        assert(redhusker_fill_screen(&b)==1);
        for(y=0;y<480;y++) for(x=0;x<2048;x++)
        {
            unsigned int expected=0xa5;
            int bytes=b.depth==8?1:2;
            if(y<gfx_height && x<gfx_width*bytes)
            {
                int sx=vis_min_x+(x/bytes)*(vis_max_x-vis_min_x+1)/gfx_width;
                int sy=vis_min_y+y*(vis_max_y-vis_min_y+1)/gfx_height;
                if(bytes==1) expected=((UINT8 *)data[sy])[sx];
                else expected=(palette[data[sy][sx]] >> (8*(x%2)))&255;
            }
            assert(output[y][x]==expected);
        }
        n++;
    }
    filltest=0; assert(!redhusker_fill_screen(&b)); filltest=1;
    vector_game=1; assert(!redhusker_fill_screen(&b)); vector_game=0;
    use_triplebuf=1; assert(!redhusker_fill_screen(&b)); use_triplebuf=0;
    scanrate15KHz=1; assert(!redhusker_fill_screen(&b)); scanrate15KHz=0;
    scanlines=1; assert(!redhusker_fill_screen(&b)); scanlines=0;
    gfx_width=1025; assert(!redhusker_fill_screen(&b)); gfx_width=640;
    gfx_height=2049; assert(!redhusker_fill_screen(&b)); gfx_height=480;
    palette_16bit_lookup=0; assert(!redhusker_fill_screen(&b));
    printf("PASS: %d full-frame cases, 8/15/16-bit, odd widths, changed palettes/crops, untouched padding and fallback guards.\n",n);
    return 0;
}
