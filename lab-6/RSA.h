#include <vector>
#include <string>
#include <algorithm>

using namespace std;

string toDec(int n) {
    string res = "";
    for (int i = 0; i < 3; i++) {
        res += (char)(n % 10 + '0');
        n /= 10;
    }
    reverse(res.begin(), res.end());
    return res;
}

string toString(int n) {
    string res = "";
    while (n) {
        res += (char)(n % 10 + '0');
        n /= 10;
    }
    reverse(res.begin(), res.end());
    return res;
}

#define li long long int
int modPow(li a, li n, li mod) {
    li res = 1;
    while (n) {
        if (n % 2) {
            res = res * a % mod;
        }
        a = a * a % mod;
        n /= 2;
    }
    return res;
}

string power(string s, int key, int mod) {
    int res = 0;
    for (int i = 0; i < (int)s.length(); i++) {
        res = res * 10 + (s[i] - '0');
    }
    string ans = toString(modPow(res, key, mod));
    return ans;
}

string encode(string plain, int publicKey, int mod) {
    vector<string> plainInDec;
    for (int i = 0; i < (int)plain.length(); i++) {
        plainInDec.push_back(toDec(plain[i]));
    }
    while ((int)plainInDec.size() % 3 != 0) {
        plainInDec.push_back(toDec('#'));
    }
    string resCipher = "";
    for (int i = 0; i < (int)plainInDec.size(); i += 3) {
        string text = "";
        for (int j = i; j < i + 3; j++) {
            text += plainInDec[j];
        }
        string cipher = power(text, publicKey, mod);
        reverse(cipher.begin(), cipher.end());
        while ((int)cipher.size() != 9) {
            cipher += '0';
        }
        reverse(cipher.begin(), cipher.end());
        resCipher += cipher;
    }
    return resCipher;
}

char toChar(string s) {
    int res = 0;
    for (int i = 0; i < (int)s.length(); i++) {
        res = res * 10 + (s[i] - '0');
    }
    return (char)res;
}

string decode(string cipher, int privateKey, int mod) {
    string resPlain = "";
    for (int i = 0; i < (int)cipher.length(); i += 9) {
        string text = "";
        for (int j = i; j < i + 9; j++) {
            text += cipher[j];
        }
        string plain = power(text, privateKey, mod);
        reverse(plain.begin(), plain.end());
        while ((int)plain.size() != 9) {
            plain += '0';
        }
        reverse(plain.begin(), plain.end());
        resPlain += plain;
    }
    string resPlainInChar = "";
    for (int i = 0; i < (int)resPlain.length(); i += 3) {
        string ch = "";
        for (int j = i; j < i + 3; j++) {
            ch += resPlain[j];
        }
        resPlainInChar += toChar(ch);
    }
    string resPlainRemoveHash = "";
    for (int i = 0; i < (int)resPlainInChar.length(); i++) {
        if (resPlainInChar[i] != '#') resPlainRemoveHash += resPlainInChar[i];
    }
    return resPlainRemoveHash;
}