#include <stdio.h>

int main() {
    FILE *file = fopen("games.txt", "r"); // replace "input.txt" with your file name if different
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    int count = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }

    fclose(file);

    // If the file isn't empty and doesn't end with a newline, we should account for the last line
    if (ch != '\n' && ch != EOF) {
        count++;
    }

    printf("The file contains %d rows.\n", count);
    return 0;
}
