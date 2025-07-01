#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
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
            if (!field->is_repeated() && !refl->HasField(msg, field)) continue; // skip unset

            switch (field->cpp_type()) {
                case FieldDescriptor::CPPTYPE_INT32:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedInt32(msg, field, 0) : refl->GetInt32(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_INT64:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedInt64(msg, field, 0) : refl->GetInt64(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_UINT32:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedUInt32(msg, field, 0) : refl->GetUInt32(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_UINT64:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedUInt64(msg, field, 0) : refl->GetUInt64(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_DOUBLE:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedDouble(msg, field, 0) : refl->GetDouble(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_FLOAT:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedFloat(msg, field, 0) : refl->GetFloat(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_BOOL:
                    doc << field->name() << (field->is_repeated() ? refl->GetRepeatedBool(msg, field, 0) : refl->GetBool(msg, field));
                    break;
                case FieldDescriptor::CPPTYPE_ENUM:
                    doc << field->name() << refl->GetEnum(msg, field)->number();
                    break;
                case FieldDescriptor::CPPTYPE_STRING: {
                    std::string val = field->is_repeated()
                        ? refl->GetRepeatedString(msg, field, 0)
                        : refl->GetString(msg, field);
                    doc << field->name() << val;
                    break;
                }
                case FieldDescriptor::CPPTYPE_MESSAGE: {
                    // Nested message; recurse
                    const Message& nested = field->is_repeated()
                        ? refl->GetRepeatedMessage(msg, field, 0)
                        : refl->GetMessage(msg, field);
                    doc << field->name() << ProtoToBson(nested).view();
                    break;
                }
            }
        }
        return doc.extract();
    }

    template<typename T>
    inline T BsonToProto(const bsoncxx::document::view& view) {
        T msg;
        auto bin = view["data"].get_binary();
        msg.ParseFromArray(bin.bytes, bin.size);
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
                coll_.update_one(filter, bsoncxx::builder::stream::document{} << "$set" << bsoncxx::builder::stream::open_document
                                            << "data" << ProtoToBson(msg).view()["data"].get_binary()
                                            << "type" << msg.GetTypeName()
                                            << bsoncxx::builder::stream::close_document << bsoncxx::builder::stream::finalize);
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