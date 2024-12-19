#include <iostream>
#include <fstream>
#include <string>
#include <openssl/evp.h>
#include <vector>
#include <iomanip>

using namespace std;

// دالة لتجزئة النص باستخدام نوع هاش محدد
string computeHash(const string& input, const string& hashType) {
    const EVP_MD* md = EVP_get_digestbyname(hashType.c_str());
    if (!md) {
        cerr << "Unsupported hash type: " << hashType << endl;
        exit(1);
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    stringstream ss;
    for (unsigned int i = 0; i < hashLen; ++i) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// دالة لاختبار كلمة مرور من الوورد ليست
bool testPassword(const string& hash, const string& hashType, const string& password) {
    return computeHash(password, hashType) == hash;
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Usage: " << argv[0] << " -h <hash> -t <hash_type> -w <wordlist_path> -n <processor/gpu>" << endl;
        return 1;
    }

    string hash, hashType, wordlistPath, mode;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) hash = argv[++i];
        else if (arg == "-t" && i + 1 < argc) hashType = argv[++i];
        else if (arg == "-w" && i + 1 < argc) wordlistPath = argv[++i];
        else if (arg == "-n" && i + 1 < argc) mode = argv[++i];
    }

    ifstream wordlist(wordlistPath);
    if (!wordlist.is_open()) {
        cerr << "Failed to open wordlist file: " << wordlistPath << endl;
        return 1;
    }

    cout << "Starting hash cracking using " << mode << "..." << endl;

    string password;
    vector<string> passwords;
    while (getline(wordlist, password)) {
        passwords.push_back(password);
    }

    size_t totalPasswords = passwords.size();
    size_t testedPasswords = 0;
    bool found = false;

    for (const auto& pass : passwords) {
        ++testedPasswords;
        cout << testedPasswords << "/" << totalPasswords
             << " Test (I test now {" << pass << "})"
             << " Progress: " << fixed << setprecision(2) << (testedPasswords * 100.0 / totalPasswords) << "%" << endl;

        if (testPassword(hash, hashType, pass)) {
            cout << "\033[32mHash Fragment Found! Password: " << pass << "\033[0m" << endl;
            found = true;
            break;
        }
    }

    if (!found) {
        cerr << "\033[31mThe fragmentation will not be found.\033[0m" << endl;
    }

    return 0;
}
