#include<stdio.h>
#include <string.h>

struct Record {
    char GAME_DATE_EST[20];
    int TEAM_ID_home;
    int PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    int AST_home;
    int REB_home;
    int HOME_TEAM_WINS; 
};


struct Record createRecord(const char* GAME_DATE_EST, float FG3_PCT_home, float FG_PCT_home) {
    struct Record record;
    strncpy(record.GAME_DATE_EST, GAME_DATE_EST, sizeof(record.GAME_DATE_EST) - 1); // Ensure null-terminated string
    record.GAME_DATE_EST[sizeof(record.GAME_DATE_EST) - 1] = '\0'; // Null-terminate the string
    record.FG3_PCT_home = FG3_PCT_home;
    record.FG_PCT_home = FG_PCT_home;
    return record;
}


const char* getGAME_DATE_EST(const struct Record* record) {
    return record->GAME_DATE_EST;
}


float getFG3_PCT_home(const struct Record* record) {
    return record->FG3_PCT_home;
}


int getFG_PCT_home(const struct Record* record) {
    return record->FG_PCT_home;
}

int getRecordSize() {
    return 11 + sizeof(float) + sizeof(float);
}

void toString(const struct Record* record, char* result, int resultSize) {
    snprintf(result, resultSize, "Record Info: GAME_DATE_EST: %s, FG3_PCT_home: %f, FG_PCT_home %f", record->GAME_DATE_EST, record->FG3_PCT_home, record->FG_PCT_home);
}