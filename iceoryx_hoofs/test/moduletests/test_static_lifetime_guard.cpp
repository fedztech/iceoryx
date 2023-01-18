// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_hoofs/design_pattern/static_lifetime_guard.hpp"

#include "test.hpp"
#include <iostream>

namespace
{
using namespace ::testing;

// allows to create different types for independent tests
template <uint64_t N>
struct Foo
{
    Foo()
        : id(++instancesCreated)
    {
        ++ctorCalled;
    }

    ~Foo()
    {
        ++dtorCalled;
    }

    static void reset()
    {
        ctorCalled = 0;
        dtorCalled = 0;
    }

    static uint32_t ctorCalled;
    static uint32_t dtorCalled;
    static uint32_t instancesCreated;

    uint32_t id;
};

constexpr uint32_t FIRST_INSTANCE_ID{1};
constexpr uint32_t SECOND_INSTANCE_ID{2};

template <uint64_t N>
uint32_t Foo<N>::ctorCalled{0};

template <uint64_t N>
uint32_t Foo<N>::dtorCalled{0};

template <uint64_t N>
uint32_t Foo<N>::instancesCreated{0};

template <uint64_t N>
using TestGuard = iox::design_pattern::StaticLifetimeGuard<Foo<N>>;

// create a bundle of types and functions that are relevant for the tests,
// since we need a different static type for each test
// to ensure independence
template <uint64_t N>
struct TestTypes : public TestGuard<N>
{
    // NB: using the base class methods would admit to argue that we test another type,
    // hence we use this alias of the Guard
    using Guard = TestGuard<N>;
    using Foo = Foo<N>;

    using TestGuard<N>::setCount;

    // the first call to testInstance() creates a static instance
    // that is guarded once implicitly

    static Foo& instance()
    {
        static Foo& f = Guard::instance();
        return f;
    }

    // init the instance but also reset the Foo ctor count,
    // used at start of some tests to simplify counting
    static Foo& initInstance()
    {
        static Foo& f = Guard::instance();
        Foo::reset();
        return f;
    }
};

// each test must use a different N, __LINE__ is unique and portable,
// __COUNTER__ is not portable
// shorten the initialization via macro, needed due to __LINE__
#define INIT_TEST using T = TestTypes<__LINE__>

// each test uses its own type for maximum separation, so we d not need to reset anything
class StaticLifetimeGuard_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(StaticLifetimeGuard_test, countIsZeroIfNoInstanceExists)
{
    ::testing::Test::RecordProperty("TEST_ID", "0bf772c8-97c7-4cdb-80a1-e1b6a1a4fdc6");
    INIT_TEST;

    EXPECT_EQ(T::Guard::count(), 0);
    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, guardDoesNotImplyInstanceConstructionIfInstanceIsNotCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "0db1455e-1b1f-4498-af3c-5e2d7e92180b");
    INIT_TEST;

    {
        T::Guard g;
        EXPECT_EQ(T::Guard::count(), 1);
    }

    EXPECT_EQ(T::Guard::count(), 0);
    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, staticInitializationSucceeded)
{
    ::testing::Test::RecordProperty("TEST_ID", "d38b436b-f079-43fe-9d33-23d18cd08ffc");
    INIT_TEST;

    // testInstance() was constructed and the instance still exists
    EXPECT_EQ(T::instance().id, FIRST_INSTANCE_ID);
    EXPECT_EQ(T::Guard::count(), 1);
    EXPECT_EQ(T::Foo::ctorCalled, 1);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

// setCount is not part of the public interface but still useful to check whether it works
TEST_F(StaticLifetimeGuard_test, setCountWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1db790f9-d49e-44b2-b7e9-af50dd6a7d67");
    INIT_TEST;

    T::Guard guard;
    auto oldCount = T::setCount(73);
    EXPECT_EQ(T::Guard::count(), 73);
    EXPECT_EQ(oldCount, 1);
}

TEST_F(StaticLifetimeGuard_test, guardPreventsDestruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a8c5953-f2d7-4539-89ba-b4686bbb6319");
    INIT_TEST;
    T::initInstance();

    EXPECT_EQ(T::instance().id, FIRST_INSTANCE_ID);
    {
        T::Guard guard;
        EXPECT_EQ(T::count(), 2);
        auto& instance = T::Guard::instance();

        EXPECT_EQ(T::Foo::ctorCalled, 0);
        EXPECT_EQ(T::Foo::dtorCalled, 0);

        // still the same instance as testInstance()
        EXPECT_EQ(instance.id, 1);
        EXPECT_EQ(&instance, &T::instance());
    }

    // the implicit guard of testInstance() prevents destruction
    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
    EXPECT_EQ(T::instance().id, FIRST_INSTANCE_ID);
}

TEST_F(StaticLifetimeGuard_test, copyIncreasesLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ab6396d-7c63-4626-92ed-c7f3ea67bbf1");
    INIT_TEST;
    T::initInstance();

    EXPECT_EQ(T::instance().id, FIRST_INSTANCE_ID);

    T::Guard guard;
    {
        EXPECT_EQ(T::Guard::count(), 2);
        // NOLINTJUSTIFICATION ctor and dtor side effects are tested
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        T::Guard copy(guard);
        EXPECT_EQ(T::Guard::count(), 3);
    }
    EXPECT_EQ(T::Guard::count(), 2);

    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, moveIncreasesLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "32a2fdbf-cb02-408c-99a3-373aa66b2764");
    INIT_TEST;
    T::initInstance();

    T::Guard guard;
    {
        EXPECT_EQ(T::Guard::count(), 2);
        // NOLINTJUSTIFICATION ctor and dtor side effects are tested
        // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
        T::Guard movedGuard(std::move(guard));
        EXPECT_EQ(T::Guard::count(), 3);
    }
    EXPECT_EQ(T::Guard::count(), 2);

    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, assignmentDoesNotChangeLifetimeCount)
{
    ::testing::Test::RecordProperty("TEST_ID", "1c04ac75-d47a-44da-b8dc-6f567a53d3fc");
    INIT_TEST;
    T::initInstance();

    T::Guard guard1;
    T::Guard guard2;

    EXPECT_EQ(T::Guard::count(), 3);
    guard1 = guard2;
    EXPECT_EQ(T::Guard::count(), 3);
    guard1 = std::move(guard2);
    EXPECT_EQ(T::Guard::count(), 3);

    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 0);
}

TEST_F(StaticLifetimeGuard_test, destructionAtZeroCountWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8b5a22a9-87bc-434b-9d07-9f3c20a6944e");
    INIT_TEST;
    T::initInstance();

    {
        T::Guard guard;
        auto& instance = T::Guard::instance();

        // count is expected to be 2,
        // we ignore the guard of testInstance() by setting it to 1,
        // hence when guard is destroyed the instance will be destroyed as well
        auto oldCount = T::setCount(1);
        EXPECT_EQ(oldCount, 2);

        EXPECT_EQ(T::Foo::ctorCalled, 0);
        EXPECT_EQ(T::Foo::dtorCalled, 0);
        EXPECT_EQ(instance.id, FIRST_INSTANCE_ID);
    }

    EXPECT_EQ(T::Guard::count(), 0);
    EXPECT_EQ(T::Foo::ctorCalled, 0);
    EXPECT_EQ(T::Foo::dtorCalled, 1);
}

TEST_F(StaticLifetimeGuard_test, constructionAfterDestructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0077e73d-ddf5-47e7-a7c6-93819f376175");
    INIT_TEST;
    T::initInstance();

    {
        T::Guard guard;
        auto& instance = T::Guard::instance();

        T::setCount(1);
        EXPECT_EQ(instance.id, FIRST_INSTANCE_ID);
    }

    // first instance destroyed (should usually only happen at the the program
    // during static destruction)

    T::Foo::reset();

    EXPECT_EQ(T::Guard::count(), 0);
    {
        T::Guard guard;
        auto& instance = T::Guard::instance();

        EXPECT_EQ(T::Foo::ctorCalled, 1);
        EXPECT_EQ(T::Foo::dtorCalled, 0);
        EXPECT_EQ(instance.id, SECOND_INSTANCE_ID);
    }

    // there was only one guard for the second instance that is destroyed
    // at scope end and hence the second instance should be destroyed as well

    EXPECT_EQ(T::Guard::count(), 0);
    EXPECT_EQ(T::Foo::ctorCalled, 1);
    EXPECT_EQ(T::Foo::dtorCalled, 1);
}

} // namespace
