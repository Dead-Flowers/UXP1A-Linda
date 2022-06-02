#ifndef PARSER_STRING_READER_H
#define PARSER_STRING_READER_H

#include <string>


namespace linda::modules {
    class StringReader {
    public:
        StringReader(const std::string& input);
        ~StringReader() = default;
        char getNextCharacter();
        bool hasFinished();
    private:
        std::string data;
        int position;
    };
}

#endif //PARSER_STRING_READER_H
