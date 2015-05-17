/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** utils.h
**  Utility methods
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_UTILS
#define _SEQUENCE_UTILS

int writeToFile(const string& file, const char* data) {
    FILE *fp = fopen(file.c_str(), "wb");

    fwrite(data, sizeof(char), sizeof(data), fp);
    if (fp == NULL) {
        return 0;
    }

    fclose(fp);
    return 1;
}


inline bool fileExists(const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

int readFromFile(const string& file, string& data) {
    FILE *fp = fopen(file.c_str(), "rb");

    if (fp == NULL) {
        return 0;
    }

    long lSize;
    char *buffer;
    size_t result;

    // obtain file size:
    fseek(fp , 0 , SEEK_END);
    lSize = ftell (fp);
    rewind(fp);

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)*lSize);
    if (buffer == NULL) {
        fclose(fp);
        LOG(ERROR) << "Memory error in readFromFile " << stderr;
        return 0;
    }

    // copy the file into the buffer:
    result = fread(buffer, 1, lSize, fp);
    if (result != lSize) {
        LOG(ERROR) << "Reading error in readFromFile " << stderr;
        return 0;
    }

    data.append(buffer);

    fclose(fp);
    free(buffer);
    return 1;
}


uHugeInt toHugeInt(const string& str) {
  std::string::size_type sz = 0;

  if (!str.empty()) {
        uHugeInt ll = std::stoll (str, &sz, 10);

        return ll;
  }

  return 0;
}

// Scan directory and get files
void getDirFiles(const char* pattern, vector<string> &fileList) {
    glob_t globbuf;

    glob(pattern, GLOB_TILDE, NULL, &globbuf);

    for (int i = 0; i < globbuf.gl_pathc; ++i)
        fileList.push_back( globbuf.gl_pathv[i] );

    if (globbuf.gl_pathc > 0)
        globfree(&globbuf);
}

// Parse Query String
void parseString(const char *queryStr, StringMap& list) {
    if (!strlen(queryStr)) {
        return;
    }

    string vName;
    string value;
    int flag = 1;
    int c;
    char buf[3];

    while (*queryStr) {
        if (*queryStr == '&') {
            if (vName.length()) {
                list[vName] = value;
            }
            
            vName.clear();
            value.clear();
            
            flag = 1;
            
        } else if (flag) {
            if (*queryStr == '=') {
                flag = 0;
            } else {
                vName.append(1, *queryStr);
            }

        } else {
            switch (*queryStr) {
                case '%':
                    buf[0] = *(++queryStr);
                    buf[1] = *(++queryStr);
                    buf[2] = '\0';

                    sscanf(buf, "%2x", &c);
                    value.append(buf);
                    break;

                case '+':
                    value.append(1, ' ');
                    break;

                default:
                    value.append(1, *queryStr);
                    break;
            }
        }

        queryStr++;
    }
    
    if (vName.length()) {
        list[vName] = value;
    }
}

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

// Parse INI file into Map
int parseIniFile(const char *file, StringMap& list) {
    if (!strlen(file)) {
        return 0;
    }
    
    string fileData;
    if (!readFromFile(file, fileData)) {
        return -1;
    }

    string vName;
    string value;
    int gotEqual = 0, inString = 0, inComment = 0, lastC = 0, c = 0, ch = 0, len = fileData.length();
    bool add = false;
    char buf[3];

    while (c < len) {
        ch = fileData[c++];
        
        if (ch == '\n') {
            vName = trim(vName);
            value = trim(value);
            
            //cout << "Got: " << vName << "=" << value << "\n";
            
            if (vName.length() && value.length()) {
                list[vName] = value;
            }
            
            vName.clear();
            value.clear();
            
            inString = 0;
            gotEqual = 0;
            inComment = 0;
        } else if (ch == '#' && !inString) {
            inComment = 1;
        } else {
            
            if (inComment) {
                continue;
            }
            
            if (gotEqual) {
                add = true;
                if (inString) {
                    if (lastC != '\\' && ch == '"') {
                        // end string
                        inString = 0;
                        add = false;
                    }
                } else {
                    if (value.length() == 0 && lastC != '\\' && ch == '"') {
                        inString = 1;
                        add = false;
                    }
                }
                
                if (ch == ' ' && value.length() == 0) {
                    // skip white space
                    add = false;
                }
                
                if (add) value.append(1, ch);
            } else {
                if (ch == '=') {
                    gotEqual = 1;
                } else {
                    vName.append(1, ch);
                }
            }
        }
        
        lastC = ch;
    }
    
    vName = trim(vName);
    value = trim(value);
    if (vName.length() && value.length()) {    
        list[vName] = value;
    }
    
    return 1;
}


int isAlphaId(const char *word) {
    if (!isalpha(*word) && *word != '_')
        return 0;

    word++;

    while (*word) {
        if (!(isalnum(*word) || *word == '_')) {
            return 0;
        }

        word++;
    }

    return 1;
}

int isAlphaNumExtra(const char *word) {
    while (*word) {
        if (!(isalnum(*word) || *word == '_')) {
            return 0;
        }

        word++;
    }

    return 1;
}

int isAlphaNum(const char *word) {
    while (*word) {
        if (!isalnum(*word)) {
            return 0;
        }

        word++;
    }

    return 1;
}

int isNumber(const char *word) {
    while (*word) {
        if (!isdigit(*word)) {
            return 0;
        }

        word++;
    }

    return 1;
}

int toInt(const string& str) {
    std::string::size_type sz;
    
    return std::stoi(str, &sz);
}

bool toBool(const string& str) {
    return (str == "true" || str == "1" || str == "on" || str == "On" || str == "ON");
}


#endif