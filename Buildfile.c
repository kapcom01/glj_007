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

typedef enum {
    LINUX,
    MACOS,
    WINDOWS,
    TARGETS,
} Target;

const char *CC[TARGETS];
const char *compiler_flags[TARGETS];
const char *linker_flags[TARGETS];

const char *output_dir[TARGETS];
const char *output_file[TARGETS];

/****************************************************
 * API functions to use inside the build() function *
 * **************************************************/

int compile_modules(Target);
int link_modules(Target);
int create_static_library(Target);

/*****************************
 *    Build Configuration    *
 * ***************************/

// add c source files here
char *c_sources[] = {
    "source/game",
    "source/map",
};

// set target configuration
void setup_targets() {
    // LINUX

    CC[LINUX] = "cc";
    compiler_flags[LINUX] = "-std=c99 -g3 "
                              "-DPLATFORM_DESKTOP "
                              "-Iassets "
                              "-Iraylib-5.5_linux_amd64/include";
    linker_flags[LINUX] = "-lm raylib-5.5_linux_amd64/lib/libraylib.a";

    output_dir[LINUX] = "build";
    output_file[LINUX] = "game.exe";

    // MACOS

    CC[MACOS] = "clang";
    compiler_flags[MACOS] = "-std=c99 -g3 -Wno-switch "
                            "-DPLATFORM_DESKTOP "
                            "-I assets "
                            "-I raylib-5.5_macos/include";
    linker_flags[MACOS] = "-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL "
                          "raylib-5.5_macos/lib/libraylib.a";

    output_dir[MACOS] = "build";
    output_file[MACOS] = "game.exe";
}

/****************************************************
 * Use the build() function to build your project.  *
 * You can call the API functions declared above.   *
 ****************************************************/

int build(Target target) {
    return compile_modules(target)
        || link_modules(target);
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
                    "[ERROR  ] Error updating timestamps for %s\n",
                    buildfile_exe_name);
            ret = true;
        }
    }

    return ret;
}

int self_rebuild(const char *buildfile_exe_name) {
    printf("[INFO   ] Self Rebuilding...\n");
    char rebuild_command[1024] = {0};
    snprintf(rebuild_command, sizeof(rebuild_command),
             "cc -std=c99 -Wall -Wpedantic -Wextra -Werror -o %s %s",
             buildfile_exe_name,
             BUILDFILE_NAME);
    return system(rebuild_command);
}

int compile_modules(Target t) {
    int ret = 0;
    for(size_t i = 0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        char compile_command[1024] = {0};
        snprintf(compile_command, sizeof(compile_command),
                 "%s %s -c %s.c -o %s.o",
                 CC[t],
                 compiler_flags[t],
                 c_sources[i],
                 c_sources[i]);
        char delete_command[256] = {0};
        snprintf(delete_command, sizeof(delete_command),
                 "rm -f %s.o",
                 c_sources[i]);
        system(delete_command);
        printf("[INFO   ] %s\n", compile_command);
        ret += system(compile_command);
    }
    return ret;
}

int link_modules(Target t) {
    if (mkdir(output_dir[t], S_IRWXU)) {
        switch (errno) {
        case EEXIST:
            break;
        default:
            fprintf(stderr, "[ERROR  ] %s: %s\n", output_dir[t], strerror(errno));
            return 1;
        }
    }

    char link_command[1024] = {0};
    snprintf(link_command, sizeof(link_command),
             "%s %s -o %s/%s",
             CC[t],
             compiler_flags[t],
             output_dir[t],
             output_file[t]);
    for (size_t i = 0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        strncat(link_command, " ",          strlen(link_command));
        strncat(link_command, c_sources[i], strlen(link_command));
        strncat(link_command, ".o",         strlen(link_command));
    }
    strncat(link_command, " ",          strlen(link_command));
    strncat(link_command, linker_flags[t], strlen(link_command));
    printf("[INFO   ] %s\n", link_command);
    return system(link_command);
}

int create_static_library(Target t) {
    char ar_command[1024] = {0};
    snprintf(ar_command, sizeof(ar_command), "ar rcs %s", output_file[t]);
    for (size_t i=0; i < sizeof(c_sources)/sizeof(c_sources[0]); i++) {
        strncat(ar_command, " ",           strlen(ar_command));
        strncat(ar_command, c_sources[i], strlen(ar_command));
        strncat(ar_command, ".o",         strlen(ar_command));
    }
    printf("[INFO   ] %s\n", ar_command);
    return system(ar_command);
}

int main(int argc, char *argv[]) {
    if (argc == 0) return 1;
    if (self_rebuild_is_necessary(argv[0])) {
        int err = 0;
        if ((err = self_rebuild(argv[0]))) {
            fprintf(stderr, "failed to rebuild \n");
            return err;
        }
        // self-run
        return system(argv[0]);
    }

    setup_targets();

#if defined(__APPLE__)
    return build(MACOS);
#elif defined(__linux__)
    return build(MACOS);
#else
    #error "Target platformed not detected correctly"
#endif
}
