#define BUILDFILE_NAME "Buildfile.c"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <utime.h>
#include <errno.h>

/****************************************************
 * API functions to use inside the build() function *
 * **************************************************/

int compile_modules(void);
int link_modules(void);
int create_static_library(void);

/*****************************
 *    Build Configuration    *
 * ***************************/

const char CC[] = "cc";
const char compiler_flags[] = "-std=c99 -g3 "
                              "-DPLATFORM_DESKTOP "
                              "-Iinclude -I/home/kapcom01/Workspace/raylib/src/Build";
const char linker_flags[] = "-lraylib "
                            "-lm -L/home/kapcom01/Workspace/raylib/src/Build"; // -lm must be before -lraylib (why?)

const char *c_sources[] = {
    // add c source files here
    "game",
    "map"
};

const char output_file[] = "exe";

/****************************************************
 * Use the build() function to build your project   *
 ****************************************************/

int build() {
    compile_modules();
    link_modules();
    return 0;
}

/************************************************
 *     No need to change anything below         *
 ************************************************/

bool self_rebuild_is_necessary(const char *buildfile_exe_name) {
    bool ret = false;
    struct stat buildfile_exe_stat;
    struct stat buildfile_stat;
    
    if (stat(buildfile_exe_name, &buildfile_exe_stat) != 0) {
        fprintf(stderr, "Could not stat: %s\n", buildfile_exe_name);
        ret = true;
    }

    if (stat(BUILDFILE_NAME, &buildfile_stat) != 0) {
        fprintf(stderr, "Could not stat: %s\n", BUILDFILE_NAME);
        ret = true;
    }

    if (difftime(buildfile_stat.st_mtime, buildfile_exe_stat.st_mtime) > 0) {
        ret = true;
    }

    // update timestamp of Buildfile executable
    if (utime(buildfile_exe_name, NULL) == -1) {
        if (errno) {
            fprintf(stderr,
                    "[%s ] Error updating timestamps for %s\n",
                    buildfile_exe_name,
                    buildfile_exe_name);
            ret = true;
        }
    }

    return ret;
}

int self_rebuild(const char *buildfile_exe_name) {
    printf("[%s] Self Rebuilding...\n", BUILDFILE_NAME);
    char rebuild_command[1024] = {0};
    snprintf(rebuild_command, sizeof(rebuild_command),
             "cc -std=c99 -Wall -Wpedantic -Wextra -Werror -o %s %s",
             buildfile_exe_name,
             BUILDFILE_NAME);
    return system(rebuild_command);
}

int self_run(const char *buildfile_exe_name) {
    char run_buildfile_command[1024] = {0};
    snprintf(run_buildfile_command, sizeof(run_buildfile_command),
             "./%s",
             buildfile_exe_name);
    return system(run_buildfile_command);
}

int compile_modules(void) {
    for(size_t i = 0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        char compile_command[1024] = {0};
        snprintf(compile_command, sizeof(compile_command),
                 "%s %s -c %s.c -o %s.o",
                 CC,
                 compiler_flags,
                 c_sources[i],
                 c_sources[i]);
        char delete_command[256] = {0};
        snprintf(delete_command, sizeof(delete_command),
                 "rm -f %s.o",
                 c_sources[i]);
        system(delete_command);
        printf("[Compiling  ] %s\n", compile_command);
        system(compile_command);
    }
    return 0;
}

int link_modules(void) {
    char link_command[1024] = {0};
    snprintf(link_command, sizeof(link_command),
             "%s %s -o %s",
             CC,
             compiler_flags,
             output_file);
    for (size_t i = 0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        strncat(link_command, " ",          strlen(link_command));
        strncat(link_command, c_sources[i], strlen(link_command));
        strncat(link_command, ".o",         strlen(link_command));
    }
    strncat(link_command, " ",          strlen(link_command));
    strncat(link_command, linker_flags, strlen(link_command));
    printf("[Linking    ] %s\n", link_command);
    system(link_command);
    return 0;
}

int create_static_library(void) {
    char ar_command[1024] = {0};
    snprintf(ar_command, sizeof(ar_command), "ar rcs %s", output_file);
    for (size_t i=0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        strncat(ar_command, " ",           strlen(ar_command));
        strncat(ar_command, c_sources[i], strlen(ar_command));
        strncat(ar_command, ".o",         strlen(ar_command));
    }
    printf("[Archiving  ] %s\n", ar_command);
    system(ar_command);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 0) return 1;
    if (self_rebuild_is_necessary(argv[0])) {
        int err = 0;
        if ((err = self_rebuild(argv[0]))) {
            fprintf(stderr, "failed to rebuild \n");
            return err;
        }
        return self_run(argv[0]);
    }

    return build();
}
