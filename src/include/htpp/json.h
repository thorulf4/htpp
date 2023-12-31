#pragma once
#include <iostream>
#include <string_view>
#include <sstream>
#include <algorithm>
#include <ranges>
#include "lib.h"

//// Example:
// struct Point{
//     using json_names = json::key_name<"message">;
//
//     std::string message{"Hello, World!"};
// } point;

namespace json{

    namespace inner{
        template<size_t N>
        struct StringLiteral {
            constexpr StringLiteral(const char (&str)[N]) {
                std::copy_n(str, N, value);
            }
            constexpr operator const char*() const {
                return value;
            }
            
            char value[N];
        };
    }

    template<inner::StringLiteral ... names>
    struct key_name {
        template<int i>
        consteval static const char* get(){
            return std::vector<const char*>{names...}[i];
        }

        consteval static size_t count(){
            return sizeof...(names);
        }
    };

    namespace inner{

        template<typename T>
        constexpr void serialize(std::stringstream& s, const T& object);

        template<typename T>
        void serialize_field(std::stringstream& s, const T& field){
            if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
                s << field;
            else if constexpr(std::is_convertible_v<T, std::string_view>){
                s << '"' << std::string_view{field} << '"';
            }else{
                serialize(s, field);
            }
        }

        template<typename T>
        constexpr void serialize(std::stringstream& s, const T& object){
            constexpr auto size = T::json_names::count();
            s << '{';
            if constexpr(size == 1){
                const auto& [v1] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1);
            }else if constexpr( size == 2){
                const auto& [v1, v2] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1); s << ',';
                s << '"' << T::json_names::template get<1>() << "\":"; serialize_field(s, v2);
            }else if constexpr( size == 3){
                const auto& [v1, v2, v3] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1); s << ',';
                s << '"' << T::json_names::template get<1>() << "\":"; serialize_field(s, v2); s << ',';
                s << '"' << T::json_names::template get<2>() << "\":"; serialize_field(s, v3);
            }else if constexpr( size == 4){
                const auto& [v1, v2, v3, v4] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1); s << ',';
                s << '"' << T::json_names::template get<1>() << "\":"; serialize_field(s, v2); s << ',';
                s << '"' << T::json_names::template get<2>() << "\":"; serialize_field(s, v3); s << ',';
                s << '"' << T::json_names::template get<3>() << "\":"; serialize_field(s, v4);
            }else if constexpr( size == 5){
                const auto& [v1, v2, v3, v4, v5] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1); s << ',';
                s << '"' << T::json_names::template get<1>() << "\":"; serialize_field(s, v2); s << ',';
                s << '"' << T::json_names::template get<2>() << "\":"; serialize_field(s, v3); s << ',';
                s << '"' << T::json_names::template get<3>() << "\":"; serialize_field(s, v4); s << ',';
                s << '"' << T::json_names::template get<4>() << "\":"; serialize_field(s, v5);
            }else if constexpr( size == 6){
                const auto& [v1, v2, v3, v4, v5, v6] = object;
                s << '"' << T::json_names::template get<0>() << "\":"; serialize_field(s, v1); s << ',';
                s << '"' << T::json_names::template get<1>() << "\":"; serialize_field(s, v2); s << ',';
                s << '"' << T::json_names::template get<2>() << "\":"; serialize_field(s, v3); s << ',';
                s << '"' << T::json_names::template get<3>() << "\":"; serialize_field(s, v4); s << ',';
                s << '"' << T::json_names::template get<4>() << "\":"; serialize_field(s, v5); s << ',';
                s << '"' << T::json_names::template get<5>() << "\":"; serialize_field(s, v6);
            }
            
            s << '}';
        }

        template<std::ranges::range T>
        constexpr void serialize(std::stringstream& s, const T& object){
            s << '[';
            auto it = std::begin(object);
            const auto end = std::end(object);
            if(it != end){
                serialize_field(s, *it);
                while(++it != end){
                    s << ',';
                    serialize_field(s, *it);
                }
            }
            s << ']';
        }

    }

    
    template<typename T>
    struct From : public htpp::OkResponse{
        const T& object; // this storage could be problematic
        explicit From(const T& object): object{object} {}
        void print_content(std::stringstream& s) const {
            inner::serialize(s, object);
        }
        htpp::ContentType content_type() const { return htpp::ContentType::ApplicationJson; }
    };
}