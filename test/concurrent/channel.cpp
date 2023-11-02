#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/concurrent/channel.hpp>

using namespace ccl;
using namespace ccl::concurrent;

template<typename T>
using test_channel = channel<T, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_channel<int> channel{16};

        equals(channel.get_capacity(), 16);
        equals(channel.is_empty(), true);
        equals(channel.is_full(), false);
    });

    suite.add_test("ctor (move)", [] () {
        test_channel<int> channel1{1};
        test_channel<int> channel2{std::move(channel1)};
    });

    suite.add_test("ctor (bad size)", [] () {
        throws<std::invalid_argument>([] () {
            test_channel<int> channel{0};
        });
    });

    suite.add_test("is_full (single)", [] () {
        test_channel<int> channel{1};

        equals(channel.is_full(), false);
        (void)channel.send(5);
        equals(channel.is_full(), true);
        (void)channel.recv();
        equals(channel.is_full(), false);
        (void)channel.send(5);
        equals(channel.is_full(), true);
    });

    suite.add_test("is_full (multiple)", [] () {
        test_channel<int> channel{2};

        equals(channel.is_full(), false);
        (void)channel.send(5);
        equals(channel.is_full(), false);
        (void)channel.send(5);
        equals(channel.is_full(), true);
        (void)channel.recv();
        equals(channel.is_full(), false);
        (void)channel.recv();
        equals(channel.is_full(), false);
    });

    suite.add_test("is_empty", [] () {
        test_channel<int> channel{2};

        equals(channel.is_empty(), true);
        (void)channel.send(5);
        equals(channel.is_empty(), false);
        (void)channel.recv();
        equals(channel.is_empty(), true);
        (void)channel.send(5);
        equals(channel.is_empty(), false);
    });

    suite.add_test("send/recv", [] () {
        test_channel<int> channel{16};

        equals(channel.send(5), true);
        equals(*channel.recv(), 5);
    });

    suite.add_test("send (full)", [] () {
        test_channel<int> channel{1};

        (void)channel.send(5);
        equals(channel.send(5), false);
    });

    suite.add_test("recv (empty)", [] () {
        test_channel<int> channel{1};

        equals(channel.recv(), std::nullopt);
    });

    suite.add_test("send/recv cycle", [] () {
        test_channel<int> channel{16};

        equals(channel.send(5), true);
        equals(*channel.recv(), 5);

        equals(channel.send(5), true);
        equals(*channel.recv(), 5);

        equals(channel.send(5), true);
        equals(*channel.recv(), 5);

        equals(channel.send(5), true);
        equals(*channel.recv(), 5);
    });

    return suite.main(argc, argv);
}
