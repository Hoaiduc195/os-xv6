#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int max_depth = -1; 
int only_dir = 0; 

void print_indent(int level)
{
    for (int i = 0; i < level - 1; i++)
    {
        printf("   ");
    }
    printf("|__ ");
}

void tree_traverse(char *path, int level)
{
    if (max_depth != -1 && level >= max_depth)
        return;

    int fd;
    struct dirent de;
    struct stat st;
    char buf[256];
    char *p;

    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
    {
        fprintf(2, "tree: path too long\n");
        close(fd);
        return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    if (p > buf && *(p - 1) != '/')
        *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
            continue;

        char name[DIRSIZ + 1];
        memmove(name, de.name, DIRSIZ);
        name[DIRSIZ] = 0;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        if (stat(buf, &st) < 0)
        {
            fprintf(2, "tree: cannot stat %s\n", buf); // skip non readable files
            continue;
        }

        if (only_dir && st.type != T_DIR)
            continue;

        print_indent(level + 1);
        printf("%s\n", name);

        if (st.type == T_DIR)
        {
            tree_traverse(buf, level + 1);
        }
    }

    close(fd);
}

void tree(char *path)
{
    struct stat st;
    if (stat(path, &st) < 0)
    {
        fprintf(2, "tree: cannot stat %s\n", path);
        exit(1);
    }

    if (st.type != T_DIR)
    {
        if (!only_dir)
            printf("%s\n", path);
        return;
    }

    printf("%s\n", path);
    tree_traverse(path, 0);
}


int main(int argc, char *argv[])
{
    char *path = ".";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            only_dir = 1;
        }
        else if (strcmp(argv[i], "-L") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(2, "tree: option requires an argument -- L\n");
                exit(1);
            }
            max_depth = atoi(argv[++i]);
            if (max_depth <= 0)
            {
                fprintf(2, "tree: invalid depth: %s\n", argv[i]);
                exit(1);
            }
        }
        else if (argv[i][0] == '-')
        {
            fprintf(2, "tree: invalid option -- %s\n", argv[i]);
            exit(1);
        }
        else
        {
            path = argv[i];
        }
    }

    tree(path);
    exit(0);
}