int writeToFile(const string& file, const char* data) {
    FILE *fp = fopen(file.c_str(), "wb");

    fwrite(data, sizeof(char), sizeof(data), fp);
    if (fp == NULL) {
        return 0;
    }

    fclose(fp);
    return 1;
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


StringMap parseString(string query) {
    StringMap list;

    if (!query.length()) {
        return list;
    }

	char *query_string;
	char *phrase;

	query_string = (char *) query.c_str();

	phrase = strtok(query_string, "&");
	while (phrase) {
        string vName;
        string value;
        int flag = 1;
        int c;
        char buf[3];

        while (*phrase) {
            if (flag) {
                if (*phrase == '=') {
                    flag = 0;
                } else {
                    vName.append(1, *phrase);
                }

            } else {
                switch (*phrase) {
                    case '%':
                        buf[0] = *(++phrase);
                        buf[1] = *(++phrase);
                        buf[2] = '\0';

                        sscanf(buf, "%2x", &c);
                        value.append(buf);
                        break;

                    case '+':
                        value.append(1, ' ');
                        break;

                    default:
                        value.append(1, *phrase);
                        break;
                }
            }

            phrase++;
        }

        if (vName.length()) {
            list[vName] = value;
        }

		phrase = strtok(NULL, "&");
	}

	return list;
}