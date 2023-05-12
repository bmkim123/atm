#ifndef ATM__DRIVER_HPP_
#define ATM__DRIVER_HPP_
#include <memory>
#include <variant>

namespace atm {

enum class DriverError {
  MONEY_BOX_OPEN_ERROR,
  MONEY_BOX_CLOSE_ERROR,
  MONEY_BOX_COUNT_ERROR,
  MONEY_BOX_PUT_MONEY_ERROR,
};

class IDriverStateListener {
public:
  virtual ~IDriverStateListener() {}

  virtual void onCardInserted(const std::string& cardNumber) = 0;
  virtual void onCardEjected() = 0;
};

class IDriverService {
public:
  virtual ~IDriverService() {}

  virtual std::variant<std::monostate, DriverError> ejectCard() = 0;

  virtual std::variant<std::monostate, DriverError> openMoneyBox() = 0;
  virtual std::variant<std::monostate, DriverError> closeMoneyBox() = 0;
  virtual std::variant<int32_t, DriverError> takeMoney() = 0;
  virtual std::variant<std::monostate, DriverError> putMoney(int32_t money) = 0;

  virtual bool registerDriverStateListener(std::shared_ptr<IDriverStateListener> listener) = 0;
};

}
#endif  // ATM__DRIVER_HPP_