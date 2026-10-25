/* Opt-in Red Husker experiment: standard linear VESA, 8/16-bit only.
 * Return 0 for unsupported cases so the original renderer stays active.
 * No custom CRT timings, cropping, ROM, input, or audio changes.
 */
static int redhusker_fill_screen(struct osd_bitmap *bitmap)
{
    static int xmap[1024];
    static int cached_width, cached_source_width;
    static int ymap[2048], cached_height, cached_source_height;
    static UINT32 row[512];
    int x,y,sy,last_sy,bpp,bytes,source_width,source_height;
    int measure;
    TICKER tick0=0, tick1;
    unsigned long address;
    unsigned char *out;

    if (!filltest || !use_vesa || vector_game || scanrate15KHz ||
        use_triplebuf || scanlines ||
        (gfx_mode != GFX_VESA2L && gfx_mode != GFX_VESA3) ||
        gfx_width < 1 || gfx_width > 1024 || gfx_height < 1 || gfx_height > 2048 ||
        (video_attributes & (VIDEO_RGB_DIRECT | VIDEO_DUAL_MONITOR)))
        return 0;
    bpp = bitmap_color_depth(screen);
    if (!((bitmap->depth == 8 && bpp == 8) ||
          ((bitmap->depth == 15 || bitmap->depth == 16) &&
           (bpp == 15 || bpp == 16) && palette_16bit_lookup)))
        return 0;
    source_width = vis_max_x - vis_min_x + 1;
    source_height = vis_max_y - vis_min_y + 1;
    if (source_width < 1 || source_height < 1 || vis_min_x < 0 ||
        vis_min_y < 0 || vis_max_x >= bitmap->width ||
        vis_max_y >= bitmap->height)
        return 0;
    if (cached_width != gfx_width || cached_source_width != source_width)
    {
        for (x=0; x<gfx_width; x++)
            xmap[x] = (unsigned long)x * source_width / gfx_width;
        cached_width = gfx_width;
        cached_source_width = source_width;
    }
    if (cached_height != gfx_height || cached_source_height != source_height)
    {
        for (y=0; y<gfx_height; y++)
            ymap[y] = (unsigned long)y * source_height / gfx_height;
        cached_height = gfx_height;
        cached_source_height = source_height;
    }
    measure = ((rh_calls++ & 15UL) == 0);
    if (measure) { rh_samples++; rh_depth=bpp; rh_sw=source_width; rh_sh=source_height; }
    bytes = gfx_width * (bpp == 8 ? 1 : 2);
    out = (unsigned char *)row;
    last_sy = -1;
    for (y=0; y<gfx_height; y++)
    {
        if (measure) tick0=ticker();
        sy = vis_min_y + ymap[y];
        if (sy != last_sy)
        {
            if (bpp == 8)
            {
                const UINT8 *src = (const UINT8 *)bitmap->line[sy] + vis_min_x;
                for (x=0; x<gfx_width; x++) out[x] = src[xmap[x]];
            }
            else
            {
                const UINT16 *src = (const UINT16 *)bitmap->line[sy] + vis_min_x;
                UINT16 *dst = (UINT16 *)row;
                for (x=0; x+1<gfx_width; x+=2)
                {
                    UINT32 first = palette_16bit_lookup[src[xmap[x]]] & 65535U;
                    UINT32 second = xmap[x] == xmap[x+1] ? first :
                        palette_16bit_lookup[src[xmap[x+1]]] & 65535U;
                    row[x/2] = first | (second << 16);
                }
                if (x < gfx_width)
                    dst[x] = (UINT16)palette_16bit_lookup[src[xmap[x]]];
            }
            last_sy = sy;
        }
        if (measure) { tick1=ticker(); rh_prepare += tick1-tick0; tick0=tick1; }
        address = bmp_write_line(screen,y);
        _farsetsel(screen->seg);
        for (x=0; x+3<bytes/4; x+=4)
        {
            _farnspokel(address+4*x,row[x]);
            _farnspokel(address+4*x+4,row[x+1]);
            _farnspokel(address+4*x+8,row[x+2]);
            _farnspokel(address+4*x+12,row[x+3]);
        }
        for (; x<bytes/4; x++) _farnspokel(address+4*x,row[x]);
        for (x=(bytes/4)*4; x<bytes; x++) _farnspokeb(address+x,out[x]);
        if (measure) { tick1=ticker(); rh_transfer += tick1-tick0; }
    }
    bmp_unwrite_line(screen);
    return 1;
}
