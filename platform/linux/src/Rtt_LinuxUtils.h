#ifndef Rtt_LinuxUtils
#define Rtt_LinuxUtils

#include <stdlib.h>
#include <string>
#include <vector>

int SortVectorByName(std::string a, std::string b);

const char* GetStartupPath(std::string* exeFileName);
const char* GetHomePath();
char* CalculateMD5(std::string filename);
std::string GetRecentDocsPath();
bool ReadRecentDocs(std::vector<std::pair<std::string, std::string>>& recentDocs);
void UpdateRecentDocs(const std::string& appName, const std::string& path);

std::string& ltrim(std::string& str, const std::string& chars);
std::string& rtrim(std::string& str, const std::string& chars);
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
void splitString(std::vector<std::string>& cont, const std::string& str, const std::string& delims);

#endif // Rtt_LinuxUtils
