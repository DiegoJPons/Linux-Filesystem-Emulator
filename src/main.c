#include <stdio.h>
#include <string.h>

#include "filesys.h"

#include "debug.h"

#include <assert.h>


void wait_for_input()
{
    printf("Press enter to continue: ");
    getchar();
}

int main()
{
    filesystem_t fs;
    FILE *f = fopen("input/large.bin", "r");
    load_filesystem(f, &fs);
    fclose(f);

    inode_shrink_data(&fs, &fs.inodes[1], 0);

    FILE *n = fopen("tests/output/ShrinkComplete1.bin", "w");
    save_filesystem(n, &fs);
    fclose(n);

    return 0;
}