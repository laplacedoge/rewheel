#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define RECV_BUFF_SIZE  1024

const char help_str[] = 
    "Read from standard input and write to standard output and files\n"
    "Usage: tee [OPTION]... [FILE]...\n"
    "\n"
    "    -a    append to the given FILEs, do not overwrite\n";

int main(int argc, char **argv)
{
    int f_append = 0;
    int f_help = 0;
    int file_num;
    int index;
    int ret;
    int ch;

    opterr = 0;
    while ((ch = getopt(argc, argv, "ah")) != -1)
    {
        switch (ch)
        {
            case 'a':
            {
                f_append = 1;
                break;
            }
            case 'h':
            {
                f_help = 1;
                break;
            }
            case '?':
            {
                if (isprint(optopt))
                {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr, "Unknown option '\\x%02X'.\n", optopt);
                }
            }
            default:
            {
                break;
            }
        }
    }
    if (f_help)
    {
        puts(help_str);
        return 0;
    }
    file_num = argc - optind;
    if (file_num > 0)
    {
        int *fds;
        uint8_t recv_buff[RECV_BUFF_SIZE];
        size_t desired_size;
        ssize_t actual_size;

        fds = (int *)malloc(sizeof(int) * file_num);
        if (fds < 0)
        {
            fprintf(stderr, "Failed to allocate memory.\n");
            return -1;
        }
        for (index = optind; index < argc; index++)
        {
            if (f_append)
            {
                fds[index] = open(argv[index], O_CREAT | O_WRONLY | O_APPEND, 0666);
            }
            else
            {
                fds[index] = open(argv[index], O_CREAT | O_WRONLY, 0666);
            }
            if (fds[index] < 0)
            {
                fprintf(stderr, "Failed to open file '%s'.", argv[index]);
                ret = -1;
                goto close_fds;
            }
        }
        desired_size = RECV_BUFF_SIZE;
        while ((actual_size = read(STDIN_FILENO, recv_buff, desired_size)) > 0)
        {
            write(STDOUT_FILENO, recv_buff, actual_size);
            for (index = optind; index < argc; index++)
            {
                write(fds[index], recv_buff, actual_size);
            }
        }
        index = file_num;
        ret = 0;
close_fds:
        for (int idx = 0; idx < index; idx++)
        {
            close(fds[idx]);
        }
        free(fds);
        return ret;
    }
    return 0;
}
