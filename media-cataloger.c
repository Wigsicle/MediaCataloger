#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

double get_duration(const char *path)
{
    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "ffprobe -v quiet -of csv=p=0 -show_entries format=duration \"%s\"",
        path);

    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    double duration = 0;
    fscanf(fp, "%lf", &duration);
    pclose(fp);

    duration = duration/60;

    return duration;
}

void list_tree(const char *path, int depth, FILE *current_csv)
{

    struct dirent *entry;
    struct stat filestat;
    const size_t size = 1024;

    char buffer[size];

    char fullpath[4096];

    DIR *folder = opendir(path);

    if (folder == NULL)
    {
        perror("Unable to read directory");
        return;
    }

    while ((entry = readdir(folder)))
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s\\%s", path, entry->d_name);

        if (stat(fullpath, &filestat) != 0)
        {
            perror("stat failed");
            continue;
        }

        if (depth == 0 && S_ISDIR(filestat.st_mode))
        {
            char buf[0x100];
            snprintf(buf, sizeof(buf), "%s.csv", entry->d_name);
            FILE *f = fopen(buf, "w+");
            fprintf(f, "File Name, File Size, Rip Location, Season, Episode, Runtime, Folder\n");

            list_tree(fullpath, depth + 1, f);
            fclose(f);
            continue;
        }
        if (depth == 1 && S_ISDIR(filestat.st_mode))
        {
            list_tree(fullpath, depth + 1, current_csv);
            continue;
        }
        if (depth == 2) {
            if (current_csv != NULL)
            {   
                double sec = get_duration(fullpath);
                fprintf(current_csv, "\"%s\", %10lld, %f, %s\n", entry->d_name, filestat.st_size, sec, fullpath); 
            }
        }
    }

    closedir(folder);
}

int main()
{
    char path[] = "\\\\192.168.68.113\\Media\\WIP";

    list_tree(path, 0, NULL);

    return 0;
}
