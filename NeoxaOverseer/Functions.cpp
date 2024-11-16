#include "Libraries.cpp"

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static string extractId64(const string& line)
{
    regex id64Regex("\\d{17}");
    smatch match;
    if (regex_search(line, match, id64Regex))
    {
        return match[0];
    }
    return "";
}

static bool MakeFile(string FileName)
{
    ofstream OutputFile(FileName);

    if (OutputFile.is_open())
    {
        OutputFile.close();
        return true;
    }
    else
    {
        cerr << "Error creating the file " << FileName << endl;
        return false;
    }
}

static void OutputToFile(vector<string>BannedIds, vector<string> PrivateIds)
{
    ofstream BannedIdsFile("BannedIds.txt");
    ofstream PrivateIdsFile("PrivateIds.txt");

    if (!BannedIds.empty())
    {
        for (int i{ 0 }; i < BannedIds.size(); i++)
        {
            BannedIdsFile << BannedIds.at(i) << endl;
        }
    }

    BannedIdsFile.close();

    if (!PrivateIds.empty())
    {
        for (int i{ 0 }; i < PrivateIds.size(); i++)
        {
            PrivateIdsFile << PrivateIds.at(i) << endl;
        }
    }

    PrivateIdsFile.close();
}