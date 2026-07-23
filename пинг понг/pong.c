#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// === DEFINES ===

#define SCREEN_W   800
#define SCREEN_H   600
#define PAD_W      15
#define PAD_H      100
#define BALL_SZ    12
#define PAD_SPD    400.0f
#define BALL_SPD   350.0f
#define BALL_ACCEL 1.08f
#define FRAME_MS   16
#define WIN_S      10
#define PAD_MARGIN 40

#define BG_R  0x1a
#define BG_G  0x1a
#define BG_B  0x2e
#define FG_R  0xcd
#define FG_G  0xd6
#define FG_B  0xf4
#define ACC_R 0x89
#define ACC_G 0xb4
#define ACC_B 0xfa

// === 5x7 BITMAP FONT ===
// each entry = uint8_t[7] where bits 4..0 = columns 0..4

static const uint8_t font[128][7] = {
    [' '] = {0,0,0,0,0,0,0},
    ['.'] = {0,0,0,0,0,0,4},
    [':'] = {0,0,4,0,0,4,0},
    ['!'] = {4,4,4,4,4,0,4},
    ['-'] = {0,0,0,0x1F,0,0,0},
    ['0'] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    ['1'] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    ['2'] = {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    ['3'] = {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    ['4'] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    ['5'] = {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    ['6'] = {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    ['7'] = {0x1F,0x01,0x02,0x04,0x04,0x04,0x04},
    ['8'] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    ['9'] = {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    ['A'] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    ['B'] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    ['C'] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    ['D'] = {0x1E,0x09,0x09,0x09,0x09,0x09,0x1E},
    ['E'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    ['F'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    ['G'] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    ['H'] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    ['I'] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    ['J'] = {0x07,0x02,0x02,0x02,0x12,0x12,0x0C},
    ['K'] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    ['L'] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    ['M'] = {0x11,0x1B,0x15,0x11,0x11,0x11,0x11},
    ['N'] = {0x11,0x11,0x13,0x15,0x19,0x11,0x11},
    ['O'] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    ['P'] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    ['Q'] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    ['R'] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    ['S'] = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},
    ['T'] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    ['U'] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    ['V'] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},
    ['W'] = {0x11,0x11,0x11,0x15,0x1B,0x11,0x11},
    ['X'] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    ['Y'] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    ['Z'] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

// === ENUMS ===

typedef enum { EASY, NORMAL, HARD, VS_FRIEND } Difficulty;
typedef enum { SCR_MENU, SCR_DIFF, SCR_PLAY } Screen;

// === TYPES ===

typedef struct {
    float x, y, w, h, vy;
} Paddle;

typedef struct {
    float x, y, sz, vx, vy;
} Ball;

typedef struct {
    int p1, p2;
    bool paused, running, over;
} State;

typedef struct {
    SDL_Window*   win;
    SDL_Renderer* ren;
    Paddle        pad1, pad2;
    Ball          ball;
    State         st;
    Uint64        freq, last;
    const Uint8*  keys;
    Screen        scr;
    Difficulty    diff;
    int           choice;
} Game;

// === PROTOS ===

static bool   init(Game* g);
static void   quit(Game* g);
static void   reset_round(Game* g);
static void   reset_all(Game* g);
static void   events(Game* g);
static void   update_bot(Game* g, float dt);
static void   update(Game* g, float dt);
static void   draw_char(SDL_Renderer* r, int x, int y, unsigned char c, int scale, Uint8 R, Uint8 G, Uint8 B);
static void   draw_text(SDL_Renderer* r, int x, int y, const char* s, int scale, Uint8 R, Uint8 G, Uint8 B);
static void   draw_centered(SDL_Renderer* r, int y, const char* s, int scale, Uint8 R, Uint8 G, Uint8 B);
static void   render_menu(const Game* g);
static void   render_diff(const Game* g);
static void   render_game(const Game* g);
static void   render(const Game* g);
static void   loop(Game* g);

// === INIT ===

static bool init(Game* g)
{
    SDL_zero(*g);
    g->freq = SDL_GetPerformanceFrequency();
    if (g->freq == 0) g->freq = 1000;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
        return false;
    }

    g->win = SDL_CreateWindow(
        "Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN
    );
    if (!g->win) {
        fprintf(stderr, "window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g->ren = SDL_CreateRenderer(
        g->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!g->ren) {
        fprintf(stderr, "renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(g->win);
        SDL_Quit();
        return false;
    }

    srand((unsigned)time(NULL));
    g->keys = SDL_GetKeyboardState(NULL);
    g->scr = SCR_MENU;
    g->diff = EASY;
    g->choice = 0;
    reset_all(g);
    return true;
}

static void quit(Game* g)
{
    if (g->ren) SDL_DestroyRenderer(g->ren);
    if (g->win) SDL_DestroyWindow(g->win);
    SDL_Quit();
}

// === PHYSICS ===

static void reset_round(Game* g)
{
    g->ball.x = (SCREEN_W - BALL_SZ) / 2.0f;
    g->ball.y = (SCREEN_H - BALL_SZ) / 2.0f;
    g->ball.sz = BALL_SZ;
    g->ball.vx = (rand() % 2 ? 1 : -1) * BALL_SPD;
    g->ball.vy = (rand() % 2 ? 1 : -1) * BALL_SPD * 0.6f;
}

static void reset_all(Game* g)
{
    g->pad1.x = PAD_MARGIN;
    g->pad1.y = (SCREEN_H - PAD_H) / 2.0f;
    g->pad1.w = PAD_W;
    g->pad1.h = PAD_H;
    g->pad1.vy = 0;

    g->pad2.x = SCREEN_W - PAD_MARGIN - PAD_W;
    g->pad2.y = (SCREEN_H - PAD_H) / 2.0f;
    g->pad2.w = PAD_W;
    g->pad2.h = PAD_H;
    g->pad2.vy = 0;

    g->st.p1 = g->st.p2 = 0;
    g->st.paused = false;
    g->st.running = true;
    g->st.over = false;

    reset_round(g);
}

static void events(Game* g)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) g->st.running = false;
        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE) {
                if (g->scr == SCR_PLAY) g->scr = SCR_MENU;
                else g->st.running = false;
            }
            if (ev.key.keysym.sym == SDLK_SPACE) {
                if (g->scr == SCR_MENU) {
                    g->scr = SCR_DIFF;
                    g->choice = 0;
                } else if (g->scr == SCR_DIFF) {
                    g->diff = (Difficulty)g->choice;
                    g->scr = SCR_PLAY;
                    reset_all(g);
                } else if (g->scr == SCR_PLAY) {
                    if (g->st.over) { g->scr = SCR_MENU; reset_all(g); }
                    else g->st.paused = !g->st.paused;
                }
            }
            if (g->scr == SCR_DIFF) {
                if ((ev.key.keysym.sym == SDLK_w || ev.key.keysym.sym == SDLK_UP) && g->choice > 0)
                    g->choice--;
                if ((ev.key.keysym.sym == SDLK_s || ev.key.keysym.sym == SDLK_DOWN) && g->choice < 3)
                    g->choice++;
            }
        }
    }
}

static void update_bot(Game* g, float dt)
{
    if (g->diff == VS_FRIEND) return;

    float target = g->ball.y + g->ball.sz / 2 - g->pad2.h / 2;
    float error = g->pad2.y - target;
    float spd = 0;

    switch (g->diff) {
    case EASY:
        spd = PAD_SPD * 0.3f;
        if (fabsf(target - SCREEN_H/2) < 50) return; // bot dazes off
        break;
    case NORMAL:
        spd = PAD_SPD * 0.55f;
        break;
    case HARD:
        spd = PAD_SPD * 0.85f;
        break;
    default: break;
    }

    if (fabsf(error) > 5) {
        g->pad2.vy = (error > 0 ? -1 : 1) * spd;
    } else {
        g->pad2.vy = 0;
    }

    g->pad2.y += g->pad2.vy * dt;
    if (g->pad2.y < 0) g->pad2.y = 0;
    if (g->pad2.y + g->pad2.h > SCREEN_H) g->pad2.y = SCREEN_H - g->pad2.h;
}

static void update(Game* g, float dt)
{
    if (g->st.paused || g->st.over || g->scr != SCR_PLAY) return;

    if (g->keys[SDL_SCANCODE_W])      g->pad1.vy = -PAD_SPD;
    else if (g->keys[SDL_SCANCODE_S]) g->pad1.vy =  PAD_SPD;
    else                              g->pad1.vy = 0;

    if (g->diff == VS_FRIEND) {
        if (g->keys[SDL_SCANCODE_UP])     g->pad2.vy = -PAD_SPD;
        else if (g->keys[SDL_SCANCODE_DOWN]) g->pad2.vy = PAD_SPD;
        else                               g->pad2.vy = 0;
    } else {
        update_bot(g, dt);
    }

    g->pad1.y += g->pad1.vy * dt;
    if (g->pad1.y < 0) g->pad1.y = 0;
    if (g->pad1.y + g->pad1.h > SCREEN_H) g->pad1.y = SCREEN_H - g->pad1.h;

    g->ball.x += g->ball.vx * dt;
    g->ball.y += g->ball.vy * dt;

    if (g->ball.y < 0) { g->ball.y = 0; g->ball.vy = -g->ball.vy; }
    if (g->ball.y + g->ball.sz > SCREEN_H) {
        g->ball.y = SCREEN_H - g->ball.sz;
        g->ball.vy = -g->ball.vy;
    }

    float bx = g->ball.x, by = g->ball.y, bs = g->ball.sz;

    if (bx + bs > g->pad1.x && bx < g->pad1.x + g->pad1.w &&
        by + bs > g->pad1.y && by < g->pad1.y + g->pad1.h)
    {
        float off = (by + bs/2 - (g->pad1.y + g->pad1.h/2)) / (g->pad1.h/2);
        g->ball.x = g->pad1.x + g->pad1.w;
        g->ball.vx = fabsf(g->ball.vx) * BALL_ACCEL;
        g->ball.vy = off * BALL_SPD * 0.8f;
    }

    if (bx + bs > g->pad2.x && bx < g->pad2.x + g->pad2.w &&
        by + bs > g->pad2.y && by < g->pad2.y + g->pad2.h)
    {
        float off = (by + bs/2 - (g->pad2.y + g->pad2.h/2)) / (g->pad2.h/2);
        g->ball.x = g->pad2.x - bs;
        g->ball.vx = -fabsf(g->ball.vx) * BALL_ACCEL;
        g->ball.vy = off * BALL_SPD * 0.8f;
    }

    if (g->ball.x + g->ball.sz < 0) {
        g->st.p2++;
        if (g->st.p2 >= WIN_S) g->st.over = true;
        else reset_round(g);
    }
    if (g->ball.x > SCREEN_W) {
        g->st.p1++;
        if (g->st.p1 >= WIN_S) g->st.over = true;
        else reset_round(g);
    }
}

// === FONT RENDER ===

static void draw_char(SDL_Renderer* r, int x, int y, unsigned char c, int s, Uint8 R, Uint8 G, Uint8 B)
{
    if (c > 127) return;
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for (int row = 0; row < 7; row++) {
        uint8_t bits = font[(int)c][row];
        for (int col = 0; col < 5; col++) {
            if (bits & (1 << (4 - col))) {
                SDL_Rect p = { x + col * s, y + row * s, s, s };
                SDL_RenderFillRect(r, &p);
            }
        }
    }
}

static void draw_text(SDL_Renderer* r, int x, int y, const char* s, int scale, Uint8 R, Uint8 G, Uint8 B)
{
    while (*s) {
        if (*s == ' ') { x += 5 * scale; s++; continue; }
        draw_char(r, x, y, *s, scale, R, G, B);
        x += 6 * scale;
        s++;
    }
}

static void draw_centered(SDL_Renderer* r, int y, const char* s, int scale, Uint8 R, Uint8 G, Uint8 B)
{
    int total = 0;
    const char* p = s;
    while (*p) {
        total += (*p == ' ') ? 5 * scale : 6 * scale;
        p++;
    }
    draw_text(r, SCREEN_W/2 - total/2, y, s, scale, R, G, B);
}

// === RENDER ===

static void render_menu(const Game* g)
{
    SDL_SetRenderDrawColor(g->ren, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(g->ren);

    draw_centered(g->ren, 120, "PONG", 10, FG_R, FG_G, FG_B);

    if ((SDL_GetTicks() / 600) % 2) {
        draw_centered(g->ren, 350, "PRESS SPACE TO START", 3, ACC_R, ACC_G, ACC_B);
    }
}

static void render_diff(const Game* g)
{
    SDL_SetRenderDrawColor(g->ren, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(g->ren);

    draw_centered(g->ren, 80, "SELECT DIFFICULTY", 3, FG_R, FG_G, FG_B);

    const char* opts[] = { "EASY", "NORMAL", "HARD", "VS FRIEND" };
    for (int i = 0; i < 4; i++) {
        int y = 200 + i * 80;
        if (i == g->choice) {
            SDL_Rect sel = { SCREEN_W/2 - 100, y - 10, 200, 50 };
            SDL_SetRenderDrawColor(g->ren, ACC_R, ACC_G, ACC_B, 80);
            SDL_RenderFillRect(g->ren, &sel);
            draw_centered(g->ren, y, opts[i], 4, ACC_R, ACC_G, ACC_B);
        } else {
            draw_centered(g->ren, y, opts[i], 3, FG_R, FG_G, FG_B);
        }
    }
}

static void render_game(const Game* g)
{
    SDL_SetRenderDrawColor(g->ren, BG_R, BG_G, BG_B, 255);
    SDL_RenderClear(g->ren);

    // center line
    SDL_SetRenderDrawColor(g->ren, FG_R, FG_G, FG_B, 51);
    for (int y = 0; y < SCREEN_H; y += 25) {
        SDL_Rect seg = { SCREEN_W/2 - 1, y, 2, 15 };
        SDL_RenderFillRect(g->ren, &seg);
    }

    // paddles
    SDL_SetRenderDrawColor(g->ren, ACC_R, ACC_G, ACC_B, 255);
    SDL_Rect p1 = { (int)g->pad1.x, (int)g->pad1.y, (int)g->pad1.w, (int)g->pad1.h };
    SDL_RenderFillRect(g->ren, &p1);
    SDL_Rect p2 = { (int)g->pad2.x, (int)g->pad2.y, (int)g->pad2.w, (int)g->pad2.h };
    SDL_RenderFillRect(g->ren, &p2);

    // ball
    SDL_SetRenderDrawColor(g->ren, FG_R, FG_G, FG_B, 255);
    SDL_Rect ball = { (int)g->ball.x, (int)g->ball.y, (int)g->ball.sz, (int)g->ball.sz };
    SDL_RenderFillRect(g->ren, &ball);

    // score bars
    int bw = SCREEN_W/2 - 100, bh = 8, by = 20;

    SDL_SetRenderDrawColor(g->ren, FG_R, FG_G, FG_B, 255);
    SDL_Rect b1 = { 50, by, bw, bh };
    SDL_RenderDrawRect(g->ren, &b1);
    if (g->st.p1 > 0) {
        SDL_SetRenderDrawColor(g->ren, ACC_R, ACC_G, ACC_B, 255);
        SDL_Rect f1 = { 51, by+1, (bw * g->st.p1)/WIN_S - 2, bh-2 };
        SDL_RenderFillRect(g->ren, &f1);
    }

    SDL_SetRenderDrawColor(g->ren, FG_R, FG_G, FG_B, 255);
    SDL_Rect b2 = { SCREEN_W - 50 - bw, by, bw, bh };
    SDL_RenderDrawRect(g->ren, &b2);
    if (g->st.p2 > 0) {
        SDL_SetRenderDrawColor(g->ren, ACC_R, ACC_G, ACC_B, 255);
        SDL_Rect f2 = { SCREEN_W - 50 - bw + 1, by+1, (bw * g->st.p2)/WIN_S - 2, bh-2 };
        SDL_RenderFillRect(g->ren, &f2);
    }

    // score digits
    char s1[4], s2[4];
    snprintf(s1, sizeof(s1), "%d", g->st.p1);
    snprintf(s2, sizeof(s2), "%d", g->st.p2);
    draw_centered(g->ren, 50, s1, 2, FG_R, FG_G, FG_B);
    draw_centered(g->ren, 50, s2, 2, FG_R, FG_G, FG_B);

    // ESC hint
    draw_centered(g->ren, SCREEN_H - 25, "ESC - MENU", 1, FG_R, FG_G, FG_B);

    // difficulty label
    const char* diff_names[] = { "EASY", "NORMAL", "HARD", "VS FRIEND" };
    draw_centered(g->ren, SCREEN_H - 45, diff_names[g->diff], 2, ACC_R, ACC_G, ACC_B);

    // overlay
    if (g->st.over) {
        SDL_SetRenderDrawColor(g->ren, 0, 0, 0, 153);
        SDL_RenderClear(g->ren);
        if ((SDL_GetTicks() / 500) % 2) {
            draw_centered(g->ren, SCREEN_H/2 - 20, "PRESS SPACE", 4, FG_R, FG_G, FG_B);
            draw_centered(g->ren, SCREEN_H/2 + 30, "TO RETURN", 3, FG_R, FG_G, FG_B);
        }
    } else if (g->st.paused) {
        SDL_SetRenderDrawColor(g->ren, 0, 0, 0, 102);
        SDL_RenderClear(g->ren);
        draw_centered(g->ren, SCREEN_H/2, "PAUSED", 5, FG_R, FG_G, FG_B);
    }
}

static void render(const Game* g)
{
    switch (g->scr) {
    case SCR_MENU:  render_menu(g);  break;
    case SCR_DIFF:  render_diff(g);  break;
    case SCR_PLAY:  render_game(g);  break;
    }
    SDL_RenderPresent(g->ren);
}

// === LOOP ===

static void loop(Game* g)
{
    g->last = SDL_GetPerformanceCounter();

    while (g->st.running) {
        Uint32 start = SDL_GetTicks();

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - g->last) / (float)g->freq;
        g->last = now;
        if (dt > 0.05f) dt = 0.05f;

        events(g);
        update(g, dt);
        render(g);

        Uint32 ft = SDL_GetTicks() - start;
        if (ft < FRAME_MS) SDL_Delay(FRAME_MS - ft);
    }
}

// === MAIN ===

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    Game g;

    if (!init(&g)) {
        fprintf(stderr, "failed to init\n");
        return 1;
    }

    loop(&g);
    quit(&g);
    return 0;
}

