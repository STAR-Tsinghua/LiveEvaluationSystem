#ifndef _DTP_ARGP_H_
#define _DTP_ARGP_H_
#include <inttypes.h>
#include <argp.h>
#include <string.h>
#include "../../submodule/DTP/include/quiche.h"
#include "helper.h"
/***** Argp configs *****/

static bool TOS_ENABLE = false;
static bool MULIP_ENABLE = false;
static int ip_cfg[8] = {0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0};

#ifndef DEFAULT
#define DEFAULT 1
#endif // DEFAULT

#if DEFAULT
const char *argp_program_version = "dtp-server 0.0.1";
static char doc[] = "live dtp server for test";
static char args_doc[] = "SERVER_IP SERVER_PORT [CONFIG_FILE]";
#define ARGS_NUM 3
#endif // if default

static bool FEC_ENABLE = false;
static uint32_t TAILSIZE = 5;
static double RATE = 0.1;
static enum quiche_cc_algorithm CC = QUICHE_CC_CUBIC;
static char SCHEDULER[100] = "dyn";
static enum log_level DEBUG_LEVEL = Debug; // debug

static struct argp_option options[] = {
    // {"log", 'l', "FILE", 0, "log file with debug and error info"},
    // {"out", 'o', "FILE", 0, "output file with received file info"},
    // {"log-level", 'v', "LEVEL", 0, "log level ERROR 1 -> TRACE 5"},
    // {"color", 'c', 0, 0, "log with color"},
    {"tos", 't', 0, 0, "set tos"},
    {"mulip", 'm', 0, 0, "set multi ip"},

    {"fec_enable", 'f', 0, 0, "enable fec"},
    {"tailsize", 'z', "TAILSIZE", 0, "set tail size"},
    {"rate", 'r', "RATE", 0, "set redundancy rate"},
    {"cc-algorithm", 'a', "CC", 0, "set the cc algorithm ['reno', 'bbr', 'cubic']"},
    {"debug-level", 'L', "LEVEL", 0, "set debug logging level ['off', 'error', 'warn', 'info', 'debug', 'trace']"},
    {"scheduler-algorithm", 's', "SCHEDULER", 0, "set scheduler algorithm ['basic', 'dtp']"},

    {0}
};

struct arguments {
    FILE *log;
    FILE *out;
    char *args[ARGS_NUM];
};

static struct arguments args;

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = (struct arguments*) state->input;
    switch (key) {
        case 'l':
            arguments->log = fopen(arg, "w+");
            break;
        case 'o':
            arguments->out = fopen(arg, "w+");
            break;
        case 'v':
            LOG_LEVEL = arg ? atoi(arg) : 3;
            break;
        case 'c':
            LOG_COLOR = 1;
            break;
        case 't':
            TOS_ENABLE = true;
            break;
        case 'm':
            MULIP_ENABLE = true;
            break;
        case 'f':
            FEC_ENABLE = true;
            fprintf(stderr, "FEC enabled\n");
            break;
        case 'z':
            TAILSIZE = atoi(arg);
            fprintf(stderr, "set tail size to %d\n", TAILSIZE);
            break;
        case 'r':
            RATE = atof(arg);
            fprintf(stderr, "set redundancy rate to %f\n", RATE);
            break;
        case 'a':
            if(strcmp(arg, "reno") == 0) {
                CC = QUICHE_CC_RENO;               
                fprintf(stderr, "set cc reno\n");
            } else if (strcmp(arg, "bbr") == 0) {
                CC = QUICHE_CC_BBR;
                fprintf(stderr, "set cc bbr\n");
            } else if (strcmp(arg, "cubic") == 0) {
                CC = QUICHE_CC_CUBIC;
                fprintf(stderr, "set cc cubic\n");
            } else {
                fprintf(stderr, "default cc algo reno\n");
                CC = QUICHE_CC_RENO;
            }
            break;
        case 's':
            if(strcmp(arg, "basic") == 0 || strcmp(arg, "dtp") == 0) {
                strcpy(SCHEDULER, arg);
                fprintf(stderr, "copy scheduler %s\n", SCHEDULER);
            } else {
                fprintf(stderr, "default scheduler dyn\n");
            }
            break;
        case 'L': {
           if(!(strcmp(arg, "5") && strcmp(arg, "off"))) {
                DEBUG_LEVEL = log_level::Off;
                fprintf(stderr, "set debug logging level: Off\n");
            } else if (!(strcmp(arg, "4") && strcmp(arg, "error"))) {
                DEBUG_LEVEL = log_level::Error;
                fprintf(stderr, "set debug logging level: Error\n");
            } else if (!(strcmp(arg, "3") && strcmp(arg, "warn"))) {
                DEBUG_LEVEL = log_level::Warn;
                fprintf(stderr, "set debug logging level: Warn\n");
            } else if (!(strcmp(arg, "2") == 0 && strcmp(arg, "info"))) {
                DEBUG_LEVEL = log_level::Info;
                fprintf(stderr, "set debug logging level: Info\n");
            } else if (!(strcmp(arg, "1") == 0 && strcmp(arg, "debug"))) {
                DEBUG_LEVEL = log_level::Debug;
                fprintf(stderr, "set debug logging level: Debug\n");
            } else if (!(strcmp(arg, "0") == 0 && strcmp(arg, "trace"))) {
                DEBUG_LEVEL = log_level::Trace;
                fprintf(stderr, "set debug logging level: Trace\n");
            } else {
                DEBUG_LEVEL = log_level::Debug;
                fprintf(stderr, "set debug logging level: Debug\n"); 
            }
            break;
        }
        case ARGP_KEY_ARG:
            if (state->arg_num >= ARGS_NUM) argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < ARGS_NUM - 1) argp_usage(state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

void dtp_argp_set_config(quiche_config* config) {
    quiche_config_set_cc_algorithm(config, CC);
    quiche_config_set_scheduler_name(config, SCHEDULER);
    if(FEC_ENABLE) {
        quiche_config_set_init_tail_size(config, TAILSIZE);
        quiche_config_set_redundancy_rate(config, RATE);
    } else {
        quiche_config_set_redundancy_rate(config, 0);
    }
    quiche_set_debug_logging_level(DEBUG_LEVEL);
}

struct dtp_ext_struct {
    int *socks;
    int ai_family;
};
#endif // _DTP_ARGP_H_