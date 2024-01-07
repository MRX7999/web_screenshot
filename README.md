# web_screenshot

This is a simple domain scanner that takes a list of domain names as input and scans them for popular admin pages. It also takes a screenshot of the domain's main page and saves it as a PNG file.
To use the domain scanner and screenshot tool, you must first ensure that your system has Chromium browser installed. Chromium is an open-source browser that this tool uses to take a screenshot of the domain's main page. You can download and install Chromium for your operating system from the official Chromium website. After installing Chromium, you will also need to ensure that you have C++11 or later and the nlohmann/json library installed.

## Dependencies

- C++11 or later
- [nlohmann/json](https://github.com/nlohmann/json) library

## Usage

To use this scanner, supply a file path to a list of domains as a command line argument. For example:

```
$ domain_scanner domains.txt
```

The scanner will output messages as it scans each domain and will save the results as a JSON file named `results.json` in the current directory.

## Functionality

For each domain, the scanner does the following:

1. Takes a screenshot of the domain's main page using the headless Chromium browser. The screenshot is saved as a PNG file in the current directory with the domain name as the file name.
2. Checks the domain for popular admin pages, such as `/wp-admin/` and `/admin/`. If an admin page is found, the URL is saved in the output results.
3. Saves the domain URL, screenshot filename, and admin pages (if any) in a JSON object.

The scanner uses threads to scan multiple domains concurrently, which will speed up the scanning process for long lists of domains.