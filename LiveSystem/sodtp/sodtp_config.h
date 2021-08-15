#ifndef SODTP_CONFIG_H
#define SODTP_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
// #include <ctype.h>
#include <string>
#include <vector>

class LogConfig
{
public:
    std::string logPathFather;

    void parse(const char *filename) {
        FILE *fd = NULL;

        fd = fopen(filename, "r");
        if (fd == NULL) {
            printf("fail to open config file.");
            return;
        }

        char data[128];
        while (fscanf(fd, "%s", data) == 1) {
            if (data[0] == '#') {
                continue;
            }
            std::string str = data;
            logPathFather = str;
        }
    }
};

class StreamConfig
{
public:
    std::vector<std::string> files;

    void parse(const char *filename) {
        FILE *fd = NULL;

        fd = fopen(filename, "r");
        if (fd == NULL) {
            printf("fail to open config file.");
            return;
        }

        char data[128];
        while (fscanf(fd, "%s", data) == 1) {
            if (data[0] == '#') {
                continue;
            }
            std::string str = data;
            files.push_back(str);
        }

        // printf("stream number: %lu\n", files.size());
        // for (auto &it : files) {
        //     printf("stream: %s\n", it.c_str());
        // }
    }
};

class SaveConfig
{
public:
    bool save;
    std::string path;

    void parse(const char *filename) {
        FILE *fd = NULL;
        save = false;

        fd = fopen(filename, "r");
        if (fd == NULL) {
            printf("fail to open config file.");
            return;
        }

        int val = 0;
        char data[128];
        if (fscanf(fd, "%d", &val) == 1) {
            save = (val != 0);
        }
        while (fscanf(fd, "%s", data) == 1) {
            if (data[0] == '#') {
                continue;
            }
            path = data;
            break;
        }
    }
};

#endif // SODTP_CONFIG_H