#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/database.hpp>
#include <google/protobuf/message.h>

#include <iostream>
#include <string>


namespace proto_mongo{
    inline bsoncxx::document::value ProtoToBson(const google::protobuf::Message& msg) {
        using namespace google::protobuf;
        const Reflection* refl = msg.GetReflection();
        const Descriptor* desc = msg.GetDescriptor();
        bsoncxx::builder::stream::document doc{};

        int field_count = desc->field_count();
        for (int i = 0; i < field_count; ++i) {
            const FieldDescriptor* field = desc->field(i);
            if (field->is_repeated()) {
                int n = refl->FieldSize(msg, field);
                if (n > 0) {
                    bsoncxx::builder::stream::array arr;
                    switch (field->cpp_type()) {
                        case FieldDescriptor::CPPTYPE_INT32:
                            for (int j = 0; j < n; ++j)
                                arr << refl->GetRepeatedInt32(msg, field, j);
                            break;
                        case FieldDescriptor::CPPTYPE_INT64:
                            for (int j = 0; j < n; ++j)
                                arr << static_cast<int64_t>(refl->GetRepeatedInt64(msg, field, j));
                            break;
                        case FieldDescriptor::CPPTYPE_UINT32:
                            for (int j = 0; j < n; ++j)
                                arr << static_cast<int64_t>(refl->GetRepeatedUInt32(msg, field, j));
                            break;
                        case FieldDescriptor::CPPTYPE_UINT64:
                            for (int j = 0; j < n; ++j)
                                arr << static_cast<int64_t>(refl->GetRepeatedUInt64(msg, field, j));
                            break;
                        case FieldDescriptor::CPPTYPE_DOUBLE:
                            for (int j = 0; j < n; ++j)
                                arr << refl->GetRepeatedDouble(msg, field, j);
                            break;
                        case FieldDescriptor::CPPTYPE_FLOAT:
                            for (int j = 0; j < n; ++j)
                                arr << static_cast<double>(refl->GetRepeatedFloat(msg, field, j));
                            break;
                        case FieldDescriptor::CPPTYPE_BOOL:
                            for (int j = 0; j < n; ++j)
                                arr << refl->GetRepeatedBool(msg, field, j);
                            break;
                        case FieldDescriptor::CPPTYPE_ENUM:
                            for (int j = 0; j < n; ++j)
                                arr << refl->GetRepeatedEnum(msg, field, j)->number();
                            break;
                        case FieldDescriptor::CPPTYPE_STRING:
                            for (int j = 0; j < n; ++j)
                                arr << refl->GetRepeatedString(msg, field, j);
                            break;
                        case FieldDescriptor::CPPTYPE_MESSAGE:
                            for (int j = 0; j < n; ++j)
                                arr << ProtoToBson(refl->GetRepeatedMessage(msg, field, j)).view();
                            break;
                    }
                    doc << field->name() << arr;
                }
                // else: skip empty repeated field
            } else if (refl->HasField(msg, field)) {
                switch (field->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_INT32:
                        doc << field->name() << refl->GetInt32(msg, field);
                        break;
                    case FieldDescriptor::CPPTYPE_INT64:
                        doc << field->name() << static_cast<int64_t>(refl->GetInt64(msg, field));
                        break;
                    case FieldDescriptor::CPPTYPE_UINT32:
                        doc << field->name() << static_cast<int64_t>(refl->GetUInt32(msg, field));
                        break;
                    case FieldDescriptor::CPPTYPE_UINT64:
                        doc << field->name() << static_cast<int64_t>(refl->GetUInt64(msg, field));
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                        doc << field->name() << refl->GetDouble(msg, field);
                        break;
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        doc << field->name() << static_cast<double>(refl->GetFloat(msg, field));
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        doc << field->name() << refl->GetBool(msg, field);
                        break;
                    case FieldDescriptor::CPPTYPE_ENUM:
                        doc << field->name() << refl->GetEnum(msg, field)->number();
                        break;
                    case FieldDescriptor::CPPTYPE_STRING:
                        doc << field->name() << refl->GetString(msg, field);
                        break;
                    case FieldDescriptor::CPPTYPE_MESSAGE:
                        doc << field->name() << ProtoToBson(refl->GetMessage(msg, field)).view();
                        break;
                }
            }
        }
        return doc.extract();
    }

    inline void BsonToProtoDynamic(const bsoncxx::document::view& view, google::protobuf::Message* msg) {
        using namespace google::protobuf;
        const Descriptor* desc = msg->GetDescriptor();
        const Reflection* refl = msg->GetReflection();

        for (int i = 0; i < desc->field_count(); ++i) {
            const FieldDescriptor* field = desc->field(i);
            auto el = view[field->name()];
            if (!el) continue; // Not present

            if (field->is_repeated()) {
                if (el.type() == bsoncxx::type::k_array) {
                    auto arr = el.get_array().value;
                    int idx = 0;
                    for (auto&& arr_el : arr) {
                        switch (field->cpp_type()) {
                            case FieldDescriptor::CPPTYPE_INT32:
                                refl->AddInt32(msg, field, arr_el.get_int32());
                                break;
                            case FieldDescriptor::CPPTYPE_INT64:
                                refl->AddInt64(msg, field, arr_el.get_int64());
                                break;
                            case FieldDescriptor::CPPTYPE_UINT32:
                                refl->AddUInt32(msg, field, static_cast<uint32_t>(arr_el.get_int64()));
                                break;
                            case FieldDescriptor::CPPTYPE_UINT64:
                                refl->AddUInt64(msg, field, static_cast<uint64_t>(arr_el.get_int64()));
                                break;
                            case FieldDescriptor::CPPTYPE_DOUBLE:
                                refl->AddDouble(msg, field, arr_el.get_double());
                                break;
                            case FieldDescriptor::CPPTYPE_FLOAT:
                                refl->AddFloat(msg, field, static_cast<float>(arr_el.get_double()));
                                break;
                            case FieldDescriptor::CPPTYPE_BOOL:
                                refl->AddBool(msg, field, arr_el.get_bool());
                                break;
                            case FieldDescriptor::CPPTYPE_STRING:
                                refl->AddString(msg, field, arr_el.get_utf8().value.to_string());
                                break;
                            // You can add enums and nested messages as well, if needed
                            default:
                                break;
                        }
                        ++idx;
                    }
                }
            } else {
                switch (field->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_INT32:
                        refl->SetInt32(msg, field, el.get_int32());
                        break;
                    case FieldDescriptor::CPPTYPE_INT64:
                        refl->SetInt64(msg, field, el.get_int64());
                        break;
                    case FieldDescriptor::CPPTYPE_UINT32:
                        refl->SetUInt32(msg, field, static_cast<uint32_t>(el.get_int64()));
                        break;
                    case FieldDescriptor::CPPTYPE_UINT64:
                        refl->SetUInt64(msg, field, static_cast<uint64_t>(el.get_int64()));
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                        refl->SetDouble(msg, field, el.get_double());
                        break;
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        refl->SetFloat(msg, field, static_cast<float>(el.get_double()));
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        refl->SetBool(msg, field, el.get_bool());
                        break;
                    case FieldDescriptor::CPPTYPE_STRING:
                        refl->SetString(msg, field, el.get_utf8().value.to_string());
                        break;
                    // You can add enums and nested messages as well, if needed
                    default:
                        break;
                }
            }
        }
    }


    template<typename T>
    inline T BsonToProto(const bsoncxx::document::view& view) {
        T msg;
        BsonToProtoDynamic(view, &msg);
        return msg;
    }

    class ProtoMongoAPI{
        public:
            ProtoMongoAPI(const std::string& uri, const std::string& dbname, const std::string& collname)
                : client_(mongocxx::uri{uri}), db_(client_[dbname]), coll_(db_[collname]) {}

            template<typename MsgT>
            void insert(const MsgT& msg) {
                coll_.insert_one(ProtoToBson(msg));
            }

            template<typename MsgT>
            bool find_one(MsgT& out_msg, const bsoncxx::document::view_or_value& filter = {}) {
                auto res = coll_.find_one(filter);
                if (!res) return false;
                out_msg = BsonToProto<MsgT>(res->view());
                return true;
            }

            template<typename MsgT>
            void update_one(const bsoncxx::document::view_or_value& filter, const MsgT& msg) {
                // Get the BSON doc for the message
                auto bson_doc = ProtoToBson(msg);
                // Build the $set update doc with *all* fields of the message
                bsoncxx::builder::stream::document update_doc;
                auto set_builder = update_doc << "$set" << bsoncxx::builder::stream::open_document;
                for (auto&& el : bson_doc.view()) {
                    set_builder << el.key().to_string() << el.get_value();
                }
                set_builder << bsoncxx::builder::stream::close_document;
                coll_.update_one(filter, update_doc.view());
            }

            void delete_one(const bsoncxx::document::view_or_value& filter) {
                coll_.delete_one(filter);
            }

        private:
            mongocxx::client client_;
            mongocxx::database db_;
            mongocxx::collection coll_;
    };
}