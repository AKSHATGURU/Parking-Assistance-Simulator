/* Parking Assistance Simulator (Dev-C++ friendly - C89 compatible)
   Updated to avoid declaration-after-statement errors on older compilers (Dev-C++ / MinGW).
   - Moves variable declarations to the top of blocks
   - Avoids declaring loop variables inside for() initializers

   Usage:
     - Open in Dev-C++, save as parking_assist.c
     - Build & Run in Dev-C++ (no special compiler flags needed)
     - Or compile with gcc: gcc -o parking_assist parking_assist.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Cross-platform sleep in milliseconds */
void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

/* Get current timestamp string (YYYY-MM-DD HH:MM:SS) */
void current_timestamp(char *buffer, size_t len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* Log a reading to CSV file */
void log_reading(const char *filename, const char *timestamp, float distance_cm) {
    FILE *f = fopen(filename, "a");
    if (!f) {
        fprintf(stderr, "Warning: could not open log file %s for writing.\n", filename);
        return;
    }
    fprintf(f, "%s,%.2f\n", timestamp, distance_cm);
    fclose(f);
}

/* Classify distance into status message */
const char* classify_distance(float d) {
    if (d < 0) return "INVALID";
    if (d < 50.0f) return "STOP";       /* very close */
    if (d < 100.0f) return "CAUTION";   /* approaching */
    return "SAFE";                      /* far enough */
}

/* Print a simple text-based bar to visualise distance */
void print_distance_bar(float d) {
    /* Declarations at the top for older C compilers */
    int max_bar = 40; /* characters when distance >= 200cm */
    float capped = d;
    int filled;
    int i;

    if (capped > 200.0f) capped = 200.0f;
    filled = (int)((capped / 200.0f) * max_bar);

    printf("[");
    for (i = 0; i < filled; ++i) putchar('#');
    for (i = filled; i < max_bar; ++i) putchar(' ');
    printf("] \n");
}

int main() {
    const char *logfile = "parking_log.csv";
    FILE *fh;

    /* Ensure log file exists and has header */
    fh = fopen(logfile, "r");
    if (!fh) {
        fh = fopen(logfile, "w");
        if (fh) {
            fprintf(fh, "timestamp,distance_cm\n");
            fclose(fh);
        }
    } else {
        fclose(fh);
    }

    printf("\n========================================\n");
    printf("   ||  PARKING ASSISTANCE SIMULATOR  ||\n");
    printf("   ||            (C Code)            ||\n");
    printf("========================================\n\n");

    /* Mode selection */
    int mode = 0;
    printf("Choose mode:\n 1) Manual input\n 2) Automatic simulation\n");
    printf("Enter choice (1 or 2): ");
    if (scanf("%d", &mode) != 1) {
        fprintf(stderr, "Invalid input. Exiting.\n");
        return 1;
    }

    if (mode == 1) {
        printf("Manual mode selected. Enter distance in cm (negative to exit).\n");
        while (1) {
            float dist;
            printf("Enter distance (cm): ");
            if (scanf("%f", &dist) != 1) {
                /* clear stdin in case of garbage */
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                printf("Please enter a number.\n");
                continue;
            }
            if (dist < 0) {
                printf("Exiting manual mode.\n");
                break;
            }
            const char *status = classify_distance(dist);
            char ts[64];
            current_timestamp(ts, sizeof(ts));
            printf("[%s] Distance: %.2f cm -> %s\n", ts, dist, status);
            print_distance_bar(dist);
            log_reading(logfile, ts, dist);
        }
    } else if (mode == 2) {
        printf("Automatic simulation mode. Press Ctrl+C to stop.\n");
        /* Seed random */
        srand((unsigned int)time(NULL));

        while (1) {
            float base;
            /* Simulate distance between 10cm and 250cm with occasional close events */
            base = (rand() % 241) + 10; /* 10..250 */
            /* Create occasional dip (simulate obstacle approaching) */
            if ((rand() % 10) < 3) {
                base = (float)(rand() % 80); /* 0..79 */
            }
            const char *status = classify_distance(base);
            char ts[64];
            current_timestamp(ts, sizeof(ts));
            printf("[%s] Distance: %.2f cm -> %s\n", ts, base, status);
            print_distance_bar(base);
            log_reading(logfile, ts, base);
            sleep_ms(800); /* 800 ms between readings */
        }
    } else {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    printf("Simulation ended. Log file: %s\n", logfile);
    return 0;
}

