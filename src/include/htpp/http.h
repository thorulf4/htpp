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

    struct Content{
        ContentType type;
        std::string data;

        constexpr Content(ContentType type, std::string data): type{type}, data{std::move(data)} {}
        constexpr Content(std::string data): type{ContentType::TextHtml}, data{std::move(data)} {}
    };

    struct Response {
        int response_code{200};
        std::optional<Content> content{std::nullopt};
        explicit Response(int response_code): response_code{response_code} {}
        Response(Content content): content{std::move(content)} {}
        Response(int response_code, Content content): response_code{response_code}, content{std::move(content)} {}
    };

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