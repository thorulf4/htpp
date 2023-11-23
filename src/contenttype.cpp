#include <map>
#include <htpp/http.h>

namespace{
    std::map<std::string_view, htpp::ContentType> init();
    const std::map<std::string_view, htpp::ContentType> extension_content_type = init();

    std::map<std::string_view, htpp::ContentType> init(){
        using enum htpp::ContentType;
        return {
            {".aac", AudioAac},
            {".abw", ApplicationXAbiWord},
            {".arc", ApplicationXFreeArc},
            {".avif", ImageAvif},
            {".avi", VideoXMsVideo},
            {".azw", ApplicationVndAmazonEbook},
            {".bmp", ImageBmp},
            {".bz", ApplicationXBzip},
            {".bz2", ApplicationXBzip2},
            {".cda", ApplicationXCdf},
            {".csh", ApplicationXCsh},
            {".csv", TextCsv},
            {".doc", ApplicationMsWord},
            {".docx", ApplicationVndOpenXMLWord},
            {".eot", ApplicationVndMsFont},
            {".epub", ApplicationEPub},
            {".gz", ApplicationGZip},
            {".gif", ImageGif},
            {".htm", TextHtml},
            {".html", TextHtml},
            {".ico", ImageVndMicrosoftIcon},
            {".ics", TextCalendar},
            {".jar", ApplicationJavaArchive},
            {".jpeg", ImageJpeg},
            {".jpg", ImageJpeg},
            {".jsonld", ApplicationLdJson},
            {".mid", AudioMidi},
            {".midi", AudioMidi},
            {".mjs", TextJavascript},
            {".mp3", AudioMpeg},
            {".mp4", VideoMp4},
            {".mpeg", VideoMpeg},
            {".mpkg", ApplicationBndAppleInstaller},
            {".odp", ApplicationVndOasisOpenPresentation},
            {".ods", ApplicationVndOasisOpenSpreadsheet},
            {".odt", ApplicationVndOasisOpenText},
            {".oga", AudioOgg},
            {".ogv", VideoOgg},
            {".ogx", ApplicationOgg},
            {".opus", AudioOpus},
            {".oft", FontOft},
            {".png", ImagePng},
            {".pdf", ApplicationPdf},
            {".php", ApplicationXHttpdPhp},
            {".ppt", ApplicationVndMsPowerpoint},
            {".pptx", ApplicationVndOpenXMLPresentation},
            {".rar", ApplicationVndRar},
            {".rtf", ApplicationRTF},
            {".sh", ApplicationXSh},
            {".svg", ImageSvg},
            {".tar", ApplicationXTar},
            {".tif", ImageTiff},
            {".tiff", ImageTiff},
            {".ts", VideoMp2t},
            {".ttf", ApplicationVndVisio},
            {".wav", AudioWav},
            {".weba", AudioWebm},
            {".webm", VideoWebm},
            {".webp", ImageWebp},
            {".woff", FontWoff},
            {".woff2", FontWoff2},
            {".xhtml", ApplicationXHtml},
            {".xls", ApplicationVndMsExcel},
            {".xlsx", ApplicationVndOpenXMLSpreadsheet},
            {".xml", ApplicationXml},
            {".xul", ApplicationVndMozillaXul},
            {".zip", ApplicationZip},
            {".7z", Application7Zip},
            {".txt", TextPlain},
            {".css", TextCss},
            {".js", TextJavascript},
            {".json", ApplicationJson},
            {".bin", ApplicationOctetStream}
        };
    }

}

namespace htpp{

    ContentType from_file_extension(std::string_view extension){
        auto it = extension_content_type.find(extension);
        if(it == extension_content_type.end())
            return ContentType::ApplicationOctetStream;
        else
            return it->second;
    }

    std::string_view to_str(ContentType type){
        using enum ContentType;
        switch (type)
        {
        case AudioAac: return "audio/aac";
        case ApplicationXAbiWord: return "application/x-abiword";
        case ApplicationXFreeArc: return "application/x-freearc";
        case ImageAvif: return "image/avif";
        case VideoXMsVideo: return "video/x-msvideo";
        case ApplicationVndAmazonEbook: return "application/vnd.amazon.ebook";
        case ImageBmp: return "image/bmp";
        case ApplicationXBzip: return "application/x-bzip";
        case ApplicationXBzip2: return "application/x-bzip2";
        case ApplicationXCdf: return "application/x-cdf";
        case ApplicationXCsh: return "application/x-csh";
        case TextCsv: return "text/csv";
        case ApplicationMsWord: return "application/msword";
        case ApplicationVndOpenXMLWord: return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        case ApplicationVndMsFont: return "application/vnd.ms-fontobject";
        case ApplicationEPub: return "application/epub+zip";
        case ApplicationGZip: return "application/gzip";
        case ImageGif: return "image/gif";
        case ImageVndMicrosoftIcon: return "image/vnd.microsoft.icon";
        case TextCalendar: return "	text/calendar";
        case ApplicationJavaArchive: return "application/java-archive";
        case ApplicationLdJson: return "application/ld+json";
        case AudioMidi: return "audio/midi"; // or audio/x-midi
        case ImageJpeg: return "image/jpeg";
        case AudioMpeg: return "audio/mpeg";
        case VideoMp4: return "video/mp4";
        case VideoMpeg: return "video/mpeg";
        case ApplicationBndAppleInstaller: return "application/vnd.apple.installer+xml";
        case ApplicationVndOasisOpenPresentation: return "application/vnd.oasis.opendocument.presentation";
        case ApplicationVndOasisOpenSpreadsheet: return "application/vnd.oasis.opendocument.spreadsheet";
        case ApplicationVndOasisOpenText: return "application/vnd.oasis.opendocument.text";
        case AudioOgg: return "audio/ogg";
        case VideoOgg: return "video/ogg";
        case ApplicationOgg: return "application/ogg";
        case FontOft: return "font/otf";
        case AudioOpus: return "audio/opus";
        case ImagePng: return "image/png";
        case ApplicationPdf: return "application/pdf";
        case ApplicationXHttpdPhp: return "application/x-httpd-php";
        case ApplicationVndMsPowerpoint: return "application/vnd.ms-powerpoint";
        case ApplicationVndOpenXMLPresentation: return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        case ApplicationVndRar: return "application/vnd.rar";
        case ApplicationRTF: return "application/rtf";
        case ApplicationXSh: return "application/x-sh";
        case ImageSvg: return "image/svg+xml";
        case ApplicationXTar: return "application/x-tar";
        case ImageTiff: return "image/tiff";
        case VideoMp2t: return "video/mp2t";
        case ApplicationVndVisio: return "application/vnd.visio";
        case AudioWav: return "audio/wav";
        case AudioWebm: return "audio/webm";
        case VideoWebm: return "video/webm";
        case ImageWebp: return "image/webp";
        case FontWoff: return "font/woff";
        case FontWoff2: return "font/woff2";
        case ApplicationXHtml: return "application/xhtml+xml";
        case ApplicationVndMsExcel: return "application/vnd.ms-excel";
        case ApplicationVndOpenXMLSpreadsheet: return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        case ApplicationXml: return "application/xml";
        case ApplicationVndMozillaXul: return "application/vnd.mozilla.xul+xml";
        case ApplicationZip: return "application/zip";
        case Application7Zip: return "application/x-7z-compressed";
        case TextPlain: return "text/plain";
        case TextHtml: return "text/html";
        case TextCss: return "text/css";
        case TextJavascript: return "text/javascript";
        case ApplicationJson: return "application/json";
        case ApplicationOctetStream: return "application/octet-stream";
        }
        return "application/octet-stream"; // Use as default based on mozilla advice https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
    }

}