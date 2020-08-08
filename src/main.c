
#include <stdio.h>
#include <dlfcn.h>
#include <allegro5/allegro.h>

#include "log.h"

const float FPS = 60.0;


int main(int argc, char **argv)
{
    Logger logger = {
        .stream = stderr,
        .level = LOG_INFO
    };
    set_logger(&logger);

    if (!al_init()) {
        ERROR("Failed to init allegro");
        return EXIT_FAILURE;
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        ERROR("Failed to create tick timer");
        return EXIT_FAILURE;
    }


    ALLEGRO_DISPLAY *display = al_create_display(600, 400);
    if (!display) {
        ERROR("Failed to create main display");
        return EXIT_FAILURE;
    }

    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    if (!event_queue) {
        ERROR("Failed to create main event queue");
        return EXIT_FAILURE;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    // display blank screen
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    al_start_timer(timer);

    bool running = true;

    INFO("Starting main game loop...");

    while (running) {
        ALLEGRO_TIMEOUT timeout;
        al_init_timeout(&timeout, 0.06);

        bool redraw = false;
        ALLEGRO_EVENT event;
        bool has_event = al_wait_for_event_until(event_queue, &event, &timeout);
        if (has_event) {
            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:
                    redraw = true;
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    running = false;
                    break;
                default:
                    fprintf(stderr, "Unsupported event: %d\n", event.type);
                    break;
            }
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_flip_display();
            redraw = false;
        }
    }


    INFO("Shutting down...");
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
