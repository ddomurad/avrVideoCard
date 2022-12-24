#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <SDL2/SDL.h>

#define SERVEVR_PORT 10601
#define GLYPH_SIZE 16

#define V_RAM_SIZE 4096

#define GLYPHS_TABLE_WIDTH 62
#define GLYPHS_TABLE_HEIGHT 52

#define GFX_CMD_NOP 0x80
#define GFX_CMD_SET_ADDR_LOW 0x01
#define GFX_CMD_SET_ADDR_HIGH 0x02
#define GFX_CMD_PUT_ADD 0x84
#define GFX_CMD_PUT 0x88
#define GFX_CMD_PUT_SUB 0x90
#define GFX_CMD_EXT 0x20
#define GFX_CMD_MOV_PTR 0x40

// Video card extended commands
#define GFX_EXTCMD_PUSH_UP 0x01
#define GFX_EXTCMD_PUSH_DOWN 0x82
#define GFX_EXTCMD_PUSH_LEFT 0x04
#define GFX_EXTCMD_PUSH_RIGHT 0x88
#define GFX_EXTCMD_CLEAR 0x10

// Video card addresses
#define GFX_GLYPHS_ADDR 0x0100
#define GFX_INVS_ADDR 0x0f0c
#define GFX_GLYPHS_SET_ADDR 0x10dc
#define GFX_TEXT_COLOR_ADDR 0x10dd
#define GFX_EXTCMD_PARAMS_ADDR 0x10de

uint8_t v_ram[V_RAM_SIZE];

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;

SDL_Surface *glyphs_map[4];
uint16_t y_ptr = 0;

void _fatal_error(const char *err)
{
    printf(err);
    exit(1);
}

uint8_t read_v_ram(uint16_t addr)
{
    return v_ram[addr - 0x0100];
}

void write_v_ram(uint16_t addr, uint8_t data)
{
    v_ram[addr - 0x0100] = data;
}

void init_ram()
{
    // clear ram
    bzero(v_ram, V_RAM_SIZE);

    // zero ram
    for (int i = 0; i < V_RAM_SIZE; i++)
    {
        v_ram[i] = (uint8_t)i;
    }

    // select default glyphs set
    write_v_ram(GFX_GLYPHS_SET_ADDR, 0x20);
}

void init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        _fatal_error("SDL_Init failed");

    window = SDL_CreateWindow(
        "VC emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        GLYPH_SIZE * 62,
        GLYPH_SIZE * 58,
        SDL_WINDOW_SHOWN);

    if (window == NULL)
        _fatal_error("SDL_CreateWindow failed");

    surface = SDL_GetWindowSurface(window);
    if (surface == NULL)
        _fatal_error("SDL_GetWindowSurface failed");

    // load glyphs
    glyphs_map[0] = SDL_LoadBMP("./assets/glyphs1.bmp");
    glyphs_map[1] = SDL_LoadBMP("./assets/der_rouge.bmp");
    glyphs_map[2] = SDL_LoadBMP("./assets/tileset_1bit.bmp");
    glyphs_map[3] = SDL_LoadBMP("./assets/extra-1bits.bmp");
}

void render_glyph(int x, int y, SDL_Surface *glyphs)
{
    int v_ram_ptr = x + y * 62;
    int glyph_index = read_v_ram(GFX_GLYPHS_ADDR + v_ram_ptr);

    int gx = glyph_index % 16;
    int gy = glyph_index / 16;

    SDL_Rect src_rect = {gx * GLYPH_SIZE, gy * GLYPH_SIZE, GLYPH_SIZE, GLYPH_SIZE};
    SDL_Rect dst_rect = {x * GLYPH_SIZE, y * GLYPH_SIZE, GLYPH_SIZE, GLYPH_SIZE};

    SDL_BlitSurface(glyphs, &src_rect, surface, &dst_rect);
}

void render_v_ram()
{
    uint8_t gs = read_v_ram(GFX_GLYPHS_SET_ADDR);
    SDL_Surface *current_glyphs = glyphs_map[0];

    // select glyphs set
    switch (gs)
    {
    case 0x30:
        current_glyphs = glyphs_map[1];
        break;
    case 0x40:
        current_glyphs = glyphs_map[2];
        break;
    case 0x50:
        current_glyphs = glyphs_map[3];
        break;
    default:
        current_glyphs = glyphs_map[0];
    }

    // clear screen
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));

    for (int iy = 0; iy < 58; iy++)
    {
        for (int ix = 0; ix < 62; ix++)
        {
            render_glyph(ix, iy, current_glyphs);
        }
    }

    SDL_UpdateWindowSurface(window);
}

void exec_extended_cmd(uint8_t ext_cmd)
{
    uint8_t x = read_v_ram(GFX_EXTCMD_PARAMS_ADDR + 0);
    uint8_t y = read_v_ram(GFX_EXTCMD_PARAMS_ADDR + 1);
    uint8_t w = read_v_ram(GFX_EXTCMD_PARAMS_ADDR + 2);
    uint8_t h = read_v_ram(GFX_EXTCMD_PARAMS_ADDR + 3);
    uint8_t d = read_v_ram(GFX_EXTCMD_PARAMS_ADDR + 4);

    if (ext_cmd == GFX_EXTCMD_PUSH_UP)
    {
        for (int iy = y; iy < y + h - 1; iy++)
        {
            for (int ix = x; ix < x + w; ix++)
            {
                uint16_t src_addr = GFX_GLYPHS_ADDR + ix + (iy + 1) * 62;
                uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + iy * 62;
                write_v_ram(dst_addr, read_v_ram(src_addr));
            }
        }
        for (int ix = x; ix < x + w; ix++)
        {
            uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + (y + h - 1) * 62;
            write_v_ram(dst_addr, d);
        }
    }
    else if (ext_cmd == GFX_EXTCMD_PUSH_DOWN)
    {
        for (int iy = y + h - 1; iy >= y; iy--)
        {
            for (int ix = x; ix < x + w; ix++)
            {
                uint16_t src_addr = GFX_GLYPHS_ADDR + ix + (iy - 1) * 62;
                uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + iy * 62;
                write_v_ram(dst_addr, read_v_ram(src_addr));
            }
        }
        for (int ix = x; ix < x + w; ix++)
        {
            uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + y * 62;
            write_v_ram(dst_addr, d);
        }
    }
    else if (ext_cmd == GFX_EXTCMD_PUSH_RIGHT)
    {
        for (int iy = y; iy < y + h; iy++)
        {
            for (int ix = x + w - 1; ix >= x; ix--)
            {
                uint16_t src_addr = GFX_GLYPHS_ADDR + ix - 1 + iy * 62;
                uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + iy * 62;
                write_v_ram(dst_addr, read_v_ram(src_addr));
            }

            uint16_t dst_addr = GFX_GLYPHS_ADDR + x + iy * 62;
            write_v_ram(dst_addr, d);
        }
    }
    else if (ext_cmd == GFX_EXTCMD_PUSH_LEFT)
    {
        for (int iy = y; iy < y + h; iy++)
        {
            for (int ix = x; ix < x + w - 1; ix++)
            {
                uint16_t src_addr = GFX_GLYPHS_ADDR + ix + 1 + iy * 62;
                uint16_t dst_addr = GFX_GLYPHS_ADDR + ix + iy * 62;
                write_v_ram(dst_addr, read_v_ram(src_addr));
            }

            uint16_t dst_addr = GFX_GLYPHS_ADDR + x + w - 1 + iy * 62;
            write_v_ram(dst_addr, d);
        }
    }
    else if (ext_cmd == GFX_EXTCMD_CLEAR)
    {
        for (int iy = y; iy < y + h; iy++)
        {
            for (int ix = x; ix < x + w; ix++)
            {
                uint16_t a = GFX_GLYPHS_ADDR + ix + iy * 62;
                write_v_ram(a, d);
            }
        }
    }
}

void exec_cmd(uint8_t cmd, uint8_t data)
{
    printf("exec: %u %u\n", cmd, data);

    if (cmd == GFX_CMD_SET_ADDR_LOW)
    {
        y_ptr = (y_ptr & 0xff00) | data;
    }
    else if (cmd == GFX_CMD_SET_ADDR_HIGH)
    {
        y_ptr = (y_ptr & 0x00ff) | (data << 8);
    }
    else if (cmd == GFX_CMD_PUT_ADD)
    {
        write_v_ram(y_ptr++, data);
        render_v_ram();
    }
    else if (cmd == GFX_CMD_PUT)
    {
        write_v_ram(y_ptr, data);
        render_v_ram();
    }
    else if (cmd == GFX_CMD_PUT_SUB)
    {
        write_v_ram(--y_ptr, data);
        render_v_ram();
    }
    else if (cmd == GFX_CMD_EXT)
    {
        exec_extended_cmd(data);
        render_v_ram();
    }
    else if (cmd == GFX_CMD_MOV_PTR)
    {
        y_ptr += data;
    }
}

void handle_client(int connfd)
{
    int protocol_state = 0;
    uint8_t cmd = 0;

    while (1)
    {
        uint8_t data = 0;
        int read_size = read(connfd, &data, 1);
        if (read_size == 0)
        {
            printf("client has disconnected ... \n");
            return;
        }

        if (protocol_state == 0)
        {
            cmd = data;
            protocol_state = 1;
        }
        else
        {
            exec_cmd(cmd, data);
            protocol_state = 0;
        }
    }
}

int main()
{
    init_ram();
    init_sdl();
    render_v_ram();

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        _fatal_error("socket creation failed...");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVEVR_PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
        _fatal_error("socket bind failed...");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
        _fatal_error("listen for connections failed...");        
    

    printf("server listening...\n");

    len = sizeof(cli);

    while (1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
        if (connfd < 0)
            _fatal_error("connection accept failed ...");        

        printf("server accept new client...\n");

        handle_client(connfd);
    }
}