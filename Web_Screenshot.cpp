#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <sstream>
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// json object to store all results
json results;

mutex mtx;

// save_results now just updates the global json object
void save_results(string domain, string screenshot_filename, vector<string> &admin_list)
{
    // lock the mutex, to protect shared data from concurrent access
    lock_guard<mutex> lock(mtx);

    json &result = results[domain];
    result["url"] = domain;
    result["screenshot"] = screenshot_filename;
    result["admin_pages"] = json::array();

    for (const string &admin : admin_list) {
        result["admin_pages"].push_back(admin);
    }

    // Close the browser
  //  system("killall chromium");
}

string get_cmd_output(string cmd)
{
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    }
    catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

string get_screenshot_filename(string domain)
{
    string filename = domain + ".png";
    std::replace(filename.begin(), filename.end(), '/', '_');
    return filename;
}

void scan_domain(string domain, vector<string> &admin_list)
{
    string screenshot_filename = get_screenshot_filename(domain);
    string cmd = "chromium --headless --screenshot=" + screenshot_filename + " --virtual-time-budget=5000 --no-sandbox --dump-dom http://" + domain + " > /dev/null 2>&1";

    string output = get_cmd_output(cmd);
    if (output.find("Failed") != string::npos) {
        cerr << "Error: Failed to take a screenshot of " << domain << endl;
        return;
    }

    // Check for popular admin pages
    vector<string> popular_admins = {
        "/wp-admin/",
        "/admin/",
        "/administrator/",
        "/login/",
        "/admin/login/",
        "/administrator/login/",
        "/user/login/"
    };

    // Add more popular admin pages here
    for (auto admin : popular_admins) {
        string admin_url = "http://" + domain + admin;
        string admin_output = get_cmd_output("curl -Is " + admin_url + " | head -1");
        if (admin_output.find("200 OK") != string::npos) {
            admin_list.push_back(admin_url);
            cout << "Found admin page for " << domain << ": " << admin_url << endl;
        }
    }

    save_results(domain, screenshot_filename, admin_list);
}

void scan_domains(vector<string> &domains)
{
    vector<thread> threads;

    for (auto domain : domains) {
        threads.push_back(thread(
            [&](const string d) {
                vector<string> admin_list;
                scan_domain(d, admin_list);
            },
            domain));
    }

    for (auto &th : threads) {
        th.join();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    string input_file = argv[1];

    vector<string> domains;

    ifstream infile(input_file);
    string line;
    while (getline(infile, line)) {
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
        domains.push_back(line);
    }

    infile.close();
    scan_domains(domains);

    // after all domains have been scanned, write the results object to a single JSON file
    ofstream out("results.json");
    out << setw(4) << results << endl;
    out.close();

    // Close all browsers before exit
   // system("killall chromium");

    return 0;
}