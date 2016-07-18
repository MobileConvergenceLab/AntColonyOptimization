#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#if __STDC_VERSION__ >= 199901L
#include <getopt.h>
#endif

#include "fon/fon.h"

static void
opt_parser(
            int argc,
            char **argv,
            int *port,
            int *table_value,
            int *flood_period,
            int *allpair_period,
            bool *monitor_flag,
            bool *logger_flag);

int
main(int argc, char **argv)
{

    return 0;
} // main()

static void opt_parser(
            int argc,
            char **argv,
            int *port,
            int *table_value,
            int *flood_period,
            int *allpair_period,
            bool *monitor_flag,
            bool *logger_flag)
{
    int c;
    int exit_value = EXIT_SUCCESS;

    opterr = 0;

    while ((c = getopt (argc, argv, "p:t:f:a:mh:l")) != -1)
    {
        switch (c)
        {
        case 'p':
            *port = atoi(optarg);  
            break;
        case 't':
            *table_value = atoi(optarg);
            break;
        case 'f':
            *flood_period = atoi(optarg);
            break;
        case 'a':
            *allpair_period = atoi(optarg);
            break;
        case 'm':
            *monitor_flag = true;
            break;
        case 'l':
            *logger_flag = true;
            break;
        case '?':
            if (optopt == 'p' || optopt == 't' || optopt == 'f' || optopt == 'a')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            exit_value = EXIT_FAILURE;
        case 'h':
            fprintf(stderr,
                    "Usage: %s [options]\n"
                    "Options:\n"
                    "   -p [Number]            designate the port to communicate a local FON daemon.\n"
                    "   -t [Number]            initiate the pheromone table manually\n"
                    "                          (indexed 0,1,2... N-1)\n"
                    "   -f [milisecond]        activate the advertizing policy(flood packet)\n"
                    "                          to make up the pheromone table with the given period\n"
                    "   -a [milisecond]        activate the all-pair routing policy(oneway packet)\n"
                    "                          to update the pheromone table with the given period\n"
                    "   -m                     activate table monitor\n"
                    "   -l                     activate custom logger(Called whenevere receive an ant)\n"
                    "   -h                     print this menu and exit\n",
                    argv[0]);
            exit(exit_value);
            break;
        default:
            abort ();
        } // switch
    } // while

} // opt_parser()

