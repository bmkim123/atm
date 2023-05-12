#include <variant>
#include <memory>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "atm/bank.hpp"
#include "atm/driver.hpp"
#include "atm/controller.hpp"
#include "controller_impl.hpp"

using ::atm::BankError;
using ::atm::AccountInfo;
using ::atm::Card;
using ::atm::DriverError;
using ::atm::IDriverStateListener;
using ::atm::IBlocker;
using ::atm::ControllerServiceImpl;

class MockBankService : public ::atm::IBankService {
public:
  MOCK_METHOD((std::variant<std::string, BankError>), authenticate,
              (const Card& card, int32_t pinNumber), (override));

  MOCK_METHOD((std::variant<AccountInfo, BankError>), requestAccountInfo,
              (const Card& card, std::string token), (override));

  MOCK_METHOD((std::variant<AccountInfo, BankError>), requestDeposit,
              (const Card& card, int32_t deposit, std::string token), (override));

  MOCK_METHOD((std::variant<std::monostate, BankError>), requestWithdraw,
              (const Card& card, int32_t withdraw, std::string token), (override));

  MOCK_METHOD((std::variant<AccountInfo, BankError>), withdrawCompleted,
              (const Card& card, int32_t withdraw, std::string token), (override));
};

class MockDriverService : public ::atm::IDriverService {
public:
  MOCK_METHOD((std::variant<std::monostate, DriverError>), ejectCard,
              (), (override));

  MOCK_METHOD((std::variant<std::monostate, DriverError>), openMoneyBox,
              (), (override));

  MOCK_METHOD((std::variant<std::monostate, DriverError>), closeMoneyBox,
              (), (override));

  MOCK_METHOD((std::variant<int32_t, DriverError>), takeMoney,
              (), (override));

  MOCK_METHOD((std::variant<std::monostate, DriverError>), putMoney,
              (int32_t money), (override));

  MOCK_METHOD(bool, registerDriverStateListener,
              (std::shared_ptr<IDriverStateListener> listener), (override));
};

class MockBlocker : public IBlocker {
public:
  MOCK_METHOD(void, block, (int32_t timeMs), (override));
};

class ControllerTest : public ::testing::Test {
protected:

  void SetUp() override {
    bankService = std::make_shared<MockBankService>();
    driverService = std::make_shared<MockDriverService>();
    blocker = std::make_shared<MockBlocker>();
    controller = std::make_shared<ControllerServiceImpl>(bankService, driverService, blocker);
  }

  std::shared_ptr<MockBankService> bankService;
  std::shared_ptr<MockDriverService> driverService;
  std::shared_ptr<MockBlocker> blocker;
  std::shared_ptr<ControllerServiceImpl> controller;
};

// TEST_F(ControllerTest, authenticationSuccess) {
//   Card card;
//   ON_CALL(*bankService, authenticate(::testing::_, ::testing::_))
//     .WillByDefault(testing::Return("token"));
//   auto ret = controller->authenticate(card, 0);
//   EXPECT_EQ(std::get<std::string>(ret), "token");
// }

// class ITestSimple {
// public:
//   virtual ~ITestSimple() {}

//   virtual int32_t test(int32_t a) = 0;
// };

// class MockTestSimple : public ITestSimple {
// public:
//   MOCK_METHOD(int32_t, test, (int32_t a), (override));
// };

// TEST_F(ControllerTest, simple) {
//   MockTestSimple mock;
//   ON_CALL(mock, test(::testing::_))
//     .WillByDefault(testing::Return(1));
//   auto ret =  mock.test(3);
//   EXPECT_EQ(ret, 1);
// }