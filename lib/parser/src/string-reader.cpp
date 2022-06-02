
#include "parser/string-reader.h"

using StringReader = linda::modules::StringReader;

StringReader::StringReader(const std::string &input) {
this->data = input;
this->position = 0;
}

char linda::modules::StringReader::getNextCharacter() {
    if (hasFinished()) {
        return (char)-1;
    }
    return this->data[position++];
}

bool linda::modules::StringReader::hasFinished() {
    return position >= this->data.size();
}
