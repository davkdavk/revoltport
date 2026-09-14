#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define _XBOX360
#include "../../rvsource/Xbox/Src/load.cpp"
#undef fopen

char LastFile[MAX_PATH];
static char log_path[] = "D:\\revolt.log";
char *DBG_LogFile = log_path;
static std::vector<std::string> opened;
static unsigned log_calls;
static bool fail_log_open;
static bool fail_asset_open;

FILE *rv_test_open(const char *filename, const char *)
{
    opened.push_back(filename);
    if ((!_stricmp(filename, "game:\\revolt.log") && fail_log_open) ||
        (!_stricmp(filename, "game:\\cars\\rc\\Parameters.txt") && fail_asset_open))
        return NULL;
    FILE *file = tmpfile();
    assert(file);
    return file;
}

void WriteLogEntry(char *)
{
    // Fail quickly on the old recursive-open bug, rather than exhausting the
    // host stack or opening thousands of descriptors.
    assert(++log_calls == 1);
    FILE *file = BKK_fopen(DBG_LogFile, "a");
    if (file) fclose(file);
}

int main()
{
    const char *log_names[] = {"D:\\revolt.log", "d:\\REVOLT.LOG", "game:\\revolt.log"};
    for (unsigned i = 0; i < 3; ++i) {
        FILE *file = BKK_fopen(log_names[i], "w");
        assert(file && log_calls == 0);
        fclose(file);
    }
    for (unsigned failure = 0; failure < 4; ++failure) {
        fail_log_open = (failure & 1) != 0;
        fail_asset_open = (failure & 2) != 0;
        opened.clear();
        log_calls = 0;
        FILE *file = BKK_fopen("D:\\cars\\rc\\Parameters.txt", "rb");
        assert((file == NULL) == fail_asset_open);
        if (file) fclose(file);
        assert(log_calls == 1 && opened.size() == 2);
        assert(opened[0] == "game:\\cars\\rc\\Parameters.txt");
        assert(opened[1] == "game:\\revolt.log");
        assert(!strcmp(LastFile, "game:\\cars\\rc\\Parameters.txt"));
    }
    log_calls = 0;
    fail_log_open = false;
    FILE *file = BKK_wfopen(L"T:\\revolt.log", L"a");
    assert(file && log_calls == 0);
    fclose(file);
    opened.clear();
    assert(BKK_fopen(NULL, "r") == NULL && errno == EINVAL);
    assert(BKK_fopen("D:\\revolt.log", NULL) == NULL && errno == EINVAL);
    assert(opened.empty());
    puts("load.cpp: log recursion and read-only failure tests passed");
}
