#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
Experiment 1: store the data (which is about NBA games and described in Part 4) on the disk (as specified in Part 1) and report the following statistics:
• the number of records;
• the size of a record;
• the number of records stored in a block;
• the number of blocks for storing the data;
 */

#define MAX_RECORDS 1957764 // Adjusted based on disk capacity
#define MAX_LINE_LENGTH 350 // Adjusted to fit within a block
#define FILENAME "games.txt" // Adjust the filename as needed

typedef struct {
    char GAME_DATE_EST[20];
    int TEAM_ID_home;
    int PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    int AST_home;
    int REB_home;
    int HOME_TEAM_WINS;
} GameRecord;

GameRecord records[MAX_RECORDS];
int numRecords = 0;

// Function to read and parse data from a text file
void readDataFromFile() {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    // Skip the header line
    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Error reading header line");
        fclose(file);
        exit(1);
    }

    while (fgets(line, sizeof(line), file)) {
        if (numRecords >= MAX_RECORDS) {
            printf("Max number of records reached.\n");
            break;
        }
        sscanf(line, "%s %d %d %f %f %f %d %d %d",
            records[numRecords].GAME_DATE_EST,
            &records[numRecords].TEAM_ID_home,
            &records[numRecords].PTS_home,
            &records[numRecords].FG_PCT_home,
            &records[numRecords].FT_PCT_home,
            &records[numRecords].FG3_PCT_home,
            &records[numRecords].AST_home,
            &records[numRecords].REB_home,
            &records[numRecords].HOME_TEAM_WINS
        );
        numRecords++;
    }

    fclose(file);
}

int main() {
    readDataFromFile();

    // Calculate the requested statistics
    int recordSize = sizeof(GameRecord);
    int recordsPerBlock = 400 / recordSize; // Assuming each block is 400 bytes
    int numBlocks = (numRecords + recordsPerBlock - 1) / recordsPerBlock;

    // Print the statistics
    printf("Number of Records: %d\n", numRecords);
    printf("Size of a Record: %d bytes\n", recordSize);
    printf("Records Stored in a Block: %d\n", recordsPerBlock);
    printf("Number of Blocks for Storing the Data: %d\n", numBlocks);

    return 0;
}