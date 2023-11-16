#pragma once
#include <htpp/http.h>

static htpp::RequestType get_type(char*& data){
    using enum htpp::RequestType;
    switch (data[0])
    {
        case 'G':
            data += 4;
            return GET;
        case 'H':
            data += 5;
            return HEAD;
        case 'P':
            switch (data[1])
            {
                case 'O':
                    data += 5;
                    return POST;
                case 'U':
                    data += 4;
                    return PUT;
                case 'A':
                    data += 6;
                    return PATCH;
                default:
                    throw std::logic_error{"4xx bad request type"};
            }
        case 'D':
            data += 7;
            return DELETE;
        case 'C':
            data += 8;
            return CONNECT;
        case 'O':
            data += 8;
            return OPTIONS;
        case 'T':
            data += 6;
            return TRACE;
        default:
            throw std::logic_error{"4xx bad request type"};
    }
    std::unreachable();
}

static std::string_view weekday(const tm& time){
    switch (time.tm_wday)
    {
        case 0: return "Mon";
        case 1: return "Tue";
        case 2: return "Wed";
        case 3: return "Thu";
        case 4: return "Fri";
        case 5: return "Sat";
        case 6: return "Sun";
    }
    std::unreachable();
}

static std::string_view month(const tm& time){
    switch (time.tm_mon)
    {
        case 0: return "Jan";
        case 1: return "Feb";
        case 2: return "Mar";
        case 3: return "Apr";
        case 4: return "May";
        case 5: return "Jun";
        case 6: return "Jul";
        case 7: return "Aug";
        case 8: return "Sep";
        case 9: return "Oct";
        case 10: return "Nov";
        case 11: return "Dec";
    }
    std::unreachable();
}

static std::string_view to_str(htpp::ContentType type){
    using enum htpp::ContentType;
    switch(type){
        case TextHtml: return "text/html";
        case ApplicationJson: return "application/json";
    }
    std::unreachable();
}

// Formats an integer with XX format e.g 05, 23 or 00 
class Fmt2Int{
    int v;
public:
    // Precondition: v < 100
    explicit Fmt2Int(int v): v{v} {}
    friend std::ostream& operator<<(std::ostream& os, const Fmt2Int& fmt){
        if(fmt.v < 10)
            os << '0';
        os << fmt.v;
        return os;
    }
};