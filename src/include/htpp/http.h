#pragma once
#include <string>
#include <string_view>
#include <optional>
#include <utility>

namespace htpp{
    enum class RequestType{
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH
    };

    enum class ContentType{
        AudioAac,
        ApplicationXAbiWord,
        ApplicationXFreeArc,
        ImageAvif,
        VideoXMsVideo,
        ApplicationVndAmazonEbook,
        ImageBmp,
        ApplicationXBzip,
        ApplicationXBzip2,
        ApplicationXCdf,
        ApplicationXCsh,
        TextCsv,
        ApplicationMsWord,
        ApplicationVndOpenXMLWord,
        ApplicationVndMsFont,
        ApplicationEPub,
        ApplicationGZip,
        ImageGif,
        ImageVndMicrosoftIcon,
        TextCalendar,
        ApplicationJavaArchive,
        ApplicationLdJson,
        AudioMidi,
        ImageJpeg,
        AudioMpeg,
        VideoMp4,
        VideoMpeg,
        ApplicationBndAppleInstaller,
        ApplicationVndOasisOpenPresentation,
        ApplicationVndOasisOpenSpreadsheet,
        ApplicationVndOasisOpenText,
        AudioOgg,
        VideoOgg,
        ApplicationOgg,
        FontOft,
        AudioOpus,
        ImagePng,
        ApplicationPdf,
        ApplicationXHttpdPhp,
        ApplicationVndMsPowerpoint,
        ApplicationVndOpenXMLPresentation,
        ApplicationVndRar,
        ApplicationRTF,
        ApplicationXSh,
        ImageSvg,
        ApplicationXTar,
        ImageTiff,
        VideoMp2t,
        ApplicationVndVisio,
        AudioWav,
        AudioWebm,
        VideoWebm,
        ImageWebp,
        FontWoff,
        FontWoff2,
        ApplicationXHtml,
        ApplicationVndMsExcel,
        ApplicationVndOpenXMLSpreadsheet,
        ApplicationXml,
        ApplicationVndMozillaXul,
        ApplicationZip,
        Application7Zip,
        TextPlain,
        TextHtml,
        TextCss,
        TextJavascript,
        ApplicationJson,
        ApplicationOctetStream // Default, provides some safety
    };

    ContentType from_file_extension(std::string_view extension);
    std::string_view to_str(ContentType type);

    template<typename T>
    concept ContentConcept = requires (T t, std::stringstream s) {
        {t.print_content(s)};
        {t.content_type()} -> std::same_as<htpp::ContentType>;
    };

    template<typename T>
    concept SizedContentConcept = ContentConcept<T> && requires (T t) {
        {t.content_size()} -> std::same_as<std::size_t>;
    };

    template<typename T>
    concept ResponseConcept = requires (const T t, std::stringstream s) {
        {t.response_code()} -> std::same_as<uint16_t>;
        {t.header_line(s)};
    };

    struct OkResponse{
        uint16_t response_code() const { return 200; }
        void header_line(std::stringstream&) const {}
    };
    static_assert(ResponseConcept<OkResponse>);

    class Response {
        uint16_t code;
    public:
        explicit Response(uint16_t code): code{code} {}
        uint16_t response_code() const { return code; }
        void header_line(std::stringstream&) const {}
    };
    static_assert(ResponseConcept<Response>);

    struct Request{
        RequestType type;
        std::string_view url;
        std::string_view param;
    };

    inline std::string_view to_str(RequestType type){
        using enum RequestType;
        switch (type)
        {
        case GET: return "GET";
        case PUT: return "PUT";
        case POST: return "POST";
        case DELETE: return "DELETE";
        case HEAD: return "HEAD";
        case OPTIONS: return "OPTIONS";
        case TRACE: return "TRACE";
        case CONNECT: return "CONNECT";
        case PATCH: return "OTHER";
        }
        return "Unknown";
    }

    struct Endpoint{
        RequestType type;
        std::string_view address;
        auto operator<=>(const Endpoint&) const = default;
    };
}

template<>
struct std::hash<htpp::Endpoint>
{
    std::size_t operator()(const htpp::Endpoint& point) const noexcept
    {
        return std::hash<std::string_view>{}(point.address) + static_cast<std::size_t>(point.type);
    }
};