#include <blasteroids/main.h>
#include <blasteroids/config.h>
#include <blasteroids/collision.h>
#include <blasteroids/text.h>
#include <blasteroids/utils.h>
#include <blasteroids.h>
#include <signal.h>
#include <stdio.h>

const char *WindowTitle = "BLASTEROIDS by Lucas59356";
bool *running;
GameContext *ctx;

void update_states(GameContext *ctx) {
    blasteroids_asteroid_update_all(ctx->asteroids->next);
}

int main() {
    info("Iniciando...");
    running = malloc(sizeof(bool)); // Se isso for falso os loops param e o programa começa a desligar
    *running = true;
    // Signal handler
    if(catch_signal(SIGINT, stop) == -1 || catch_signal(SIGTERM, stop) == -1)
        error("Não foi possível setar o handler de interrupção");
    // Criando o contexto
    ctx = malloc(sizeof(GameContext));
    info("Inicializando...");
    if (!al_init())
        error("Não foi possível inicializar biblioteca de suporte!");
    if(!al_init_primitives_addon())
        error("Não foi possível inicializar a primitives addon");
    if(!al_init_font_addon())
        error("Não foi possível iniciar o módulo de fontes");
    if(!al_init_ttf_addon())
        error("Falha ao iniciar esquema de texto");
    // Queue
    ctx->event_queue = al_create_event_queue();
    // Timer
    ctx->timer = al_create_timer(1); // Tick a cada 1s
    if (!ctx->timer)
        error("Não foi possível iniciar o timer");
    al_start_timer(ctx->timer);
    al_register_event_source(ctx->event_queue, al_get_timer_event_source(ctx->timer));
    // Mutex
    ctx->mutex = al_create_mutex();
    if (!ctx->mutex)
        error("Não foi possível criar o mutex");
    // Teclado
    if(!al_install_keyboard())
        error("Não foi possível conectar-se ao teclado");
    al_register_event_source(ctx->event_queue, al_get_keyboard_event_source());
    // Display
    //al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    ctx->display = al_create_display(DISPLAY_ALTURA, DISPLAY_LARGURA);
    al_set_window_title(ctx->display, WindowTitle); // Título da janela
    al_register_event_source(ctx->event_queue, al_get_display_event_source(ctx->display));
    // Fonte
    ctx->font = al_load_font("font.ttf", 24, 0);
    // Criando spaceship de exemplo
    Spaceship *sp = malloc(sizeof(Spaceship));
    sp->sx = 200;
    sp->sy = 200;
    sp->heading = 20;
    sp->speed = 10;
    sp->color = al_map_rgb(255, 255, 0);
    sp->health = 100;
    ctx->ship = sp;
    // Criando asteroide genesis, para facilitar o trabalho
    ctx->asteroids = malloc(sizeof(Asteroid)); // Esse precisa ficar fora do radar
    ctx->asteroids->sx = 0;
    ctx->asteroids->sy = 0;
    ctx->asteroids->heading = 0;
    ctx->asteroids->speed = 0;
    ctx->asteroids->rot_velocity = 0;
    ctx->asteroids->scale = 0;
    ctx->asteroids->health = 9999;
    ctx->asteroids->color = al_map_rgb(0, 0, 0);
    ctx->asteroids->next = NULL;
    // Asteroide de exemplo
    Asteroid as;
    as.sx = 300.0;
    as.sy = 350.0;
    as.heading = 230.0;
    as.speed = 12.0;
    as.rot_velocity = 5.0;
    as.scale = 1;
    as.health = 100;
    as.color = al_map_rgb(15, 135, 88);
    as.next = NULL;
    blasteroids_asteroid_append(ctx->asteroids, as);
    // Event loop in main thread
    ALLEGRO_EVENT event; // Apenas para não ter de redeclarar a cada iteração
    while(1) {
        if (!*running) break;
        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));
        blasteroids_ship_draw(ctx->ship);
        blasteroids_asteroid_draw_all(ctx->asteroids->next);
        if (is_collision(ctx)) 
            debug("COLISÃO"); // debug é uma macro
        event_loop_once(ctx, &event);
        draw_life(ctx);
        blasteroids_fix_positions(ctx);
#ifdef DEBUG
        blasteroids_asteroid_draw_life(ctx);
#endif
    }
    // ============= SAINDO ===========
    handle_shutdown(SIGINT);
}

void handle_shutdown() {
    info("Saindo....");
    /*debug("Destroy timer");
      al_destroy_timer(ctx->timer);*/
    debug("Destroy mutex");
    al_destroy_mutex(ctx->mutex);
    debug("Destroy timer");
    al_destroy_timer(ctx->timer);
    // Queue
    debug("Destroy queue");
    al_destroy_event_queue(ctx->event_queue);
    debug("Free ship");
    free(ctx->ship);
    debug("Free running");
    free(running);
    debug("Free asteroids");
    blasteroids_destroy_asteroid(ctx->asteroids);
    debug("Destroy display");
    al_destroy_display(ctx->display);
    debug("Destroy font");
    al_destroy_font(ctx->font);
    debug("Free ctx");
    free(ctx);
    //raise(SIGKILL);
    exit(1);
}

void stop(int sig) {
    *running = false;
}
