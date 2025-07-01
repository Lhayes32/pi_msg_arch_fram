#include <mpi.h>
#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include "ProtoMongoAPI.h" // Put the above API code here
#include "proto/test_proto.pb.h"

using test::AllTypes;
using namespace proto_mongo;

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    mongocxx::instance instance{};

    // Use the API class
    ProtoMongoAPI api("mongodb://localhost:27017", "testdb", "all_types_coll");

    // Clean up: only rank 0 clears the collection for a fresh test
    if (world_rank == 0) {
        api.delete_one({});  // This deletes just one; to clear all, you could add delete_many
        std::cout << "Cleared collection for fresh test.\n";
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Each rank inserts a doc
    AllTypes msg;
    msg.set_my_int32(world_rank * 10);
    msg.set_my_string("Hello from rank " + std::to_string(world_rank));
    api.insert(msg);
    std::cout << "[Rank " << world_rank << "] Inserted document.\n";

    MPI_Barrier(MPI_COMM_WORLD);

    // Rank 0: read all documents
    if (world_rank == 0) {
        mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
        auto db = client["testdb"];
        auto coll = db["all_types_coll"];
        std::cout << "Reading all AllTypes messages from MongoDB...\n";
        int found = 0;
        for (auto&& doc : coll.find({})) {
            AllTypes found_msg = BsonToProto<AllTypes>(doc);
            std::cout << "Found: my_int32=" << found_msg.my_int32()
                      << ", my_string=" << found_msg.my_string() << "\n";
            found++;
        }
        if (found == 0) {
            std::cout << "No documents found! Check insertion logic.\n";
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // [Bonus] Test update: update a document for this rank
    msg.set_my_string("Updated by rank " + std::to_string(world_rank));
    api.update_one(
        bsoncxx::builder::stream::document{} << "type" << msg.GetTypeName() << bsoncxx::builder::stream::finalize, 
        msg
    );
    std::cout << "[Rank " << world_rank << "] Updated document.\n";

    MPI_Barrier(MPI_COMM_WORLD);

    // [Bonus] Test find_one
    AllTypes out_msg;
    if (api.find_one(out_msg)) {
        std::cout << "[Rank " << world_rank << "] Sample doc: my_int32=" << out_msg.my_int32()
                  << ", my_string=" << out_msg.my_string() << "\n";
    } else {
        std::cout << "[Rank " << world_rank << "] No doc found in find_one()\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // [Bonus] Rank 0 deletes one document
    if (world_rank == 0) {
        api.delete_one({});
        std::cout << "Deleted one document from collection.\n";
    }

    MPI_Finalize();
    return 0;
}