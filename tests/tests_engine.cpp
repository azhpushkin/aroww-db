#include "catch2/catch.hpp"

#include "engine/engine.hpp"

bool operator==(const OpResult& lhs, const OpResult& rhs) {
    return (
        lhs.success == rhs.success
        and lhs.error_msg == rhs.error_msg
        and lhs.value == rhs.value
    );
}
bool operator!=(const OpResult& lhs, const OpResult& rhs) {
    return !(lhs==rhs);
}

TEST_CASE( "Empty test" ) {
    REQUIRE(OpResult{true, "val", "err"} == OpResult{true, "val", "err"});
}
