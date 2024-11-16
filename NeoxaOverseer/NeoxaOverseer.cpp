#include "Libraries.cpp"
#include "Functions.cpp"

using json = nlohmann::json;
using namespace std;

int main()
{
    string url;
    string apiKey;  // Replace with your Steam API key

    vector<string>PrivateIds;
    vector<string>BannedIds;

    char DisplayChoice{ ' ' };

    CURL* curl;
    curl = curl_easy_init();
    CURLcode res;
    string readBuffer;

    cout << "\n******************************************\n";
    cout << "\tWelcome to NeoxaOverseer\n";
    cout << "******************************************\n\n";

    ifstream ApiKeyFile("SteamApiKey.txt");

    if (ApiKeyFile.is_open())
    {
        ApiKeyFile >> apiKey;
        ApiKeyFile.close();
    }
    else
    {
        cout << "Api key file not detected, making one...\n\n";

        if (MakeFile("SteamApiKey.txt"))
        {
            cout << "Api key file has been made, kindly enter your steam api key in it and restart the program\n";
            system("pause");
            return 0;
        }
        else
        {
            curl_easy_cleanup(curl);
            system("pause");
            return 0;
        }
    }

    DisplayChoiceMenu:
    cout << "Would you like to display the ids that are being checked? (Y/N): ";
    DisplayChoice = _getch();
    cout << "\n\n";

    DisplayChoice = toupper(DisplayChoice);

    bool DisplayId;

    if (DisplayChoice != 'Y' && DisplayChoice != 'N')
    {
        cout << "\nInvalid option selected, Try again\n";
        goto DisplayChoiceMenu;
    }
    else
    {
        if (DisplayChoice == 'Y')
        {
            DisplayId = true;
        }
        else
        {
            DisplayId = false;
        }
    }

    // Check if the file exists
    ifstream InputFile("steamid.txt");

    if (InputFile.is_open())
    {
        vector<string> lines;
        string line;

        while (getline(InputFile, line))
        {
            lines.push_back(line);
        }
        InputFile.close();

        if (lines.empty())
        {
            cerr << "Steam ID is empty.\n";
            system("pause");
            return 1;
        }
        else
        {
            for (const auto& line : lines)
            {
                string id64 = extractId64(line);

                if (!id64.empty())
                {
                    if (DisplayId)
                    {
                        cout << "For id " << id64;
                    }

                    // Check profile visibility using GetPlayerSummaries API

                    url = "http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=" + apiKey + "&steamids=" + id64;

                    if (curl)
                    {
                        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

                        readBuffer.clear();

                        res = curl_easy_perform(curl);
                        if (res != CURLE_OK)
                        {
                            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                        }
                        else
                        {
                            try
                            {
                                // Parse JSON response for profile visibility
                                auto jsonResponse = json::parse(readBuffer);
                                int visibilityState = jsonResponse["response"]["players"][0]["communityvisibilitystate"];

                                if (visibilityState == 3)
                                {
                                    if (DisplayId)
                                    {
                                        cout << " the profile is public.\n";
                                    }
                                }
                                else if (visibilityState == 1)
                                {
                                    if (DisplayId)
                                    {
                                        cout << " the profile is private.\n";
                                    }
                                    PrivateIds.push_back(id64);
                                }
                                else
                                {
                                    cout << " unknown visibility state: " << visibilityState << endl;
                                    PrivateIds.push_back(id64);
                                }
                            }
                            catch (const json::exception& e)
                            {
                                cerr << "\nJSON parsing error: " << e.what() << endl;
                            }
                        }
                    }

                    // Now check for VAC and Game bans using GetPlayerBans API

                    url = "http://api.steampowered.com/ISteamUser/GetPlayerBans/v1/?key=" + apiKey + "&steamids=" + id64;

                    if (curl)
                    {
                        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

                        readBuffer.clear();

                        res = curl_easy_perform(curl);

                        if (res != CURLE_OK)
                        {
                            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                        }
                        else
                        {
                            try
                            {
                                // Parse JSON response for VAC and game bans
                                auto jsonResponse = json::parse(readBuffer);
                                auto playerBans = jsonResponse["players"][0];

                                int numberOfVacBans = 0;
                                if (playerBans.contains("NumberOfVACBans"))
                                {
                                    numberOfVacBans = playerBans["NumberOfVACBans"].get<int>();
                                }

                                int daysSinceLastBan = 0;
                                if (playerBans.contains("DaysSinceLastBan"))
                                {
                                    daysSinceLastBan = playerBans["DaysSinceLastBan"].get<int>();
                                }

                                int numberOfGameBans = 0;
                                if (playerBans.contains("NumberOfGameBans"))
                                {
                                    numberOfGameBans = playerBans["NumberOfGameBans"].get<int>();
                                }


                                // Check VAC Ban
                                if (numberOfVacBans > 0)
                                {
                                    if (DisplayId)
                                    {
                                        cout << "VAC Ban: Yes, " << numberOfVacBans << " bans, last ban " << daysSinceLastBan << " ago." << endl;
                                    }
                                    if (daysSinceLastBan < 365)
                                    {
                                        BannedIds.push_back(id64);
                                    }
                                }
                                else
                                {
                                    if (DisplayId)
                                    {
                                        cout << "VAC Ban: No" << endl;
                                    }
                                }

                                // Check Game Ban
                                if (numberOfGameBans > 0)
                                {
                                    if (DisplayId)
                                    {
                                        // Game Ban if more than 0 bans
                                        cout << "Game Ban: Yes, " << numberOfGameBans << " bans." << endl;
                                    }
                                    if (daysSinceLastBan < 365)
                                    {
                                        BannedIds.push_back(id64);
                                    }
                                }
                                else
                                {
                                    if (DisplayId)
                                    {
                                        cout << "Game Ban: No" << endl;
                                    }
                                }

                            }
                            catch (const json::exception& e)
                            {
                                cerr << "JSON parsing error: " << e.what() << endl;
                            }
                        }
                    }
                    cout << "\n";
                }
            }
        }
    }
    else
    {
        //Makes file if it doesn't exist
        if (MakeFile("steamid.txt"))
        {
            cout << "File made successfully, kindly enter IDs in it and restart the program.";
        }
    }

    OutputToFile(BannedIds, PrivateIds);

    cout << "\nCheck BannedIds.txt and PrivateIds.txt for any ids that are in violation of the rules\n";

    curl_easy_cleanup(curl);

    system("pause");

    return 0;
}