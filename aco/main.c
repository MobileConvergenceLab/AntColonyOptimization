#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#if __STDC_VERSION__ >= 199901L
#include <getopt.h>
#endif

#include "main.h"
#include "fon/fon.h"
#include "ant-def.h"
#include "ant.h"

static gboolean table_init_from_daemon(AcoTable *table);
static gboolean init(MainObj *obj, int port);
static gboolean table_manual_insert(AcoTable *table, int number_of_hops);
static gboolean recv_callack(const packet *pkt, MainObj* obj);
static gboolean flood_timeout_event_hlder(MainObj *obj);
static gboolean oneway_timeout_event_hlder(MainObj *obj);
static gboolean forward_timeout_event_hlder(MainObj *obj);
static gboolean logging_timeout_event_hlder(MainObj *obj);
static gboolean table_update_timeout_event_hlder(MainObj *obj);
static gboolean add_timeout_source(MainObj *obj, GSourceFunc event_hdler, guint interval);
static void add_exit(GMainContext *context);

/* the logger is called when received ant */
static void logger(const Ant* ant);


/**
 * Argument parser and handler
 */
static void arg_hndler_table(MainObj *obj, int nhops);
static void arg_hndler_allpair(MainObj *obj, int period);
static void arg_hndler_oneway(MainObj *obj, int period);
static void arg_hndler_monitor(MainObj *obj, bool flag);
static void opt_parser(
            int argc,
            char **argv,
            int *port,
            int *table_value,
            int *flood_period,
            int *allpair_period,
            bool *monitor_flag);

/**
 * Main Function
 */
int main(int argc, char **argv) {

    MainObj  obj[1]             = {{0,}};
    int     port                = FON_CORE_LISTEN_PORT;
    int     table_value         = 0;
    int     flood_period        = 0;
    int     allpair_period      = 0;
    bool    monitor_flag        = false;

    opt_parser(argc,
                argv,
                &port,
                &table_value,
                &flood_period,
                &allpair_period,
                &monitor_flag);

    //g_print("Call init()\n");
    if(!init(obj, port))
    {
        perror("Call init()");
        exit(EXIT_FAILURE);
    }

    ant_callback_logger_set(logger);

    arg_hndler_table(obj, table_value);
    arg_hndler_allpair(obj, flood_period);
    arg_hndler_oneway(obj, allpair_period);
    arg_hndler_monitor(obj, monitor_flag);

    //g_print("Call Ready to run\n");
    g_main_loop_run(obj->loop);

    return 0;
}

/**
 * 코어 데몬으로부터 테이블이 변경되었다는 통보를 받으면
 * 해당 루틴이 호출되어야 한다.
 */
static gboolean table_init_from_daemon(AcoTable *table) {

    int             i;
    int             len;
    GArray     *tuple_array    = NULL;
    table_tuple     *tuple;

    /* wait for the local daemon to make up the forwarding table.
    During this time, the daemon communicates with neighbor daemons. */
    //sleep(2);

    fon_table_get(&tuple_array);
    len = tuple_array->len;
    tuple = &g_array_index(tuple_array, table_tuple, 0);

    for(i=0; i<len; i++) {
        if(tuple[i].hops == HOPS_NEIGH) {
            aco_table_add_col(table, tuple[i].id);
        }
        else {
            aco_table_add_row(table, tuple[i].id);
        }
    }


    g_array_unref(tuple_array);

    return TRUE;
}

static gboolean table_manual_insert(AcoTable *table, int number_of_hops)
{
    int host_id = table->host_id;

    for(int i=0; i<number_of_hops; i++)
    {
        if(host_id != i)
        {
            aco_table_add_row(table, i);
        }
    }

    return TRUE;
}

static gboolean init(MainObj *obj, int port) {
    int host_id = -1;

    obj->context    = g_main_context_new();
    obj->loop       = g_main_loop_new(obj->context, FALSE);
    g_main_context_unref(obj->context);

    //g_print("Call fon_init()\n");
    if(!fon_init(obj->context, FON_FUNC_TYPE_ACO, port)) {
        perror("Call fon_init()");
        exit(EXIT_FAILURE);
    }

    //g_print("Call fon_host_get()\n");
    if(!fon_host_get(&host_id)) {
        perror("Call fon_host_get()");
        exit(EXIT_FAILURE);
    }

    obj->table      = aco_table_new(host_id, PHEROMONE_MIN, PHEROMONE_MAX);

    //g_print("Call table_init_from_daemon()\n");
    if(!table_init_from_daemon(obj->table))
    {
        perror("Call table_init_from_daemon()");
        exit(EXIT_FAILURE);
    }

#if CYCLE_PERIOD_MS&&FORWARD_TARGET
    if(host_id == 0)
    {
        //g_print("Call forward_timeout_add()\n");
        if(!add_timeout_source(obj, (GSourceFunc)forward_timeout_event_hlder, CYCLE_PERIOD_MS))
        {
            perror("Call forward_timeout_add()");
            exit(EXIT_FAILURE);
        }
    }
#endif /* CYCLE_PERIOD_MS */

    if(!add_timeout_source(obj, (GSourceFunc)table_update_timeout_event_hlder, CYCLE_PERIOD_MS))
    {
        perror("Call forward_timeout_add()");
        exit(EXIT_FAILURE);
    }

    //g_print("Call fon_set_callback_recv()\n");
    fon_set_callback_recv((FonCallbackRecv)recv_callack, obj);

    return TRUE;
}

static gboolean recv_callack(const packet *pkt, MainObj* obj) {

    Ant*    ant     = ant_restore(pkt, obj->table);

    ant_callback(ant);
    ant_unref(ant);

    return TRUE;
}

/**
 * 자기를 중심으로 플러딩한다.
 */
static gboolean flood_timeout_event_hlder(MainObj *obj) {

    int dummy   = -1;
    int source  = obj->table->host_id;
    Ant* ant    = ant_factory(ANT_TYPE_FLOOD,
                            source,
                            dummy,          /* dummy destination */
                            obj->table);

    ant_send(ant);
    ant_unref(ant);

    return TRUE;
}

static gboolean oneway_timeout_event_hlder(MainObj *obj)
{
    static int i = 0;
    if(i++ < NUMBER_OF_CYCLES)
    {
        int dummy           = -1;
        int source          = obj->table->host_id;
        Ant* ant            = ant_factory(ANT_TYPE_ONEWAY,
                                            source,
                                            dummy,          /* dummy destination. it will be filled in loop. */
                                            obj->table);
        int* destination    = &ant->obj->destination;
        int* array          = aco_table_new_dests(obj->table);
        int  i;

        i=-1;
        while(array[++i] != -1)
        {
            *destination = array[i];
            ant_send(ant);
        }

        aco_table_free_array(array);
        ant_unref(ant);
    }
    else
    {
        /* add exit event and delete this event source */
        add_exit(obj->context);
        return FALSE;
    }

    return TRUE;
}


static gboolean forward_timeout_event_hlder(MainObj *obj)
{
    static int cycle = 0;
    if(cycle++ < NUMBER_OF_CYCLES)
    {
        int source          = obj->table->host_id;
        Ant* ant            = ant_factory(ANT_TYPE_FORWARD,
                                            source,
                                            FORWARD_TARGET,
                                            obj->table);

        for(int i=0; i< PACKETS_PER_CYCLE; i++)
        {
            ant_send(ant);
        }
        ant_unref(ant);
    }
    else
    {
        add_exit(obj->context);
        return FALSE;
    }

    return TRUE;
}


static gboolean logging_timeout_event_hlder(MainObj *obj)
{
    aco_table_print_all(obj->table);

    return TRUE;
}

static bool __table_decrease_iterator(AcoTable *table, AcoValue *value, void *noused)
{
    value->pheromone *= ANT_REMAINS_RATE;

    return TRUE;
}

static gboolean table_update_timeout_event_hlder(MainObj *obj)
{
    aco_table_iterate_all(obj->table, __table_decrease_iterator, NULL);

    return TRUE;
}

static gboolean add_timeout_source(MainObj *obj, GSourceFunc event_hdler, guint interval)
{
    if(interval < 0)
    {
        return TRUE;
    }

    gint            src_id;
    GSource*        source     = g_timeout_source_new(interval);

    // always success
    g_source_set_callback(  source,
                            (GSourceFunc)event_hdler,
                            obj,
                            NULL);

    src_id = g_source_attach(source, obj->context);
    g_source_unref(source);
    if(src_id < 0) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}


static void add_exit(GMainContext *context)
{
    gint            src_id;
    GSource*        source     = g_timeout_source_new(WAIT_REMAIN_PKT);

    // always success
    g_source_set_callback(  source,
                            (GSourceFunc)exit,
                            EXIT_SUCCESS,
                            NULL);

    src_id = g_source_attach(source, context);
    g_source_unref(source);
    if(src_id < 0) {
        return;
    }
    else {
        return;
    }
}

static void logger(const Ant* ant)
{
    if(ant_object_get_direction(ant->obj) == ANT_OBJ_DIRECTION_BACKWARD)
    {
        ant_object_print_dbg_hops(ant->obj);
    }
}


/**
 * Argument parser and handler
 */
static void arg_hndler_table(MainObj *obj, int nhops)
{
    if(!nhops) return;

    //g_print("Call table_manual_insert()\n");
    if(!table_manual_insert(obj->table, nhops))
    {
        perror("Call table_manual_insert()");
        exit(EXIT_FAILURE);
    }
}

static void arg_hndler_allpair(MainObj *obj, int period)
{
    if(!period) return;

    //g_print("Call flood_timeout_add()\n");
    if(!add_timeout_source(obj, (GSourceFunc)flood_timeout_event_hlder, period))
    {
        perror("Call flood_timeout_add()");
        exit(EXIT_FAILURE);
    }
}

static void arg_hndler_oneway(MainObj *obj, int period)
{
    if(!period) return;

    //g_print("Call oneway_timeout_add()\n");
    if(!add_timeout_source(obj, (GSourceFunc)oneway_timeout_event_hlder, period))
    {
        perror("Call oneway_timeout_add()");
        exit(EXIT_FAILURE);
    }
}

static void arg_hndler_monitor(MainObj *obj, bool flag)
{
    if(!flag) return;

        //g_print("Call logging_timeout_add()\n");
        if(!add_timeout_source(obj, (GSourceFunc)logging_timeout_event_hlder, MONITOR_PERIOD_MS))
        {
            perror("Call logging_timeout_add()");
            exit(EXIT_FAILURE);
        }
}

static void opt_parser(
            int argc,
            char **argv,
            int *port,
            int *table_value,
            int *flood_period,
            int *allpair_period,
            bool *monitor_flag)
{
    int c;
    int exit_value = EXIT_SUCCESS;

    opterr = 0;

    while ((c = getopt (argc, argv, "p:t:f:a:mh")) != -1)
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
                        "   -m                     activate table monitoring\n"
                        "   -h                     print this menu and exit\n",
                        argv[0]);
                exit(exit_value);
                break;
            default:
                abort ();
          }
    }

}

